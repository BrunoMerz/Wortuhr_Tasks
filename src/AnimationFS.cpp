#include "Arduino.h"
#include "Configuration.h"
#include "Modes.h"
#include "Settings.h"
#include "WebHandler.h"
#include "Languages.h"
#include "Html_content.h"
#include "OpenWeather.h"
#include "MyWifi.h"
#include "MyTime.h"
#include "Animation.h"
#include "LedDriver_FastLED.h"
#include "Helper.h"
#include <LittleFS.h>

//#define DEBUG_ANIMATION

//#define myDEBUG
#include "MyDebug.h"

static LedDriver *ledDriver = LedDriver::getInstance();

AnimationFS* AnimationFS::instance = 0;



AnimationFS *AnimationFS::getInstance()
{
  if (!instance)
  {
      instance = new AnimationFS();
  }
  return instance;
}

AnimationFS::AnimationFS()
{

}

// Animationsliste
void AnimationFS::getAnimationList()
{
  int aidx = 0;
  int sidx = 0;
  String Aname;
  uint8_t lidx = 0;
  for ( uint8_t z = 0;z <= MAXANIMATION;z++) myanimationslist[z] = "";
#if defined(ESP8266)
  Dir dir = LittleFS.openDir("/");
  while (dir.next()) {    
    if (!dir.isDirectory())
    {
      aidx = dir.fileName().indexOf("ani_");
      if ( aidx >= 0 ) 
      {
        sidx = dir.fileName().lastIndexOf(".json");
        if ( sidx >= 0)
        {
          Aname = dir.fileName().substring(aidx+4,sidx);
          if ( Aname != LANG_NEW ) 
          {
            myanimationslist[lidx] = Aname;
//            Serial.println ( myanimationslist[lidx] );
            lidx++;
          }
        }
      }
    }
  }
#else
  File root = LittleFS.open("/");
  File file = root.openNextFile();
 
  while(file) {
      String fn(file.name());
      aidx = fn.indexOf("ani_");
      if ( aidx >= 0 ) 
      {
        sidx = fn.lastIndexOf(".json");
        if ( sidx >= 0)
        {
          Aname = fn.substring(aidx+4,sidx);
          if ( Aname != LANG_NEW ) 
          {
            myanimationslist[lidx] = Aname;
//            Serial.println ( myanimationslist[lidx] );
            lidx++;
          }
        }
      }

      file = root.openNextFile();
  }
#endif
// sortieren
  for (int i = 0; i < lidx; i++) {
    for (int j = i + 1; j < lidx; j++) {
      if (myanimationslist[j] < myanimationslist[i]) {
        std::swap(myanimationslist[i], myanimationslist[j]);
      }
    }
  }
  saveAnimationsListe();
}

void AnimationFS::saveAnimationsListe()
{ 
//  String filename = "
  String anioutput;
  int bytesWritten = 0;
  bool fk = false;
  
  File file = LittleFS.open(ANIMATIONSLISTE, "w");
  if (!file) {
    Serial.println(F("Save Animation: Failed to create file"));
  }
   anioutput = F("{\n \"Animationsliste\" : [ "); 
   for ( uint8_t z = 0;z <= MAXANIMATION;z++) 
   {
     
     if ( myanimationslist[z].length() > 2 ) {
      if ( fk ) anioutput += F(",");
      anioutput+= "\"" + myanimationslist[z] + "\"";
     }
     fk = true;
   }
   anioutput+= F(" ]\n}\n");
   bytesWritten = file.print(anioutput);
#ifdef DEBUG_ANIMATION
   if (bytesWritten > 0) 
   {
    Serial.println(F("File " ANIMATIONSLISTE " was written"));
    Serial.println(bytesWritten);
   } else {
    Serial.println(F("File " ANIMATIONSLISTE " write failed"));
   }
#endif
  // Close the file
  file.close();
}

