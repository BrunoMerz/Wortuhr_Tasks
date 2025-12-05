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
#include "TaskStructs.h"
#include <LittleFS.h>
#include "helper.h"
#include "LedDriver_FastLED.h"

//#define myDEBUG
#include "MyDebug.h"

extern s_taskParams taskParams;
extern EventGroupHandle_t xEvent;

static AnimationFS *anifs = AnimationFS::getInstance();
static LedDriver *ledDriver = LedDriver::getInstance();


// Set Corner Pixel
void setCornerPixel(uint32_t colornum_in)
{
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  red = num_to_color(colornum_in).red * ledDriver->getBrightness() * 0.0039;
  green = num_to_color(colornum_in).green * ledDriver->getBrightness() * 0.0039;
  blue = num_to_color(colornum_in).blue * ledDriver->getBrightness() * 0.0039;
  for (uint8_t cp = 0; cp <= 3; cp++)
  {
    ledDriver->setPixelRGB(110 + cp, red, green, blue);
  }
  ledDriver->show();
}


// Zeige den aktuellen Frame während MakeAnimation an.
void showMakeAnimation(void)
{
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  uint8_t brightness = ledDriver->getBrightness();

  DEBUG_PRINTLN("showMakeAnimation: " + String(anifs->myanimation.name) + " Frame: " + String(anifs->akt_aniframe) );

  for ( uint8_t z = 0; z <= 9; z++)
  {
    for ( uint8_t x = 0; x <= 10; x++)
    {
      red = anifs->myanimation.frame[anifs->akt_aniframe].color[x][z].red * brightness * 0.0039;
      green = anifs->myanimation.frame[anifs->akt_aniframe].color[x][z].green * brightness * 0.0039;
      blue = anifs->myanimation.frame[anifs->akt_aniframe].color[x][z].blue * brightness * 0.0039;
      ledDriver->setPixelRGB(x, z, red, green, blue);
    }
  }
  ledDriver->show();
}

//###########################################################################
// Animationsmenü
//###########################################################################
void makeAnimationmenue(AsyncWebServerRequest *request)
{
  String checked = F("checked");
  uint8_t anzani = 0;
  
  String message = F("<!doctype html>"
  "<html>"
  "<head>"
  "<title>");
  message += String(HOSTNAME);
  message += F(" " LANG_ANIMATIONS "</title>\n"
  "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
  "<meta charset=\"UTF-8\">\n"
  "<link rel=\"icon\" type=\"image/png\" sizes=\"192x192\"  href=\"/web/android-icon-192x192.png\">"
  "<link rel=\"icon\" type=\"image/png\" sizes=\"32x32\"  href=\"/web/favicon-32x32.png\">\n"
  "<script src=\"/web/jquery-3.1.1.min.js\"></script>\n"
  "<link rel=\"stylesheet\" href=\"/web/animenue.css\" >\n"
  "</head><body>\n"
  "<h2>\n"
  LANG_ANIMATIONMENU
  "</h2>"
  "<form name=\"animenue\" action=\"/\" method=\"POST\">\n"
  "<div class=\"background-color\"></div>\n"
  "<div class=\"section over-hide z-bigger\">\n"
    "<div class=\"row justify-content-center pb-5\">\n"
      "<div class=\"checkboxcontainer\">\n");

  for ( int aninr=0; aninr <= MAXANIMATION; aninr++ )
  {
    if ( anifs->myanimationslist[aninr].length() == 0 || anifs->myanimationslist[aninr] == LANG_NEW ) continue;
    anzani++;
    message += F("<input class=\"checkbox-tools\" type=\"radio\" name=\"myselect\" id=\"");
    message += String(anifs->myanimationslist[aninr]);
    message += F("\" ");
    message += checked;
    message += F(" value=\"");
    message += String(anifs->myanimationslist[aninr]);
    message += F("\">\n");
    message += F("<label class=\"for-checkbox-tools\" for=\""); 
    message += String(anifs->myanimationslist[aninr]);
    message += F("\">\n");
    message += F("<i class='uil'>");
    message += String(anzani); 
    message += F("</i>");
    message += String(anifs->myanimationslist[aninr]); 
    message += F("</label>\n");
    checked = "";
  }

  if ( anzani < MAXANIMATION ) 
  {
    message += F("<input class=\"checkbox-tools\" type=\"radio\" name=\"myselect\" id=\"" LANG_NEW "\" ");
    message += checked;
    message += F(" value=\"" LANG_NEW "\">\n");
    message += F("<label class=\"for-checkbox-tools\" for=\"" LANG_NEW "\">" LANG_NEW "</label>\n");
  }
  message += F("</div>"
    "</div>\n"
    "<br>\n"
    "<hr>\n"
    "<div class=\"buttonscontainer\">\n"
    "<button class=\"buttons\" title=\"" LANG_BACK "\" type=\"submit\" formaction=\"/back\">&#128281; " LANG_BACK "</button>\n"
    "<button class=\"buttons\" title=\"" LANG_CHANGEBUTTON "\" type=\"submit\" formaction=\"/makeanimation\">&#127912; " LANG_CHANGEBUTTON "</button>\n"
    "</div>"
    "</div>"
    "</form>\n"
    
    "<script>\n"
    "var urlBase = \"/\";\n"
    "$(\".checkbox-tools\").click(function() {\n"
    "newvalue = $(\"input[class='checkbox-tools']:checked\").val();"
    "$.post(urlBase + \"myaniselect\" + \"?value=\" + newvalue );" 
    "});\n"
    "document.addEventListener('DOMContentLoaded', function() {\n"
     "$.post(urlBase + \"myaniselect\" + \"?value=BACK\" );"
    "});\n"
    "</script>\n"
    "</body></html>\n");

  request->send(200, TEXT_HTML, message);
}

