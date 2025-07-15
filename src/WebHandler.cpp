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
#include "LedDriver_FastLED.h"
#include "SecondBell.h"
#include "TaskStructs.h"
#include "Game.h"

#include "ESPAsyncWebServer.h"

#include <LittleFS.h>

//#define DEBUG_WEB

//#define myDEBUG
#include "MyDebug.h"

extern s_taskParams taskParams;
extern EventGroupHandle_t xEvent;

extern void handleRoot(AsyncWebServerRequest *request, Mode mode, uint8_t moonphase, uint8_t web_moonphase, time_t upTime, uint8_t sunriseMinute, uint8_t sunriseHour, uint8_t sunsetMinute, uint8_t sunsetHour);
extern void handleButtonSettings(AsyncWebServerRequest *request);
extern void handleFSExplorer(AsyncWebServerRequest *request);
extern void handleFSExplorerCSS(AsyncWebServerRequest *request);
extern void startAnimationsmenue(AsyncWebServerRequest *request);
extern void startmakeAnimation(AsyncWebServerRequest *request);
extern void handleaniselect(AsyncWebServerRequest *request);
extern void handlebacktoMODE_TIME(AsyncWebServerRequest *request);
extern void debugClock(AsyncWebServerRequest *request);
extern void handleCommitSettings(AsyncWebServerRequest *request);
extern void handleShowText(AsyncWebServerRequest *request);
extern void handleEvents(AsyncWebServerRequest *request);
extern void buttonModePressed(AsyncWebServerRequest *request);
extern void buttonTimePressed(AsyncWebServerRequest *request);
extern void startGame(AsyncWebServerRequest *request);
extern void handleGameControl(AsyncWebServerRequest *request);
extern void formatFS();
extern void handleUpload();

static Settings *settings = Settings::getInstance();
static MyWifi *mywifi = MyWifi::getInstance();
static MyTime *mt = MyTime::getInstance();
static AsyncWebServer *webServer = mywifi->getServer();
static OpenWeather *ow = OpenWeather::getInstance();
static LedDriver *ledDriver = LedDriver::getInstance();
static Game *game = Game::getInstance();


WebHandler* WebHandler::instance = 0;

void callRoot(AsyncWebServerRequest *request)
{
  request->send(200, TEXT_HTML, "<!doctype html><html><head><script>window.onload=function(){window.location.replace('/');}</script></head></html>");
}


bool handleFile(AsyncWebServerRequest *request) {
  String path=request->urlDecode(request->url());

#ifdef DEBUG_WEB
  DEBUG_PRINT (F("LittleFS handleFile: "));
  DEBUG_PRINTLN(path);
#endif
  
  if (path.endsWith("/")) path += F("index.html");
  if (path.endsWith(F("player"))) path=F("/web/spieler.html");
  if (path.endsWith(F("spieler"))) path=F("/web/spieler.html");
  if (path.endsWith(F("favicon.ico"))) path=F("/web/favicon.ico");
  if ( LittleFS.exists(path) )
  {
      String mime_type;
#if defined(ESP8266)
      mime_type=mime::getContentType(path);
#else
    if(path.endsWith(F(".htm")) || path.endsWith(F(".html")))
        mime_type = F("text/html");
    else if(path.endsWith(F(".jpg")) || path.endsWith(F(".jpeg")))
        mime_type = F("image/jpeg");
    else if(path.endsWith(".png"))
      mime_type = F("image/png");
    else if(path.endsWith(".ico"))
      mime_type = F("image/x-icon");
    else if(path.endsWith(".bmp"))
      mime_type = F("image/bmp");
    else if(path.endsWith(".gif"))
      mime_type = F("image/gif");
    else if(path.endsWith(".css"))
      mime_type = F("text/css");
    else if(path.endsWith(".pdf"))
      mime_type = F("application/pdf");
    else if(path.endsWith(".txt"))
      mime_type = F("text/plain");
    else if(path.endsWith(".json"))
      mime_type = F("application/json");
    else if(path.endsWith(".mp3"))
      mime_type = F("audio/mpeg");
    else if(path.endsWith(".js"))
      mime_type = F("text/javascript");
    else
      mime_type = F("application/octet-stream");
#endif
#ifdef DEBUG_WEB
    glb->highWaterMark = uxTaskGetStackHighWaterMark(NULL);
    glb->codeline = __LINE__;
    glb->codetab = __NAME__;
    DEBUG_PRINTF("handleFile: mime_type=%s uxHighWaterMark=%d\n", mime_type.c_str(), glb->HighWaterMark);
#endif
    request->send(LittleFS, path, mime_type);
    return true;
  }
  else
  {
    return false;
  }
}

void buttonOnOffPressed(AsyncWebServerRequest *request)
{
  DEBUG_PRINTF("On/off pressed: mode=%d\n", ledDriver->mode);
  ledDriver->setOnOff();
  callRoot(request);
  xEventGroupSetBits(xEvent, MODE_TIME);
}


// Zurück zum Hauptmenü + MODE_TIME
void handlebacktoMODE_TIME(AsyncWebServerRequest *request)
{
  //mz  curControl = BTN_EXIT;
  //mz  setMode(MODE_TIME);
  taskParams.animation = "";
  taskParams.endless_loop = false;
  taskParams.updateScreen = true;
  callRoot(request);
}