// Lade die Animation
bool AnimationFS::loadAnimation(String aniname )
{
#ifdef DEBUG_ANIMATION
  Serial.print(F("Loading Animation "));
  Serial.println (aniname);
#endif
  String filename = "/ani_" + aniname + ".json";
  String filezeile;
  String typ;
  String wert;
  uint32_t farbwert;
  uint8_t frame = 0;
  uint8_t zeile = 0;
  String jsonBuffer;

  File file = LittleFS.open(filename,"r");
  if(!file)
  {
   Serial.print(F("There was an error opening the file "));
   Serial.print(aniname);
   Serial.println(F(" for reading"));
   return false;
  }
  // animation leer initialisieren
  for ( uint8_t f = 0;f < MAXFRAMES;f++)
  {
     myanimation.frame[f].delay = 0;
     for ( uint8_t zeile = 0;zeile <= 9;zeile++)
     {
        for ( uint8_t x = 0;x <= 10;x++)
        {
           myanimation.frame[f].color[x][zeile] = num_to_color(0);
        }
     }          
  }
  while (file.available()) 
  {
    filezeile = file.readStringUntil('\n');
    typ = filezeile.substring(0,filezeile.indexOf(":"));
    wert = filezeile.substring(filezeile.indexOf(":")+1);
    
    typ.replace("\"","");
    typ.trim();
    
    wert.replace(",","");
    wert.replace("\"","");
    wert.trim();
    
//    Serial.printf("typ: %s = %s\n" , typ.c_str(), wert.c_str());
    
    if ( typ == "Name" ) aniname.toCharArray(myanimation.name,aniname.length()+1);
    if ( typ == "Loops" ) myanimation.loops = wert.toInt();
    if ( typ == "Laufmode" ) myanimation.laufmode = wert.toInt();
    if ( typ == "Palette") 
    {
      jsonBuffer = filezeile;
      jsonBuffer.replace("],","]");
      jsonBuffer = '{' + jsonBuffer + "}";
//      Serial.print( "jsonBuffer0: " );
//      Serial.println (jsonBuffer);
      if ( !loadjsonarry(-1,0,jsonBuffer) ) return false;  //Load Palette
      jsonBuffer="";
    }
    if ( typ.substring(0,5) == "Frame" ) frame = typ.substring(typ.indexOf("_")+1).toInt();
    if ( typ == "Delay") myanimation.frame[frame].delay = wert.toInt();
    if ( typ.substring(0,5) == "Zeile" ) 
    {
      zeile = typ.substring(typ.indexOf("_")+1).toInt();
      jsonBuffer = filezeile;
      jsonBuffer.replace("],","]");
      jsonBuffer = '{' + jsonBuffer + "}";
      
      if ( !loadjsonarry(frame,zeile,jsonBuffer) ) return false; //Load Zeilen pro Frame
      jsonBuffer="";
    }
//    Serial.printf("AnimationFrames ESP.getMaxFreeBlockSize: %i Codezeile: %u\n", ESP.getMaxFreeBlockSize(),  __LINE__);

  }
  file.close();
#ifdef DEBUG_ANIMATION
  Serial.print( "myanimation.name: " );
  Serial.println (myanimation.name);
  Serial.print( "myanimation.loops: " );
  Serial.println (myanimation.loops); 
  Serial.print( "myanimation.laufmode: " );
  Serial.println (myanimation.laufmode);    
  Serial.print( "anipalette: " );
  for ( uint8_t palidx = 0;palidx <= 9;palidx++)
  {
    Serial.print(num_to_string(anipalette[palidx]));
    Serial.print(" ");
  }
  Serial.println("");
  for ( uint8_t f = 0;f < MAXFRAMES;f++)
  {
    Serial.print ("myanimation.frame: ");
    Serial.println (f);
    Serial.print ("myanimation.frame.delay: ");
    Serial.println (myanimation.frame[f].delay);
    for ( uint8_t zeile = 0;zeile <= 9;zeile++)
    {
      Serial.print ("myanimation.frame.zeile: ");
      Serial.print (zeile);
      Serial.print (": ");
      for ( uint8_t x = 0;x <= 10;x++)
      {
        Serial.print (color_to_string(myanimation.frame[f].color[x][zeile]));
        Serial.print (" ");
      }
      Serial.println(" ");
    }          
  }
  //Serial.printf("AnimationFrames ESP.getMaxFreeBlockSize: %i Codezeile: %u\n", ESP.getMaxFreeBlockSize(),  __LINE__);
#endif
  return true;
}

