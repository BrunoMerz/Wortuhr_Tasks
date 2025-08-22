//******************************************************************************
// OpenWeather.cpp - Get weather data from OpenWeather
//******************************************************************************

#include "OpenWeather.h"
#include "Languages.h"
#include "MyTime.h"
#include "Settings.h"

//#define DEBUG_OW

OpenWeather* OpenWeather::instance = 0;

static MyTime *mt = MyTime::getInstance();
static Settings *settings = Settings::getInstance();

OpenWeather *OpenWeather::getInstance() {
  if (!instance)
  {
      instance = new OpenWeather();
  }
  return instance;
}

OpenWeather::OpenWeather()
{
  errorCounterOutdoorWeather = 0;
  LastOutdoorWeatherTime = 0;
  WetterSequenz = 1;
}


String OpenWeather::urlEncode(String str) {
  String encodedString = "";
  char c;
  char code0;
  char code1;
  char code2;
  
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (c == ' ') {
      encodedString += '+';
    } else if (isalnum(c)) {
      encodedString += c;
    } else {
      code1 = (c & 0xF) + '0';
      if ((c & 0xF) > 9) {
        code1 = (c & 0xF) - 10 + 'A';
      }
      code0 = ((c >> 4) & 0xF) + '0';
      if (((c >> 4) & 0xF) > 9) {
        code0 = ((c >> 4) & 0xF) - 10 + 'A';
      }
      encodedString += '%';
      encodedString += code0;
      encodedString += code1;
    }
    yield();
  }
  return encodedString;
}

