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

#define DEBUG_ANIMATION

//#define myDEBUG
#include "MyDebug.h"

extern s_taskParams taskParams;
extern EventGroupHandle_t xEvent;

static AnimationFS *anifs = AnimationFS::getInstance();

//###########################################################################
// Animationsmenü
//###########################################################################
void makeAnimationmenue(AsyncWebServerRequest *request)
{
  AsyncResponseStream *response = request->beginResponseStream(TEXT_HTML);
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
// ################################################# sende html Teil 1
    response->print(message);
    message = "";
// ###################################################################
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
//########################################################################### sende Liste html
    if ( aninr %5 == 0 )
    {
      response->print(message);
      message = "";
      delay(0);
    }
//########################################################################### 
  }
  if ( anzani < MAXANIMATION ) 
  {
    message += F("<input class=\"checkbox-tools\" type=\"radio\" name=\"myselect\" id=\"" LANG_NEW "\" ");
    message += checked;
    message += F(" value=\""LANG_NEW"\">\n");
    message += F("<label class=\"for-checkbox-tools\" for=\"" LANG_NEW "\">"LANG_NEW"</label>\n");
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
//##################### sende letzen html Teil
  response->print(message);
  request->send(response);
//    return message;
}

//###########################################################################
// Animationsmenü Ende
//###########################################################################




// Starte Animationsmenü
void startAnimationsmenue(AsyncWebServerRequest *request)
{
  anifs->getAnimationList();
  taskParams.animation = anifs->myanimationslist[0];
  anifs->akt_aniframe = 0;
  anifs->akt_aniloop = 0;
  anifs->frame_fak = 1;


#ifdef  DEBUG_ANIMATION
  DEBUG_PRINTLN("Start Animationsmenue: " + taskParams.animation + " Frame: " + String(anifs->akt_aniframe) );
#endif
//  mode = MODE_SHOWANIMATION;
//  screenBufferNeedsUpdate = true;
  //  webServer.send(200, TEXT_HTML, makeAnimationmenue());
  makeAnimationmenue(request);
}

//Starte Animationseditor
void startmakeAnimation(AsyncWebServerRequest *request)
{
  anifs->akt_aniframe = 0;
  anifs->akt_aniloop = 0;
  anifs->frame_fak = 1;
//mz  mode = MODE_MAKEANIMATION;
//mz  screenBufferNeedsUpdate = true;
  // fülle die Zwischenablage mit dem ersten Frame
  for ( uint8_t z = 0; z <= 9; z++)
  {
    for ( uint8_t x = 0; x <= 10; x++)
    {
      copyframe.color[x][z].red = anifs->myanimation.frame[0].color[x][z].red;
      copyframe.color[x][z].green = anifs->myanimation.frame[0].color[x][z].green;
      copyframe.color[x][z].blue = anifs->myanimation.frame[0].color[x][z].blue;
      copyframe.delay = anifs->myanimation.frame[0].delay;
    }
  }
  request->send(200, TEXT_HTML, "<!doctype html><html><head><script>window.onload=function(){window.location.replace('/web/animation.html?animation=" + String(taskParams.animation) + "');}</script></head></html>");
}

//Verarbeitung Animationsauswahl
void handleaniselect(AsyncWebServerRequest *request)
{
  if ( request->arg("value") != "BACK" ) // wenn wir vom Editor zurückkommen
  {
    taskParams.animation = request->arg("value");
    taskParams.animation.toUpperCase();
#ifdef  DEBUG_ANIMATION
  DEBUG_PRINTLN("Animation gewählt: " + taskParams.animation );
#endif
  }
#ifdef  DEBUG_ANIMATION
  else
  {
    DEBUG_PRINTLN("BACK");
  }
#endif
  bool load = anifs->loadAnimation(taskParams.animation);
  request->send(200, TEXT_PLAIN, F("OK"));
  if(load)
    xEventGroupSetBits(xEvent, MODE_SHOWANIMATION);
}