bool AnimationFS::loadjsonarry( int8_t frame, uint8_t zeile, String &jsonBuffer)
{
  String farbwert;
  String zeile_json;
 
  JsonDocument myObject;
  DeserializationError error = deserializeJson(myObject, jsonBuffer);

  if (error) { Serial.println(F("Load JSON_ARRAY: Parsing the Animationsfile failed")); return false;}
  vTaskDelay(pdMS_TO_TICKS(0));
  if ( frame == -1 && zeile == 0 ) 
  {
    for ( uint8_t palidx = 0;palidx <= 9;palidx++)
    {
       farbwert = (const char*)myObject["Palette"][palidx];
       anipalette[palidx]=string_to_num(farbwert);
    }
  }
  else
  {
    zeile_json = "Zeile_" + String(zeile);
    JsonArray ja = myObject[zeile_json];
    for ( uint8_t x = 0;x < ja.size();x++)
    {
      farbwert = (const char*)myObject[zeile_json][x];
//      Serial.printf ("pixel x/y %i/%i = %s ",x,zeile,farbwert.c_str());
      myanimation.frame[frame].color[x][zeile] = string_to_color(farbwert);
    }
  }
  return true;
}


// Sichere die Animation
bool AnimationFS::saveAnimation(String aniname ) {

  aniname.toUpperCase();
#ifdef DEBUG_ANIMATION
  Serial.print(F("Saving Animation "));
  Serial.println (aniname);
#endif  
  String filename = "/ani_" + aniname + ".json";
  String anioutput;
  int bytesWritten = 0;
  bool fk = false;
  bool retval = true;

  LittleFS.remove(filename);

  // Open file for writing

  File file = LittleFS.open(filename, "a");
  if (!file) {
    Serial.println(F("Save Animation: Failed to create file"));
    return false;
  }
  strcpy ( myanimation.name, aniname.c_str());
  anioutput = F("{\n \"Name\" : \""); 
  anioutput += String(myanimation.name);
  anioutput += F("\",\n");
  anioutput += F(" \"Loops\" : ");
  anioutput += String(myanimation.loops) + ",\n";
  anioutput += F(" \"Laufmode\" : ");
  anioutput += String(myanimation.laufmode) + ",\n";
  anioutput += F(" \"Palette\" : [ ");
  for ( uint8_t palidx = 0;palidx < 10;palidx++)
  {
    if ( fk ) anioutput += F(",");
    anioutput += F("\"");
    anioutput += num_to_string(anipalette[palidx]);
    anioutput += F("\"");
    fk = true;
  }
  anioutput += F(" ],\n");

  bytesWritten = file.print(anioutput);
  if (bytesWritten > 0) {
    anioutput = "";
    for ( uint8_t f = 0;f < MAXFRAMES;f++)
    {
      if ( f > 0 && myanimation.frame[f].delay == 0 ) break;
      anioutput += F(" \"Frame_");  anioutput += String(f); anioutput += F("\" : {\n");
      anioutput += F("  \"Delay\" : "); anioutput += String(myanimation.frame[f].delay); anioutput += F(",\n");
      for ( uint8_t z = 0;z <= 9;z++)
      {
         fk = false;
         anioutput += F("  \"Zeile_"); anioutput += String(z); anioutput += F("\" : [ ");
         for ( uint8_t x = 0;x <= 10;x++)
         {
           if ( fk ) anioutput += F(",");
           anioutput += F("\""); anioutput += color_to_string(myanimation.frame[f].color[x][z]);anioutput += F("\"");
           fk = true;
         } 
         delay(0);
         if ( z < 9 ) { anioutput += F(" ],\n");} else {anioutput += F(" ]\n");}
      }
      if ( f < MAXFRAMES-1 ) 
      { 
        if ( f > 0 && myanimation.frame[f+1].delay == 0 )
        {
          anioutput += F(" }\n");
        }
        else
        {
          anioutput += F(" },\n");
        }
      } 
      else 
      {
        anioutput += F(" }\n");
      }

      bytesWritten = bytesWritten + file.print(anioutput);
      anioutput = "";
    }
    anioutput = "}\n";
    bytesWritten = bytesWritten + file.print(anioutput);
  }
  if (bytesWritten > 0) 
  {
#ifdef DEBUG_ANIMATION
    Serial.println(F("Save Animation: File was written"));
    Serial.print(bytesWritten);
    Serial.println(F(" bytes"));
#endif
    retval = true;
  } else {
    Serial.println(F("File write failed"));
    retval = false;
  }
  // Close the file
  file.close();
  return retval;
}