uint16_t OpenWeather::getOutdoorConditions(String location, String apiKey)
{
  JsonDocument weatherArray;
  String response;
  String sdescription;
  String dout;
  int retcode;
  int timezoneshift;
  retcode = 0;
  owfehler = LANG_OW_OK;
  if ( apiKey.length() > 25 ) 
  {
    if(!(mt->mytm.tm_loc > LastOutdoorWeatherTime + OPENWEATHER_PERIODE))
      return true;

    WiFiClient client;
   
#ifdef DEBUG_OW
  Serial.println("Call Open Weather" );
#endif    
    if ((retcode=client.connect("api.openweathermap.org", 80)))
    {
        retcode = 0;
        String url = F("/data/2.5/weather?q=");
        url += urlEncode(location);
        url += F("&lang=");
        url += F(LANG_OPENWAETHER_LANG);
        url += F("&units=metric&appid=");
        url += String(apiKey);
        
#ifdef DEBUG_OW
  Serial.printf("Open Weather url: %s\n",url.c_str() );
#endif       
        client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: api.openweathermap.org" + "\r\n" + "Connection: close\r\n\r\n");
        unsigned long timeout = millis();
        while (client.available() == 0)
        {
            if (millis() - timeout > 5000)
            {
                client.stop();
#ifdef DEBUG_OW
                Serial.println("Open Weather timeout after 5 Sec");
#endif  
                return 1;
            }
        }
        client.setTimeout(2000);
        int i;
        char c;
        int n = 0;
        while (client.available()) {
#if defined(ESP8266)
            response = client.readStringUntil('\r');
            response.trim();
#else
            if((i=client.read()) != -1 ) {
              c = (char)i;
#ifdef DEBUG_OW
              Serial.printf("client read %c, %d\n",c,n);
#endif
              if(n)
              {
                if(c=='\r' || c=='\n' )
                  break;
                response += c;
              } else {
                if(c=='{') {
                  n = 1;
                  response += c;
                }
              }
            }
#endif
        }
#ifdef DEBUG_OW
  Serial.printf("Open Weather Response: %s\n",response.c_str() );
#endif
        DeserializationError error = deserializeJson(weatherArray, response);
        if (error) { Serial.println(F("Load JSON_ARRAY: Parsing the weather failed")); return false;}
#ifdef DEBUG_OW
#if defined(ESP8266)
        Serial.printf("OpenWeather: %i Codezeile: %u\n", ESP.getMaxFreeBlockSize(),  __LINE__);
#endif
#endif
        description = "";
        weathericon2 = "";   
        weathericon1 = "";
        weatherid1 = 0;
        weatherid2 = 0;
        JsonArray ja = weatherArray[F("weather")];
        for ( uint8_t z = 0; z < ja.size(); z++)
        {
          sdescription = (const char*)weatherArray["weather"][z]["description"];
          description += sdescription;
          if ( z+1 < ja.size()) description += ", ";
          if ( z == 0 ) 
          {
            weathericon1 = (const char*)weatherArray["weather"][z]["icon"];
            weatherid1   = (int)weatherArray["weather"][z]["id"];
          }
          if ( z == 1 ) 
          {
            weathericon2 = (const char*)weatherArray["weather"][z]["icon"];
            weatherid2   = (int)weatherArray["weather"][z]["id"];
          }
#ifdef DEBUG_OW
  Serial.printf("Open Weather description: %i ",z );
  Serial.print( weatherArray["weather"][z]["description"].as<String>());
  Serial.print (" ID: ");
  Serial.print ( weatherArray["weather"][z]["id"].as<String>());
  Serial.print (" Icon: ");
  Serial.println( weatherArray["weather"][z]["icon"].as<String>());
#endif    
        }
        description.trim();
#ifdef DEBUG_OW
  Serial.print("Open Weather description:");
  Serial.println(description);
#endif        
        if(! isnan((double)weatherArray["main"]["temp"]) ) 
        {
          temperature = (double)weatherArray["main"]["temp"];
#ifdef DEBUG_OW
  Serial.print("Open Weather temperature:");
  Serial.println(temperature);
#endif  
        }
        else
        {
          retcode = retcode + 2;
          owfehler = LANG_OW_ERRTEMP;
        }
        if(! isnan((int)weatherArray["main"]["humidity"]))
        {
          humidity = (int)weatherArray["main"]["humidity"];
#ifdef DEBUG_OW
  Serial.print("Open Weather humidity:");
  Serial.println(humidity);
#endif  
        }
        else
        {
          retcode = retcode + 4;
          owfehler = LANG_OW_ERRHUM;
        }
        if(! isnan((int)(int)weatherArray["main"]["pressure"]))
        {
          pressure = (int)weatherArray["main"]["pressure"];
#ifdef DEBUG_OW
  Serial.print("Open Weather pressure:");
  Serial.println(pressure);
#endif  
        }
        else
        {
          retcode = retcode + 8;
          owfehler = LANG_OW_ERRPRES;
        }
//Clouds
        if(! isnan((int)(int)weatherArray["clouds"]["all"]))
        {
          clouds = (int)weatherArray["clouds"]["all"];
        }
// Sonnenaufgang
        if(! isnan((int)(int)weatherArray["sys"]["sunrise"]))
        {
          sunrise = (int)weatherArray["sys"]["sunrise"];
#ifdef DEBUG_OW
  Serial.print("Open Weather sunrise:");
  Serial.println(sunrise);
  dout = "Sonnenaufgang: " + String(mt->hour(sunrise)) + ":";
  if (mt->minute(sunrise) < 10) dout += "0";
  dout += String(mt->minute(sunrise));
  dout += ":";
  if (mt->second(sunrise) < 10) dout += "0";
  dout += String(mt->second(sunrise));
  Serial.println(dout);
#endif  
        }
        else
        {
          retcode = retcode + 16;
          owfehler = LANG_OW_ERRSUNRISE;
        }
// Sonnenuntergang
        if(! isnan((int)(int)weatherArray["sys"]["sunset"]))
        {
          sunset = (int)weatherArray["sys"]["sunset"];
#ifdef DEBUG_OW
  Serial.print("Open Weather sunset:");
  Serial.println(sunset);
  dout = "Sonnenuntergang: " + String(mt->hour(sunset)) + ":";
  if (mt->minute(sunset) < 10) dout += "0";
  dout += String(mt->minute(sunset));
  dout += ":";
  if (mt->second(sunset) < 10) dout += "0";
  dout += String(mt->second(sunset));
  Serial.println(dout);
#endif  
        }
        else
        {
          retcode = retcode + 32;
          owfehler = LANG_OW_ERRSUNSET;
        }
        if(! isnan((double)weatherArray["wind"]["speed"]) ) 
        {
          windspeed = (double)weatherArray["wind"]["speed"];
#ifdef DEBUG_OW
  Serial.print("Wind Speed:");
  Serial.println(windspeed);
#endif  
        }
      client.stop();
    }
    else
    {
#ifdef DEBUG_OW
      Serial.printf("Keine Connect zu Open Weather!! error=%d\n",retcode);
#endif
      retcode = 64;
      owfehler = LANG_OW_ERRCONNECT;
    }
  }
  else
  {
    retcode = 1;
    owfehler = LANG_OW_ERRAPI;
    description = "";
    weathericon1 = "";
    weathericon2 = "";
    weatherid1 = 0;
    weatherid2 = 0;
    temperature = 0;
    windspeed = 0;
    humidity = 0;
    clouds = 0;
    pressure = 0;
  }
#ifdef DEBUG_OW
  Serial.printf("Open Weather retcode=%d\n",retcode);
#endif
  LastOutdoorWeatherTime = mt->mytm.tm_loc;
  retcodeOutdoorWeather = retcode;
  return retcode;
}


