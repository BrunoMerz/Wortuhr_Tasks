#include "Arduino.h"
#include "Configuration.h"
#include "Languages.h"
#include "Settings.h"
#include "Renderer.h"
#include "LedDriver_FastLED.h"
#include "MyTime.h"
#include "Html_content.h"
#include "Helper.h"

//#define myDEBUG
#include "MyDebug.h"

#include "ESPAsyncwebServer.h"
#include <LittleFS.h>

extern void callRoot(AsyncWebServerRequest *request);
extern uint16_t matrix[];
extern uint16_t nightOffTime;
extern uint16_t dayOnTime;


static Settings *settings = Settings::getInstance();
static MyTime *mt = MyTime::getInstance();
static LedDriver *ledDriver = LedDriver::getInstance();
static Renderer *renderer = Renderer::getInstance();


void handleCommitSettings(AsyncWebServerRequest *request) 
{
  DEBUG_PRINTF("commit: Core=%d\n", xPortGetCoreID());
#if defined(myDEBUG)
  Serial.println(F("Commit settings pressed."));
  for (int i = 0; i < request->args(); i++)
    Serial.printf("name=%s, value=%s\n", request->argName(i).c_str(), request->arg(i).c_str());
#endif
  // ------------------------------------------------------------------------
#if defined(BUZZER) || defined(WITH_AUDIO)
  time_t alarmTimeFromWeb = 0;
  // Alarm1
  if (request->arg("a1") == "0")
    settings->mySettings.alarm1 = false;
  if (request->arg("a1") == "1")
    settings->mySettings.alarm1 = true;

  if (request->arg("a1t") != "")
  {
    alarmTimeFromWeb = request->arg("a1t").substring(0, 2).toInt() * 3600 + request->arg("a1t").substring(3, 5).toInt() * 60;
    if (settings->mySettings.alarm1Time != alarmTimeFromWeb)
      settings->mySettings.alarm1 = true;
    settings->mySettings.alarm1Time = alarmTimeFromWeb;
    settings->mySettings.alarm1Weekdays =
        request->arg("a1w1").toInt() +
        request->arg("a1w2").toInt() +
        request->arg("a1w3").toInt() +
        request->arg("a1w4").toInt() +
        request->arg("a1w5").toInt() +
        request->arg("a1w6").toInt() +
        request->arg("a1w7").toInt();
  }
  // ------------------------------------------------------------------------
  // Alarm2
  if (request->arg("a2") == "0")
    settings->mySettings.alarm2 = false;
  if (request->arg("a2") == "1")
    settings->mySettings.alarm2 = true;
  if (request->arg("a2t") != "")
  {
    alarmTimeFromWeb = request->arg("a2t").substring(0, 2).toInt() * 3600 + request->arg("a2t").substring(3, 5).toInt() * 60;
    if (settings->mySettings.alarm2Time != alarmTimeFromWeb)
      settings->mySettings.alarm2 = true;
    settings->mySettings.alarm2Time = alarmTimeFromWeb;
    settings->mySettings.alarm2Weekdays =
        request->arg("a2w1").toInt() +
        request->arg("a2w2").toInt() +
        request->arg("a2w3").toInt() +
        request->arg("a2w4").toInt() +
        request->arg("a2w5").toInt() +
        request->arg("a2w6").toInt() +
        request->arg("a2w7").toInt();
  }
  // ------------------------------------------------------------------------
  // Stundenschlag
  if (request->arg("hb") == "0")
    settings->mySettings.hourBeep = false;
  if (request->arg("hb") == "1")
    settings->mySettings.hourBeep = true;
  // ------------------------------------------------------------------------
  // Timer
  if (request->arg("ti") != "")
  {
    if (request->arg("ti").toInt())
    {
      alarmTimer = request->arg("ti").toInt();
      alarmTimerSecond = second();
      alarmTimerSet = true;
      setMode(MODE_TIMER);
    }
  }
#endif
#ifdef WITH_AUDIO
  // Lautstärke
  if (request->arg("vol") != "")
    settings->mySettings.volume = request->arg("vol").toInt();
  // Zufallssound
  if (request->arg(F("srand")) == "0")
    settings->mySettings.randomsound = false;
  if (request->arg(F("srand")) == "1")
    settings->mySettings.randomsound = true;

  // Stundensound für Wochentage
  for (uint8_t wti = 0; wti < 7; wti++)
  {
    if (request->arg("wsf" + String(wti)) != "")
      settings->mySettings.weekdaysoundfile[wti] = request->arg("wsf" + String(wti)).toInt();
  }
  // Sprecher
  if (request->arg(F("sprech")) == "1")
  {
    settings->mySettings.sprecher = true;
    ANSAGEBASE = AUDIO_BASENR_VICKI;
  }
  if (request->arg(F("sprech")) == "0")
  {
    settings->mySettings.sprecher = false;
    ANSAGEBASE = AUDIO_BASENR_HANS;
  }
#ifdef DEBUG
  DEBUG_PRINTF("ANSAGEBASE: %i\r\n", ANSAGEBASE);
#endif

  // Wochenend später lauter
  if (request->arg("wsl") == "0")
    settings->mySettings.weekendlater = false;
  if (request->arg("wsl") == "1")
    settings->mySettings.weekendlater = true;
  // Singlegong
  if (request->arg("sg") == "0")
    settings->mySettings.singlegong = false;
  if (request->arg("sg") == "1")
    settings->mySettings.singlegong = true;
  // 12/24 Stunden Ansage
  if (request->arg("vh24") == "0")
    settings->mySettings.vickihans24h = false;
  if (request->arg("vh24") == "1")
    settings->mySettings.vickihans24h = true;

#endif
  // ------------------------------------------------------------------------
  // AutoModeChange / AutoModeChange Timer
  if (request->arg("mc") == "0")
    settings->mySettings.modeChange = false;
  if (request->arg("mc") == "1")
  {
    settings->mySettings.modeChange = true;
    settings->mySettings.auto_mode_change = request->arg("amct").toInt();
    if (settings->mySettings.auto_mode_change < 2)
      settings->mySettings.auto_mode_change = 2;
//mz    if (autoModeChangeTimer != settings->mySettings.auto_mode_change * 60)
//mz      autoModeChangeTimer = settings->mySettings.auto_mode_change * 60;
  }
  // ------------------------------------------------------------------------
#if defined(SunRiseLib) || defined(APIKEY)
  // Sunrise/ Sunset
  if (request->arg("sunr") == "0")
    settings->mySettings.ani_sunrise = false;
  if (request->arg("sunr") == "1")
    settings->mySettings.ani_sunrise = true;
  if (request->arg("suns") == "0")
    settings->mySettings.ani_sunset = false;
  if (request->arg("suns") == "1")
    settings->mySettings.ani_sunset = true;
#endif
  // ------------------------------------------------------------------------
  // Helligkeit in Prozent
  if (request->arg("br") != "")
    settings->mySettings.brightness = request->arg("br").toInt();

  // ------------------------------------------------------------------------
#ifdef LDR
  // ABC
  if (request->arg("ab") == "0")
  {
    settings->mySettings.useAbc = false;
    ledDriver->setBrightness(map(settings->mySettings.brightness, 10, 100, MIN_BRIGHTNESS, MAX_BRIGHTNESS));
  }
  if (request->arg("ab") == "1")
  {
    settings->mySettings.useAbc = true;
    ledDriver->abcBrightness = map(settings->mySettings.brightness, 10, 100, MIN_BRIGHTNESS, MAX_BRIGHTNESS);
    if (ledDriver->lastLdrValue <= 200)
    ledDriver->lastLdrValue = ledDriver->lastLdrValue + 99;
   else
    ledDriver->lastLdrValue = ledDriver->lastLdrValue - 99;
  }
#else
  settings->mySettings.useAbc = false;
  ledDriver->setBrightness(map(settings->mySettings.brightness, 10, 100, MIN_BRIGHTNESS, MAX_BRIGHTNESS));
#endif

#ifdef WITH_SECOND_HAND
  // SECONDHAND
  if (request->arg("sh") == "0")
  {
    settings->mySettings.secondHand = false;
  }
  if (request->arg("sh") == "1")
  {
    settings->mySettings.secondHand = true;
  }
  if (request->arg("sa") == "1")
  {
    settings->mySettings.secondsAll = true;
  }
  if (request->arg("sa") == "0")
  {
    settings->mySettings.secondsAll = false;
  }
#endif

#ifdef WITH_SECOND_BELL
  // SECONDBELL
  if (request->arg("sb") == "0")
  {
    settings->mySettings.secondsBell = false;
  }
  if (request->arg("sb") == "1")
  {
    settings->mySettings.secondsBell = true;
  }
#endif

  // ------------------------------------------------------------------------
  // Hauptfarbe
  if (request->arg(F("ledcol")).length())
  {
    String farbwert = request->arg("ledcol");
    farbwert.toUpperCase();
    settings->mySettings.ledcol = string_to_num(farbwert);
  }
  // Minutenfarbe
  if (request->arg(F("corcol")).length())
  {
    String farbwert = request->arg("corcol");
    farbwert.toUpperCase();
    settings->mySettings.corcol = string_to_num(farbwert);
  }
  // ------------------------------------------------------------------------
  // Farbwechsel
  if (request->arg("cc") != "")
  {
    switch (request->arg("cc").toInt())
    {
    case 0:
      settings->mySettings.colorChange = COLORCHANGE_NO;
      break;
    case 1:
      settings->mySettings.colorChange = COLORCHANGE_FIVE;
      break;
    case 2:
      settings->mySettings.colorChange = COLORCHANGE_HOUR;
      break;
    case 3:
      settings->mySettings.colorChange = COLORCHANGE_DAY;
      break;
    }
  }
  // ------------------------------------------------------------------------
  // Farbwechsel Minutenfarbe
  if (request->arg("ccc") != "")
  {
    switch (request->arg("ccc").toInt())
    {
    case 0:
      settings->mySettings.corner_colorChange = COLORCHANGE_NO;
      break;
    case 1:
      settings->mySettings.corner_colorChange = COLORCHANGE_FIVE;
      break;
    case 2:
      settings->mySettings.corner_colorChange = COLORCHANGE_HOUR;
      break;
    case 3:
      settings->mySettings.corner_colorChange = COLORCHANGE_DAY;
      break;
    case 4:
      settings->mySettings.corner_colorChange = COLORCHANGE_MAIN;
      break;
    }
    if (settings->mySettings.corner_colorChange == COLORCHANGE_MAIN)
      settings->mySettings.corcol = settings->mySettings.ledcol;
  }
  // ------------------------------------------------------------------------
  // Hintergrundfarbe Modus
  if (request->arg("bgce") == "0")
    settings->mySettings.enable_bg_color = 0;
  if (request->arg("bgce") == "1")
    settings->mySettings.enable_bg_color = 1;
  if (request->arg("bgce") == "2")
    settings->mySettings.enable_bg_color = 2;
  // Hintergrundfarbe
  if (request->arg("bgc") != "")
  {
    String farbwert = request->arg("bgc");
    farbwert.toUpperCase();
    settings->mySettings.bg_color = string_to_num(farbwert);
  }
  // ------------------------------------------------------------------------
  // Transitions
  if (request->arg("tr") != "")
  {
    switch (request->arg("tr").toInt())
    {
    case 0:
      settings->mySettings.transition = TRANSITION_NORMAL;
      break;
    case 1:
      settings->mySettings.transition = TRANSITION_FARBENMEER;
      break;
    case 2:
      settings->mySettings.transition = TRANSITION_MOVEUP;
      break;
    case 3:
      settings->mySettings.transition = TRANSITION_MOVEDOWN;
      break;
    case 4:
      settings->mySettings.transition = TRANSITION_MOVELEFT;
      break;
    case 5:
      settings->mySettings.transition = TRANSITION_MOVERIGHT;
      break;
    case 6:
      settings->mySettings.transition = TRANSITION_MOVELEFTDOWN;
      break;
    case 7:
      settings->mySettings.transition = TRANSITION_MOVERIGHTDOWN;
      break;
    case 8:
      settings->mySettings.transition = TRANSITION_MOVECENTER;
      break;
    case 9:
      settings->mySettings.transition = TRANSITION_FADE;
      break;
    case 10:
      settings->mySettings.transition = TRANSITION_MATRIX;
      break;
    case 11:
      settings->mySettings.transition = TRANSITION_SPIRALE_LINKS;
      break;
    case 12:
      settings->mySettings.transition = TRANSITION_SPIRALE_RECHTS;
      break;
    case 13:
      settings->mySettings.transition = TRANSITION_ZEILENWEISE;
      break;
    case 14:
      settings->mySettings.transition = TRANSITION_REGENBOGEN;
      break;
    case 15:
      settings->mySettings.transition = TRANSITION_MITTE_LINKSHERUM;
      break;
    case 16:
      settings->mySettings.transition = TRANSITION_QUADRATE;
      break;
    case 17:
      settings->mySettings.transition = TRANSITION_KREISE;
      break;

    case 20:
      settings->mySettings.transition = TRANSITION_ALLE_NACHEINANDER;
      break;
    case 21:
      settings->mySettings.transition = TRANSITION_RANDOM;
      break;
    }
  }
  // ------------------------------------------------------------------------
  // Nachts aus Zeit
  if (request->arg("no") != "")
  {
    settings->mySettings.nightOffTime = request->arg("no").substring(0, 2).toInt() * 3600 + request->arg("no").substring(3, 5).toInt() * 60;
    DEBUG_PRINTF("nightOffTime=%ld\n", settings->mySettings.nightOffTime);
    //settings->mySettings.nightOffTime -= mt->mytm.tm_tzoffset * 60;
    //if (mt->mytm.tm_isdst)
    //  settings->mySettings.nightOffTime += 3600;
    //DEBUG_PRINTF("nightOffTime=%lld\n", settings->mySettings.nightOffTime);
    DEBUG_PRINTF("tm_isdst=%d\n", mt->mytm.tm_isdst);
  }
  // ------------------------------------------------------------------------
  // Tag ein Zeit
  if (request->arg("do") != "")
  {
    settings->mySettings.dayOnTime = request->arg("do").substring(0, 2).toInt() * 3600 + request->arg("do").substring(3, 5).toInt() * 60;
    DEBUG_PRINTF("dayOnTime=%ld\n", settings->mySettings.dayOnTime);
    //settings->mySettings.dayOnTime -= mt->mytm.tm_tzoffset * 60;
    //if (mt->mytm.tm_isdst)
    //  settings->mySettings.dayOnTime += 3600;
  }
  // ------------------------------------------------------------------------
  // Zeige "ES IST"
  if (request->arg("ii") == "0")
    settings->mySettings.itIs = false;
  if (request->arg("ii") == "1")
    settings->mySettings.itIs = true;
  
  // ------------------------------------------------------------------------
  // Dialekt
  if (request->arg("di") != "")
  {
    switch (request->arg("di").toInt())
    {
    case 0:
      settings->mySettings.language = FC_DE_DE;
      break;
    case 1:
      settings->mySettings.language = FC_DE_SW;
      break;
    case 2:
      settings->mySettings.language = FC_DE_BA;
      break;
    case 3:
      settings->mySettings.language = FC_DE_SA;
      break;
    }
  }

  // ------------------------------------------------------------------------
  // NTP-Server
  if (request->arg(F("ntphost")).length() && request->arg(F("ntphost")) != settings->mySettings.ntphost)
  {
    request->arg(F("ntphost")).toCharArray(settings->mySettings.ntphost, request->arg(F("ntphost")).length() + 1);
    // strcpy(ntpServer, settings->mySettings.ntphost);
    // aktntptime = true;
  }
  // ------------------------------------------------------------------------
  // Timezone
  if (request->arg(F("timezone")).length() && request->arg(F("timezone")) != settings->mySettings.timezone)
  {
    request->arg(F("timezone")).toCharArray(settings->mySettings.timezone, request->arg(F("timezone")).length() + 1);
    mt->mytm.tm_timezone = settings->mySettings.timezone;
#if defined(ESP8266)
    configTime(mt->mytm.tm_timezone.c_str(), mt->mytm.tm_ntpserver.c_str());
#else
    configTzTime(mt->mytm.tm_timezone.c_str(), mt->mytm.tm_ntpserver.c_str());
#endif
  }
  // ------------------------------------------------------------------------
  // Systemname
  if (request->arg(F("sysname")).length() && request->arg(F("sysname")) != settings->mySettings.systemname)
  {
    request->arg(F("sysname")).toCharArray(settings->mySettings.systemname, request->arg(F("sysname")).length() + 1);
  }
  // ------------------------------------------------------------------------
  // openweatherapikey
  if (request->arg(F("owkey")).length() && request->arg(F("owkey")) != settings->mySettings.openweatherapikey)
  {
    request->arg(F("owkey")).toCharArray(settings->mySettings.openweatherapikey, request->arg(F("owkey")).length() + 1);
  }
  // openweatherlocation
  if (request->arg(F("owloc")).length() && request->arg(F("owloc")) != settings->mySettings.openweatherlocation)
  {
    request->arg(F("owloc")).toCharArray(settings->mySettings.openweatherlocation, request->arg(F("owloc")).length() + 1);
  }
  // hoehe_ueber_0
  if (request->arg(F("hoehe")).length() && request->arg(F("hoehe")).toInt() != settings->mySettings.standort_hoehe)
  {
    settings->mySettings.standort_hoehe = request->arg(F("hoehe")).toInt();
  }
  // Standort
  if (request->arg(F("latitude")).length())
  {
    settings->mySettings.latitude = request->arg(F("latitude")).toDouble();
  }
  if (request->arg(F("longitude")).length())
  {
    settings->mySettings.longitude = request->arg(F("longitude")).toDouble();
  }

  // ------------------------------------------------------------------------
  // Setze /Datum/Uhrzeit
  if (request->arg("st").length())
  {
    DEBUG_PRINTLN(request->arg("st"));
    // setTime(request->arg("st").substring(11, 13).toInt(), request->arg("st").substring(14, 16).toInt(), 0, request->arg("st").substring(8, 10).toInt(), request->arg("st").substring(5, 7).toInt(), request->arg("st").substring(0, 4).toInt());
#ifdef RTC_BACKUP
    RTC.set(now());
#endif
  }
  // ------------------------------------------------------------------------
  // Highscores reset
  if (request->arg(F("hsres")) == "1")
  {
    //highscore[SNAKE] = 0;
    //highscore[TETRIS] = 0;
    //highscore[BRICKS] = 0;
    //highscore[VIERGEWINNT] = 0;

//mz    settings->mySettings.highscore[SNAKE] = highscore[SNAKE];
//mz    settings->mySettings.highscore[TETRIS] = highscore[TETRIS];
//mz    settings->mySettings.highscore[BRICKS] = highscore[BRICKS];
//mz    settings->mySettings.highscore[VIERGEWINNT] = highscore[VIERGEWINNT];
  }
// ------------------------------------------------------------------------
// MQTT
#if defined(WITH_MQTT)
  if (request->arg(F("mqsv")).length())
    request->arg(F("mqsv")).toCharArray(settings->mySettings.mqtt_server, request->arg(F("mqsv")).length() + 1);
  else
    settings->mySettings.mqtt_server[0] = '\0';

  if (request->arg(F("mqpo")).length())
    settings->mySettings.mqtt_port = request->arg(F("mqpo")).toDouble();
  else
    settings->mySettings.mqtt_port = 0;

  if (request->arg(F("mqbe")).length())
    request->arg(F("mqbe")).toCharArray(settings->mySettings.mqtt_user, request->arg(F("mqbe")).length() + 1);
  else
    settings->mySettings.mqtt_user[0] = '\0';

  if (request->arg(F("mqpw")).length())
    request->arg(F("mqpw")).toCharArray(settings->mySettings.mqtt_password, request->arg(F("mqpw")).length() + 1);
  else
    settings->mySettings.mqtt_password[0] = '\0';

#endif

  if (request->args() < 10)
  {
    request->send(200, TEXT_PLAIN, F("Parameter gesetzt!"));
  }
  else
  {
    // Sichern der Einstellungen
    settings->saveToNVS();

    renderer->language = settings->mySettings.language;
    renderer->itIs = settings->mySettings.itIs;
    ledDriver->setDegreeOffset((mt->mytm.tm_hour*15+mt->mytm.tm_min*6)%360);
    ledDriver->corcol=settings->mySettings.corcol;
    renderer->clearScreenBuffer(matrix);
    renderer->setTime(mt->mytm.tm_hour, mt->mytm.tm_min, matrix);
    renderer->setCorners(mt->mytm.tm_min, matrix);
    if (!settings->mySettings.itIs && ((mt->mytm.tm_min / 5) % 6))
      renderer->clearEntryWords(matrix);
    ledDriver->writeScreenBufferFade(matrix, settings->mySettings.ledcol);

    nightOffTime  =  (settings->mySettings.nightOffTime / 3600) * 60;
    nightOffTime  += (settings->mySettings.nightOffTime % 3600) / 60;
    dayOnTime     =  (settings->mySettings.dayOnTime / 3600) * 60;
    dayOnTime     += (settings->mySettings.dayOnTime % 3600) / 60;

    callRoot(request);
  }
  //ldr_update = true;
  //screenBufferNeedsUpdate = true;
}
//------------------------------------------------------------------------
// Ende handleCommitSettings()
//------------------------------------------------------------------------
