//******************************************************************************
// Settings.cpp
//******************************************************************************

#include "Settings.h"
#include "MyFileAccess.h"

// File Access
static MyFileAccess *fa = MyFileAccess::getInstance();


Settings* Settings::instance = 0;

Settings *Settings::getInstance() {
  if (!instance)
  {
      instance = new Settings();
  }
  return instance;
}

Settings::Settings()
{
}


void Settings::init()
{
    loadFromNVS();
}

// Set all defaults
void Settings::resetToDefault()
{
    mySettings.magicNumber = SETTINGS_MAGIC_NUMBER;
    mySettings.version = SETTINGS_VERSION;
    strcpy (mySettings.systemname,WIFIHOSTNAME);
    mySettings.useAbc = true;
    mySettings.brightness = 30;
    //mySettings.color = WHITE;
    mySettings.ledcol = 0x0;
    mySettings.colorChange = COLORCHANGE_NO;
    mySettings.transition = TRANSITION_ALLE_NACHEINANDER;
    mySettings.modeChange = true;
    mySettings.itIs = true;
    mySettings.alarm1 = false;
    mySettings.alarm1Time = 0;
    mySettings.alarm1Weekdays = 0b11111110;
    mySettings.alarm2 = false;
    mySettings.alarm2Time = 0;
    mySettings.alarm2Weekdays = 0b11111110;
    mySettings.nightOffTime = 1;
    mySettings.dayOnTime = 18000;
    mySettings.hourBeep = true;
    mySettings.volume = 100;
    mySettings.sprecher = true;          //vicki
    mySettings.randomsound = false;
    mySettings.weekdaysoundfile[0] = 8; //Sonntag
    mySettings.weekdaysoundfile[1] = 0;
    mySettings.weekdaysoundfile[2] = 1;
    mySettings.weekdaysoundfile[3] = 2;
    mySettings.weekdaysoundfile[4] = 3;
    mySettings.weekdaysoundfile[5] = 4;
    mySettings.weekdaysoundfile[6] = 5;  //Samstag
    mySettings.weekendlater = true;
    mySettings.vickihans24h = true;
    strcpy (mySettings.ntphost,NTP_SERVER);
    //mySettings.corner_color = ORANGE;
    mySettings.corcol = 0x000000;
    mySettings.corner_colorChange = COLORCHANGE_NO;
    mySettings.bg_color = 0x000000;
    mySettings.enable_bg_color = 0;
    strcpy (mySettings.openweatherapikey,DEFAULTAPIKEY);
    strcpy (mySettings.openweatherlocation,LOCATION);
    mySettings.highscore[0] = 0;
    mySettings.highscore[1] = 40;
    mySettings.highscore[2] = 163;
    mySettings.highscore[3] = 176;
    mySettings.highscore[4] = 0;
    mySettings.highscore[5] = 0;
    mySettings.highscore[6] = 0;
    mySettings.highscore[7] = 0;
    mySettings.highscore[8] = 0;
    mySettings.standort_hoehe = HOEHE_UEBER_0;
    mySettings.auto_mode_change = AUTO_MODECHANGE_TIME / 60;
    mySettings.ani_sunrise = true;
    mySettings.ani_sunset = true;
    mySettings.latitude = LATITUDE;
    mySettings.longitude = LONGITUDE;
    mySettings.singlegong = false;
    mySettings.dummybool = false;
    mySettings.dummyunit16 = 0;
    mySettings.ssid[0]='\0';
    mySettings.passwd[0]='\0';
    if(fa->readSettings()) {
        String val = fa->getSetting("wifi_pass");
        if(val.length()) {
            strcpy(mySettings.passwd,val.c_str());
        }
        val = fa->getSetting("wifi_ssid");
        if(val.length()) {
            strcpy(mySettings.ssid,val.c_str());
        }
    }
    strcpy (mySettings.timezone,"CET-1CEST,M3.5.0,M10.5.0/3");
    mySettings.mqtt_server[0] = '\0';
    mySettings.mqtt_port = 0;
    mySettings.mqtt_user[0] = '\0';
    mySettings.mqtt_password[0] = '\0';
    mySettings.logging = false;
    mySettings.secondsAll = false;
#if defined(FRONTCOVER_DE)
    mySettings.language = FC_DE;
#endif
#if defined(FRONTCOVER_EN)
    mySettings.language = FC_EN;
#endif
#if defined(FRONTCOVER_FR)
    mySettings.language = FC_FR;
#endif
#if defined(FRONTCOVER_IT)
    mySettings.language = FC_IT;
#endif
#if defined(FRONTCOVER_ES)
    mySettings.language = FC_ES;
#endif
#if defined(FRONTCOVER_AE_DE)
    mySettings.language = FC_AE_DE;
#endif
#if defined(FRONTCOVER_AE_EN)
    mySettings.language = FC_AE_EN;
#endif

    saveToNVS();
}

#if defined(ESP8266)
// Load settings from EEPROM
void Settings::loadFromNVS()
{
    EEPROM.begin(sizeof(mySettings)+8);
    EEPROM.get(0, mySettings);
    if ((mySettings.magicNumber != SETTINGS_MAGIC_NUMBER) || (mySettings.version != SETTINGS_VERSION))
        resetToDefault();
    EEPROM.end();
    if(mySettings.language < FC_EN || mySettings.language > FC_BINARY)
        mySettings.language = FC_DE_DE;
}

// Write settings to EEPROM
void Settings::saveToNVS()
{
#ifdef DEBUG
    Serial.println("Settings saved to EEPROM.");
#endif
    EEPROM.begin(sizeof(mySettings)+8);
    EEPROM.put(0, mySettings);
    //EEPROM.commit();
    EEPROM.end();
    
}
#else
// Load settings from NVS
void Settings::loadFromNVS()
{
#ifdef DEBUG
   Serial.println("loadFromNVS");
#endif
   bool ret = preferences.begin("wordclock", false);
   size_t lng = preferences.getBytes("all", (void *)&mySettings, sizeof(mySettings)+8);
#ifdef DEBUG
   Serial.printf("loadFromNVS: ret=%d, lng=%d, magicNumber=%d, versiom=%d\n",ret, lng, mySettings.magicNumber, mySettings.version);
#endif
    if ((mySettings.magicNumber != SETTINGS_MAGIC_NUMBER) || (mySettings.version != SETTINGS_VERSION)) {
        preferences.end();
        resetToDefault();
    } else
        preferences.end();
}

// Write settings to NVS
void Settings::saveToNVS()
{
#ifdef DEBUG
    Serial.println("Settings saved to NVS.");
#endif
    bool ret = preferences.begin("wordclock", false);
    size_t lng = preferences.putBytes("all",(const void*)&mySettings, sizeof(mySettings)+8);
    preferences.end();
#ifdef DEBUG
    Serial.printf("saveToNVS: ret=%d, lng=%d\n", ret, lng);
#endif
}
#endif
