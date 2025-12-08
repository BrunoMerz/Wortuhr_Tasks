#include "Arduino.h"
#include "Configuration.h"
#include "Languages.h"
#include "Settings.h"
#include "Renderer.h"
#include "MyTime.h"
#include "MyWifi.h"
#include "WebHandler.h"
#include "Html_content.h"
#include "OpenWeather.h"
#include "TaskStructs.h"
#include "BuildDateTime.h"

#if defined(SYSLOGSERVER_SERVER)
#include "Syslog.h"
extern Syslog syslog;
#endif

//#define myDEBUG
#include "MyDebug.h"

#include "ESPAsyncWebServer.h"
#include <LittleFS.h>

extern s_taskParams taskParams;

static Settings *settings = Settings::getInstance();
static MyTime *mt = MyTime::getInstance();
static MyWifi *myWifi = MyWifi::getInstance();
static AsyncWebServer *webServer = myWifi->getServer();
static OpenWeather *outdoorWeather = OpenWeather::getInstance();
static Renderer *renderer = Renderer::getInstance();

#if defined(SENSOR_BME280)
#include "MyBME.h"
static MyBME *myBME = MyBME::getInstance();
#endif

#ifdef APIKEY
String getWeatherIcon(String weathericonnummer)
{
  String tagnacht;
  String wnummer;
  uint8_t winummer;
  String icon = " ";
  //  tagnacht = weathericonnummer.substring(weathericonnummer.length()-1,weathericonnummer.length());
  wnummer = weathericonnummer.substring(0, weathericonnummer.length() - 1);
  winummer = wnummer.toInt();

  if ( mt->localTm() > outdoorWeather->sunrise && mt->localTm() < outdoorWeather->sunset ) tagnacht = "d";
  else tagnacht = "n";

  if ( winummer == 1 || winummer == 2 || winummer == 10 )
  {
    if ( tagnacht == "d" ) icon = wnummer + tagnacht;
    else
    {
      if ( outdoorWeather->moonphase > 9 && outdoorWeather->moonphase < 13 ) icon = wnummer + tagnacht + "a";
      else icon = wnummer + tagnacht + "b";
    }
    if ( winummer == 10 && outdoorWeather->clouds > 80 ) icon = "09";
  }
  else
  {
    icon = wnummer;
  }
#ifdef DEBUG_OW
  Serial.print(F( "Wetter Icon Nummer: "));
  Serial.print(wnummer);
  Serial.print(F( " Wolken: "));
  Serial.print(outdoorWeather.clouds);
  Serial.print(F( "% Tag/Nacht: "));
  Serial.print(tagnacht);
  Serial.print(F( " -> Icon: "));
  Serial.println(icon);
#endif
  return icon;
}

#endif

