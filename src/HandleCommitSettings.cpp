#include "Arduino.h"
#include "Configuration.h"
#include "Languages.h"
#include "Settings.h"
#include "Renderer.h"
#include "LedDriver_FastLED.h"
#include "MyTime.h"
#include "Html_content.h"
#include "Helper.h"
#include "Game.h"

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
static Game *game = Game::getInstance();


void handleCommitSettings(AsyncWebServerRequest *request) 
{
  DEBUG_PRINTF("commit: Core=%d\n", xPortGetCoreID());
#if defined(myDEBUG)
  Serial.println(F("Commit settings pressed."));
  for (int i = 0; i < request->args(); i++)
    Serial.printf("name=%s, value=%s\n", request->argName(i).c_str(), request->arg(i).c_str());
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
    //Serial.printf("ab: %d, %d, %d\n",settings->mySettings.brightness, map(settings->mySettings.brightness, 10, 100, MIN_BRIGHTNESS, MAX_BRIGHTNESS), ledDriver->getBrightness());
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
    game->highscore[SNAKE] = 0;
    game->highscore[TETRIS] = 0;
    game->highscore[BRICKS] = 0;
    game->highscore[VIERGEWINNT] = 0;

    settings->mySettings.highscore[SNAKE] = game->highscore[SNAKE];
    settings->mySettings.highscore[TETRIS] = game->highscore[TETRIS];
    settings->mySettings.highscore[BRICKS] = game->highscore[BRICKS];
    settings->mySettings.highscore[VIERGEWINNT] = game->highscore[VIERGEWINNT];
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
}
//------------------------------------------------------------------------
// Ende handleCommitSettings()
//------------------------------------------------------------------------
