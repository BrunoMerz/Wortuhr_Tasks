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

//#define myDEBUG
#include "MyDebug.h"

#include "ESPAsyncWebServer.h"
#include <LittleFS.h>

static Settings *settings = Settings::getInstance();
static MyTime *mt = MyTime::getInstance();
static MyWifi *myWifi = MyWifi::getInstance();
static AsyncWebServer *webServer = myWifi->getServer();
static OpenWeather *outdoorWeather = OpenWeather::getInstance();
static Renderer *renderer = Renderer::getInstance();


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

  if ( mt->local() > outdoorWeather->sunrise && mt->local() < outdoorWeather->sunset ) tagnacht = "d";
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
    String message;
    message = F("<!doctype html>");
    message += F("<html><head>");
    message += F("<title>");
    message += String(settings->mySettings.systemname);
    message += F("</title>");
    message += F("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
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
                  "<body>");
    message += F("<h1>");
    message += String(settings->mySettings.systemname);
    message += F("</h1>\n");
    message += F("<h2>");
    message += renderer->WEB_Uhrtext;
    message += F("</h2>\n");
  #ifdef DEDICATION
    message += F(DEDICATION);
    message += F("<br><br>");
  #endif
    if (mode == MODE_BLANK) message += F("<button title=\"LEDs " LANG_ON "\" onclick=\"window.location.href='/handleButtonOnOff'\">&#9899;</button>\n");
    else message += F("<button title=\"LEDs " LANG_OFF "\" onclick=\"window.location.href='/handleButtonOnOff'\">&#9898;</button>\n");
    message += F("<button title=\"" LANG_SETTINGS "\" onclick=\"window.location.href='/handleButtonSettings'\">&#128295;</button>\n");
    message += F("<button title=\"" LANG_ANIMATIONS "\" onclick=\"window.location.href='/animationmenue'\">&#127916;</button>\n");
    message += F("<button title=\"" LANG_EVENTS "\" onclick=\"window.location.href='/web/events.html'\">&#128197;</button>\n"
                  "<button title=\"" LANG_MESSAGE "\" onclick=\"window.location.href='/web/nachricht.html'\">&#128172;</button>\n"
                  "<button title=\"" LANG_GAMES "\" onclick=\"window.location.href='/web/Gamemenue.html'\">&#127922;</button>\n"
                  "<button title=\"" LANG_MODEBUTTON "\" id=\"button_mode\" type=\"button\" \">&#128512;</button>\n"
                  "<button title=\"" LANG_TIMEBUTTON "\" id=\"button_zeit\" type=\"button\" \">&#128344;</button>\n");

    message += F("<hr>\n");

    // Abschnitt Mondphase
  #ifdef SHOW_MODE_MOONPHASE

    message += F("<table><tr>");
    message += F("<td style=\"width:30%\">");
    message += F("<img id=\"Sonnen\" title=\"" LANG_SUNRISE "\" src=\"/sunrise\" alt=\"Sonnenaufgang\">");
    message += F("</td>\n");

    message += F("<td style=\"width:30%\">");
    message += F("<img id=\"Sonnen\" title=\"" LANG_SUNSET "\" src=\"/sunset\" alt=\"Sonnenuntergang\">");
    message += F("</td>\n");

    message += F("<td style=\"padding-top: 17px;width:40%;cursor:pointer\">");
    message += F("<span id=\"Mond\" title=\"" LANG_MOONPHASE "\" onclick=\"moonphase()\">");
    switch (web_moonphase)
    {
      case 0: message += F("&#127761;"); //Neumond
        break;
      case 1: message += F("&#127762;");
        break;
      case 2: message += F("&#127763;");
        break;
      case 3: message += F("&#127764;");
        break;
      case 4: message += F("&#127765;"); //Vollmond
        break;
      case 5: message += F("&#127766;");
        break;
      case 6: message += F("&#127767;");
        break;
      case 7: message += F("&#127768;");
    }
    message += F("</span>");
    message += F("</td>\n");
    message += F("</tr>\n");
    message += F("<tr>");
    message += F("<td>");

  #if defined(SunRiseLib) || defined(APIKEY)
    message += F("<span id=\"Sonnenzeit\" title=\"" LANG_SUNRISE "\">");
    message += String(sunriseHour);
  #ifdef FRONTCOVER_FR
    message += F("h");
  #else
    message += F(":");
  #endif  
    if (sunriseMinute < 10) message += F("0");
    message += String(sunriseMinute);
    message += F(" ");
    message += F(LANG_AM);
    message += F("</span></td>\n");
  #endif

    message += F("<td>");

  #if defined(SunRiseLib) || defined(APIKEY)
    message += F("<span id=\"Sonnenzeit\" title=\"" LANG_SUNSET "\">");
  #ifdef FRONTCOVER_EN
    message += String(sunsetHour-12);
  #else
    message += String(sunsetHour);
  #endif
  #ifdef FRONTCOVER_FR
    message += F("h");
  #else
    message += F(":");
  #endif
    if (sunsetMinute < 10) message += F("0");
    message += String(sunsetMinute);
    message += F(" ");
    message += F(LANG_PM);
    message += F("</span></td>\n");
  #endif

    message += F("<td>");
    message += F("<span id=\"Mondphase\" title=\"" LANG_MOONPHASE "\">");
    if ( moonphase == 0 )                   message += F(LANG_NEWMOON);     // Neumond
    if ( moonphase == 11 )                  message += F(LANG_FULLMOON);    // Vollmond
    if ( moonphase > 0 && moonphase < 11 )  message += F(LANG_INCREASINGMOON);   // zunehmend
    if ( moonphase > 11 && moonphase < 22 ) message += F(LANG_WANINGMOON);   // abnehmend
    message += F("</span></td>\n");
    message += F("</tr></table>");
    message += F("<hr>\n");
  #endif

    // Abschnitt Innentemperatur + Luftfeuchtigkeit + Luftdruck

  #if defined(RTC_BACKUP) || defined(SENSOR_BME280)
    message += F("<br><span title=\"" LANG_INDOOR "\" style=\"font-size:30px;\">&#127968;</span>");  //Haus
    message += F("<br><br><span style=\"font-size:24px;\">&#127777;</span> <span style=\"font-size:20px;cursor:pointer\" onclick=\"modetemp()\">");
    message += String(roomTemperature,1);
    message += F("&deg;C");
  #ifdef FRONTCOVER_EN  
    message += F("/ "); 
    message += String(roomTemperature * 1.8 + 32.0,1);
    message += F("&deg;F");
  #endif
    message +=F("</span>");
  #endif

    // ################### sende html Teil 1
    //webServer.sendHeader("Cache-Control", "no-cache");
    //webServer.send(200, TEXT_HTML, message);
    response->print(message);
    message = "";

  #ifdef SENSOR_BME280
    message += "<br><br><span style=\"font-size:18px;\">&#128167;</span> <span style=\"font-size:20px;cursor:pointer\" onclick=\"modehum()\">" + String(roomHumidity,0) + "%RH</span>"
                "<br><span style=\"font-size:24px;\">";
    if (roomHumidity < 30) message += F("<span style=\"color:Red;\">&#9751;</span>");
    else message += F("<span style=\"color:Red;\">&#9750;</span>");
    if ((roomHumidity >= 30) && (roomHumidity < 40)) message += F("<span style=\"color:Orange;\">&#9751;</span>");
    else message += F("<span style=\"color:Orange;\">&#9750;</span>");
    if ((roomHumidity >= 40) && (roomHumidity <= 50)) message += F("<span style=\"color:MediumSeaGreen;\">&#9751;</span>");
    else message += F("<span style=\"color:MediumSeaGreen;\">&#9750;</span>");
    if ((roomHumidity > 50) && (roomHumidity < 60)) message += F("<span style=\"color:Lightblue;\">&#9751;</span>");
    else message += F("<span style=\"color:Lightblue;\">&#9750;</span>");
    if (roomHumidity >= 60) message += F("<span style=\"color:Blue;\">&#9751;</span>");
    else message += F("<span style=\"color:Blue;\">&#9750;</span>");
    message += F("</span>");
    message += F("<br><br><span style=\"font-size:20px;\">&#128168;</span><span style=\"font-size:20px;\">rel. ");
    message += String(Pressure_red,0);
    message += F(" hPa <br>(abs. "); 
    message += String(Pressure,0); 
    message += F(" hPa) </span>");

    //#####################
    // Luftdruck Diagramm
    //#####################
    message += F("<br>\n");
    message += F("<br><span style=\"font-size:14px;\">");
    message += F(LANG_AIRPRESSUREHIST);
    message += F("  </span>");
    message += F("<span style=\"font-size:24px;\">");
    if ( luftdrucktendenz_web == 1 ) message += F(" &#8595;");
    if ( luftdrucktendenz_web == 2 ) message += F(" &#8600;");
    if ( luftdrucktendenz_web == 3 ) message += F(" &#8594;");
    if ( luftdrucktendenz_web == 4 ) message += F(" &#8599;");
    if ( luftdrucktendenz_web == 5 ) message += F(" &#8593;");
    message += F("</span><span style=\"font-size:14px;\"> )</span>\n");
    message += F("<div>\n");
    message += F("<canvas id=\"canvas_druckdiagramm\" width=\"300\" height=\"270\" style=\"border:2px solid #d3d3d3;cursor:pointer\" onclick=\"luftdruck()\">");
    message += F("<script type=\"text/javascript\">");
    message += F("var canvas = document.getElementById(\"canvas_druckdiagramm\");\n");
    message += F("var canvasWidth = 300;\n");
    message += F("var canvasHeight = 270;\n");
    message += F("canvas.setAttribute('width', canvasWidth);\n");
    message += F("canvas.setAttribute('height', canvasHeight);\n");
    message += F("var cv = canvas.getContext(\"2d\");\n");

    //Options Grid

    message += F("var yAchse = { values:[\n");
    for (uint8_t i = 0; i <= 65; i = i + 5) {
      message += F("{Y:\"");
      if ( LUFTDRUCKMIN + i < 1000 ) message += F("  ");
      message += String(LUFTDRUCKMIN + i);
      message += F("hPa\",C:\"#000000\"},{Y:\" \",C:\"#000000\"},\n");
    }
    // ################### sende html Teil 2
    webServer.sendContent(message);
    message = "";
    delay(0);
    message += F("]};\n");
    message += F("var graphxoffset = 42;\n");
    message += F("var graphyoffset = 15;\n");
    message += F("var graphGridSize = (canvasHeight - graphyoffset) / yAchse.values.length;\n");
    message += F("var graphGridY = (canvasHeight - graphyoffset) / graphGridSize;\n");
    message += F("cv.lineWidth = 1;\n");
    message += F("for(var i = 0; i < graphGridY; i++){\n");
    message += F("cv.moveTo(canvasWidth,  canvasHeight - graphGridSize*i-graphyoffset);\n");
    message += F("cv.lineTo(graphxoffset, canvasHeight - graphGridSize*i-graphyoffset);\n");
    message += F("}\n");
    message += F("cv.strokeStyle = \"#FFFFFF\";\n");
    message += F("cv.stroke();\n");

    message += F("cv.fillStyle = '#000000';\n");
    message += F("cv.font = \"10px Arial\";\n");
    message += F("for(var i = 0; i < graphGridY; i++){\n");
    message += F("cv.fillStyle = yAchse.values[i].C;\n");
    message += F("cv.fillText(yAchse.values[i].Y,0,canvasHeight - graphGridSize*i-graphyoffset+3,graphxoffset);\n");
    message += F("}\n");

    //Data Graph
    message += F("var data = { values:[");
    message += F("{A:\"-30h\",B:");
    message += String(luftdruck_hist[0]);
    message += F(",C:\"#353746\"},\n");
    message += F("{A:\"-27h\",B:");
    message += String(luftdruck_hist[1]);
    message += F(",C:\"#353746\"},\n");
    message += F("{A:\"-24h\",B:");
    message += String(luftdruck_hist[2]);
    message += F(",C:\"#353746\"},\n");
    message += F("{A:\"-21h\",B:");
    message += String(luftdruck_hist[3]);
    message += F(",C:\"#353746\"},\n");
    message += F("{A:\"-18h\",B:");
    message += String(luftdruck_hist[4]);
    message += F(",C:\"#353746\"},\n");
    message += F("{A:\"-15h\",B:");
    message += String(luftdruck_hist[5]);
    message += F(",C:\"#353746\"},\n");
    message += F("{A:\"-12h\",B:");
    message += String(luftdruck_hist[6]);
    message += F(",C:\"#353746\"},\n");
    message += F("{A:\" -9h\",B:");
    message += String(luftdruck_hist[7]);
    message += F(",C:\"#353746\"},\n");
    message += F("{A:\" -6h\",B:");
    message += String(luftdruck_hist[8]);
    message += F(",C:\"#353746\"},\n");
    message += F("{A:\" -3h\",B:");
    message += String(luftdruck_hist[9]);
    message += F(",C:\"#353746\"},\n");
    message += F("{A:\" akt\",B:");
    message += String(luftdruck_hist[10]);
    message += F(",C:\"#353746\"}\n");
    message += F("]};\n");

    //Options Graph
    message += F("var graphMinValue = "); 
    message += String(LUFTDRUCKMIN);
    message += F(";\n");    // Value der Nulllinie
    message += F("var graphXDiff = 2.5;\n");       // Value für eine Abschnittsdifferenz
    message += F("var graphPadding = 5;\n");
    message += F("var graphFaktor = graphGridSize / graphXDiff;\n");
    message += F("var graphWidth = (canvasWidth-graphPadding-graphxoffset) / data.values.length;\n");
    message += F("var graphTextcolor = '#000000';\n");

    //Draw Graph
    message += F("for(var i = 0; i < data.values.length; i ++){\n");
    message += F("Value = data.values[i].B-graphMinValue;\n");
    message += F("tmpTop = (canvasHeight-graphyoffset-(graphFaktor*Value)).toFixed();\n");
    message += F("tmpHeight = ((Value*graphFaktor)).toFixed();\n");
    message += F("cv.fillStyle = data.values[i].C;\n");
    message += F("cv.fillRect(graphWidth+((i-1)*graphWidth)+graphPadding+graphxoffset, tmpTop, graphWidth-graphPadding, tmpHeight);\n");
    message += F("cv.fillStyle = graphTextcolor;\n");
    message += F("cv.font = \"10px Arial\";\n");
    message += F("cv.fillText(data.values[i].A, graphWidth+((i-1)*graphWidth)+graphPadding-2+graphxoffset, canvasHeight-3, graphWidth);\n");
    message += F("}\n");
    message += F("</script>");
    message += F("</div>\n");
    message += F("<hr>\n");
    //##################### sende Luftdruck Diagramm html Teil 3
    webServer.sendContent(message);
    message = "";
    delay(0);
  #else
    message += F("<br>\n");
  #endif

    // Abschnitt Außentemperatur + Luftfeuchtigkeit
  #ifdef APIKEY
    if ( strlen(settings->mySettings.openweatherapikey) > 25 )
    {
      message += F("<br>");
      message += F("<span title=\"" LANG_OUTDOOR "\" style=\"font-size:30px;\">&#127794;  "); //Baum
      char *result = strchr(settings->mySettings.openweatherlocation, ',');
      if (result) { // Falls ein Komma gefunden wurde
        size_t len = result - settings->mySettings.openweatherlocation;
        message += String(settings->mySettings.openweatherlocation).substring(0,len);
      }
      message += F("</span>");
      message += F("<br><br><span style=\"font-size:24px;\">&#127777;</span> <span style=\"font-size:20px;cursor:pointer\" onclick=\"modeexttemp()\">");
      message += String(outdoorWeather->temperature,1);
      message += F("&deg;C");
  #ifdef FRONTCOVER_EN  
      message += F("/ "); 
      message += String(outdoorWeather->temperature * 1.8 + 32.0,1);
      message += F("&deg;F");
  #endif
      message += F("</span>");
      message += F("<br><br><span style=\"font-size:18px;\">&#128167;</span> <span style=\"font-size:20px;cursor:pointer\" onclick=\"modeexthum()\">");
      message += String(outdoorWeather->humidity);
      message += F("%RH</span><br><br><span style=\"font-size:20px;\">");
      message += F(LANG_WINDSPEED); 
      message += F(" ");
      message += String(outdoorWeather->windspeed,1);
      message += F(" &#13223;</span><br><br><span style=\"font-size:20px;cursor:pointer\" onclick=\"wetter()\">");
      message += outdoorWeather->description;
      message += F("</span><br>");
      if ( outdoorWeather->weathericon1.length() > 1 )
      {
        message += F("<img id=\"Wettersymbole\" title=\"" LANG_WEATHER "\" src=\"/web/web_" );
        message += getWeatherIcon(outdoorWeather->weathericon1);
        message += F(".png\" alt=\"Wetter\">");
      }
      if ( outdoorWeather->weathericon2.length() > 1 && outdoorWeather->weathericon1 != outdoorWeather->weathericon2 )
      {
        message += F("<img id=\"Wettersymbole\" title=\"" LANG_WEATHER "\" src=\"/web/web_" );
        message += getWeatherIcon(outdoorWeather->weathericon2);
        message += F(".png\" alt=\"Wetter\">");
      }

      message += F("<br><br><hr>\n");
      //##################### sende Außentemperatur + Luftfeuchtigkeit html Teil 4
      response->print(message);
      message = "";
      delay(0);
    }
  #endif

  #ifdef SENSOR_BME280
    //#####################
    // Temperatur Diagramm
    //#####################
    message += F("<br>\n");
    message += F("<span style=\"font-size:14px;\">");
    message += F(LANG_TEMPHIST);
    message += F("</span>\n");
    message += F("<div>\n");
    message += F("<canvas id=\"canvas_tempdiagramm\" width=\"400\" height=\"200\" style=\"border:1px solid #d3d3d3;\">\n");
    message += F("<script type=\"text/javascript\">\n");
    message += F("var canvas = document.getElementById(\"canvas_tempdiagramm\");\n");
    message += F("var canvasWidth = 330;\n");
    message += F("var canvasHeight = 270;\n");
    message += F("canvas.setAttribute('width', canvasWidth);\n");
    message += F("canvas.setAttribute('height', canvasHeight);\n");
    message += F("var cv = canvas.getContext(\"2d\");\n");
    //Options Grid

    message += F("var yAchse = { values:[\n"
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
                  "]};\n");

    //X-Grid:
    message += F("var graphxoffset = 30;\n");
    message += F("var graphyoffset = 15;\n");
    message += F("var graphGridSize = (canvasHeight - graphyoffset) / yAchse.values.length;\n");
    message += F("var graphGridY = (canvasHeight - graphyoffset) / graphGridSize;\n");

    message += F("cv.lineWidth = 1;\n");
    message += F("for(var i = 0; i < graphGridY; i++){\n");
    message += F("cv.moveTo(canvasWidth,  canvasHeight - graphGridSize*i-graphyoffset);\n");
    message += F("cv.lineTo(graphxoffset, canvasHeight - graphGridSize*i-graphyoffset);\n");
    message += F("}\n");
    message += F("cv.strokeStyle = \"#FFFFFF\";\n");
    message += F("cv.stroke();\n");

    //Y-Achse Text:
    message += F("cv.fillStyle = '#000000';\n");
    message += F("cv.font = \"10px Arial\";\n");
    message += F("for(var i = 0; i < graphGridY; i++){\n");
    message += F("cv.fillStyle = yAchse.values[i].C;\n");
    message += F("cv.fillText(yAchse.values[i].Y,0,canvasHeight - graphGridSize*i-graphyoffset+3,graphxoffset);\n");
    message += F("}\n");
  //##################### sende Außentemperatur + Luftfeuchtigkeit html Teil 4.1
    webServer.sendContent(message);
    message = "";
    delay(0);
    //Data Graph:
    message += "var data = { values:[\n";
    for ( int i = 0; i <= 71; i++)
    {
      message += F("{T:\"");
      message += temperatur_hist[i].stundeminute;
      message += F("\",GA:");
      message += String(temperatur_hist[i].aussentemp);
      message += F(",GB:");
      message += String(temperatur_hist[i].innentemp);
      message += F("},\n");
    }
    message += F("{T:\"");
    message += temperatur_hist[72].stundeminute;
    message += F("\",GA:");
    message += String(temperatur_hist[72].aussentemp);
    message += F(",GB:");
    message += String(temperatur_hist[72].innentemp);
    message += F("}\n");
    message += F("]};\n");

    //##################### sende Temp Diagramm html Teil 4.2
    webServer.sendContent(message);
    message = "";
    delay(0);
    //Options Graph

    message += F("var graphMinValue = -20;\n"                         // Value der Nulllinie
                  "var graphXDiff = 5;\n"                              // Value für eine Abschnittsdifferenz
                  "var graphFaktor = graphGridSize/graphXDiff;\n"
                  "var graphPadding = 1;\n"
                  "var graphWidth = (canvasWidth-graphPadding-graphxoffset)/data.values.length;\n"
                  "var graphTextcolor = '#000000';\n"
                  "var gradient = cv.createLinearGradient(0,canvasHeight,0,0);\n");
    message += F("gradient.addColorStop(\"0\",\"magenta\");"
                  "gradient.addColorStop(\"0.3\",\"blue\");"
                  "gradient.addColorStop(\"0.4\",\"green\");"
                  "gradient.addColorStop(\"0.6\",\"yellow\");"
                  "gradient.addColorStop(\"1.0\",\"red\");\n");

    //Draw Graph
    message += F("for(var i = 1; i < data.values.length; i ++){\n");
    // Graph1
    message += F("ValueA1 = data.values[i-1].GA-graphMinValue;\n");
    message += F("ValueA2 = data.values[i].GA-graphMinValue;\n");
    message += F("tmpTopA1 = (canvasHeight-graphyoffset-(graphFaktor*ValueA1)).toFixed();\n");
    message += F("tmpTopA2 = (canvasHeight-graphyoffset-(graphFaktor*ValueA2)).toFixed();\n");

    // Graph2
    message += F("ValueB1 = data.values[i-1].GB-graphMinValue;\n");
    message += F("ValueB2 = data.values[i].GB-graphMinValue;\n");
    message += F("tmpTopB1 = (canvasHeight-graphyoffset-(graphFaktor*ValueB1)).toFixed();\n");
    message += F("tmpTopB2 = (canvasHeight-graphyoffset-(graphFaktor*ValueB2)).toFixed();\n");

    // Y-Grid + X-Beschriftung
    message += F("if ( data.values[i-1].T.substr(-2) == \"00\" ) {\n");
    message += F("cv.fillStyle = graphTextcolor;\n");
    message += F("cv.font = \"9px Arial\";\n");
    message += F("cv.fillText(data.values[i-1].T.substr(0,2), graphWidth+((i-1)*graphWidth)+graphPadding-5+graphxoffset, canvasHeight-2,graphWidth*3);\n");
    message += F("cv.beginPath();\n");
    message += F("cv.moveTo(graphWidth+((i-1)*graphWidth)+graphPadding+graphxoffset, canvasHeight - graphyoffset+4);\n");
    message += F("cv.lineTo(graphWidth+((i-1)*graphWidth)+graphPadding+graphxoffset, 0);\n");
    message += F("cv.lineWidth = 1;\n");
    message += F("cv.strokeStyle = \"#FFFFFF\";\n");
    message += F("cv.stroke();\n");
    message += F("}\n");
    message += F("cv.beginPath();\n");
    message += F("cv.lineWidth = 3;\n");
    message += F("cv.moveTo(graphWidth+((i-1)*graphWidth)+graphPadding+graphxoffset, tmpTopA1);\n");
    message += F("cv.lineTo(graphWidth+((i)*graphWidth)+graphPadding+graphxoffset, tmpTopA2);\n");
    message += F("cv.strokeStyle = gradient;\n");
    message += F("cv.stroke();\n");
    message += F("cv.beginPath();\n");
    message += F("cv.lineWidth = 3;\n");
    message += F("cv.moveTo(graphWidth+((i-1)*graphWidth)+graphPadding+graphxoffset, tmpTopB1);\n");
    message += F("cv.lineTo(graphWidth+((i)*graphWidth)+graphPadding+graphxoffset, tmpTopB2);\n");
    message += F("cv.strokeStyle = \"#000000\";\n");
    message += F("cv.stroke();\n");
    message += F("}\n");
    message += F("</script></div><hr>\n");
    //##################### sende Temp Diagramm html Teil 4.3
    webServer.sendContent(message);
    message = "";
    delay(0);
  #endif

    // Abschnitt Wortuhr und Uptime
    message += F("<table>");
    message += F("<tr>");
    message += F("<td>");

    message += F("<span style=\"font-size:12px;\">\n");
    message += F("<br>");
    message += F(LANG_WORDCLOCK);
    message += F("<br>\nUptime: ");
    message += mt->convertSeconds(upTime);
    message += F("<br>\nFirmware: ");
    message += String(FIRMWARE_VERSION);
    message += " ";
    message += __DATE__;
    message += F("<br>\n");

    // QR-Code Button
    message += F("</td>");
    message += F("<td>");
    message += F("<button style=\"background: none; border: none;\" onclick=\"window.location.href='https://merz-aktuell.de/Wordclock/Doku/WortuhrBeschreibung10.x.pdf'\">");
    message += F("<img src=\"/web/qr-code.png\" alt=\"Handbuch\" width=\"120\" height=\"120\">");
    message += F("</button>");
    message += F("</td>");
    message += F("</tr>");
    message += F("</table>");

    // Abschnitt Info Button
    message += F("<hr>\n");
    message += F("<button id=\"Infobutton\" title=\"Info\" name=\"Info\" onclick=\"window.location.href='/debugClock'\">Info</button>");
    message += F("</span>");
    //##################### sende Temp Diagramm html Teil 5
    response->print(message);
    message = "";
    delay(0);
    message += F("\n<script>\n"
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
                  "});\n");

  #ifdef SHOW_MODE_MOONPHASE
    message += F("function moonphase() {"
                    "$.post(\"/control?mode="); 
    message += String(MODE_MOONPHASE);
    message += F("&sound=" SMODE_SOUND "\");"
                  "};\n");
  #endif

  #ifdef APIKEY 
    message += F("function modeexttemp() {"
                    "$.post(\"/control?mode="); 
    message += String(MODE_EXT_TEMP);
    message += F("&sound=" SMODE_SOUND "\");"
                  "};\n");
    message += F("function modeexthum() {"
                    "$.post(\"/control?mode="); 
    message += String(MODE_EXT_HUMIDITY);
    message += F("&sound=" SMODE_SOUND "\");"
                  "};\n");
    message += F("function wetter() {"
                    "$.post(\"/control?mode="); 
    message += String(MODE_WETTER);
    message += F("&sound=" SMODE_SOUND "\");"
                  "};\n");
  #endif

    message += F("</script>\n");


    // Ende Webserver
    message += F("</body></html>");
    //##################### sende letzen html Teil 6
    response->print(message);
    request->send(response);

}