//******************************************************************************
// OpenWeather.h - Get weather data from OpenWeather
//******************************************************************************

#ifndef OPENWEATHER_H
#define OPENWEATHER_H

#include <ArduinoJson.h>
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#else
  #include <WiFi.h>
  /*
  #include <WiFiClient.h>
  #include <WebServer.h>
  #include <ESPmDNS.h>
  */
#endif
#include "Configuration.h"
#include "MyTime.h"

#ifdef SunRiseLib
#include <SunRise.h>
#endif

#if defined(LILYGO_T_HMI)
#include "MyTFT.h"
#include "MyFileAccess.h"
#endif

class OpenWeather
{
public:
    static OpenWeather* getInstance(); 

    String owfehler = "";
    String description = "";
    String weathericon1 = "";
    String weathericon2 = "";
    uint16_t weatherid1 = 0;
    uint16_t weatherid2 = 0;
    double temperature = 0;
    double windspeed = 0;
    uint8_t humidity = 0;
    uint8_t clouds = 0;
    uint16_t pressure = 0;
    time_t sunrise;
    time_t sunset;
    int moonphase;
    uint8_t web_moonphase;
    // Sunrise
    uint8_t sunriseHour = 0;
    uint8_t sunriseMinute = 0;
    uint8_t sunriseSecond = 0;
    boolean ani_sunrise_done = false;


    uint8_t sunsetHour = 0;
    uint8_t sunsetMinute = 0;
    uint8_t sunsetSecond = 0;
    boolean ani_sunset_done = false;
    
#ifdef SunRiseLib
    time_t sunRiseTime;
    time_t sunSetTime;
    SunRise sr;
#endif

    int errorCounterOutdoorWeather;
    uint16_t retcodeOutdoorWeather;
    time_t LastOutdoorWeatherTime;
    uint16_t WetterSequenz;
    uint8_t WetterAnsageSchritt;

    uint16_t getOutdoorConditions(String location, String apiKey);
    String getWeatherIcon(String weathericonnummer);
    int getMoonphase(int y, int m, int d);
    void sunriseset(void);
    String  urlEncode(String str);

private:
  OpenWeather();
  static OpenWeather *instance;
  
};

#endif