void handleContent(AsyncWebServerRequest *request, const uint8_t * image, size_t size, const char * mime_type) {
  uint8_t buffer[512];
  size_t buffer_size = sizeof(buffer);
  size_t sent_size = 0;

  AsyncResponseStream *response = request->beginResponseStream(mime_type, -1);

  while (sent_size < size) {
    size_t chunk_size = min(buffer_size, size - sent_size);
    memcpy_P(buffer, image + sent_size, chunk_size);
    response->write((const uint8_t*)buffer, chunk_size);
    sent_size += chunk_size;
#ifdef DEBUG_WEB
    DEBUG_PRINTF("sendContent: %i byte : %i byte of %i byte\n", chunk_size, sent_size,size );
#endif
  }
  //response->setContentLength(sent_size);
  request->send(response);
}


WebHandler *WebHandler::getInstance() {
  if (!instance)
  {
      instance = new WebHandler();
  }
  return instance;
}

WebHandler::WebHandler(void) {

}

// Alle webRequests
void WebHandler::webRequests()
{
  webServer->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleRoot(request, ledDriver->mode, ow->moonphase, ow->web_moonphase, mt->mytm.upTime, mt->minute(ow->sunrise), mt->hour(ow->sunrise), mt->minute(ow->sunset), mt->hour(ow->sunset));
  });

  
  webServer->on("/handleButtonSettings", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleButtonSettings(request);
  });

  webServer->on("/handleButtonOnOff", HTTP_GET, [](AsyncWebServerRequest *request) {
    buttonOnOffPressed(request);
  });

  webServer->on("/fs", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleFSExplorer(request);
  });


  webServer->on("/fsstyle.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleFSExplorerCSS(request);
  });

  //Starte Animationsmenü
  webServer->on("/animationmenue", HTTP_GET, [](AsyncWebServerRequest *request) {
    startAnimationsmenue(request);
  });

  //Starte Animationsoberfläche
  webServer->on("/makeanimation", HTTP_POST, [](AsyncWebServerRequest *request) {
    startmakeAnimation(request);
  });

  // Speichern
  webServer->on("/commitSettings", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleCommitSettings(request);
  });

  //Starte Animationsoberfläche
  webServer->on("/myaniselect", HTTP_POST, [](AsyncWebServerRequest *request) {
    handleaniselect(request);
  });  

  // zurück zum Hauptmenü +  MODE_TIME
  webServer->on("/back", HTTP_POST, [](AsyncWebServerRequest *request) {
    handlebacktoMODE_TIME(request);
  }); 

  // sunrise Image
  webServer->on("/sunrise", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, IMAGE_PNG, p_sunrise, sizeof(p_sunrise));
  });
  
  // sunset Image
  webServer->on("/sunset", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, IMAGE_PNG, p_sunset, sizeof(p_sunset));
  });
  
  // sanduhr Image
  webServer->on("/sanduhr", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, IMAGE_PNG, p_hourglass, sizeof(p_hourglass));
  });
 
  webServer->on("/handleButtonTime", [](AsyncWebServerRequest *request) {
    request->send(200, TEXT_PLAIN, F("OK"));
  });

  // sunrise/sunset Image
  webServer->on("/debugClock", HTTP_GET, [](AsyncWebServerRequest *request) {
    debugClock(request);
  });

  // Show Text
  webServer->on("/showText", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleShowText(request);
  });
  
  // Reboot
  webServer->on("/reboot", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, TEXT_PLAIN, F("OK. I'll be back!"));
    delay(5000);
    ESP.restart();
  });

  // WiFi Reset
  webServer->on("/wifireset", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, TEXT_PLAIN, F("OK. I'll be back as AP!"));
    mywifi->doReset();
  });


  webServer->on("/handleevents", HTTP_POST, [](AsyncWebServerRequest *request) { // Eventseite
    handleEvents(request);
  });


  webServer->on("/handleButtonMode", [](AsyncWebServerRequest *request) {
    buttonModePressed(request);
    request->send(200, TEXT_PLAIN, F("OK")); 
  });

  webServer->on("/handleButtonTime", [](AsyncWebServerRequest *request) {
      if ( ledDriver->mode != MODE_TIME) 
        buttonTimePressed(request);
#ifdef SHOW_MODE_ANSAGE
      else 
      {
        //single_mode = true;
        //setMode(MODE_ANSAGE);
      }
#endif
    request->send(200, TEXT_PLAIN, F("OK")); 
  });

  webServer->on("/format", HTTP_POST, [](AsyncWebServerRequest *request) {
    formatFS();
    request->send(200, "text/plain", "Format erfolgreich");
  });

  webServer->on("/StartGame", [](AsyncWebServerRequest *request) {
    game->startGame(request);
  });

  webServer->on("/game", HTTP_POST, [](AsyncWebServerRequest *request) {
    game->handleGameControl(request);
  });


  webServer->on("/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->redirect("/fs");
    //request->send(200, "text/plain", "Upload erfolgreich");
  }, 
  [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    if(!index) {
      DEBUG_PRINTF("Upload beginnt: <%s>\n", (request->getParam("f")->value() + "/" + filename).c_str());
      // Datei im Schreibmodus öffnen
      request->_tempFile = LittleFS.open(request->getParam("f")->value() + "/" + filename, "w");
      if (!request->_tempFile) 
        Serial.println("Fehler: Datei konnte nicht geöffnet werden!");
    }
    if(request->_tempFile){
      request->_tempFile.write(data, len);
    }
    if(final){
      DEBUG_PRINTF("Upload abgeschlossen: %s, Größe: %u\n", filename.c_str(), index+len);
      if(request->_tempFile){
        request->_tempFile.close();
      }
    }
  });


  webServer->onNotFound([](AsyncWebServerRequest *request) {
    if (!handleFile(request))
      request->send(404, TEXT_PLAIN, F("FileNotFound"));
  });

  

  webServer->begin();
  
}