//###########################################################################
// Animationsmenü Ende
//###########################################################################




// Starte Animationsmenü
void startAnimationsmenue(AsyncWebServerRequest *request)
{
  anifs->getAnimationList();
  anifs->animation = anifs->myanimationslist[0];
  anifs->akt_aniframe = 0;
  anifs->akt_aniloop = 0;
  anifs->frame_fak = 1;

  DEBUG_PRINTLN("startAnimationsmenue: " + anifs->animation + " Frame: " + String(anifs->akt_aniframe) );

  makeAnimationmenue(request);
}

// Start Animations Editor
void startmakeAnimation(AsyncWebServerRequest *request)
{

  DEBUG_PRINTF("startmakeAnimation: %s\n",anifs->animation.c_str());
  taskParams.endless_loop = false;
  // in order to terminate animation
  anifs->akt_aniloop = anifs->myanimation.loops;
  // wait for last animation loop
  while(anifs->akt_aniframe) vTaskDelay(pdMS_TO_TICKS(1));

  // fülle die Zwischenablage mit dem ersten Frame
  for ( uint8_t z = 0; z <= 9; z++)
  {
    for ( uint8_t x = 0; x <= 10; x++)
    {
      anifs->copyframe.color[x][z].red = anifs->myanimation.frame[0].color[x][z].red;
      anifs->copyframe.color[x][z].green = anifs->myanimation.frame[0].color[x][z].green;
      anifs->copyframe.color[x][z].blue = anifs->myanimation.frame[0].color[x][z].blue;
      anifs->copyframe.delay = anifs->myanimation.frame[0].delay;
    }
  }
  request->send(200, TEXT_HTML, "<!doctype html><html><head><script>window.onload=function(){window.location.replace('/web/animation.html?animation=" + String(anifs->animation) + "');}</script></head></html>");
  showMakeAnimation();
}

// Verarbeitung Animationsauswahl
void handleaniselect(AsyncWebServerRequest *request)
{
  DEBUG_PRINTLN("handleaniselect called");
  if ( request->arg("value") != "BACK" ) // wenn wir vom Editor zurückkommen
  {
    DEBUG_PRINTLN("handleaniselect: starting first animation from list");
    anifs->animation = request->arg("value");
    anifs->animation.toUpperCase();
    taskParams.animation = anifs->animation;
 
    DEBUG_PRINTLN("Animation gewählt: " + anifs->animation);
  } else
  {
    DEBUG_PRINTLN("BACK");
    taskParams.endless_loop = true;
  }
  // disable TASK_SCHEDULER an TASK_TIME
  taskParams.taskInfo[TASK_TIME].handleEvent = false;
  taskParams.taskInfo[TASK_SCHEDULER].handleEvent = false;
  DEBUG_PRINTF("Calling loadAnimation: %s\n",anifs->animation.c_str());
  bool load = anifs->loadAnimation(anifs->animation);
  //showMakeAnimation();
  request->send(200, TEXT_PLAIN, F("OK"));
  if(load)
    xEventGroupSetBits(xEvent, MODE_SHOWANIMATION);
}

