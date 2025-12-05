
#include "Arduino.h"
#include "Configuration.h"
#include "Languages.h"
#include "Settings.h"
#include "MyTime.h"
#include "Html_content.h"
#include "Game.h"
#include "OpenWeather.h"
#include "MyFileAccess.h"
#include "LedDriver_FastLED.h"
#include "Events.h"
#include "MyWifi.h"
#include "taskStructs.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#if defined(SYSLOGSERVER_SERVER)
#include "Syslog.h"
extern Syslog syslog;
#endif

//#include myDEBUG
#include "MyDebug.h"

#include "ESPAsyncWebServer.h"
#include <LittleFS.h>

extern float getHeapFragmentation();
extern s_taskParams taskParams;
extern uint32_t autoModeChangeTimer;
extern uint8_t akt_transition;

static MyTime *mt = MyTime::getInstance();
static Settings *settings = Settings::getInstance();
static OpenWeather *outdoorWeather = OpenWeather::getInstance();
static MyFileAccess *fa = MyFileAccess::getInstance();
static OpenWeather *ow = OpenWeather::getInstance();
static LedDriver *ledDriver = LedDriver::getInstance();
static Events *evt = Events::getInstance();
static Game *game = Game::getInstance();
static MyWifi *myWifi = MyWifi::getInstance();

#if defined(SENSOR_BME280)
#include "MyBME.h"
static MyBME *myBME = MyBME::getInstance();
#endif