String OpenWeather::getWeatherIcon(String weathericonnummer)
{
  String tagnacht;
  String wnummer;
  uint8_t winummer;
  String icon = " ";
  //  tagnacht = weathericonnummer.substring(weathericonnummer.length()-1,weathericonnummer.length());
  wnummer = weathericonnummer.substring(0, weathericonnummer.length() - 1);
  winummer = wnummer.toInt();

  if (mt->mytm.tm_loc > sunrise && mt->mytm.tm_loc < sunset)
    tagnacht = "d";
  else
    tagnacht = "n";

  if (winummer == 1 || winummer == 2 || winummer == 10)
  {
    if (tagnacht == "d")
      icon = wnummer + tagnacht;
    else
    {
      if (moonphase > 9 && moonphase < 13)
        icon = wnummer + tagnacht + "a";
      else
        icon = wnummer + tagnacht + "b";
    }
    if (winummer == 10 && clouds > 80)
      icon = "09";
  }
  else
  {
    icon = wnummer;
  }
#ifdef DEBUG_OW
  Serial.print(F("Wetter Icon Nummer: "));
  Serial.print(wnummer);
  Serial.print(F(" Wolken: "));
  Serial.print(clouds);
  Serial.print(F("% Tag/Nacht: "));
  Serial.print(tagnacht);
  Serial.print(F(" -> Icon: "));
  Serial.println(icon);
#endif
  return icon;
}



int OpenWeather::getMoonphase(int y, int m, int d)
{

  int b;
  int c;
  int e;
  double jd;
  if (m < 3)
  {
    y--;
    m += 12;
  }
  ++m;
  c = 365.25 * y;
  e = 30.6 * m;
  jd = c + e + d + 0.8 - 694039.09; // jd is total days elapsed
  jd /= 29.5306;                // divide by the moon cycle (29.53 days)
  b = jd;                     // int(jd) -> b, take integer part of jd
  jd -= b;                    // subtract integer part to leave fractional part of original jd
  b = jd * 22 + 0.2 ;                // scale fraction from 0-22
  web_moonphase = jd * 8 + 0.5 ; // scale fraction from 0-9 and round by adding 0.5
  web_moonphase = web_moonphase & 7;
  if (b >= 22 ) b = 0;
  return b;
}


//******************************************************************************
// Sunrise/Sunset
//******************************************************************************
void OpenWeather::sunriseset(void)
{
  sunriseHour = 0;
  sunriseMinute = 0;
  sunriseSecond = 0;
  sunsetHour = 0;
  sunsetMinute = 0;
  sunsetSecond = 0;
#if defined(DEBUG_OW)
  Serial.printf("sunriseset sunRiseTime=%2d\n", mt->hour(sunRiseTime));
#endif
#ifdef APIKEY
  if (strlen(settings->mySettings.openweatherapikey) > 25)
  {
    if (mt->hour(sunrise) > 0)
    {
      sunriseHour = mt->hour(sunrise);
      sunriseMinute = mt->minute(sunrise);
      sunriseSecond = mt->second(sunrise);
      if (sunriseSecond > 40)
        sunriseSecond = 40;
      if (sunriseSecond < 10)
        sunriseSecond = 10;
    }
    if (mt->hour(sunset) > 0)
    {
      sunsetHour = mt->hour(sunset);
      sunsetMinute = mt->minute(sunset);
      sunsetSecond = mt->second(sunset);
      if (sunsetSecond > 40)
        sunsetSecond = 40;
      if (sunsetSecond < 10)
        sunsetSecond = 10;
    }
  }
  else
  {
#endif // APIKEY
#ifdef SunRiseLib
    if (mt->hour(sunRiseTime) > 0)
    {
      sunriseHour = mt->hour(sunRiseTime);
      sunriseMinute = mt->minute(sunRiseTime);
      sunriseSecond = mt->second(sunRiseTime);
      if (sunriseSecond > 40)
        sunriseSecond = 40;
      if (sunriseSecond < 10)
        sunriseSecond = 10;
    }
    if (mt->hour(sunSetTime) > 0)
    {
      sunsetHour = mt->hour(sunSetTime);
      sunsetMinute = mt->minute(sunSetTime);
      sunsetSecond = mt->second(sunSetTime);
      if (sunsetSecond > 40)
        sunsetSecond = 40;
      if (sunsetSecond < 10)
        sunsetSecond = 10;
    }
#endif // SunRiseLib
#ifdef APIKEY
  }
#endif

}