// Verarbeite Animationsoberfläche
void handlemakeAnimation(AsyncWebServerRequest *request)
{
  String webargname;
  String webargvalue;
  String typ;
  String wert;
  uint8_t palidx;
  uint8_t x, y;

  if (request->arg(F("_action")) == "save")
  {
    if (anifs->saveAnimation(anifs->animation))
    {
      request->send(200, TEXT_PLAIN, anifs->animation);
    }
    else
    {
      request->send(200, TEXT_PLAIN, F("E"));
    }
    //    request->send(200, TEXT_HTML, "<!doctype html><html><head><script>window.onload=function(){window.location.replace('/animationmenue');}</script></head></html>");
  }
  else
  {
    for (int i = 0; i < request->args(); i++)
    {
      webargname += request->argName(i);
      webargvalue += request->arg(i);

      DEBUG_PRINTLN(" WebArg: " + webargname + " = " + webargvalue);

      if (webargname == "aniname")
      {
        webargvalue.trim();
        webargvalue.replace(" ", "_");
        webargvalue.toUpperCase();
        webargvalue.toCharArray(anifs->myanimation.name, webargvalue.length() + 1);
        anifs->animation = webargvalue;
        DEBUG_PRINTF("aniname=%s\n",anifs->animation);
      }
      if (webargname == "frame")
        anifs->akt_aniframe = webargvalue.toInt();
      if (webargname == "loops")
        anifs->myanimation.loops = webargvalue.toInt();
      if (webargname == "laufmode")
        anifs->myanimation.laufmode = webargvalue.toInt();
      if (webargname == "aktivfarbe")
      {
        wert = "#" + webargvalue;
        setCornerPixel(string_to_num(wert));
      }
      if (webargname == "palette")
      {
        typ = webargvalue.substring(0, webargvalue.indexOf(":"));
        wert = webargvalue.substring(webargvalue.indexOf(":") + 1);
        wert = "#" + wert;
        typ.replace("colorselect", "");
        //         DEBUG_PRINTLN(typ + ":" + wert);
        palidx = typ.toInt();
        //         DEBUG_PRINTF("PALIDX: %i \n",palidx);
        //         DEBUG_PRINTLN(String(palidx) + " = " + wert );
        anifs->anipalette[palidx] = string_to_num(wert);
        setCornerPixel(string_to_num(wert));
        //         DEBUG_PRINTF ("anipalette %i = %i\n",palidx,anipalette[palidx]);
      }
      if (webargname == "delay")
      {
        anifs->myanimation.frame[anifs->akt_aniframe].delay = webargvalue.toInt();
      }
      if (webargname == "pixel")
      {
        typ = webargvalue.substring(0, webargvalue.indexOf(":"));
        wert = webargvalue.substring(webargvalue.indexOf(":") + 1);
        wert = "#" + wert;
        typ.replace("c_", "");
        x = typ.substring(0, typ.indexOf("_")).toInt();
        y = typ.substring(typ.indexOf("_") + 1).toInt();
        DEBUG_PRINTF("pixel x/y %i / %i = %s frame=%d\n", x, y, wert.c_str(), anifs->akt_aniframe);
        anifs->myanimation.frame[anifs->akt_aniframe].color[x][y] = string_to_color(wert);
      }
      if (webargname == "copyframe")
      {
        for (uint8_t z = 0; z <= 9; z++)
        {
          for (uint8_t x = 0; x <= 10; x++)
          {
            anifs->copyframe.color[x][z].red = anifs->myanimation.frame[anifs->akt_aniframe].color[x][z].red;
            anifs->copyframe.color[x][z].green = anifs->myanimation.frame[anifs->akt_aniframe].color[x][z].green;
            anifs->copyframe.color[x][z].blue = anifs->myanimation.frame[anifs->akt_aniframe].color[x][z].blue;
            anifs->copyframe.delay = anifs->myanimation.frame[anifs->akt_aniframe].delay;
          }
        }
      }
      if (webargname == "pasteframe")
      {
        for (uint8_t z = 0; z <= 9; z++)
        {
          for (uint8_t x = 0; x <= 10; x++)
          {
            anifs->myanimation.frame[anifs->akt_aniframe].color[x][z].red = anifs->copyframe.color[x][z].red;
            anifs->myanimation.frame[anifs->akt_aniframe].color[x][z].green = anifs->copyframe.color[x][z].green;
            anifs->myanimation.frame[anifs->akt_aniframe].color[x][z].blue = anifs->copyframe.color[x][z].blue;
            anifs->myanimation.frame[anifs->akt_aniframe].delay = anifs->copyframe.delay;
          }
        }
      }
    }
    request->send(200, TEXT_PLAIN, F("OK"));
    showMakeAnimation();
  }
  vTaskDelay(pdMS_TO_TICKS(1));
}