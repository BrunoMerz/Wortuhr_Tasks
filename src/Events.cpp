#include "Events.h"
#include "Animation.h"
#include "TaskStructs.h"
#include "MyTime.h"
#include "Settings.h"

//#define DEBUG_EVENTS

//#define myDEBUG
#include "MyDebug.h"

extern EventGroupHandle_t xEvent;
extern s_taskParams taskParams;

Events* Events::instance = 0;

Events *Events::getInstance() {
  if (!instance)
  {
      instance = new Events();
  }
  return instance;
}

Events::Events() {
  mustLoad=true;
}


// Lade Events
bool Events::loadEvents()
{
#ifdef DEBUG_EVENTS
   Serial.println(F("Loading Events "));
#endif
   String filename = EVENTFILE;
   String jsonBuffer;
   uint8_t eventnr = 0;
   
  // Open file for reading
  File file = LittleFS.open(filename,"r");
  if(!file)
  {
#ifdef DEBUG_EVENTS
   Serial.println(F("There was an error opening the file " EVENTFILE " for reading"));
#endif
   mustLoad = false;
   return false;
  }
  // alle events deaktivieren
  for ( uint8_t z = 0;z <= MAXEVENTS;z++)
  {
    events[z].aktiv=false;
    events[z].year=0;
    events[z].month=0;
    events[z].day=0;
    events[z].text = "";
    events[z].color=0xffffff;
    events[z].audio_file=0;
    events[z].preani="";
    events[z].postani="";
    events[z].intervall=0;
    events[z].start=ST_DATE;
  }
  while (file.available()) 
  {
    JsonDocument myObject;
    jsonBuffer = file.readStringUntil('\r');
    DeserializationError error = deserializeJson(myObject, jsonBuffer);

    if (error) {
#ifdef DEBUG_EVENTS
      Serial.println(F("Parsing the Eventfile " EVENTFILE " failed"));
#endif
      return false;
    }
    JsonArray ja = myObject["events"];
    int evl = ja.size();
#ifdef DEBUG_EVENTS
    char buffer[200];
    serializeJsonPretty(myObject, buffer);
    Serial.println(buffer);
    Serial.print("myObject[\"events\"].as<JsonArray>().size() = ");
    Serial.print("Anzahl Events: ");
    Serial.println(evl);
#endif
    for ( uint8_t z = 1;z <= MAXEVENTS ;z++)
    {
      if ( z <= evl ) 
      {
        events[z].aktiv=true;
        events[z].year=(int)myObject["events"][z-1]["jahr"];
        events[z].month=(int)myObject["events"][z-1]["monat"];
        events[z].day=(int)myObject["events"][z-1]["tag"];
        events[z].text = (const char*)myObject["events"][z-1]["text"];
        events[z].text.trim();
        events[z].color=(uint32_t)myObject["events"][z-1]["farbe"];
        events[z].audio_file=(int)myObject["events"][z-1]["audionr"];
        events[z].preani=(const char*)myObject["events"][z-1]["preani"];
        events[z].preani.trim();
        events[z].postani=(const char*)myObject["events"][z-1]["postani"];
        events[z].postani.trim();
        events[z].intervall=(int)myObject["events"][z-1]["intervall"];
        int tmp = (int)myObject["events"][z-1]["start"];
        events[z].start=(STType)tmp;
      }
    }
  }
  
  file.close();  
#ifdef DEBUG_EVENTS
  for ( uint8_t eventnr = 0;eventnr <= MAXEVENTS;eventnr++)
  {
    if ( events[eventnr].aktiv ) Serial.printf("Event: %i , Jahr: %i Monat: %i Tag: %i\n",eventnr, events[eventnr].year, events[eventnr].month,events[eventnr].day);
    if ( events[eventnr].aktiv ) Serial.println(events[eventnr].text);
    if ( events[eventnr].aktiv ) Serial.printf("Farbe: %i Audio: %i Intervall: %i Pre/Post Ani: ",events[eventnr].color, events[eventnr].audio_file, events[eventnr].intervall);
    if ( events[eventnr].aktiv ) Serial.print(events[eventnr].preani);
    if ( events[eventnr].aktiv ) Serial.print(" / ");
    if ( events[eventnr].aktiv ) Serial.println(events[eventnr].postani);
    if ( events[eventnr].aktiv ) Serial.println(events[eventnr].start);
  } 
#endif
  mustLoad = false;
  return true;
}