void handleRoot(AsyncWebServerRequest *request, Mode mode, uint8_t moonphase, uint8_t web_moonphase, time_t upTime, uint8_t sunriseMinute, uint8_t sunriseHour, uint8_t sunsetMinute, uint8_t sunsetHour)
{
    AsyncResponseStream *response = request->beginResponseStream(TEXT_HTML);
    
#if defined(SYSLOGSERVER_SERVER)
    syslog.log(LOG_INFO, "handleRoot: start");
#endif
    response->print(F("<!doctype html>"));
    response->print(F("<html><head>"));
    response->print(F("<title>"));
    response->print(String(settings->mySettings.systemname));
    response->print(F("</title>"));
    response->print(F("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
                  "<meta http-equiv=\"Cache-Control\" content=\"no-cache, no-store, must-revalidate\"\n>"
                  "<meta http-equiv=\"refresh\" content=\"60\" charset=\"UTF-8\">\n"
                  "<link rel=\"icon\" type=\"image/png\" sizes=\"192x192\"  href=\"/web/android-icon-192x192.png\">"
                  "<link rel=\"icon\" type=\"image/png\" sizes=\"32x32\" href=\"/web/favicon-32x32.png\">"
                  "<link rel=\"icon\" type=\"image/png\" sizes=\"96x96\" href=\"/web/favicon-96x96.png\">"
                  "<link rel=\"icon\" type=\"image/png\" sizes=\"16x16\" href=\"/web/favicon-16x16.png\">"
                  "<link rel=\"shortcut icon\" type=\"image/x-icon\" href=\"favicon.ico\">"
                  "<link rel=\"stylesheet\" href=\"/web/main.css\" >"
                  "<script src=\"/web/jquery-3.1.1.min.js\"></script>\n"
                  "</head>"
                  "<body>"));
    response->print(F("<h1>"));
    response->print(String(settings->mySettings.systemname));
    response->print(F("</h1>\n"));
    response->print(F("<h2>"));
    response->print(renderer->WEB_Uhrtext);
    response->print(F("</h2>\n"));
  #ifdef DEDICATION
    response->print(F(DEDICATION));
    response->print(F("<br><br>"));
  #endif
    if (mode == MODE_BLANK) response->print(F("<button title=\"LEDs " LANG_ON "\" onclick=\"window.location.href='/handleButtonOnOff'\">&#9899;</button>\n"));
    else response->print(F("<button title=\"LEDs " LANG_OFF "\" onclick=\"window.location.href='/handleButtonOnOff'\">&#9898;</button>\n"));
    response->print(F("<button title=\"" LANG_SETTINGS "\" onclick=\"window.location.href='/handleButtonSettings'\">&#128295;</button>\n"));
    response->print(F("<button title=\"" LANG_ANIMATIONS "\" onclick=\"window.location.href='/animationmenue'\">&#127916;</button>\n"));
    response->print(F("<button title=\"" LANG_EVENTS "\" onclick=\"window.location.href='/web/events.html'\">&#128197;</button>\n"
                  "<button title=\"" LANG_MESSAGE "\" onclick=\"window.location.href='/web/nachricht.html'\">&#128172;</button>\n"
                  "<button title=\"" LANG_GAMES "\" onclick=\"window.location.href='/web/Gamemenue.html'\">&#127922;</button>\n"
                  "<button title=\"" LANG_MODEBUTTON "\" id=\"button_mode\" type=\"button\" \">&#128512;</button>\n"
                  "<button title=\"" LANG_TIMEBUTTON "\" id=\"button_zeit\" type=\"button\" \">&#128344;</button>\n"));

    response->print(F("<hr>\n"));

    // Abschnitt Mondphase
  #ifdef SHOW_MODE_MOONPHASE

    response->print(F("<table><tr>"));
    response->print(F("<td style=\"width:30%\">"));
    response->print(F("<img id=\"Sonnen\" title=\"" LANG_SUNRISE "\" src=\"/sunrise\" alt=\"Sonnenaufgang\">"));
    response->print(F("</td>\n"));

    response->print(F("<td style=\"width:30%\">"));
    response->print(F("<img id=\"Sonnen\" title=\"" LANG_SUNSET "\" src=\"/sunset\" alt=\"Sonnenuntergang\">"));
    response->print(F("</td>\n"));

    response->print(F("<td style=\"padding-top: 17px;width:40%;cursor:pointer\">"));
    response->print(F("<span id=\"Mond\" title=\"" LANG_MOONPHASE "\" onclick=\"moonphase()\">"));
    switch (web_moonphase)
    {
      case 0: response->print(F("&#127761;")); //Neumond
        break;
      case 1: response->print(F("&#127762;"));
        break;
      case 2: response->print(F("&#127763;"));
        break;
      case 3: response->print(F("&#127764;"));
        break;
      case 4: response->print(F("&#127765;")); //Vollmond
        break;
      case 5: response->print(F("&#127766;"));
        break;
      case 6: response->print(F("&#127767;"));
        break;
      case 7: response->print(F("&#127768;"));
    }
    response->print(F("</span>"));
    response->print(F("</td>\n"));
    response->print(F("</tr>\n"));
    response->print(F("<tr>"));
    response->print(F("<td>"));

  #if defined(SunRiseLib) || defined(APIKEY)
    response->print(F("<span id=\"Sonnenzeit\" title=\"" LANG_SUNRISE "\">"));
    response->print(String(sunriseHour));
  #ifdef FRONTCOVER_FR
    response->print(F("h"));
  #else
    response->print(F(":"));
  #endif  
    if (sunriseMinute < 10) response->print(F("0"));
    response->print(String(sunriseMinute));
    response->print(F(" "));
    response->print(F(LANG_AM));
    response->print(F("</span></td>\n"));
  #endif

    response->print(F("<td>"));

  #if defined(SunRiseLib) || defined(APIKEY)
    response->print(F("<span id=\"Sonnenzeit\" title=\"" LANG_SUNSET "\">"));
  #ifdef FRONTCOVER_EN
    response->print(String(sunsetHour-12));
  #else
    response->print(String(sunsetHour));
  #endif
  #ifdef FRONTCOVER_FR
    response->print(F("h"));
  #else
    response->print(F(":"));
  #endif
    if (sunsetMinute < 10) response->print(F("0"));
    response->print(String(sunsetMinute));
    response->print(F(" "));
    response->print(F(LANG_PM));
    response->print(F("</span></td>\n"));
  #endif

    response->print(F("<td>"));
    response->print(F("<span id=\"Mondphase\" title=\"" LANG_MOONPHASE "\">"));
    if ( moonphase == 0 )                   response->print(F(LANG_NEWMOON));     // Neumond
    if ( moonphase == 11 )                  response->print(F(LANG_FULLMOON));    // Vollmond
    if ( moonphase > 0 && moonphase < 11 )  response->print(F(LANG_INCREASINGMOON));   // zunehmend
    if ( moonphase > 11 && moonphase < 22 ) response->print(F(LANG_WANINGMOON));   // abnehmend
    response->print(F("</span></td>\n"));
    response->print(F("</tr></table>"));
    response->print(F("<hr>\n"));
  #endif

  // Abschnitt Innentemperatur + Luftfeuchtigkeit + Luftdruck

  #if defined(RTC_BACKUP) || defined(SENSOR_BME280)
    response->print(F("<br><span title=\"" LANG_INDOOR "\" style=\"font-size:30px;\">&#127968;</span>"));  //Haus
    response->print(F("<br><br><span style=\"font-size:24px;\">&#127777;</span> <span style=\"font-size:20px;cursor:pointer\" onclick=\"modetemp()\">"));
    response->print(String(myBME->roomTemperature,1));
    response->print(F("&deg;C"));
  #ifdef FRONTCOVER_EN  
    response->print(F("/ ")); 
    response->print(String(myBME->roomTemperature * 1.8 + 32.0,1));
    response->print(F("&deg;F"));
  #endif
    message +=F("</span>"));
  #endif

  #ifdef SENSOR_BME280
    response->print("<br><br><span style=\"font-size:18px;\">&#128167;</span> <span style=\"font-size:20px;cursor:pointer\" onclick=\"modehum()\">" + String(myBME->roomHumidity,0) + "%RH</span>"
                "<br><span style=\"font-size:24px;\">");
    if (myBME->roomHumidity < 30) response->print(F("<span style=\"color:Red;\">&#9751;</span>"));
    else response->print(F("<span style=\"color:Red;\">&#9750;</span>"));
    if ((myBME->roomHumidity >= 30) && (myBME->roomHumidity < 40)) response->print(F("<span style=\"color:Orange;\">&#9751;</span>"));
    else response->print(F("<span style=\"color:Orange;\">&#9750;</span>"));
    if ((myBME->roomHumidity >= 40) && (myBME->roomHumidity <= 50)) response->print(F("<span style=\"color:MediumSeaGreen;\">&#9751;</span>"));
    else response->print(F("<span style=\"color:MediumSeaGreen;\">&#9750;</span>"));
    if ((myBME->roomHumidity > 50) && (myBME->roomHumidity < 60)) response->print(F("<span style=\"color:Lightblue;\">&#9751;</span>"));
    else response->print(F("<span style=\"color:Lightblue;\">&#9750;</span>"));
    if (myBME->roomHumidity >= 60) response->print(F("<span style=\"color:Blue;\">&#9751;</span>"));
    else response->print(F("<span style=\"color:Blue;\">&#9750;</span>"));
    response->print(F("</span>"));
    response->print(F("<br><br><span style=\"font-size:20px;\">&#128168;</span><span style=\"font-size:20px;\">rel. "));
    response->print(String(myBME->Pressure_red,0));
    response->print(F(" hPa <br>(abs. ")); 
    response->print(String(myBME->Pressure,0)); 
    response->print(F(" hPa) </span>"));

    //#####################
    // Luftdruck Diagramm
    //#####################
    response->print(F("<br>\n"));
    response->print(F("<br><span style=\"font-size:14px;\">"));
    response->print(F(LANG_AIRPRESSUREHIST));
    response->print(F("  </span>"));
    response->print(F("<span style=\"font-size:24px;\">"));
    if ( myBME->luftdrucktendenz_web == 1 ) response->print(F(" &#8595;"));
    if ( myBME->luftdrucktendenz_web == 2 ) response->print(F(" &#8600;"));
    if ( myBME->luftdrucktendenz_web == 3 ) response->print(F(" &#8594;"));
    if ( myBME->luftdrucktendenz_web == 4 ) response->print(F(" &#8599;"));
    if ( myBME->luftdrucktendenz_web == 5 ) response->print(F(" &#8593;"));
    response->print(F("</span><span style=\"font-size:14px;\"> )</span>\n"));
    response->print(F("<div>\n"));
    response->print(F("<canvas id=\"canvas_druckdiagramm\" width=\"300\" height=\"270\" style=\"border:2px solid #d3d3d3;cursor:pointer\" onclick=\"luftdruck()\">"));
    response->print(F("<script type=\"text/javascript\">"));
    response->print(F("var canvas = document.getElementById(\"canvas_druckdiagramm\"));\n"));
    response->print(F("var canvasWidth = 300;\n"));
    response->print(F("var canvasHeight = 270;\n"));
    response->print(F("canvas.setAttribute('width', canvasWidth));\n"));
    response->print(F("canvas.setAttribute('height', canvasHeight));\n"));
    response->print(F("var cv = canvas.getContext(\"2d\"));\n"));

    //Options Grid

    response->print(F("var yAchse = { values:[\n"));
    for (uint8_t i = 0; i <= 65; i = i + 5) {
      response->print(F("{Y:\""));
      if ( LUFTDRUCKMIN + i < 1000 ) response->print(F("  "));
      response->print(String(LUFTDRUCKMIN + i));
      response->print(F("hPa\",C:\"#000000\"},{Y:\" \",C:\"#000000\"},\n"));
    }

    response->print(F("]};\n"));
    response->print(F("var graphxoffset = 42;\n"));
    response->print(F("var graphyoffset = 15;\n"));
    response->print(F("var graphGridSize = (canvasHeight - graphyoffset) / yAchse.values.length;\n"));
    response->print(F("var graphGridY = (canvasHeight - graphyoffset) / graphGridSize;\n"));
    response->print(F("cv.lineWidth = 1;\n"));
    response->print(F("for(var i = 0; i < graphGridY; i++){\n"));
    response->print(F("cv.moveTo(canvasWidth,  canvasHeight - graphGridSize*i-graphyoffset));\n"));
    response->print(F("cv.lineTo(graphxoffset, canvasHeight - graphGridSize*i-graphyoffset));\n"));
    response->print(F("}\n"));
    response->print(F("cv.strokeStyle = \"#FFFFFF\";\n"));
    response->print(F("cv.stroke());\n"));

    response->print(F("cv.fillStyle = '#000000';\n"));
    response->print(F("cv.font = \"10px Arial\";\n"));
    response->print(F("for(var i = 0; i < graphGridY; i++){\n"));
    response->print(F("cv.fillStyle = yAchse.values[i].C;\n"));
    response->print(F("cv.fillText(yAchse.values[i].Y,0,canvasHeight - graphGridSize*i-graphyoffset+3,graphxoffset));\n"));
    response->print(F("}\n"));

    //Data Graph
    response->print(F("var data = { values:["));
    response->print(F("{A:\"-30h\",B:"));
    response->print(String(myBME->luftdruck_hist[0]));
    response->print(F(",C:\"#353746\"},\n"));
    response->print(F("{A:\"-27h\",B:"));
    response->print(String(myBME->luftdruck_hist[1]));
    response->print(F(",C:\"#353746\"},\n"));
    response->print(F("{A:\"-24h\",B:"));
    response->print(String(myBME->luftdruck_hist[2]));
    response->print(F(",C:\"#353746\"},\n"));
    response->print(F("{A:\"-21h\",B:"));
    response->print(String(myBME->luftdruck_hist[3]));
    response->print(F(",C:\"#353746\"},\n"));
    response->print(F("{A:\"-18h\",B:"));
    response->print(String(myBME->luftdruck_hist[4]));
    response->print(F(",C:\"#353746\"},\n"));
    response->print(F("{A:\"-15h\",B:"));
    response->print(String(myBME->luftdruck_hist[5]));
    response->print(F(",C:\"#353746\"},\n"));
    response->print(F("{A:\"-12h\",B:"));
    response->print(String(myBME->luftdruck_hist[6]));
    response->print(F(",C:\"#353746\"},\n"));
    response->print(F("{A:\" -9h\",B:"));
    response->print(String(myBME->luftdruck_hist[7]));
    response->print(F(",C:\"#353746\"},\n"));
    response->print(F("{A:\" -6h\",B:"));
    response->print(String(myBME->luftdruck_hist[8]));
    response->print(F(",C:\"#353746\"},\n"));
    response->print(F("{A:\" -3h\",B:"));
    response->print(String(myBME->luftdruck_hist[9]));
    response->print(F(",C:\"#353746\"},\n"));
    response->print(F("{A:\" akt\",B:"));
    response->print(String(myBME->luftdruck_hist[10]));
    response->print(F(",C:\"#353746\"}\n"));
    response->print(F("]};\n"));

    //Options Graph
    response->print(F("var graphMinValue = ")); 
    response->print(String(LUFTDRUCKMIN));
    response->print(F(";\n"));    // Value der Nulllinie
    response->print(F("var graphXDiff = 2.5;\n"));       // Value für eine Abschnittsdifferenz
    response->print(F("var graphPadding = 5;\n"));
    response->print(F("var graphFaktor = graphGridSize / graphXDiff;\n"));
    response->print(F("var graphWidth = (canvasWidth-graphPadding-graphxoffset) / data.values.length;\n"));
    response->print(F("var graphTextcolor = '#000000';\n"));

    //Draw Graph
    response->print(F("for(var i = 0; i < data.values.length; i ++){\n"));
    response->print(F("Value = data.values[i].B-graphMinValue;\n"));
    response->print(F("tmpTop = (canvasHeight-graphyoffset-(graphFaktor*Value)).toFixed());\n"));
    response->print(F("tmpHeight = ((Value*graphFaktor)).toFixed());\n"));
    response->print(F("cv.fillStyle = data.values[i].C;\n"));
    response->print(F("cv.fillRect(graphWidth+((i-1)*graphWidth)+graphPadding+graphxoffset, tmpTop, graphWidth-graphPadding, tmpHeight));\n"));
    response->print(F("cv.fillStyle = graphTextcolor;\n"));
    response->print(F("cv.font = \"10px Arial\";\n"));
    response->print(F("cv.fillText(data.values[i].A, graphWidth+((i-1)*graphWidth)+graphPadding-2+graphxoffset, canvasHeight-3, graphWidth));\n"));
    response->print(F("}\n"));
    response->print(F("</script>"));
    response->print(F("</div>\n"));
    response->print(F("<hr>\n"));

  #else
    response->print(F("<br>\n"));
  #endif

  // Abschnitt Außentemperatur + Luftfeuchtigkeit
  #ifdef APIKEY
    if ( strlen(settings->mySettings.openweatherapikey) > 25 )
    {
      response->print(F("<br>"));
      response->print(F("<span title=\"" LANG_OUTDOOR "\" style=\"font-size:30px;\">&#127794;  ")); //Baum
      char *result = strchr(settings->mySettings.openweatherlocation, ',');
      if (result) { // Falls ein Komma gefunden wurde
        size_t len = result - settings->mySettings.openweatherlocation;
        response->print(String(settings->mySettings.openweatherlocation).substring(0,len));
      }
      response->print(F("</span>"));
      response->print(F("<br><br><span style=\"font-size:24px;\">&#127777;</span> <span style=\"font-size:20px;cursor:pointer\" onclick=\"modeexttemp()\">"));
      response->print(String(outdoorWeather->temperature,1));
      response->print(F("&deg;C"));
  #ifdef FRONTCOVER_EN  
      response->print(F("/ ")); 
      response->print(String(outdoorWeather->temperature * 1.8 + 32.0,1));
      response->print(F("&deg;F"));
  #endif
      response->print(F("</span>"));
      response->print(F("<br><br><span style=\"font-size:18px;\">&#128167;</span> <span style=\"font-size:20px;cursor:pointer\" onclick=\"modeexthum()\">"));
      response->print(String(outdoorWeather->humidity));
      response->print(F("%RH</span><br><br><span style=\"font-size:20px;\">"));
      response->print(F(LANG_WINDSPEED)); 
      response->print(F(" "));
      response->print(String(outdoorWeather->windspeed,1));
      response->print(F(" &#13223;</span><br><br><span style=\"font-size:20px;cursor:pointer\" onclick=\"wetter()\">"));
      response->print(outdoorWeather->description);
      response->print(F("</span><br>"));
      if ( outdoorWeather->weathericon1.length() > 1 )
      {
        response->print(F("<img id=\"Wettersymbole\" title=\"" LANG_WEATHER "\" src=\"/web/web_" ));
        response->print(getWeatherIcon(outdoorWeather->weathericon1));
        response->print(F(".png\" alt=\"Wetter\">"));
      }
      if ( outdoorWeather->weathericon2.length() > 1 && outdoorWeather->weathericon1 != outdoorWeather->weathericon2 )
      {
        response->print(F("<img id=\"Wettersymbole\" title=\"" LANG_WEATHER "\" src=\"/web/web_" ));
        response->print(getWeatherIcon(outdoorWeather->weathericon2));
        response->print(F(".png\" alt=\"Wetter\">"));
      }

      response->print(F("<br><br><hr>\n"));
    }
   #endif

  #ifdef SENSOR_BME280
    //#####################
    // Temperatur Diagramm
    //#####################
    response->print(F("<br>\n"));
    response->print(F("<span style=\"font-size:14px;\">"));
    response->print(F(LANG_TEMPHIST));
    response->print(F("</span>\n"));
    response->print(F("<div>\n"));
    response->print(F("<canvas id=\"canvas_tempdiagramm\" width=\"400\" height=\"200\" style=\"border:1px solid #d3d3d3;\">\n"));
    response->print(F("<script type=\"text/javascript\">\n"));
    response->print(F("var canvas = document.getElementById(\"canvas_tempdiagramm\"));\n"));
    response->print(F("var canvasWidth = 330;\n"));
    response->print(F("var canvasHeight = 270;\n"));
    response->print(F("canvas.setAttribute('width', canvasWidth));\n"));
    response->print(F("canvas.setAttribute('height', canvasHeight));\n"));
    response->print(F("var cv = canvas.getContext(\"2d\"));\n"));
    //Options Grid

    response->print(F("var yAchse = { values:[\n"
                  "{Y:\" -20°C\",C:\"#990099\"},\n"
                  "{Y:\" -15°C\",C:\"#330099\"},\n"
                  "{Y:\" -10°C\",C:\"#0000aa\"},\n"
                  "{Y:\"  -5°C\",C:\"#0000bb\"},\n"
                  "{Y:\"   0°C\",C:\"#003399\"},\n"
                  "{Y:\"   5°C\",C:\"#005555\"},\n"
                  "{Y:\"  10°C\",C:\"#007733\"},\n"
                  "{Y:\"  15°C\",C:\"#777722\"},\n"
                  "{Y:\"  20°C\",C:\"#775522\"},\n"
                  "{Y:\"  25°C\",C:\"#995522\"},\n"
                  "{Y:\"  30°C\",C:\"#992222\"},\n"
                  "{Y:\"  35°C\",C:\"#771111\"},\n"
                  "{Y:\"  40°C\",C:\"#770000\"}\n"
                  "]};\n"));

    //X-Grid:
    response->print(F("var graphxoffset = 30;\n"));
    response->print(F("var graphyoffset = 15;\n"));
    response->print(F("var graphGridSize = (canvasHeight - graphyoffset) / yAchse.values.length;\n"));
    response->print(F("var graphGridY = (canvasHeight - graphyoffset) / graphGridSize;\n"));

    response->print(F("cv.lineWidth = 1;\n"));
    response->print(F("for(var i = 0; i < graphGridY; i++){\n"));
    response->print(F("cv.moveTo(canvasWidth,  canvasHeight - graphGridSize*i-graphyoffset));\n"));
    response->print(F("cv.lineTo(graphxoffset, canvasHeight - graphGridSize*i-graphyoffset));\n"));
    response->print(F("}\n"));
    response->print(F("cv.strokeStyle = \"#FFFFFF\";\n"));
    response->print(F("cv.stroke());\n"));

    //Y-Achse Text:
    response->print(F("cv.fillStyle = '#000000';\n"));
    response->print(F("cv.font = \"10px Arial\";\n"));
    response->print(F("for(var i = 0; i < graphGridY; i++){\n"));
    response->print(F("cv.fillStyle = yAchse.values[i].C;\n"));
    response->print(F("cv.fillText(yAchse.values[i].Y,0,canvasHeight - graphGridSize*i-graphyoffset+3,graphxoffset));\n"));
    response->print(F("}\n"));

    //Data Graph:
    response->print("var data = { values:[\n");
    for ( int i = 0; i <= 71; i++)
    {
      response->print(F("{T:\""));
      response->print(myBME->temperatur_hist[i].stundeminute);
      response->print(F("\",GA:"));
      response->print(String(myBME->temperatur_hist[i].aussentemp));
      response->print(F(",GB:"));
      response->print(String(myBME->temperatur_hist[i].innentemp));
      response->print(F("},\n"));
    }
    response->print(F("{T:\""));
    response->print(myBME->temperatur_hist[72].stundeminute);
    response->print(F("\",GA:"));
    response->print(String(myBME->temperatur_hist[72].aussentemp));
    response->print(F(",GB:"));
    response->print(String(myBME->temperatur_hist[72].innentemp));
    response->print(F("}\n"));
    response->print(F("]};\n"));

    //Options Graph

    response->print(F("var graphMinValue = -20;\n"                         // Value der Nulllinie
                  "var graphXDiff = 5;\n"                              // Value für eine Abschnittsdifferenz
                  "var graphFaktor = graphGridSize/graphXDiff;\n"
                  "var graphPadding = 1;\n"
                  "var graphWidth = (canvasWidth-graphPadding-graphxoffset)/data.values.length;\n"
                  "var graphTextcolor = '#000000';\n"
                  "var gradient = cv.createLinearGradient(0,canvasHeight,0,0));\n"));
    response->print(F("gradient.addColorStop(\"0\",\"magenta\");"
                  "gradient.addColorStop(\"0.3\",\"blue\");"
                  "gradient.addColorStop(\"0.4\",\"green\");"
                  "gradient.addColorStop(\"0.6\",\"yellow\");"
                  "gradient.addColorStop(\"1.0\",\"red\");\n"));

    //Draw Graph
    response->print(F("for(var i = 1; i < data.values.length; i ++){\n"));
    // Graph1
    response->print(F("ValueA1 = data.values[i-1].GA-graphMinValue;\n"));
    response->print(F("ValueA2 = data.values[i].GA-graphMinValue;\n"));
    response->print(F("tmpTopA1 = (canvasHeight-graphyoffset-(graphFaktor*ValueA1)).toFixed();\n"));
    response->print(F("tmpTopA2 = (canvasHeight-graphyoffset-(graphFaktor*ValueA2)).toFixed();\n"));

    // Graph2
    response->print(F("ValueB1 = data.values[i-1].GB-graphMinValue;\n"));
    response->print(F("ValueB2 = data.values[i].GB-graphMinValue;\n"));
    response->print(F("tmpTopB1 = (canvasHeight-graphyoffset-(graphFaktor*ValueB1)).toFixed();\n"));
    response->print(F("tmpTopB2 = (canvasHeight-graphyoffset-(graphFaktor*ValueB2)).toFixed();\n"));

    // Y-Grid + X-Beschriftung
    response->print(F("if ( data.values[i-1].T.substr(-2) == \"00\" ) {\n"));
    response->print(F("cv.fillStyle = graphTextcolor;\n"));
    response->print(F("cv.font = \"9px Arial\";\n"));
    response->print(F("cv.fillText(data.values[i-1].T.substr(0,2), graphWidth+((i-1)*graphWidth)+graphPadding-5+graphxoffset, canvasHeight-2,graphWidth*3);\n"));
    response->print(F("cv.beginPath());\n"));
    response->print(F("cv.moveTo(graphWidth+((i-1)*graphWidth)+graphPadding+graphxoffset, canvasHeight - graphyoffset+4);\n"));
    response->print(F("cv.lineTo(graphWidth+((i-1)*graphWidth)+graphPadding+graphxoffset, 0);\n"));
    response->print(F("cv.lineWidth = 1;\n"));
    response->print(F("cv.strokeStyle = \"#FFFFFF\";\n"));
    response->print(F("cv.stroke());\n"));
    response->print(F("}\n"));
    response->print(F("cv.beginPath());\n"));
    response->print(F("cv.lineWidth = 3;\n"));
    response->print(F("cv.moveTo(graphWidth+((i-1)*graphWidth)+graphPadding+graphxoffset, tmpTopA1);\n"));
    response->print(F("cv.lineTo(graphWidth+((i)*graphWidth)+graphPadding+graphxoffset, tmpTopA2);\n"));
    response->print(F("cv.strokeStyle = gradient;\n"));
    response->print(F("cv.stroke());\n"));
    response->print(F("cv.beginPath());\n"));
    response->print(F("cv.lineWidth = 3;\n"));
    response->print(F("cv.moveTo(graphWidth+((i-1)*graphWidth)+graphPadding+graphxoffset, tmpTopB1);\n"));
    response->print(F("cv.lineTo(graphWidth+((i)*graphWidth)+graphPadding+graphxoffset, tmpTopB2);\n"));
    response->print(F("cv.strokeStyle = \"#000000\";\n"));
    response->print(F("cv.stroke();\n"));
    response->print(F("}\n"));
    response->print(F("</script></div><hr>\n"));

  #endif

    // Abschnitt Wortuhr und Uptime
    response->print(F("<table>"));
    response->print(F("<tr>"));
    response->print(F("<td>"));

    response->print(F("<span style=\"font-size:12px;\">\n"));
    response->print(F("<br>"));
    response->print(F(LANG_WORDCLOCK));
    response->print(F("<br>\nUptime: "));
    response->print(mt->convertSeconds(upTime));
    response->print(F("<br>\nFirmware: "));
    response->print(FIRMWARE_VERSION);
    response->print(" ");
    response->print(BUILD_DATE);
    response->print(F("<br>\n"));

    // QR-Code Button
    response->print(F("</td>"));
    response->print(F("<td>"));
    response->print(F("<button style=\"background: none; border: none;\" onclick=\"window.location.href='https://merz-aktuell.de/Wordclock/Doku/WortuhrBeschreibung10.x.pdf'\">"));
    response->print(F("<img src=\"/web/qr-code.png\" alt=\"Handbuch\" width=\"120\" height=\"120\">"));
    response->print(F("</button>"));
    response->print(F("</td>"));
    response->print(F("</tr>"));
    response->print(F("</table>"));

    // Abschnitt Info Button
    response->print(F("<hr>\n"));
    response->print(F("<button id=\"Infobutton\" title=\"Info\" name=\"Info\" onclick=\"window.location.href='/debugClock'\">Info</button>"));
    response->print(F("</span>"));
    response->print(F("\n<script>\n"
                  "$(\"#button_zeit\").click(function() {"
                  "$.post(\"/handleButtonTime\");"
                  "});\n"
                  "$(\"#button_mode\").click(function() {"
                  "$.post(\"/handleButtonMode\");"
                  "});\n"
                  "document.addEventListener(\"visibilitychange\", function() {"
                  "if (!document.hidden){"
                  " location.reload();"
                  "}"
                  "});\n"));

  #ifdef SHOW_MODE_MOONPHASE
    response->print(F("function moonphase() {"
                    "$.post(\"/control?mode=")); 
    response->print(String(MODE_MOONPHASE));
    response->print(F("&sound=" SMODE_SOUND "\");"
                  "};\n"));
  #endif

  #ifdef APIKEY 
    response->print(F("function modeexttemp() {"
                    "$.post(\"/control?mode=")); 
    response->print(String(MODE_EXT_TEMP));
    response->print(F("&sound=" SMODE_SOUND "\");"
                  "};\n"));
    response->print(F("function modeexthum() {"
                    "$.post(\"/control?mode=")); 
    response->print(String(MODE_EXT_HUMIDITY));
    response->print(F("&sound=" SMODE_SOUND "\");"
                  "};\n"));
    response->print(F("function wetter() {"
                    "$.post(\"/control?mode=")); 
    response->print(String(MODE_WETTER));
    response->print(F("&sound=" SMODE_SOUND "\");"
                  "};\n"));
  #endif

  // Ende Script
  response->print(F("</script>\n"));

  // Ende Webserver
  response->print(F("</body></html>"));
  request->send(response);
#if defined(SYSLOGSERVER_SERVER)
  syslog.log(LOG_INFO, "handleRoot: end");
#endif
}