// ################################################################################################################
//  ANIMATIONEN
bool AnimationFS::showAnimation(uint8_t brightness)
{
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  //  unsigned long aktmillis = millis();
  // beim ersten Frame in der ersten loop die Corner LEDs löschen
  if (!akt_aniframe && !akt_aniloop)
  {
    for (uint8_t cp = 0; cp <= 3; cp++)
    {
      ledDriver->setPixelRGB(110 + cp, 0, 0, 0);
    }
    if (myanimation.laufmode < 2)
      frame_fak = 1;
  }
#ifdef DEBUG_ANIMATION
    DEBUG_PRINTF("Start Animation: akt_aniloop=%d, loops=%d\n", akt_aniloop, myanimation.loops);
#endif
  if (akt_aniloop >= myanimation.loops)
  {
    akt_aniloop = 0;
    akt_aniframe = 0;
    frame_fak = 1;
#ifdef DEBUG_ANIMATION
    DEBUG_PRINTLN("Start Animation: returning false");
#endif

    return false;
  }
  else
  {
#ifdef DEBUG_ANIMATION
    DEBUG_PRINTLN("Start Animation: " + String(myanimation.name) + " Loop: " + String(akt_aniloop) + " Frame: " + String(akt_aniframe));
#endif

    for (uint8_t z = 0; z <= 9; z++)
    {
      for (uint8_t x = 0; x <= 10; x++)
      {
        red = myanimation.frame[akt_aniframe].color[x][z].red * brightness * 0.0039;
        green = myanimation.frame[akt_aniframe].color[x][z].green * brightness * 0.0039;
        blue = myanimation.frame[akt_aniframe].color[x][z].blue * brightness * 0.0039;
        ledDriver->setPixelRGB(x, z, red, green, blue);
      }
    }
    ledDriver->show();

    vTaskDelay(pdMS_TO_TICKS(myanimation.frame[akt_aniframe].delay));

    if (myanimation.laufmode == 0)
    {
      akt_aniframe++;
      if ((myanimation.frame[akt_aniframe].delay == 0 || akt_aniframe >= MAXFRAMES))
      {
        akt_aniframe = 0;
        akt_aniloop++;
      }
    }
    if (myanimation.laufmode == 1)
    {
      akt_aniframe = akt_aniframe + frame_fak;
      if (myanimation.frame[akt_aniframe].delay == 0 || akt_aniframe >= MAXFRAMES)
      {
        frame_fak = -1;
        akt_aniframe = akt_aniframe - 2;
        akt_aniloop++;
      }
      if (akt_aniframe == 0)
      {
        frame_fak = 1;
        akt_aniloop++;
      }
    }
    if (myanimation.laufmode == 2)
    {
      frame_fak++;
      for (uint8_t i = 0; i <= 20; i++)
      {
        akt_aniframe = random(0, MAXFRAMES);
        if (myanimation.frame[akt_aniframe].delay != 0)
          break;
      }
      if (frame_fak == 20)
      {
        frame_fak = 0;
        akt_aniloop++;
      }
    }
//mz    if (mode == MODE_MAKEANIMATION) // falls der Editor gestartet wurde den aktuellen Frame + Loop + Richtung nochmal zurücksetzen
//mz    {
//mz      akt_aniframe = 0;
//mz      akt_aniloop = 0;
//mz      frame_fak = 1;
//mz    }
    //screenBufferNeedsUpdate = true;
#ifdef DEBUG_ANIMATION
    DEBUG_PRINTLN("Start Animation: returning true");
#endif
    return true;
  }
}