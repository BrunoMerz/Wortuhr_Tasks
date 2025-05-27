#include "Arduino.h"
#include "Configuration.h"
#include "Languages.h"
#include "Settings.h"
#include "MyTime.h"
#include "Html_content.h"
#include "Helper.h"

//#define myDEBUG
#include "MyDebug.h"

#include "ESPAsyncWebServer.h"
#include <LittleFS.h>

static Settings *settings = Settings::getInstance();
static MyTime *mt = MyTime::getInstance();

void handleButtonSettings(AsyncWebServerRequest *request)
{
  DEBUG_PRINTLN("Settings pressed.");
  
  String message = F("<!doctype html>"
                     "<html>"
                     "<head>");
  message += F("<title>");
  message += String(settings->mySettings.systemname);
  message += F(" " LANG_SETTINGS "</title>");
  message += F("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
               "<meta charset=\"UTF-8\">\n"
               "<link rel=\"icon\" type=\"image/png\" sizes=\"192x192\"  href=\"/web/android-icon-192x192.png\">\n"
               "<link rel=\"icon\" type=\"image/png\" sizes=\"32x32\" href=\"/web/favicon-32x32.png\">\n"
               "<link rel=\"icon\" type=\"image/png\" sizes=\"96x96\" href=\"/web/favicon-96x96.png\">\n"
               "<link rel=\"icon\" type=\"image/png\" sizes=\"16x16\" href=\"/web/favicon-16x16.png\">\n"
               "<style>\n"
               "body{background-color:#aaaaaa;text-align:center;color:#333333;font-family:Sans-serif;}\n"
               "button{background-color:#353746;text-align:center;line-height: 22px;color:#FFFFFF;width:150px;height:32px;margin:5px;padding:1px;border:2px solid #FFFFFF;font-size:14px;border-radius:15px;cursor: pointer;}\n"
               "h2 {color: #FFFFFF;text-shadow: 2px 2px 2px Black;}\n"
               "span {color: Black;}\n"
               "input[type=submit]{background-color:#1FA3EC;text-align:center;color:#FFFFFF;width:200px;padding:12px;border:5px solid #FFFFFF;font-size:20px;border-radius:10px;}\n"
               "table{border-collapse:collapse;margin:0px auto;} td{padding:12px;border-bottom:1px solid #ddd;} tr:first-child{border-top:1px solid #ddd;} td:first-child{text-align:right;} td:last-child{text-align:left;}\n"
               "select{font-size:16px;}\n"
               "#ssel{border-top:3px solid #aaaaaa;border-bottom:3px solid #aaaaaa;}"
               "#wt{padding:3px}"
               "</style>"
               "<script src=\"/web/jquery-3.1.1.min.js\"></script>\n"
               "</head>"
               "<body>\n");
  message += F("<h2>");
  message += String(settings->mySettings.systemname);
  message += F(" " LANG_SETTINGS "</h2>");
  // ################################################# sende html Teil 1
  //request->send(200, TEXT_HTML, message);
  //message = "";
  //delay(0);
  // ###################################################################
  message += F("<form action=\"/commitSettings\">\n"
               "<table>\n");
  // ------------------------------------------------------------------------
#if defined(BUZZER) || defined(WITH_AUDIO)
  message += F("<tr><td>");
  message += F(LANG_ALARM);
  message += F(" 1:"
               "</td><td>"
               "<input type=\"radio\" id=\"a1on\" name=\"a1\" value=\"1\"");
  if (settings->mySettings.alarm1) message += F(" checked");
  message += F("> " LANG_ON
               "\n<input type=\"radio\" id=\"a1off\" name=\"a1\" value=\"0\"");
  if (!settings->mySettings.alarm1) message += F(" checked");
  message += F("> " LANG_OFF);
  
  message += F("\n<div id=\"alarm1\"");
  if (!settings->mySettings.alarm1) message += F(" hidden");
  
  message += F(">\n<br><input type=\"time\" name=\"a1t\" value=\"");
  if (mt->hour(settings->mySettings.alarm1Time) < 10) message += F("0");
  message += String(mt->(hour(settings->mySettings.alarm1Time)); message += F(":");
  if (mt->minute(settings->mySettings.alarm1Time) < 10) message += F("0");
  message += String((mt->(settings->mySettings.alarm1Time)); message += F("\">");
  message += F(" h<br><br><small>"
               "<input type=\"checkbox\" name=\"a1w2\" value=\"4\"");
  if (bitRead(settings->mySettings.minutealarm1Weekdays, 2)) message += F(" checked");
  message += F("> ");
  message += F(LANG_MONDAY_SHORT);
  message += F(". <input type=\"checkbox\" name=\"a1w3\" value=\"8\"");
  if (bitRead(settings->mySettings.alarm1Weekdays, 3)) message += F(" checked");
  message += F("> ");
  message += F(LANG_TUESDAY_SHORT);
  message += F(". <input type=\"checkbox\" name=\"a1w4\" value=\"16\"");
  if (bitRead(settings->mySettings.alarm1Weekdays, 4)) message += F(" checked");
  message += F("> ");
  message += F(LANG_WEDNESDAY_SHORT);
  message += F(". <input type=\"checkbox\" name=\"a1w5\" value=\"32\"");
  if (bitRead(settings->mySettings.alarm1Weekdays, 5)) message += F(" checked");
  message += F("> ");
  message += F(LANG_THURSDAY_SHORT);
  message += F(". <input type=\"checkbox\" name=\"a1w6\" value=\"64\"");
  if (bitRead(settings->mySettings.alarm1Weekdays, 6)) message += F(" checked");
  message += F("> ");
  message += F(LANG_FRIDAY_SHORT);
  message += F(".<br><input type=\"checkbox\" name=\"a1w7\" value=\"128\"");
  if (bitRead(settings->mySettings.alarm1Weekdays, 7)) message += F(" checked");
  message += F("> ");
  message += F(LANG_SATURDAY_SHORT);
  message += F(". <input type=\"checkbox\" name=\"a1w1\" value=\"2\"");
  if (bitRead(settings->mySettings.alarm1Weekdays, 1)) message += F(" checked");
  message += F("> ");
  message += F(LANG_SUNDAY_SHORT);
  message += F(".\n</small></div>\n</td></tr>\n");
  // ------------------------------------------------------------------------
  message += F("<tr><td>");
  message += F(LANG_ALARM);
  message += F(" 2:"
               "</td><td>"
               "<input type=\"radio\" id=\"a2on\" name=\"a2\" value=\"1\"");
  if (settings->mySettings.alarm2) message += F(" checked");
  message += F("> " LANG_ON
               "\n<input type=\"radio\" id=\"a2off\" name=\"a2\" value=\"0\"");
  if (!settings->mySettings.alarm2) message += F(" checked");
  message += F("> " LANG_OFF);
  
  message += F("\n<div id=\"alarm2\"");
  if (!settings->mySettings.alarm2) message += F(" hidden");
  
  message += F(">\n<br><input type=\"time\" name=\"a2t\" value=\"");
  if (hour(settings->mySettings.alarm2Time) < 10) message += "0";
  message += String(hour(settings->mySettings.alarm2Time)) + ":";
  if (mt->minute(settings->mySettings.alarm2Time) < 10) message += "0";
  message += String(mt->minute(settings->mySettings.alarm2Time)) + "\">";
  message += F(" h<br><br><small>"
               "<input type=\"checkbox\" name=\"a2w2\" value=\"4\"");
  if (bitRead(settings->mySettings.alarm2Weekdays, 2)) message += F(" checked");
  message += F("> ");
  message += F(LANG_MONDAY_SHORT);
  message += F(". <input type=\"checkbox\" name=\"a2w3\" value=\"8\"");
  if (bitRead(settings->mySettings.alarm2Weekdays, 3)) message += F(" checked");
  message += F("> ");
  message += F(LANG_TUESDAY_SHORT);
  message += F(". <input type=\"checkbox\" name=\"a2w4\" value=\"16\"");
  if (bitRead(settings->mySettings.alarm2Weekdays, 4)) message += F(" checked");
  message += F("> ");
  message += F(LANG_WEDNESDAY_SHORT);
  message += F(". <input type=\"checkbox\" name=\"a2w5\" value=\"32\"");
  if (bitRead(settings->mySettings.alarm2Weekdays, 5)) message += F(" checked");
  message += F("> ");
  message += F(LANG_THURSDAY_SHORT);
  message += F(". <input type=\"checkbox\" name=\"a2w6\" value=\"64\"");
  if (bitRead(settings->mySettings.alarm2Weekdays, 6)) message += F(" checked");
  message += F("> ");
  message += F(LANG_FRIDAY_SHORT);
  message += F(".<br><input type=\"checkbox\" name=\"a2w7\" value=\"128\"");
  if (bitRead(settings->mySettings.alarm2Weekdays, 7)) message += F(" checked");
  message += F("> ");
  message += F(LANG_SATURDAY_SHORT);
  message += F(". <input type=\"checkbox\" name=\"a2w1\" value=\"2\"");
  if (bitRead(settings->mySettings.alarm2Weekdays, 1)) message += F(" checked");
  message += F("> ");
  message += F(LANG_SUNDAY_SHORT);
  message += F(".\n</small></div>\n</td></tr>\n");
  // ------------------------------------------------------------------------
  message += F("<tr><td>"
               LANG_TIMER
               ":"
               "</td><td>"
               "<select name=\"ti\">");
  for (int i = 0; i <= 10; i++)
  {
    message += F("<option value=\""); message += String(i) + "\">";
    if (i < 10) message += "0";
    message += String(i) + "</option>";
  }
  message += F("<option value=\"15\">15</option>"
               "<option value=\"20\">20</option>"
               "<option value=\"25\">25</option>"
               "<option value=\"30\">30</option>"
               "<option value=\"45\">45</option>"
               "<option value=\"60\">60</option>"
               "</select> ");
   message += F(LANG_MINUTES);
   message += F("</td></tr>\n");
  // ------------------------------------------------------------------------
#ifndef WITH_AUDIO
  message += F("<tr><td>"
               LANG_CHIME
               ":</td><td>"
               "<input type=\"checkbox\" id=\"hb\" name=\"hb\" value=\"1\"");
  if (settings->mySettings.hourBeep) message += F(" checked");
  message += F(">"
               "\n<input type=\"checkbox\" id=\"hbdummy\" name=\"hb\" value=\"0\"");
  if (!settings->mySettings.hourBeep) message += F(" checked");
  message += F(" hidden>" 
               "\n</td></tr>\n");
#endif
#endif

  // ########################################################################### sende Settings html Teil 2
  //webServer.sendContent(message);
  //message = "";
  //delay(0);
  // ########################################################################### sende Settings html Teil 2
  // ------------------------------------------------------------------------
#ifdef WITH_AUDIO
  message += F("<tr><td>"
               LANG_CHIME
               ":</td><td>"
               "<input type=\"checkbox\" id=\"hb\" name=\"hb\" value=\"1\"");
  if (settings->mySettings.hourBeep) message += F(" checked");
  message += F(">"
               "\n<input type=\"checkbox\" id=\"hbdummy\" name=\"hb\" value=\"0\"");
  if (!settings->mySettings.hourBeep) message += F(" checked");
  message += F(" hidden>"
               "\n</td></tr>"
               "<tr><td>"
               LANG_VOLUME
               ":</td><td>"
               "<select name=\"vol\" id=\"vol\">");
  for (int i = 10; i <= 100; i += 10)
  {
    message += F("<option value=\"");  message += String(i) + "\"";
    if (i == settings->mySettings.volume) message += F(" selected");
    message += F(">");
    message += String(i); message += F("</option>");
  }
  message += F("</select> %"
               "</td></tr>"
               "<tr><td>"
               LANG_CHIME
               ":</td><td>"
               LANG_RANDOM
               ":<input type=\"radio\" id=\"srandon\" name=\"srand\" value=\"1\"");
  if (settings->mySettings.randomsound ) message += F(" checked");
  message += F("> " LANG_ON
               "\n<input type=\"radio\" id=\"srandoff\" name=\"srand\" value=\"0\"");
  if (!settings->mySettings.randomsound ) message += F(" checked");
  message += F("> " LANG_OFF
               "\n<div id=\"wochentage\">"
               "<br>"
               "<table>");

  for ( uint8_t wti = 1; wti < 8; wti++)
  {
    uint8_t wt = wti;
    if ( wti == 7 ) wt = 0;
    message += F("<tr id=\"ssel\">"
                 "<td id=\"wt\">");
                 
    if ( wt == 0 ) message += F(LANG_SUNDAY);
    if ( wt == 1 ) message += F(LANG_MONDAY);
    if ( wt == 2 ) message += F(LANG_TUESDAY);
    if ( wt == 3 ) message += F(LANG_WEDNESDAY);
    if ( wt == 4 ) message += F(LANG_THURSDAY);
    if ( wt == 5 ) message += F(LANG_FRIDAY);
    if ( wt == 6 ) message += F(LANG_SATURDAY);

//    message += Wochentage[wt];
    message += F(":</td>"
                 "<td id=\"wt\">"
                 "<select name=\"wsf");
    message += String(wt);
    message += F("\" ");
    message += F(" id=\"soundfile");
    message += String(wt);
    message += F("\" >\n");
    message += F("<option value=\"0\"");
    if (settings->mySettings.weekdaysoundfile[wt] == 0) message += F(" selected");
    message += F(">");
    message += AUDIO_WEBNAME0;
    message += F("</option>\n"
                 "<option value=\"1\"");
    if (settings->mySettings.weekdaysoundfile[wt] == 1) message += F(" selected");
    message += F(">");
    message += AUDIO_WEBNAME1;
    message += F("</option>\n"

                 "<option value=\"2\"");
    if (settings->mySettings.weekdaysoundfile[wt] == 2) message += F(" selected");
    message += F(">");
    message += AUDIO_WEBNAME2;
    message += F("</option>\n"

                 "<option value=\"3\"");
    if (settings->mySettings.weekdaysoundfile[wt] == 3) message += F(" selected");
    message += F(">");
    message += AUDIO_WEBNAME3;
    message += F("</option>\n"

                 "<option value=\"4\"");
    if (settings->mySettings.weekdaysoundfile[wt] == 4) message += F(" selected");
    message += F(">");
    message += AUDIO_WEBNAME4;
    message += F("</option>\n"

                 "<option value=\"5\"");
    if (settings->mySettings.weekdaysoundfile[wt] == 5) message += F(" selected");
    message += F(">");
    message += AUDIO_WEBNAME5;
    message += F("</option>\n"

                 "<option value=\"6\"");
    if (settings->mySettings.weekdaysoundfile[wt] == 6) message += F(" selected");
    message += F(">");
    message += AUDIO_WEBNAME6;
    message += F("</option>\n"

                 "<option value=\"7\"");
    if (settings->mySettings.weekdaysoundfile[wt] == 7) message += F(" selected");
    message += F(">");
    message += AUDIO_WEBNAME7;
    message += F("</option>\n"

                 "<option value=\"8\"");
    if (settings->mySettings.weekdaysoundfile[wt] == 8) message += F(" selected");
    message += F(">");
    message += AUDIO_WEBNAME8;
    message += F("</option>\n"

                 "<option value=\"9\"");
    if (settings->mySettings.weekdaysoundfile[wt] == 9) message += F(" selected");
    message += F(">");
    message += AUDIO_WEBNAME9;
    message += F("</option>\n"

                 "<option value=\"10\"");
    if (settings->mySettings.weekdaysoundfile[wt] == 10) message += F(" selected");
    message += F(">");
    message += AUDIO_WEBNAME10;
    message += F("</option>\n"

                 "<option value=\"11\"");
    if (settings->mySettings.weekdaysoundfile[wt] == 11) message += F(" selected");
    message += F(">");
    message += AUDIO_WEBNAME11;
    message += F("</option>\n"

                 "<option value=\"12\"");
    if (settings->mySettings.weekdaysoundfile[wt] == 12) message += F(" selected");
    message += F(">");
    message += AUDIO_WEBNAME12;
    message += F("</option>\n"

                 "<option value=\"13\"");
    if (settings->mySettings.weekdaysoundfile[wt] == 13) message += F(" selected");
    message += F(">");
    message += AUDIO_WEBNAME13;
    message += F("</option>\n"

                 "<option value=\"14\"");
    if (settings->mySettings.weekdaysoundfile[wt] == 14) message += F(" selected");
    message += F(">");
    message += AUDIO_WEBNAME14;
    message += F("</option>\n"

                 "<option value=\"15\"");
    if (settings->mySettings.weekdaysoundfile[wt] == 15) message += F(" selected");
    message += F(">");
    message += AUDIO_WEBNAME15;
    message += F("</option>\n"

                 "</select>"
                 "</td>");
    message += F("<td id=\"wt\">"
                 "<input style=\"background-color:#353746; margin:3px; font-size:14px; border-radius:7px;\" title=\"Soundcheck\" id=\"soundcheck");
    message += String(wt);
    message += F("\" type=\"button\" value=\"&#128266\">"
                 "</td>\n");
    message += F("</tr>");
    // ######################################################### sende html Wochentage Teile
    //webServer.sendContent(message);
    //message = "";
    //delay(0);
    // ###########################################################################
  }
  message += F("</table></div>\n");
  message += F("</td></tr>\n");

  message += F("<tr><td>"
               LANG_LOUDER_ON_WEEKEND
               ":</td><td>"
               "<input type=\"checkbox\" id=\"wsl\" name=\"wsl\" value=\"1\"");
  if (settings->mySettings.weekendlater) message += F(" checked");
  message += F(">"
               "\n<input type=\"checkbox\" id=\"wsldummy\" name=\"wsl\" value=\"0\"");
  if (!settings->mySettings.weekendlater) message += F(" checked");
  message += F(" hidden>"
               "\n</td></tr>\n");

  message += F("<tr><td>"
               LANG_SINGLEGONG
               ":</td><td>"
               "<input type=\"checkbox\" id=\"sgon\" name=\"sg\" value=\"1\"");
  if (settings->mySettings.singlegong) message += F(" checked");
  message += F(">"
               "\n<input type=\"checkbox\" id=\"sgondummy\" name=\"sg\" value=\"0\"");
  if (!settings->mySettings.singlegong) message += F(" checked");
  message += F(" hidden>"
               "\n</td></tr>\n");

  message += F("<tr><td>"
               LANG_VICKYHANS_FORMAT
               ":</td><td>"
               "<input type=\"radio\" name=\"vh24\" id=\"vh24on\" value=\"1\"");
  if (settings->mySettings.vickihans24h) message += F(" checked");
  message += F("> 24h "
               "<input type=\"radio\" name=\"vh24\" id=\"vh24off\" value=\"0\"");
  if (!settings->mySettings.vickihans24h) message += F(" checked");
  message += F("> 12h"
               "</td></tr>\n");

  message += F("<tr><td>"
               LANG_SPEAKER
               ":</td><td>"
               "<input type=\"radio\" name=\"sprech\" value=\"1\"");
  if (settings->mySettings.sprecher) message += F(" checked");
  message += F("> Vicki "
               "<input type=\"radio\" name=\"sprech\" value=\"0\"");
  if (!settings->mySettings.sprecher) message += F(" checked");
  message += F("> Hans"
               "</td></tr>\n");

#endif
  // ------------------------------------------------------------------------
#if defined(RTC_BACKUP) || defined(SENSOR_BME280)
  message += F("<tr><td>"
               LANG_DISPLAY
               "<br> "
               LANG_TEMPERATURE
               " "
#ifdef SHOW_MODE_MOONPHASE
               LANG_MOONPHASE
               " "
#endif
#ifdef SHOW_MODE_WETTER
               LANG_WEATHER
               " "
#endif
#ifdef SHOW_MODE_DATE
               LANG_DATE
               " "
#endif
               ":</td><td>"
               "<input type=\"radio\" id=\"mcon\" name=\"mc\" value=\"1\"");
  if (settings->mySettings.modeChange) message += F(" checked");
  message += F("> " LANG_ON
               "\n<input type=\"radio\" id=\"mcoff\" name=\"mc\" value=\"0\"");
  if (!settings->mySettings.modeChange) message += F(" checked");
  message += F("> " LANG_OFF);
  message += F("\n<div id=\"amct\"> "
              LANG_EVERY
                "<input type=\"number\" style=\"width: 40px;\" value=\"");
  message += String(settings->mySettings.auto_mode_change);
  message += F("\" name=\"amct\" min=\"2\" max=\"180\">"
               " min</div>");
  message += F("</td></tr>\n");
#endif
  // ------------------------------------------------------------------------
#if defined(SunRiseLib) || defined(APIKEY)
  message += F("<tr><td>"
               LANG_SUNRISE_SUNSET
               ":</td><td>"
               "<br><input type=\"checkbox\" id=\"sunr\" name=\"sunr\" value=\"1\"");
  if (settings->mySettings.ani_sunrise) message += F(" checked");
  message += F(">"
               "\n<input type=\"checkbox\" id=\"sunrdummy\" name=\"sunr\" value=\"0\"");
  if (!settings->mySettings.ani_sunrise) message += F(" checked");
  message += F(" hidden>"
               "\n<br>"
               "<input type=\"checkbox\" id=\"suns\" name=\"suns\" value=\"1\"");
  if (settings->mySettings.ani_sunset) message += F(" checked");
  message += F(">"
               "\n<input type=\"checkbox\" id=\"sunsdummy\" name=\"suns\" value=\"0\"");
  if (!settings->mySettings.ani_sunset) message += F(" checked");
  message += F(" hidden>"
               "\n</td></tr>\n");
#endif
  // ------------------------------------------------------------------------
#ifdef LDR

    message += F("<tr><td>"
                LANG_AUTO_BRIGHTNESS
                ":</td><td>"
                "<input type=\"checkbox\" id=\"ab\" name=\"ab\" value=\"1\"");
    if (settings->mySettings.useAbc) message += F(" checked");
    message += F(">"
                "\n<input type=\"checkbox\" id=\"abdummy\" name=\"ab\" value=\"0\"");
    if (!settings->mySettings.useAbc) message += F(" checked");
    message += F(" hidden>"
                "\n</td></tr>\n");

#endif
  // ------------------------------------------------------------------------
#if defined(WITH_SECOND_HAND) || defined(WITH_SECOND_BELL)
  message += F("<tr><td>" LANG_SECONDSETTINGS );
  
#if defined(WITH_SECOND_BELL)
  message += F("- <br>" LANG_SECONDSBELL ":");
#endif

#if defined(WITH_SECOND_HAND)
  message += F("<br>" LANG_SECONDHAND ":");
#endif

#if defined(WITH_SECOND_BELL)
  message += F("</td><td>"
    "<br><input type=\"checkbox\" id=\"sb\" name=\"sb\" value=\"1\"");
  if (settings->mySettings.secondsBell) message += F(" checked");
  message += F(">"
    "\n<input type=\"checkbox\" id=\"sbdummy\" name=\"sb\" value=\"0\"");
  if (!settings->mySettings.secondsBell) message += F(" checked");
  message += F(" hidden>");
#endif

#if defined(WITH_SECOND_HAND)
  message += F("\n<br>"
              "<input type=\"checkbox\" id=\"sh\" name=\"sh\" value=\"1\"");
  if (settings->mySettings.secondHand) message += F(" checked");
  message += F(">"
              "\n<input type=\"checkbox\" id=\"shdummy\" name=\"sh\" value=\"0\"");
  if (!settings->mySettings.secondHand) message += F(" checked");
  message += F(" hidden>");
  message += F(LANG_SECONDSALL);
  message += F("<input type=\"checkbox\" id=\"sa\" name=\"sa\" value=\"1\"");
  if (settings->mySettings.secondsAll) message += F(" checked");
  message += F(">"
              "\n<input type=\"checkbox\" id=\"sadummy\" name=\"sa\" value=\"0\"");
  if (!settings->mySettings.secondsAll) message += F(" checked");
  message += F(" hidden>");
#endif

  message += F("\n</td></tr>\n");
#endif
  // ------------------------------------------------------------------------
  message += F("<tr><td>"
               LANG_BRIGHTNESS
               ":</td><td>"
               "<select name=\"br\">");
  for (int i = 10; i <= 100; i += 10)
  {
    message += F("<option value=\"");
    message += String(i) + "\"";
    if (i == settings->mySettings.brightness) message += F(" selected");
    message += F(">");
    message += String(i);
    message += F("</option>");
  }
  message += F("</select> %"
               "</td></tr>\n");
  // ---------------------LedFarbe---------------------------------------
  message += F("<tr><td>"
               LANG_COLOURS
               ":</td><td>"
               "<input type=\"color\" name=\"ledcol\" value=\"");
  message += num_to_string(settings->mySettings.ledcol);
  message += F("\">"
               "</td></tr>\n");
  //-----------------------------------------------------------------------
  //##################################### sende Settings html script Teil 3
  //webServer.sendContent(message);
  //message = "";
  //delay(0);
  // ######################################################################
  message += F("<tr><td>"
               LANG_MINUTE_COLOUR
               ":</td><td>"
               "<input type=\"color\" name=\"corcol\" value=\"");
  message += num_to_string(settings->mySettings.corcol);
  message += F("\">"
               "</td></tr>\n");
  // Farbenwechsel-------------------------------------------------------------
  message += F("<tr><td>"
               LANG_COLOUR_CHANGE
               ":</td><td>"
               "<input type=\"radio\" name=\"cc\" value=\"3\"");
  if (settings->mySettings.colorChange == 3) message += F(" checked");
  message += F("> "
               LANG_DAILY
               " <input type=\"radio\" name=\"cc\" value=\"2\"");
  if (settings->mySettings.colorChange == 2) message += F(" checked");
  message += F("> "
               LANG_HOURLY
               " <input type=\"radio\" name=\"cc\" value=\"1\"");
  if (settings->mySettings.colorChange == 1) message += F(" checked");
  message += F("> 5 Min. "
               "<input type=\"radio\" name=\"cc\" value=\"0\"");
  if (settings->mySettings.colorChange == 0) message += F(" checked");
  message += F("> "
               LANG_OFF
               "</td></tr>\n");
  // ------------------------------------------------------------------------
  message += F("<tr><td>"
               LANG_MINUTE_COLOUR_CHANGE
               ":</td><td>"
               "<input type=\"radio\" name=\"ccc\" value=\"4\"");
  if (settings->mySettings.corner_colorChange == 4) message += F(" checked");
  message += F("> "
               LANG_LIKEMAIN
               "<br>"
               "<input type=\"radio\" name=\"ccc\" value=\"3\"");
  if (settings->mySettings.corner_colorChange == 3) message += F(" checked");
  message += F("> "
               LANG_DAILY
               " <input type=\"radio\" name=\"ccc\" value=\"2\"");
  if (settings->mySettings.corner_colorChange == 2) message += F(" checked");
  message += F("> "
               LANG_HOURLY
               " <input type=\"radio\" name=\"ccc\" value=\"1\"");
  if (settings->mySettings.corner_colorChange == 1) message += F(" checked");
  message += F("> 5 Min. "
               "<input type=\"radio\" name=\"ccc\" value=\"0\"");
  if (settings->mySettings.corner_colorChange == 0) message += F(" checked");
  message += F("> "
               LANG_OFF
               "</td></tr>\n");
  // ------------------------------------------------------------------------
  message += F("<tr><td>"
               LANG_BACKGROUNDCOLOUR
               ":</td><td>"
               "<input type=\"radio\" name=\"bgce\" id=\"bgce_off\" value=\"0\"");
  if (settings->mySettings.enable_bg_color == 0) message += F(" checked");
  message += F("> "
               LANG_OFF
               " <input type=\"radio\" name=\"bgce\" id=\"bgce_zeit\" value=\"1\"");
  if (settings->mySettings.enable_bg_color == 1 ) message += F(" checked");
  message += F("> "
               LANG_TIME
               " <input type=\"radio\" name=\"bgce\" id=\"bgce_immer\" value=\"2\"");
  if (settings->mySettings.enable_bg_color == 2 ) message += F(" checked");
  message += F("> ");
  message += F(LANG_ALWAYS);
  message += F("  <input type=\"color\" color=\"");
  message += num_to_string(settings->mySettings.bg_color);
  message += F("\" value=\"");
  message += num_to_string(settings->mySettings.bg_color);
  message += F("\" name=\"bgc\" id=\"bgcolor\" >"

               "</td></tr>\n");
  // ------------------------------------------------------------------------

  //######################################################### sende Settings html Teil 4
  //webServer.sendContent(message);
  //message = "";
  //delay(0);
  //#########################################################

#ifndef FRONTCOVER_BINARY
  message += F("<tr><td>"
               LANG_TRANSITIONS
               ":</td><td>"
               "<select name=\"tr\">"
               "<optgroup label=\""
               LANG_SINGLE
               ":\">"
               "<option value=\"0\"");
  if (settings->mySettings.transition == 0) message += F(" selected");
  message += F(">"
               LANG_TRANSITION_00
               "</option>"
               "<option value=\"1\"");
  if (settings->mySettings.transition == 1) message += F(" selected");
  message += F(">"
               LANG_TRANSITION_01
               "</option>"
               "<option value=\"2\"");
  if (settings->mySettings.transition == 2) message += F(" selected");
  message += F(">"
               LANG_TRANSITION_02
               "</option>"
               "<option value=\"3\"");
  if (settings->mySettings.transition == 3) message += F(" selected");
  message += F(">"
               LANG_TRANSITION_03
               "</option>"
               "<option value=\"4\"");
  if (settings->mySettings.transition == 4) message += F(" selected");
  message += F(">"
               LANG_TRANSITION_04
               "</option>"
               "<option value=\"5\"");
  if (settings->mySettings.transition == 5) message += F(" selected");
  message += F(">"
               LANG_TRANSITION_05
               "</option>"
               "<option value=\"6\"");
  if (settings->mySettings.transition == 6) message += F(" selected");
  message += F(">"
               LANG_TRANSITION_06
               "</option>"
               "<option value=\"7\"");
  if (settings->mySettings.transition == 7) message += F(" selected");
  message += F(">"
               LANG_TRANSITION_07
               "</option>"
               "<option value=\"8\"");
  if (settings->mySettings.transition == 8) message += F(" selected");
  message += F(">"
               LANG_TRANSITION_08
               "</option>"
               "<option value=\"9\"");
  if (settings->mySettings.transition == 9) message += F(" selected");
  message += F(">"
               LANG_TRANSITION_09
               "</option>"
               "<option value=\"10\"");
  if (settings->mySettings.transition == 10) message += F(" selected");
  message += F(">"
               LANG_TRANSITION_10
               "</option>"
               "<option value=\"11\"");
  if (settings->mySettings.transition == 11) message += F(" selected");
  message += F(">"
               LANG_TRANSITION_11
               "</option>"
               "<option value=\"12\"");
  if (settings->mySettings.transition == 12) message += F(" selected");
  message += F(">"
               LANG_TRANSITION_12
               "</option>"
               "<option value=\"13\"");
  if (settings->mySettings.transition == 13) message += F(" selected");
  message += F(">"
               LANG_TRANSITION_13
               "</option>"
               "<option value=\"14\"");
  if (settings->mySettings.transition == 14) message += F(" selected");
  message += F(">"
               LANG_TRANSITION_14
               "</option>"
               "<option value=\"15\"");
  if (settings->mySettings.transition == 15) message += F(" selected");
  message += F(">"
               LANG_TRANSITION_15
               "</option>"
               "<option value=\"16\"");
  if (settings->mySettings.transition == 16) message += F(" selected");
  message += F(">"
               LANG_TRANSITION_16
               "</option>"
               "<option value=\"17\"");
  if (settings->mySettings.transition == 17) message += F(" selected");
  message += F(">"
               LANG_TRANSITION_17
               "</option>"
               "</optgroup>"
               "<optgroup label=\""
               LANG_ALL
               ":\">"
               "<option value=\"20\"");
  if (settings->mySettings.transition == 20) message += F(" selected");
  message += F(">"
               LANG_ONE_AFTER_ANOTHER
               "</option>"
               "<option value=\"21\"");
  if (settings->mySettings.transition == 21) message += F(" selected");
  message += F(">"
               LANG_RANDOM
               "</option>"
               "</optgroup>"

               "</select>"
               "</td></tr>\n");
#endif

  message += F("<tr><td>"
               LANG_NIGHT_OFF
               ":</td><td>"
               "<input type=\"time\" name=\"no\" value=\"");
  int h=settings->mySettings.nightOffTime / 3600;
  int m=(settings->mySettings.nightOffTime % 3600) / 60;
  if (h < 10) message += "0";
  message += String(h) + ":";
  if (m  < 10) message += "0";
  message += String(m);
  message += F("\"> h</td></tr>\n");
  // ------------------------------------------------------------------------
  message += F("<tr><td>"
               LANG_DAY_ON
               ":</td><td>"
               "<input type=\"time\" name=\"do\" value=\"");
  h=settings->mySettings.dayOnTime / 3600;
  m=(settings->mySettings.dayOnTime % 3600) / 60;
  if (h < 10) message += "0";
  message += String(h) + ":";
  if (m < 10) message += "0";
  message += String(m);
  message += F("\"> h</td></tr>\n");
 // ------------------------------------------------------------------------
  message += F("<tr><td>"
               LANG_SHOWITIS
               ":</td><td>"
               "<input type=\"checkbox\" id=\"ii\" name=\"ii\" value=\"1\"");
  if (settings->mySettings.itIs) message += F(" checked");
  message += ">"
             "\n<input type=\"checkbox\" id=\"iidummy\" name=\"ii\" value=\"0\"";
  if (!settings->mySettings.itIs) message += F(" checked");
  message += " hidden>" 
             "\n</td></tr>\n";
  
#if defined(FRONTCOVER_DE_DE) || defined(FRONTCOVER_DE_SW) || defined(FRONTCOVER_DE_BA) || defined(FRONTCOVER_DE_SA)
 // Dialekt ------------------------------------------------------------------
  message += F("<tr><td>"
               "Dialekt"
               ":</td><td>"
               "<input type=\"radio\" name=\"di\" value=\"0\"");
  if (settings->mySettings.language == FC_DE_DE) message += F(" checked");
  message += F("> "
               "DE"
               " <input type=\"radio\" name=\"di\" value=\"1\"");
  if (settings->mySettings.language == FC_DE_SW) message += F(" checked");
  message += F("> "
               "SW"
               " <input type=\"radio\" name=\"di\" value=\"2\"");
  if (settings->mySettings.language == FC_DE_BA) message += F(" checked");
  message += F("> BA "
               "<input type=\"radio\" name=\"di\" value=\"3\"");
  if (settings->mySettings.language == FC_DE_SA) message += F(" checked");
  message += F("> "
               "SA"
               "</td></tr>\n");
#endif

  // ------------------------------------------------------------------------
  // Zeit
  message += F("<tr><td>"
               LANG_TIME "<br>" LANG_SETDATETIME ":<br>"
               LANG_TIMEHOST
               ":<br>"
               LANG_TIMEZONE
               ":");

  message += F("</td><td><br>");  
  message += F("<input type=\"datetime-local\" name=\"st\">"
               "<br>");
  // ------------------------------------------------------------------------
  message += F("<input type=\"search\" list=\"ntpserver\" placeholder=\"");
  message += String(settings->mySettings.ntphost);
  message += F("\" name=\"ntphost\">");
  message += F("<datalist id=\"ntpserver\">"
               "<option value=\"fritz.box\">"
               "<option value=\"time.google.com\">"
               "<option value=\"ptbtime1.ptb.de\">"
               "<option value=\"pool.ntp.org\">"
               "<option value=\"ntp.web.de\">"
               "</datalist>"
               "<br>");
  // ------------------------------------------------------------------------
  message += F("<input type=\"text\" value=\"");
  message += String(settings->mySettings.timezone);
  message += F("\" name=\"timezone\" minlength=\"3\" maxlength=\"29\" size=\"24\">");
  message += F("</td></tr>\n");
    // ------------------------------------------------------------------------
  message += F("<tr><td>"
               LANG_SYSTEMNAME
               ":</td><td>");
  message += F("<input type=\"text\" value=\"");
  message += String(settings->mySettings.systemname);
  message += F("\" name=\"sysname\" minlength=\"3\" maxlength=\"29\">"
               "</td></tr>\n");
  // ------------------------------------------------------------------------
#if defined(SunRiseLib) || defined(APIKEY)
  message += F("<tr><td>"
               "OpenWeather<br>ApiKey:<br>"
               LANG_LOCATION
               ":<br>"
               LANG_ALTITUDE
               ":");
#ifdef SunRiseLib
  message += F("<br>"
               LANG_LATITUDE
               ":<br>"
               LANG_LONGITUDE
               ":");
#endif
  message += F("</td><td><br>");
  message += F("<input type=\"text\" value=\"");
  message += String(settings->mySettings.openweatherapikey);
  message += F("\" name=\"owkey\" minlength=\"9\" maxlength=\"34\" size=\"32\">"
               "<br><input type=\"text\" value=\"");
  message += String(settings->mySettings.openweatherlocation);
  message += F("\" name=\"owloc\" minlength=\"3\" maxlength=\"39\">"
               "<br><input type=\"number\" value=\"");
  message += String(settings->mySettings.standort_hoehe);
  message += F("\" name=\"hoehe\" min=\"0\" max=\"10000\">"
               " m");
#ifdef SunRiseLib
  message += F("<br><input type=\"number\" value=\"");
  message += String(settings->mySettings.latitude);
  message += F("\" name=\"latitude\" min=\"-90\" max=\"90\" step=0.01 >"
               "<br><input type=\"number\" value=\"");
  message += String(settings->mySettings.longitude);
  message += F("\" name=\"longitude\" min=\"-180\" max=\"180\" step=0.01 >");
#endif
  message += F("</td></tr>\n");

#endif
  // ------------------------------------------------------------------------
  // Highscore Reset
  message += F("<tr><td>"
               LANG_RESET_HIGHSCORE
               ":</td><td>"
               "<input type=\"checkbox\" name=\"hsres\" value=\"1\">"
               "</td></tr>\n");

  // ------------------------------------------------------------------------
  // MQTT 
  #if defined(WITH_MQTT)
  message += F("<tr><td>"
               "MQTT<br>" LANG_MQTTSV ":<br>"
               LANG_MQTTPO
               ":<br>"
               LANG_MQTTBE
               ":<br>"
               LANG_MQTTPW
               ":");

  message += F("</td><td><br>");
  message += F("<input type=\"text\" value=\"");
  message += String(settings->mySettings.mqtt_server);
  message += F("\" name=\"mqsv\" maxlength=\"30\" size=\"32\">"
               "<br><input type=\"number\" value=\"");
  message += String(settings->mySettings.mqtt_port);
  message += F("\" name=\"mqpo\" maxlength=\"5\">"
               "<br><input type=\"text\" value=\"");
  message += String(settings->mySettings.mqtt_user);
  message += F("\" name=\"mqbe\" maxlength=\"30\">");
  message += F("<br><input type=\"text\" value=\"");
  message += String(settings->mySettings.mqtt_password);
  message += F("\" name=\"mqpw\" maxlength=\"30\">");
  message += F("</td></tr>\n");
#endif

  // ------------------------------------------------------------------------
  // Logging on/off
  /*
  message += F("<tr><td>"
               LANG_LOGGING
               ":</td><td>"
               "<input type=\"checkbox\" id=\"log\" name=\"log\" value=\"1\"");
  if (settings->mySettings.logging) message += F(" checked");
  message += ">"
             "\n<input type=\"checkbox\" id=\"logdummy\" name=\"log\" value=\"0\"";
  if (!settings->mySettings.logging) message += F(" checked");
  message += " hidden>" 
             "\n</td></tr>\n";
*/
  // ------------------------------------------------------------------------
  message += F("</table>\n"
               "<br><button title=\"Explorer\" type=\"button\" onclick=\"window.location.href='/fs'\"><span style=\"color:White;font-size:14px;\">&#128193;LittleFS</span></button>"
               "<br><button title=\"Explorer\" type=\"button\" onclick=\"window.location.href='/update'\"><span style=\"color:White;font-size:14px;\">&#128193;" LANG_INSTALL "</span></button>"
               "<hr>"
               "<br><button title=\"" LANG_BACK "\" type=\"button\" onclick=\"window.location.href='/'\"><span style=\"color:White;font-size:14px;\">&#128281; " LANG_BACK "</span></button>"
               "<button title=\"" LANG_SAVE "\"><span style=\"color:White;font-size:14px;\">&#128077; " LANG_SAVE "</span></button>"
               "</form>\n");
  //##################### sende Settings html script Teil1
  //DEBUG_PRINTLN(message);
  //webServer.sendContent(message);
  //message = "";
  //delay(0);
  message += F("<script>\n"
               "var urlBase = \"/\";\n");
// Alarm1
  message += F("$(\"#a1on\").click(function(){"
               "$(\"#alarm1\").show();"
               "});\n"
               "$(\"#a1off\").click(function(){"
               "$(\"#alarm1\").hide();"
               "});\n");
// Alarm2
  message += F("$(\"#a2on\").click(function(){"
               "$(\"#alarm2\").show();"
               "});\n"
               "$(\"#a2off\").click(function(){"
               "$(\"#alarm2\").hide();"
               "});\n");
               
  for ( uint8_t wti = 0; wti < 7; wti++)
  {

    message += F("$(\"#soundcheck");
    message += String(wti);
    message += F("\").click(function() {");
    message += F("if ( $(\"#vh24on\").prop('checked') ) vh24=1;");
    message += F("else vh24=0;");
    message += F("if ( $(\"#sgon\").prop('checked') ) sg=1;");
    message += F("else sg=0;");
    message += F("var posturl = urlBase + \"AudioTest?soundfile=\" + $(\"#soundfile");
    message += String(wti);
    message += F("\").val() + \"&volume=\" + $(\"#vol\").val() + \"&vh24=\" + vh24 + \"&sg=\" + sg;"
                 "$.post(posturl);"
                 "});\n");
  }
  //##################### sende Settings html script Teil2
  //webServer.sendContent(message);
  //message = "";
  //elay(0);
  message += F("$(\"#bgce_off\").click(function(){"
               "var posturl = urlBase + \"BackgroundColor?bgce=off\";\n"
               "$.post(posturl);"
               "});\n");
  message += F("$(\"#bgce_zeit\").click(function(){"
               "var posturl = urlBase + \"BackgroundColor?bgce=zeit\";\n"
               "$.post(posturl);"
               "});\n");
  message += F("$(\"#bgce_immer\").click(function(){"
               "var posturl = urlBase + \"BackgroundColor?bgce=immer\";\n"
               "$.post(posturl);"
               "});\n");
  message += F("$(\"#bgcolor\").change(function(){"
               "bgcolor = this.value;"
               "$(this).attr(\"color\",bgcolor);"
               //    "console.log(\"bgcolor: \" + bgcolor);"
               "var posturl = urlBase + \"BackgroundColor?bgcolor=\" + bgcolor.slice(1, 7).toUpperCase();\n"
               "$.post(posturl);"
               "});\n");
// Display temp,mondphase,wetter,datum
  message += F("$(\"#mcon\").click(function(){"
               "$(\"#amct\").show();"
               "});\n"
               "$(\"#mcoff\").click(function(){"
               "$(\"#amct\").hide();"
               "});\n"

               "$(\"#srandoff\").click(function(){"
               "$(\"#wochentage\").show();"
               "});\n"
               "$(\"#srandon\").click(function(){"
               "$(\"#wochentage\").hide();"
               "});\n");
//##################### sende Settings html script Teil3              
  //webServer.sendContent(message);
  //message = "";
  //delay(0);
// hb
  message += F("$(\"#hb\").click(function(){if($(this).prop('checked')) $(\"#hbdummy\").prop('checked', false); else $(\"#hbdummy\").prop('checked', true);});");
// wsl
  message += F("$(\"#wsl\").click(function(){if($(this).prop('checked')) $(\"#wsldummy\").prop('checked', false); else $(\"#wsldummy\").prop('checked', true);});");      
// sgon
  message += F("$(\"#sgon\").click(function(){if($(this).prop('checked')) $(\"#sgondummy\").prop('checked', false); else $(\"#sgondummy\").prop('checked', true);});");
// sunr
  message += F("$(\"#sunr\").click(function(){if($(this).prop('checked')) $(\"#sunrdummy\").prop('checked', false); else $(\"#sunrdummy\").prop('checked', true);});");
// suns
  message += F("$(\"#suns\").click(function(){if($(this).prop('checked')) $(\"#sunsdummy\").prop('checked', false); else $(\"#sunsdummy\").prop('checked', true);});");      
// ab
  message += F("$(\"#ab\").click(function(){if($(this).prop('checked')) $(\"#abdummy\").prop('checked', false); else $(\"#abdummy\").prop('checked', true);});");
#ifdef WITH_SECOND_HAND
// sh
  message += F("$(\"#sh\").click(function(){if($(this).prop('checked')) $(\"#shdummy\").prop('checked', false); else $(\"#shdummy\").prop('checked', true);});");
// sa
  message += F("$(\"#sa\").click(function(){if($(this).prop('checked')) $(\"#sadummy\").prop('checked', false); else $(\"#sadummy\").prop('checked', true);});");
#endif
#ifdef WITH_SECOND_BELL
// sb
  message += F("$(\"#sb\").click(function(){if($(this).prop('checked')) $(\"#sbdummy\").prop('checked', false); else $(\"#sbdummy\").prop('checked', true);});");
#endif
// zeige "Es ist"
  message += F("$(\"#ii\").click(function(){if($(this).prop('checked')) $(\"#iidummy\").prop('checked', false); else $(\"#iidummy\").prop('checked', true);});");
// zeige Logging
  message += F("$(\"#log\").click(function(){if($(this).prop('checked')) $(\"#logdummy\").prop('checked', false); else $(\"#logdummy\").prop('checked', true);});");
             
  message += F("document.addEventListener('DOMContentLoaded', function() {"
               "if ( $(\"#mcoff\").prop('checked') )  $(\"#amct\").hide();"
               "else $(\"#amct\").show();\n"

               "if ( $(\"#srandon\").prop('checked') )  $(\"#wochentage\").hide();"
               "else $(\"#wochentage\").show();\n"

               "});\n"
               "</script>\n"
               "</body>\n</html>");
    //##################### sende letzen html Teil 5
    request->send(200, TEXT_HTML, message);

}


