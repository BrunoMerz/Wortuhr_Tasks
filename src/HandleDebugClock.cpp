#include "Arduino.h"
#include "Configuration.h"
#include "Languages.h"
#include "Settings.h"
#include "MyTime.h"
#include "Html_content.h"
#include "Spiel_externals.h"
#include "Spiel_main.h"
#include "OpenWeather.h"
#include "MyFileAccess.h"
#include "Global.h"
#include "LedDriver_FastLED.h"

#include "MyDebug.h"

#include "ESPAsyncWebServer.h"
#include <LittleFS.h>

const char compile_date[] = __DATE__ " " __TIME__;

extern float getHeapFragmentation();
extern uint16_t getWeatherSound(uint16_t wetterid);

static MyTime *mt = MyTime::getInstance();
static Settings *settings = Settings::getInstance();
static OpenWeather *outdoorWeather = OpenWeather::getInstance();
static MyFileAccess *fa = MyFileAccess::getInstance();
static OpenWeather *ow = OpenWeather::getInstance();
static Global *glb = Global::getInstance();
static LedDriver *ledDriver = LedDriver::getInstance();


//debugClock
void debugClock(AsyncWebServerRequest *request)
{
  AsyncResponseStream *response = request->beginResponseStream(TEXT_HTML);
#ifdef DEBUG
  DEBUG_PRINTLN("Info Seite refresh!");
#endif

#if defined(ESP8266)
  String str_freeheap = String(ESP.getFreeHeap());
  String str_maxfreeblocks = String(ESP.getMaxFreeBlockSize());
  String str_heapfragmentation = String(ESP.getHeapFragmentation());
#else
  String str_freeheap = String(esp_get_free_heap_size());
  String str_maxfreeblocks = String(heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
  String str_heapfragmentation = String(getHeapFragmentation());  
#endif
  delay(0);
  response->setContentLength(CONTENT_LENGTH_UNKNOWN);
  String message;
  message = F("<!doctype html>"
              "<html>"
              "<head>");
  message += F("<title>");
  message += String(settings->mySettings.systemname);
  message += F(" Info</title>");
  message += F("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
               "<meta http-equiv=\"refresh\" content=\"30\" charset=\"UTF-8\">"
               "<link rel=\"icon\" type=\"image/png\" sizes=\"192x192\" href=\"/web/android-icon-192x192.png\">"
               "<link rel=\"icon\" type=\"image/png\" sizes=\"32x32\" href=\"/web/favicon-32x32.png\">"
               "<link rel=\"icon\" type=\"image/png\" sizes=\"96x96\" href=\"/web/favicon-96x96.png\">"
               "<link rel=\"icon\" type=\"image/png\" sizes=\"16x16\" href=\"/web/favicon-16x16.png\">"
               "<style>"
               "body{background-color:#FFFFFF;color:#333333;font-family:Sans-serif;font-size:16px;}"
               "</style>"
               "</head>\n"
               "<body>");
  // ################### sende html Teil 1
  response->print(message);
  message = "";


  message += F("<h2>");
  message += String(settings->mySettings.systemname);
  message += F(" Info</h2>");
  message += F("<hr>\n"
               "<small><br></small>");
  message += F("<big><b>&bull;  Firmware: ");
  message += String(FIRMWARE_VERSION);
  message += " ";
  message += __DATE__;
  message += F("</b></big>\n");
#if defined(FRONTCOVER_DE_DE) || defined(FRONTCOVER_DE_SW) || defined(FRONTCOVER_DE_BA) || defined(FRONTCOVER_DE_SA) || defined(FRONTCOVER_D3) || defined(FRONTCOVER_DE_MKF_DE) || defined(FRONTCOVER_DE_MKF_SW) || defined(FRONTCOVER_DE_MKF_BA) || defined(FRONTCOVER_DE_MKF_SA) || defined(FRONTCOVER_CH) || defined(FRONTCOVER_CH_AG) || defined(FRONTCOVER_CH_AL)
  message += F("<a href=\"https://merz-aktuell.de/Wordclock/Doku/WortuhrBeschreibung10.x.pdf\" target=\"_blank\" style=\"font-size:30px;\">&#128214;</a>\n");
#else
  message += F("<a href=\"https://merz-aktuell.de/Wordclock/Doku/WortuhrBeschreibung10.x.pdf\" target=\"_blank\" style=\"font-size:30px;\">&#128214;</a>\n");
#endif
  message += F("<ul>\n"
               // ######################### WIFI/NETZWERK ##################
               "<li><b>"
               LANG_NETWORKWIFI
               "</b>\n"
               "<ul>\n");
  message += F("<li>WLan-SID: ");
  message += WiFi.SSID();
  message += F("</li>\n");
  message += F("<li>WLan-BSSID:");
  message += WiFi.BSSIDstr();
  message += F("</li>\n");
  message += F("<li>MAC-ADDR:");
  message += WiFi.macAddress();
  message += F("</li>\n");
  message += F("<li>" LANG_WIFISIGNAL ": ");
  message += WiFi.RSSI();
  message += F(" dBm (");
  if ( WiFi.RSSI() >= -50 ) message += F(LANG_WIFIQ1);
  if ( WiFi.RSSI() >= -65 && WiFi.RSSI() < -50 ) message += F(LANG_WIFIQ2);
  if ( WiFi.RSSI() >= -70 && WiFi.RSSI() < -65 ) message += F(LANG_WIFIQ3);
  if ( WiFi.RSSI() >= -75 && WiFi.RSSI() < -70 ) message += F(LANG_WIFIQ4);
  if ( WiFi.RSSI() >= -80 && WiFi.RSSI() < -75 ) message += F(LANG_WIFIQ5);
  if ( WiFi.RSSI() >= -90 && WiFi.RSSI() < -80 ) message += F(LANG_WIFIQ6);
  if ( WiFi.RSSI() < -90 ) message += F(LANG_WIFIQ7);
  message += F(")</li>\n");
  message += F("<li>" LANG_WIFIRECON ": ");
  message += String(glb->WLAN_reconnect) + "</li>\n";
  message += F("<li>" LANG_IPADRESS ": ");
  message += String(glb->myIP[0]) + '.' + String(glb->myIP[1]) + '.' + String(glb->myIP[2]) + '.' + String(glb->myIP[3]);
  message += F("</li>\n<li>Client IP-Addr: ");
  message += request->client()->remoteIP().toString();
  message += F("</li>\n");
  message += F("</ul>\n</li>\n");

  // ######################### ZEIT ##################
  message += F("<li><b>" LANG_TIME "</b>\n"
               "<ul>\n");
  message += F("<li>" LANG_TIMEHOST ": ");
  message += String(settings->mySettings.ntphost);
  message += F("</li>\n");
  message += F("<li>" LANG_TIMEZONE ": ");
  message += String(settings->mySettings.timezone);
  message += F("</li>\n");
  //message += F("<li>Error (NTP): ");
  //message += String(errorCounterNTP);
  //message += F("</li>\n");
  //message += F("<li>ESP-Time Drift in sek: ");
  //message += String(mt->mytm.esptimedrift);
  //message += F(" <small>(max: ");
  //message += String(mt->mytm.maxesptimedrift);
  //message += F(")</small></li>\n");
  time_t tempEspTime = mt->local();
  message += F("<li>" LANG_TIME ": ");
  message += String(mt->hour(tempEspTime)) + ":";
  if (mt->minute(tempEspTime) < 10) message += "0";
  message += String(mt->minute(tempEspTime));
  message += ":";
  if (mt->second(tempEspTime) < 10) message += "0";
  message += String(mt->second(tempEspTime));
  //if (timeZone.locIsDST(mt->local(())) message += " (Sommerzeit)";
  if (mt->mytm.tm_isdst) message += " (Sommerzeit)";
  message += F("<li>" LANG_DATE ":");
  message += String(mt->dayStr(mt->weekday(tempEspTime)));
  message += F(", ");
  message += String(mt->day(tempEspTime));
  message += ". ";
  message += String(mt->monthStr(mt->month(tempEspTime)));
  message += " ";
  message += String(mt->year(tempEspTime));
  message += F("</li>\n");
  message += F("</li>\n<li>UTC-TST:");
  //message += String(timeZone.mt->toUTC(tempEspTime));
  message += String(mt->toUTC(tempEspTime));
  message += F("</li>\n");
  message += F("<li>Uptime: ");
  message += mt->convertSeconds(mt->mytm.upTime, true);
  message += F("</li>\n");
  //time_t lokalstartTime = timeZone.toLocal(startTime);
  time_t lokalstartTime = mt->mytm.startTime;
  message += F("<li>Starttime: ");
  message += String(mt->hour(lokalstartTime)) + ":";
  if (mt->minute(lokalstartTime) < 10) message += "0";
  message += String(mt->minute(lokalstartTime));
  message += ":";
  if (mt->second(lokalstartTime) < 10) message += "0";
  message += String(mt->second(lokalstartTime));
  message += " ";
  message += String(mt->day(lokalstartTime));
  message += ".";
  message += String(mt->month(lokalstartTime));
  message += ".";
  message += String(mt->year(lokalstartTime));
  message += F("</li>\n");
  message += F("<li>" LANG_DAILY " " LANG_HOUR ": ");
  message += String(glb->randomHour);
  message += F("," LANG_HOURLY " " LANG_MINUTE ": ");
  message += String(glb->randomMinute);
  message += F("</li>\n");
  message += F("<li>" LANG_MOONPHASE " " LANG_CLOCK ": ");
  message += String(ow->moonphase);
  message += F(" Web: ");
  message += String(ow->web_moonphase);
  message += F("</li>\n");
  message += F("</ul>\n</li>\n");

  //##################### sende 2. html Teil
  response->print(message);
  message = "";

  // ######################### SYSTEM ##################
  message += F("<li><b>System</b>\n<ul>\n");
  message += F("<li>ESP-Board: ");
  message += BOARD;
  message += F("</li>\n");
  message += F("<li>Free RAM: ");
  message += str_freeheap + F(" bytes</li>\n");
  message += F("<li>MaxFreeBlockSize: ");
  message += str_maxfreeblocks;
  message += F(" bytes</li>\n");
  message += F("<li>MinFreeBlockSize: ");
  message += String(glb->minFreeBlockSize);
  message += F("<small> bytes Codeline: <br>");
  message += glb->codetab;
  message += ":";
  message += String(glb->codeline);
  message += F("</small></li>\n");
  message += F("<li>HeapFragmentation: ");
  message += str_heapfragmentation;
  message += F(" %</li>\n");
  message += F("<li>CpuFreq: ");
  message += String(ESP.getCpuFreqMHz());
  message += F(" MHz</li>\n");
  
  message += F("<li>FlashSize (real/ide): ");
#if defined(ESP8266)
  message += String(ESP.getFlashChipRealSize() / 1024 / 1024);
  message += "/";
#endif
  message += String(ESP.getFlashChipSize() / 1024 / 1024);
  message += F(" MB</li>\n");
  message += F("<li>" LANG_RESETREASON ": ");
#if defined(ESP8266)
  message += ESP.getResetReason();
#else
  switch (esp_reset_reason()) {
    case ESP_RST_POWERON:      message += F("Power-on Reset"); break;
    case ESP_RST_EXT:          message += F("External Reset"); break;
    case ESP_RST_SW:           message += F("Software Reset"); break;
    case ESP_RST_PANIC:        message += F("Exception/Panic Reset"); break;
    case ESP_RST_INT_WDT:      message += F("Hardware Watchdog"); break;
    case ESP_RST_TASK_WDT:     message += F("Task Watchdog Reset"); break;
    case ESP_RST_WDT:          message += F("Other Watchdog Reset"); break;
    case ESP_RST_DEEPSLEEP:    message += F("Deep Sleep Wakeup"); break;
    case ESP_RST_BROWNOUT:     message += F("Brownout Reset"); break;
    case ESP_RST_SDIO:         message += F("SDIO Reset"); break;
    case ESP_RST_UNKNOWN:      message += F("Unknown Reset"); break;
    default:                   message += F("Invalid Reset Code");
  }
#endif
  message += F("</li>\n");

  message += F("<li>LittleFS Size (free/total): ");
  message += String(fa->getFreeSpace());
  message += "/";
  message += String(fa->getTotalSpace());
  message += F(" Bytes</li>\n");

  message += F("<li>Compiled: ");
  message += compile_date;
  message += F("</li>\n");
  message += F("</ul>\n</li>\n");

  // ######################### AUDIO ##################
#ifdef WITH_AUDIO
  message += F("<li><b>Audio</b>\n"
               "<ul>\n");
  message += F("<li>" LANG_SPEAKER ": ");
  if ( settings->mySettings.sprecher ) message += F("Vicki"); else message += F("Hans");
  message += F("</li>\n");
  message += F("<li>" LANG_VOLUME " (0-30): ");  message += String(VOLUME_ALT);
  message += F("</li>\n");
#ifdef CHECK_MP3
  message += F("<li>MP3-Resets: ");  message += String(mp3resetcount);
  message += F("</li>\n");
#endif
  message += F("</ul>\n"
               "</li>\n");
#endif

  // ######################### LDR ##################
#ifdef LDR
    message += F("<li><b>LDR</b>\n<ul>\n");
    message += F("<li>" LANG_BRIGHTNESS ": ");
    message += String(ledDriver->getBrightness());
    message += F("<small> (min: ");
    message += String(MIN_BRIGHTNESS);
    message += F(", max : ");
    message += String(ledDriver->abcBrightness);
    message += F(")</small></li>\n<li>ABC: ");
    settings->mySettings.useAbc ? message += F("enabled") : message += F("disabled");
    message += F("</li>\n<li>" LANG_LDR_VALUE ": ");
    message += String(ledDriver->ldrValue);
    message += F("<small> (min: ");
    message += String(ledDriver->minLdrValue);
    message += F(", max: ");
    message += String(ledDriver->maxLdrValue);
    message += F(")</small></li>\n");
    message += F("</ul>\n</li>\n");
#endif



  // ######################### SECONDHAND ##################
#if defined(WITH_SECOND_HAND) || defined(WITH_SECOND_BELL)
  message += F("<li><b>SECONDS</b>\n<ul>\n");

#if defined(WITH_SECOND_BELL)
  message += F("<li>" LANG_SECONDSBELL ": ");
  message += String(settings->mySettings.secondsBell);
  message += F("</li>\n");
#endif

#if defined(WITH_SECOND_HAND)
  message += F("<li>" LANG_SECONDHAND ": ");
  message += String(settings->mySettings.secondHand);
  message += F("</li>\n<li>" LANG_SECONDSALL ": ");
  message += String(settings->mySettings.secondsAll);
  message += F("</li>\n");
#endif

  message += F("</ul>\n</li>\n");
#endif


  //##################### sende 3. html Teil
  response->print(message);
  message = "";

  // ######################### BME280 ##################
#ifdef SENSOR_BME280
  message += F("<li><b>BME280</b>\n"
               "<ul>\n");
  message += F("<li>Error (BME): ");
  message += String(errorCounterBME);
  message += F("</li>\n");
  message += F("<li>" LANG_TEMPERATURE ": ");
  message += String(bme.readTemperature());
  message += F("</li>\n");
  message += F("<li>" LANG_HUMIDITY ": ");
  message += String(bme.readHumidity());
  message += F("</li>\n");
  message += F("<li>" LANG_AIRPRESSURE ": ");
  message += String(bme.readPressure() / 100.0F);
  message += F("</li>\n");
  message += F("<li>" LANG_AIRPRESSUREDIFF ": ");
  message += String(info_luftdruckdiff);
  message += F("</li>\n");
  message += F("<li>" LANG_AIRPRESSURETHRES ": <small>(A: ");
  message += String(LUFTDRUCK_DIFF_LEICHTFALLEND);
  message += "/";
  message += String(LUFTDRUCK_DIFF_LEICHTSTEIGEND);
  message += " B: ";
  message += String(LUFTDRUCK_DIFF_FALLEND);
  message += "/";
  message += String(LUFTDRUCK_DIFF_STEIGEND);
  message += F(")</small></li>\n");
  message += F("</ul>\n</li>\n");
#endif

  // ######################### OPENWEATHER ##################
#ifdef APIKEY
  message += F("<li><b>OpenWeather</b>\n"
               "<ul>\n");
  message += F("<li>OpenWeather Error: ");
  message += String(outdoorWeather->errorCounterOutdoorWeather);
  message += F(" <small>(Code: ");
  message += String(outdoorWeather->retcodeOutdoorWeather, BIN);
  message += F(")</small></li>\n");
  message += F("<li>Errortext: ");
  message += outdoorWeather->owfehler;
  message += F("</li>\n");
  message += F("<li>" LANG_OW_LASTUPDATE ": ");
  //message += String(mt->hour(timeZone.toLocal(LastOutdoorWeatherTime)));
  message += String(mt->hour(outdoorWeather->LastOutdoorWeatherTime));
  message += ":";
  //if (mt->minute(timeZone.toLocal(LastOutdoorWeatherTime)) < 10) message += "0";
  if (mt->minute(outdoorWeather->LastOutdoorWeatherTime) < 10) message += "0";
  //message += String(mt->minute(timeZone.toLocal(LastOutdoorWeatherTime)));
   message += String(mt->minute(outdoorWeather->LastOutdoorWeatherTime));
  message += ":";
  //if (mt->second(timeZone.toLocal(LastOutdoorWeatherTime)) < 10) message += "0";
  if (mt->second(outdoorWeather->LastOutdoorWeatherTime) < 10) message += "0";
  //message += String(mt->second(timeZone.toLocal(LastOutdoorWeatherTime)));
  message += String(mt->second(outdoorWeather->LastOutdoorWeatherTime));
  message += F("</li>\n");
  message += F("<li>" LANG_WEATHER " Info 1: <small>ID: ");
  message += String(outdoorWeather->weatherid1) + F(" ICON: ");
  message += outdoorWeather->weathericon1 + F(" CLOUDS: ");
  message += outdoorWeather->clouds;
  message += F("</small></li>\n");
  message += F("<li>" LANG_WEATHER " Sound 1: <small>");
  message += String(getWeatherSound(outdoorWeather->weatherid1));
  message += F("</small></li>\n");
  message += F("<li>" LANG_WEATHER " Info 2: <small>ID: ");
  message += String(outdoorWeather->weatherid2) + F(" ICON: ");
  message += outdoorWeather->weathericon2;
  message += F("</small></li>\n");
  message += F("<li>" LANG_INFOSUNSETRISE ": ");
  //if ( !global->ani_sunrise_done ) message += F("<b>");
  message += String(mt->hour(outdoorWeather->sunrise));
  message += ":";
  if (mt->minute(outdoorWeather->sunrise) < 10) message += "0";
  message += String(mt->minute(outdoorWeather->sunrise)) + ":";
  if (mt->second(outdoorWeather->sunrise) < 10) message += "0";
  message += String(mt->second(outdoorWeather->sunrise));
  //if ( !global->ani_sunrise_done ) message += F("</b>");

  message += F("/");
  //if ( !global->ani_sunset_done ) message += F("<b>");
  message += String(mt->hour(outdoorWeather->sunset)) + ":";
  if (mt->minute(outdoorWeather->sunset) < 10) message += "0";
  message += String(mt->minute(outdoorWeather->sunset)) + ":";
  if (mt->second(outdoorWeather->sunset) < 10) message += "0";
  message += String(mt->second(outdoorWeather->sunset));
  //if ( !global->ani_sunset_done ) message += F("</b>");
  message += F("</li>\n");
  message += F("</ul>\n"
               "</li>\n");
#endif

  // ######################### SUNRISELIB ##################
#ifdef SunRiseLib
  message += F("<li><b>SunRiseLib</b>\n"
               "<ul>\n");
  message += F("<li>" LANG_INFOSUNSETRISE ": ");
  //if ( !global->ani_sunrise_done ) message += F("<b>");
  message += String(mt->hour(ow->sunRiseTime)) + ":";
  if (mt->minute(ow->sunRiseTime) < 10) message += "0";
  message += String(mt->minute(ow->sunRiseTime)) + ":";
  if (mt->second(ow->sunRiseTime) < 10) message += "0";
  message += String(mt->second(ow->sunRiseTime));
  //if ( !global->ani_sunrise_done ) message += F("</b>");

  message += F("/");
  //if ( !global->ani_sunset_done ) message += F("<b>");
  message += String(mt->hour(ow->sunSetTime)) + ":";
  if (mt->minute(ow->sunSetTime) < 10) message += "0";
  message += String(mt->minute(ow->sunSetTime)) + ":";
  if (mt->second(ow->sunSetTime) < 10) message += "0";
  message += String(mt->second(ow->sunSetTime));
  //if ( !global->ani_sunset_done ) message += F("</b>");
  message += F("</li>\n");
  message += F("</ul>\n"
               "</li>\n");
#endif

  //##################### sende 4. html Teil
  response->print(message);
  message = "";


  // ######################### Events/Modes/Transitions ##################
  message += F("<li><b>Events/Mode/Transitions</b>\n"
               "<ul>\n");
  message += F("<li>" LANG_MODECOUNT ": ");
  message += String(glb->Modecount);
  message += F("</li>\n");
  message += F("<li>Event-Timer: ");
  message += String(glb->showEventTimer);
  message += F("</li>\n");
  message += F("<li>autoModeChange-Timer: ");
  message += String(glb->autoModeChangeTimer);
  message += F("</li>\n");
  message += F("<li>" LANG_LASTTRANS ": (");
  message += String(glb->akt_transition);
  message += F(")<small>");
  if      (glb->akt_transition == TRANSITION_NORMAL)        message += F(LANG_TRANSITION_00);
  else if (glb->akt_transition == TRANSITION_FARBENMEER)    message += F(LANG_TRANSITION_01);
  else if (glb->akt_transition == TRANSITION_MOVEUP)        message += F(LANG_TRANSITION_02);
  else if (glb->akt_transition == TRANSITION_MOVEDOWN)      message += F(LANG_TRANSITION_03);
  else if (glb->akt_transition == TRANSITION_MOVELEFT)      message += F(LANG_TRANSITION_04);
  else if (glb->akt_transition == TRANSITION_MOVERIGHT)     message += F(LANG_TRANSITION_05);
  else if (glb->akt_transition == TRANSITION_MOVELEFTDOWN)  message += F(LANG_TRANSITION_06);
  else if (glb->akt_transition == TRANSITION_MOVERIGHTDOWN) message += F(LANG_TRANSITION_07);
  else if (glb->akt_transition == TRANSITION_MOVECENTER)    message += F(LANG_TRANSITION_08);
  else if (glb->akt_transition == TRANSITION_FADE)          message += F(LANG_TRANSITION_09);
  else if (glb->akt_transition == TRANSITION_MATRIX)        message += F(LANG_TRANSITION_10);
  else if (glb->akt_transition == TRANSITION_SPIRALE_LINKS) message += F(LANG_TRANSITION_11);
  else if (glb->akt_transition == TRANSITION_SPIRALE_RECHTS) message += F(LANG_TRANSITION_12);
  else if (glb->akt_transition == TRANSITION_ZEILENWEISE)   message += F(LANG_TRANSITION_13);
  else if (glb->akt_transition == TRANSITION_REGENBOGEN)    message += F(LANG_TRANSITION_14);
  else if (glb->akt_transition == TRANSITION_MITTE_LINKSHERUM) message += F(LANG_TRANSITION_15);
  else if (glb->akt_transition == TRANSITION_QUADRATE)      message += F(LANG_TRANSITION_16);
  else if (glb->akt_transition == TRANSITION_KREISE)        message += F(LANG_TRANSITION_17);
  else
    message += F("unbekannt");
  message += F(" </small></li>\n");
  message += F("</ul>\n"
               "</li>\n");

  // ######################### Spiele ##################
  message += F("<li><b>" LANG_GAMES "</b>\n"
               "<ul>\n");
  message += F("<li>" LANG_SNAKE " Count/Highscore: ");
  message += String(gamecount[SNAKE]);
  message += "/";
  message += String(highscore[SNAKE]);
  message += F("</li>\n");
  message += F("<li>" LANG_TETRIS " Count/Highscore: ");
  message += String(gamecount[TETRIS]);
  message += "/";
  message += String(highscore[TETRIS]);
  message += F("</li>\n");
  message += F("<li>" LANG_BRICKS " Count/Highscore: ");
  message += String(gamecount[BRICKS]);
  message += "/" + String(highscore[BRICKS]);
  message += F("</li>\n");
  message += F("<li>" LANG_4GEWINNT " Count/Highscore: ");
  message += String(gamecount[VIERGEWINNT]);
  message += "/";
  message += String(highscore[VIERGEWINNT]);
  message += F("</li>\n");
  message += F("<li>" LANG_MEMORY1 " Count: ");
  message += String(gamecount[TIERMEMORY]);  
  message += F("</li>\n");
  message += F("<li>" LANG_MEMORY2 " Count: ");
  message += String(gamecount[MUSIKMEMORY]);
  message += F("</li>\n");
  message += F("<li>" LANG_MEMORY3 " Count: ");
  message += String(gamecount[ABBAMEMORY]) + F("</li>\n");

  message += F("</ul>\n</li>\n");

  // ######################### Flags ##################
  message += F("<li><b>Flags</b>\n"
               "<ul>\n<li>");
#ifdef RTC_BACKUP
  message += F("RTC ");
#endif
#ifdef SENSOR_BME280
  message += F("BME280 ");
#endif
#ifdef LDR
  message += F("LDR ");
#endif
#ifdef BUZZER
  message += F("BUZZER ");
#endif
#ifdef WITH_AUDIO
  message += F("AUDIO ");
#endif
  message += F("</li>\n<li>");
#ifdef IR_RECEIVER
  message += F("IR_RECEIVER ");
#endif
#ifdef ESP_LED
  message += F("ESP_LED ");
#endif
#if defined(ONOFF_BUTTON) || defined(MODE_BUTTON) || defined(SHOW_TIME_BUTTON)
  message += F("BUTTONS ");
#endif
#ifdef WITH_SECOND_HAND
  message += F("SECOND_HAND ");
#endif
#ifdef WITH_SECOND_BELL
  message += F("SECOND_BELL ");
#endif
#ifdef SYSLOGSERVER_SERVER
  message += F("SYSLOG ");
#endif
#ifdef WITH_ALEXA
  message += F("ALEXA ");
#endif
#ifdef WITH_MQTT
  message += F("MQTT ");
#endif
  message += F("</li>\n");
  message += F("</ul>\n"
               "</li>\n");

  message += F("</ul>\n");

  message += F("</body></html>");
  //##################### sende letzen html Teil
  response->print(message);
  request->send(response);
  message = "";

}