//debugClock
void debugClock(AsyncWebServerRequest *request)
{
  DEBUG_PRINTLN("Info Seite refresh!");
  TaskHandle_t currentTaskHandle = xTaskGetCurrentTaskHandle();
  
  // Task-Name abrufen
  const char* taskName = pcTaskGetName(currentTaskHandle);
  UBaseType_t stackHighWaterMark = uxTaskGetStackHighWaterMark(currentTaskHandle);

#if defined(SYSLOGSERVER_SERVER)
  syslog.log(LOG_INFO,"debugClock: start");
#endif
  AsyncResponseStream *response = request->beginResponseStream(TEXT_HTML);

  String str_freeheap = String(heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
  String str_minheap = String(heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL));
  String str_maxfreeblocks = String(heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
  String str_heapfragmentation = String(getHeapFragmentation());

  //String message;
  response->print(F("<!doctype html>"
              "<html lang=\""));
  response->print(LANG_HTMLLANG);
  response->print(F("\">"
              "<head>"));
  response->print(F("<title>"));
  response->print(String(settings->mySettings.systemname));
  response->print(F("Info</title>"));
  response->print(F("<meta charset=\"UTF-8\">"
               "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
               "<meta http-equiv=\"refresh\" content=\"30\">"
               "<link rel=\"icon\" type=\"image/png\" sizes=\"192x192\" href=\"/web/android-icon-192x192.png\">"
               "<link rel=\"icon\" type=\"image/png\" sizes=\"32x32\" href=\"/web/favicon-32x32.png\">"
               "<link rel=\"icon\" type=\"image/png\" sizes=\"96x96\" href=\"/web/favicon-96x96.png\">"
               "<link rel=\"icon\" type=\"image/png\" sizes=\"16x16\" href=\"/web/favicon-16x16.png\">"
               "<style>"
               "body{background-color:#FFFFFF;color:#333333;font-family:Sans-serif;font-size:16px;}"
               "</style>"
               "</head>\n"
               "<body>"));
  
  response->print(F("<h2>"));
  response->print(String(settings->mySettings.systemname));
  response->print(F(" Info</h2>"));
  response->print(F("<hr>\n"));
  
  response->print(F("<b>&bull;  Firmware: "));
  response->print(FIRMWARE_VERSION);
  response->print(" ");
  response->print(BUILD_DATE);
  response->print(F("</b>\n"));
#if defined(FRONTCOVER_DE_DE) || defined(FRONTCOVER_DE_SW) || defined(FRONTCOVER_DE_BA) || defined(FRONTCOVER_DE_SA) || defined(FRONTCOVER_D3) || defined(FRONTCOVER_DE_MKF_DE) || defined(FRONTCOVER_DE_MKF_SW) || defined(FRONTCOVER_DE_MKF_BA) || defined(FRONTCOVER_DE_MKF_SA) || defined(FRONTCOVER_CH) || defined(FRONTCOVER_CH_AG) || defined(FRONTCOVER_CH_AL)
  response->print(F("<a href=\"https://merz-aktuell.de/Wordclock/Doku/WortuhrBeschreibung10.x.pdf\" target=\"_blank\" style=\"font-size:30px;\">&#128214;</a>\n"));
#else
  response->print(F("<a href=\"https://merz-aktuell.de/Wordclock/Doku/WortuhrBeschreibung10.x.pdf\" target=\"_blank\" style=\"font-size:30px;\">&#128214;</a>\n"));
#endif
  response->print(F("<ul>\n"
               // ######################### WIFI/NETZWERK ##################
               "<li><b>"
               LANG_NETWORKWIFI
               "</b>\n"
               "<ul>\n"));
  response->print(F("<li>WLan-SID: "));
  response->print(WiFi.SSID());
  response->print(F("</li>\n"));
  response->print(F("<li>WLan-BSSID:"));
  response->print(WiFi.BSSIDstr());
  response->print(F("</li>\n"));
  response->print(F("<li>MAC-ADDR:"));
  response->print(WiFi.macAddress());
  response->print(F("</li>\n"));
  response->print(F("<li>" LANG_WIFISIGNAL ": "));
  response->print(WiFi.RSSI());
  response->print(F(" dBm ("));
  if ( WiFi.RSSI() >= -50 ) response->print(F(LANG_WIFIQ1));
  if ( WiFi.RSSI() >= -65 && WiFi.RSSI() < -50 ) response->print(F(LANG_WIFIQ2));
  if ( WiFi.RSSI() >= -70 && WiFi.RSSI() < -65 ) response->print(F(LANG_WIFIQ3));
  if ( WiFi.RSSI() >= -75 && WiFi.RSSI() < -70 ) response->print(F(LANG_WIFIQ4));
  if ( WiFi.RSSI() >= -80 && WiFi.RSSI() < -75 ) response->print(F(LANG_WIFIQ5));
  if ( WiFi.RSSI() >= -90 && WiFi.RSSI() < -80 ) response->print(F(LANG_WIFIQ6));
  if ( WiFi.RSSI() < -90 ) response->print(F(LANG_WIFIQ7));
  response->print(F(")</li>\n"));
//  response->print(F("<li>" LANG_WIFIRECON ": "));
//  response->print(String(glb->WLAN_reconnect) + "</li>\n";
  response->print(F("<li>" LANG_IPADRESS ": "));
  response->print(myWifi->IP().toString());
  response->print(F("</li>\n<li>Client IP-Addr: "));
  response->print(request->client()->remoteIP().toString());
  response->print(F("</li>\n"));
  response->print(F("</ul>\n</li>\n"));

  // ######################### ZEIT ##################
  response->print(F("<li><b>" LANG_TIME "</b>\n"
               "<ul>\n"));
  response->print(F("<li>" LANG_TIMEHOST ": "));
  response->print(String(settings->mySettings.ntphost));
  response->print(F("</li>\n"));
  response->print(F("<li>" LANG_TIMEZONE ": "));
  response->print(String(settings->mySettings.timezone));
  response->print(F("</li>\n"));
 
  time_t tempEspTime = mt->localTm();
  response->print(F("<li>" LANG_TIME ": "));
  response->print(String(mt->hour(tempEspTime)) + ":");
  if (mt->minute(tempEspTime) < 10) response->print("0");
  response->print(String(mt->minute(tempEspTime)));
  response->print(":");
  if (mt->second(tempEspTime) < 10) response->print("0");
  response->print(String(mt->second(tempEspTime)));
  
  if (mt->mytm.tm_isdst) response->print(" (Sommerzeit)");
  response->print(F("<li>" LANG_DATE ":"));
  response->print(String(mt->dayStr(mt->weekday(tempEspTime))));
  response->print(F(", "));
  response->print(String(mt->day(tempEspTime)));
  response->print(". ");
  response->print(String(mt->monthStr(mt->month(tempEspTime))));
  response->print(" ");
  response->print(String(mt->year(tempEspTime)));
  response->print(F("</li>\n"));
  response->print(F("<li>Uptime: "));
  response->print(mt->convertSeconds(mt->mytm.upTime, true));
  response->print(F("</li>\n"));
  
  time_t lokalstartTime = mt->mytm.startTime;
  response->print(F("<li>Starttime: "));
  response->print(String(mt->hour(lokalstartTime)) + ":");
  if (mt->minute(lokalstartTime) < 10) response->print("0");
  response->print(String(mt->minute(lokalstartTime)));
  response->print(":");
  if (mt->second(lokalstartTime) < 10) response->print("0");
  response->print(String(mt->second(lokalstartTime)));
  response->print(" ");
  response->print(String(mt->day(lokalstartTime)));
  response->print(".");
  response->print(String(mt->month(lokalstartTime)));
  response->print(".");
  response->print(String(mt->year(lokalstartTime)));
  response->print(F("</li>\n"));
  /*
  response->print(F("<li>" LANG_DAILY " " LANG_HOUR ": "));
  response->print(String(glb->randomHour));
  response->print(F("," LANG_HOURLY " " LANG_MINUTE ": "));
  response->print(String(glb->randomMinute));
  response->print(F("</li>\n"));
  */
  response->print(F("<li>" LANG_MOONPHASE " " LANG_CLOCK ": "));
  response->print(String(ow->moonphase));
  response->print(F(" Web: "));
  response->print(String(ow->web_moonphase));
  response->print(F("</li>\n"));
  response->print(F("</ul>\n</li>\n"));

  // ######################### SYSTEM ##################
  response->print(F("<li><b>System</b>\n<ul>\n"));
  response->print(F("<li>ESP-Board: "));
  response->print(BOARD);
  response->print(F("</li>\n"));
  response->print(F("<li>RAM: "));
  response->print("free: " + str_freeheap + F("<br>"));
  response->print("min: "  + str_minheap  + F(" bytes</li>\n"));
  response->print(F("<li>Memory:<br>"));
  
  uint32_t ss;
  UBaseType_t hwm;
  float percent;
  for(uint8_t t=0; t<TASK_MAX; t++) {
    response->print(taskParams.taskInfo[t].name);
    response->print(": Core=");
    response->print(taskParams.taskInfo[t].core);
    response->print(", ");

    ss = taskParams.taskInfo[t].stackSize;
    response->print("  stackSize: ");
    response->print(ss);
    response->print(", ");
    
    if((int)taskParams.taskInfo[t].taskHandle) {
       hwm = uxTaskGetStackHighWaterMark(taskParams.taskInfo[t].taskHandle);
      response->print(" highWaterMark: ");
      response->print(hwm);
      response->print(", ");
    
      percent = ((float) (ss - hwm) / ss) * 100;
      response->print(" percent: ");
      response->print(percent);
      response->print(" %");
    }
    
    response->print("<br>");
  }

  response->print(F("</li>\n"));
  response->print(F("<li>HeapFragmentation: "));
  response->print(str_heapfragmentation);
  response->print(F(" %</li>\n"));
  response->print(F("<li>CpuFreq: "));
  response->print(String(ESP.getCpuFreqMHz()));
  response->print(F(" MHz</li>\n"));
  
  response->print(F("<li>FlashSize (real/ide): "));
#if defined(ESP8266)
  response->print(String(ESP.getFlashChipRealSize() / 1024 / 1024));
  response->print("/");
#endif
  response->print(String(ESP.getFlashChipSize() / 1024 / 1024));
  response->print(F(" MB</li>\n"));
  response->print(F("<li>" LANG_RESETREASON ": "));
#if defined(ESP8266)
  response->print(ESP.getResetReason());
#else
  switch (esp_reset_reason()) {
    case ESP_RST_POWERON:      response->print(F("Power-on Reset")); break;
    case ESP_RST_EXT:          response->print(F("External Reset")); break;
    case ESP_RST_SW:           response->print(F("Software Reset")); break;
    case ESP_RST_PANIC:        response->print(F("Exception/Panic Reset")); break;
    case ESP_RST_INT_WDT:      response->print(F("Hardware Watchdog")); break;
    case ESP_RST_TASK_WDT:     response->print(F("Task Watchdog Reset")); break;
    case ESP_RST_WDT:          response->print(F("Other Watchdog Reset")); break;
    case ESP_RST_DEEPSLEEP:    response->print(F("Deep Sleep Wakeup")); break;
    case ESP_RST_BROWNOUT:     response->print(F("Brownout Reset")); break;
    case ESP_RST_SDIO:         response->print(F("SDIO Reset")); break;
    case ESP_RST_UNKNOWN:      response->print(F("Unknown Reset")); break;
    default:                   response->print(F("Invalid Reset Code"));
  }
#endif
  response->print(F("</li>\n"));

  response->print(F("<li>LittleFS Size (free/total): "));
  response->print(String(fa->getFreeSpace()));
  response->print("/");
  response->print(String(fa->getTotalSpace()));
  response->print(F(" Bytes</li>\n"));

  response->print(F("<li>Compiled: "));
  response->print(BUILD_DATE " " BUILD_TIME);
  response->print(F("</li>\n"));
  response->print(F("</ul>\n</li>\n"));

  // #############
  // ######################### LDR ##################
#ifdef LDR
    response->print(F("<li><b>LDR</b>\n<ul>\n"));
    response->print(F("<li>" LANG_BRIGHTNESS ": "));
    response->print(String(ledDriver->getBrightness()));
    response->print(F("<small> (min: "));
    response->print(String(MIN_BRIGHTNESS));
    response->print(F(", max : "));
    response->print(String(ledDriver->abcBrightness));
    response->print(F(")</small></li>\n<li>ABC: "));
    settings->mySettings.useAbc ? response->print(F("enabled")) : response->print(F("disabled"));
    response->print(F("</li>\n<li>" LANG_LDR_VALUE ": "));
    response->print(String(ledDriver->ldrValue));
    response->print(F("<small> (min: "));
    response->print(String(ledDriver->minLdrValue));
    response->print(F(", max: "));
    response->print(String(ledDriver->maxLdrValue));
    response->print(F(")</small></li>\n"));
    response->print(F("</ul>\n</li>\n"));
#endif

// ######################### SECONDHAND ##################
#if defined(WITH_SECOND_HAND) || defined(WITH_SECOND_BELL)
  response->print(F("<li><b>SECONDS</b>\n<ul>\n"));

#if defined(WITH_SECOND_BELL)
  response->print(F("<li>" LANG_SECONDSBELL ": "));
  response->print(String(settings->mySettings.secondsBell));
  response->print(F("</li>\n"));
#endif

#if defined(WITH_SECOND_HAND)
  response->print(F("<li>" LANG_SECONDHAND ": "));
  response->print(String(settings->mySettings.secondHand));
  response->print(F("</li>\n<li>" LANG_SECONDSALL ": "));
  response->print(String(settings->mySettings.secondsAll));
  response->print(F("</li>\n"));
#endif

  response->print(F("</ul>\n</li>\n"));
#endif

// ######################### BME280 ##################
#ifdef SENSOR_BME280
  response->print(F("<li><b>BME280</b>\n"
               "<ul>\n"));
  response->print(F("<li>Error (BME): "));
  response->print(String(myBME->errorCounterBME));
  response->print(F("</li>\n"));
  response->print(F("<li>" LANG_TEMPERATURE ": "));
  response->print(String(myBME->bme.readTemperature()));
  response->print(F("</li>\n"));
  response->print(F("<li>" LANG_HUMIDITY ": "));
  response->print(String(myBME->bme.readHumidity()));
  response->print(F("</li>\n"));
  response->print(F("<li>" LANG_AIRPRESSURE ": "));
  response->print(String(myBME->bme.readPressure() / 100.0F));
  response->print(F("</li>\n"));
  response->print(F("<li>" LANG_AIRPRESSUREDIFF ": "));
  response->print(String(myBME->info_luftdruckdiff));
  response->print(F("</li>\n"));
  response->print(F("<li>" LANG_AIRPRESSURETHRES ": <small>(A: "));
  response->print(String(LUFTDRUCK_DIFF_LEICHTFALLEND));
  response->print("/");
  response->print(String(LUFTDRUCK_DIFF_LEICHTSTEIGEND));
  response->print(" B: ");
  response->print(String(LUFTDRUCK_DIFF_FALLEND));
  response->print("/");
  response->print(String(LUFTDRUCK_DIFF_STEIGEND));
  response->print(F(")</small></li>\n"));
  response->print(F("</ul>\n</li>\n"));
#endif

// ######################### OPENWEATHER ##################
#ifdef APIKEY
  response->print(F("<li><b>OpenWeather</b>\n"
               "<ul>\n"));
  response->print(F("<li>OpenWeather Error: "));
  response->print(String(outdoorWeather->errorCounterOutdoorWeather));
  response->print(F(" <small>(Code: "));
  response->print(String(outdoorWeather->retcodeOutdoorWeather, BIN));
  response->print(F(")</small></li>\n"));
  response->print(F("<li>Errortext: "));
  response->print(outdoorWeather->owfehler);
  response->print(F("</li>\n"));
  response->print(F("<li>" LANG_OW_LASTUPDATE ": "));
   response->print(String(mt->hour(outdoorWeather->LastOutdoorWeatherTime)));
  response->print(":");
  if (mt->minute(outdoorWeather->LastOutdoorWeatherTime) < 10) response->print("0");
   response->print(String(mt->minute(outdoorWeather->LastOutdoorWeatherTime)));
  response->print(":");
  if (mt->second(outdoorWeather->LastOutdoorWeatherTime) < 10) response->print("0");
  response->print(String(mt->second(outdoorWeather->LastOutdoorWeatherTime)));
  response->print(F("</li>\n"));
  response->print(F("<li>" LANG_WEATHER " Info 1: <small>ID: "));
  response->print(String(outdoorWeather->weatherid1) + F(" ICON: "));
  response->print(outdoorWeather->weathericon1 + F(" CLOUDS: "));
  response->print(outdoorWeather->clouds);
  response->print(F("</small></li>\n"));
  response->print(F("<li>" LANG_WEATHER " Info 2: <small>ID: "));
  response->print(String(outdoorWeather->weatherid2) + F(" ICON: "));
  response->print(outdoorWeather->weathericon2);
  response->print(F("</small></li>\n"));
  response->print(F("<li>" LANG_INFOSUNSETRISE ": "));
  response->print(String(mt->hour(outdoorWeather->sunrise)));
  response->print(":");
  if (mt->minute(outdoorWeather->sunrise) < 10) response->print("0");
  response->print(String(mt->minute(outdoorWeather->sunrise)) + ":");
  if (mt->second(outdoorWeather->sunrise) < 10) response->print("0");
  response->print(String(mt->second(outdoorWeather->sunrise)));

  response->print(F("/"));
  response->print(String(mt->hour(outdoorWeather->sunset)) + ":");
  if (mt->minute(outdoorWeather->sunset) < 10) response->print("0");
  response->print(String(mt->minute(outdoorWeather->sunset)) + ":");
  if (mt->second(outdoorWeather->sunset) < 10) response->print("0");
  response->print(String(mt->second(outdoorWeather->sunset)));
  response->print(F("</li>\n"));
  response->print(F("</ul>\n"
               "</li>\n"));
#endif

// ######################### SUNRISELIB ##################
#ifdef SunRiseLib
  response->print(F("<li><b>SunRiseLib</b>\n"
               "<ul>\n"));
  response->print(F("<li>" LANG_INFOSUNSETRISE ": "));
  response->print(String(mt->hour(ow->sunRiseTime)) + ":");
  if (mt->minute(ow->sunRiseTime) < 10) response->print("0");
  response->print(String(mt->minute(ow->sunRiseTime)) + ":");
  if (mt->second(ow->sunRiseTime) < 10) response->print("0");
  response->print(String(mt->second(ow->sunRiseTime)));

  response->print(F("/"));
  response->print(String(mt->hour(ow->sunSetTime)) + ":");
  if (mt->minute(ow->sunSetTime) < 10) response->print("0");
  response->print(String(mt->minute(ow->sunSetTime)) + ":");
  if (mt->second(ow->sunSetTime) < 10) response->print("0");
  response->print(String(mt->second(ow->sunSetTime)));
  response->print(F("</li>\n"));
  response->print(F("</ul>\n"
               "</li>\n"));
#endif

  // ######################### Events/Modes/Transitions ##################
  response->print(F("<li><b>Events/Mode/Transitions</b>\n"
               "<ul>\n"));
  //response->print(F("<li>" LANG_MODECOUNT ": "));
  //response->print(String(glb->Modecount));
  //response->print(F("</li>\n"));
  response->print(F("<li>autoModeChange-Timer: "));
  response->print(String(autoModeChangeTimer));
  response->print(F("</li>\n"));
  response->print(F("<li>" LANG_LASTTRANS ": ("));
  response->print(String(akt_transition));
  response->print(F(")<small>"));
  if      (akt_transition == TRANSITION_NORMAL)        response->print(F(LANG_TRANSITION_00));
  else if (akt_transition == TRANSITION_FARBENMEER)    response->print(F(LANG_TRANSITION_01));
  else if (akt_transition == TRANSITION_MOVEUP)        response->print(F(LANG_TRANSITION_02));
  else if (akt_transition == TRANSITION_MOVEDOWN)      response->print(F(LANG_TRANSITION_03));
  else if (akt_transition == TRANSITION_MOVELEFT)      response->print(F(LANG_TRANSITION_04));
  else if (akt_transition == TRANSITION_MOVERIGHT)     response->print(F(LANG_TRANSITION_05));
  else if (akt_transition == TRANSITION_MOVELEFTDOWN)  response->print(F(LANG_TRANSITION_06));
  else if (akt_transition == TRANSITION_MOVERIGHTDOWN) response->print(F(LANG_TRANSITION_07));
  else if (akt_transition == TRANSITION_MOVECENTER)    response->print(F(LANG_TRANSITION_08));
  else if (akt_transition == TRANSITION_FADE)          response->print(F(LANG_TRANSITION_09));
  else if (akt_transition == TRANSITION_MATRIX)        response->print(F(LANG_TRANSITION_10));
  else if (akt_transition == TRANSITION_SPIRALE_LINKS) response->print(F(LANG_TRANSITION_11));
  else if (akt_transition == TRANSITION_SPIRALE_RECHTS) response->print(F(LANG_TRANSITION_12));
  else if (akt_transition == TRANSITION_ZEILENWEISE)   response->print(F(LANG_TRANSITION_13));
  else if (akt_transition == TRANSITION_REGENBOGEN)    response->print(F(LANG_TRANSITION_14));
  else if (akt_transition == TRANSITION_MITTE_LINKSHERUM) response->print(F(LANG_TRANSITION_15));
  else if (akt_transition == TRANSITION_QUADRATE)      response->print(F(LANG_TRANSITION_16));
  else if (akt_transition == TRANSITION_KREISE)        response->print(F(LANG_TRANSITION_17));
  else
    response->print(F("unbekannt"));

  response->print(F(" </small></li>\n"));
  response->print(F("</ul>\n"
               "</li>\n"));

  // ######################### Spiele ##################
  response->print(F("<li><b>" LANG_GAMES "</b>\n"
               "<ul>\n"));
  response->print(F("<li>" LANG_SNAKE " Count/Highscore: "));
  response->print(String(game->gamecount[SNAKE]));
  response->print("/");
  response->print(String(settings->mySettings.highscore[SNAKE]));
  response->print(F("</li>\n"));
  response->print(F("<li>" LANG_TETRIS " Count/Highscore: "));
  response->print(String(game->gamecount[TETRIS]));
  response->print("/");
  response->print(String(settings->mySettings.highscore[TETRIS]));
  response->print(F("</li>\n"));
  response->print(F("<li>" LANG_BRICKS " Count/Highscore: "));
  response->print(String(game->gamecount[BRICKS]));
  response->print("/" + String(settings->mySettings.highscore[BRICKS]));
  response->print(F("</li>\n"));
  response->print(F("<li>" LANG_4GEWINNT " Count/Highscore: "));
  response->print(String(game->gamecount[VIERGEWINNT]));
  response->print("/");
  response->print(String(settings->mySettings.highscore[VIERGEWINNT]));
  response->print(F("</li>\n"));

  response->print(F("</ul>\n</li>\n"));

  // ######################### Flags ##################
  response->print(F("<li><b>Flags</b>\n"
               "<ul>\n<li>"));

#ifdef LDR
  response->print(F("LDR "));
#endif
  response->print(F("</li>\n<li>"));
#ifdef ESP_LED
  response->print(F("ESP_LED "));
#endif
#if defined(ONOFF_BUTTON) || defined(MODE_BUTTON) || defined(SHOW_TIME_BUTTON)
  response->print(F("BUTTONS "));
#endif
#ifdef WITH_SECOND_HAND
  response->print(F("SECOND_HAND "));
#endif
#ifdef WITH_SECOND_BELL
  response->print(F("SECOND_BELL "));
#endif
#ifdef SYSLOGSERVER_SERVER
  response->print(F("SYSLOG "));
#endif
#ifdef WITH_ALEXA
  response->print(F("ALEXA "));
#endif
#ifdef WITH_MQTT
  response->print(F("MQTT "));
#endif
  response->print(F("</li>\n"));
  response->print(F("</ul>\n"
               "</li>\n"));

  response->print(F("</ul>\n"));

  response->print(F("</body></html>"));

  request->send(response);

#if defined(SYSLOGSERVER_SERVER)
  syslog.log(LOG_INFO,"debugClock: end");
#endif
}