// Sichere Events
bool Events::saveEvents() 
{
#ifdef DEBUG_EVENTS
  Serial.println(F("Saving Events "));
#endif
  String filename = EVENTFILE;
  String fileout;
  int bytesWritten = 0;
  bool retval = true;
  bool fk = false;
  LittleFS.remove(filename);

  // Open file for writing
  File file = LittleFS.open(filename, "a");
  if (!file) {
#ifdef DEBUG_EVENTS
    Serial.println(F("Failed to create file " EVENTFILE));
#endif
    return false;
  }
  fileout = "{\"maxevents\": " + String(MAXEVENTS) + ",\n";
  fileout += F(" \"events\": [\n");
  bytesWritten = file.print(fileout);
  fileout = "";
  if (bytesWritten > 0) 
  {
    for ( uint8_t eventnr = 1;eventnr <= MAXEVENTS;eventnr++)
    {
      if ( events[eventnr].aktiv ) 
      {
        fileout = "";
        if ( fk ) fileout += ",\n";
        fileout += " {\n";
        fileout +=  "  \"jahr\":" + String(events[eventnr].year) + ",\n";
        fileout +=  "  \"monat\":" + String(events[eventnr].month) + ",\n";
        fileout +=  "  \"tag\":" + String(events[eventnr].day) + ",\n";
        fileout +=  "  \"text\": \"" + String(events[eventnr].text) + "\",\n";
        fileout +=  "  \"farbe\":" + String(events[eventnr].color) + ",\n";
        fileout +=  "  \"audionr\":" + String(events[eventnr].audio_file) + ",\n";
        fileout +=  "  \"preani\": \"" + String(events[eventnr].preani) + "\",\n";
        fileout +=  "  \"postani\": \"" + String(events[eventnr].postani) + "\",\n";
        fileout +=  "  \"intervall\":" + String(events[eventnr].intervall) + ",\n";
        fileout +=  "  \"start\":" + String(events[eventnr].start) + "\n";
        fileout +=  " }";
        fk = true;
#ifdef DEBUG_EVENTS
        Serial.printf("start=%d\n", events[eventnr].start);
#endif
      } 
      bytesWritten = bytesWritten + file.print(fileout); 
      fileout = "";
    }
    fileout = "\n ]\n}";
    bytesWritten = bytesWritten + file.print(fileout);
  }
  if (bytesWritten > 0) 
  {
#ifdef DEBUG_EVENTS
    Serial.println("File " EVENTFILE " was written");
    Serial.println(bytesWritten);
#endif
    retval = true;
  } else 
  {
#ifdef DEBUG_EVENTS
    Serial.println("File " EVENTFILE " write failed");
#endif
    retval = false;
  }
 
  // Close the file
  file.close();
  mustLoad = true;
  return retval;
}

/******************************************************************************
  Events
 ******************************************************************************/
void Events::runEvent(uint8_t akteventidx)
{
  Settings *settings = Settings::getInstance();
  AnimationFS *anifs = AnimationFS::getInstance();
  MyTime *mt = MyTime::getInstance();

  String animation;

  DEBUG_PRINTLN("runEvent called");

  taskParams.taskInfo[TASK_TIME].handleEvent = false;
  taskParams.taskInfo[TASK_MODES].handleEvent = false;



    // Lade Pre Animation
    if (events[akteventidx].preani != "KEINE")
    {
      animation = events[akteventidx].preani;
      if (anifs->loadAnimation(animation))
      {
        anifs->akt_aniframe = 0;
        anifs->akt_aniloop = 0;
        anifs->frame_fak = 0;
        while (anifs->showAnimation())
        {
#ifdef DEBUG_EVENTS
          Serial.println("Starte Event Pre Animation: " + String(anifs->myanimation.name) + " Loop: " + String(anifs->akt_aniloop) + " Frame: " + String(anifs->akt_aniframe));
#endif
        }
      }
    }
    //    if (events[akteventidx].year > 1799 ) feedText = "  " + events[akteventidx].text + " (" + String(year() - events[akteventidx].year) + ")   ";
    //    else feedText = "  " + events[akteventidx].text + "   ";

    //taskParams.feedColor = string_to_num(farbwert);
    //taskParams.feedText = "  " + request->arg(F("text")).substring(0, 80) + "   ";
    taskParams.feedText = "  " + events[akteventidx].text + "   ";
    String eventalter = String(mt->year() - events[akteventidx].year);
    taskParams.feedText.replace("YY", eventalter);
    taskParams.feedPosition = 0;
    taskParams.feedColor = events[akteventidx].color;
#ifdef DEBUG_EVENTS
    Serial.println("Event: \"" + taskParams.feedText + "\"");
#endif
    taskParams.taskInfo[TASK_TEXT].state = STATE_INIT;
    xEventGroupSetBits(xEvent, MODE_FEED);
    while(!(taskParams.taskInfo[TASK_TEXT].state==STATE_PROCESSED))
      vTaskDelay(pdMS_TO_TICKS(100));

      // Lade Post Animation
    if (events[akteventidx].postani != "KEINE")
    {
      animation = events[akteventidx].postani;
      if (anifs->loadAnimation(animation))
      {
        anifs->akt_aniframe = 0;
        anifs->akt_aniloop = 0;
        anifs->frame_fak = 0;
        while (anifs->showAnimation())
        {
#ifdef DEBUG_EVENTS
          Serial.println("Starte Event Pre Animation: " + String(anifs->myanimation.name) + " Loop: " + String(anifs->akt_aniloop) + " Frame: " + String(anifs->akt_aniframe));
#endif
        }
      }
    }
    taskParams.taskInfo[TASK_TIME].handleEvent = true;
    taskParams.taskInfo[TASK_MODES].handleEvent = true;
}
