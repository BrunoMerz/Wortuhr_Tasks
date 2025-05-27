#include "Arduino.h"
#include "Configuration.h"
#include "Languages.h"
#include "Settings.h"
#include "Html_content.h"
#include "Helper.h"
#include "TaskStructs.h"
#include "Events.h"

//#define myDEBUG
#include "MyDebug.h"

#include "ESPAsyncWebServer.h"
#include <LittleFS.h>

extern void callRoot(AsyncWebServerRequest *request);

static Events *evt = Events::getInstance();

// ################################################################################################################
//  EVENTS
void handleEvents(AsyncWebServerRequest *request)
{
  String webarg;
  uint8_t eventnr = 0;
  if (request->arg(F("_action")) == "0")
    callRoot(request);
  if (request->arg(F("_action")) == "1")
  {
    //evt->showEventTimer = 10; // nach Speichern nächste Prüfung in 10 Sek.
    for (uint8_t eventnr = 1; eventnr <= MAXEVENTS; eventnr++)
    {
      evt->events[eventnr].aktiv = false;
    }
    for (int i = 0; i <request->args(); i++)
    {
      webarg = String(i) + ": ";
      webarg += request->argName(i) + " : ";
      webarg += request->arg(i);
      DEBUG_PRINTLN(webarg);
      if (request->argName(i).startsWith("t"))
      {
        eventnr++;
        if (eventnr <= MAXEVENTS)
        {
          evt->events[eventnr].aktiv = true;
          evt->events[eventnr].text =request->arg(i);
          evt->events[eventnr].text.trim();
        }
      }
      if (eventnr <= MAXEVENTS)
      {
        if (request->argName(i).startsWith("d"))
        {
          evt->events[eventnr].year =request->arg(i).substring(0, 4).toInt();
          evt->events[eventnr].month =request->arg(i).substring(5, 7).toInt();
          evt->events[eventnr].day =request->arg(i).substring(8).toInt();
        }
        if (request->argName(i).startsWith("s"))
        {
          evt->events[eventnr].start = (STType)request->arg(i).toInt();
        }
        String farbwert =request->arg(i);
        farbwert.toUpperCase();
        if (request->argName(i).startsWith("c"))
          evt->events[eventnr].color = string_to_num(farbwert);
        if (request->argName(i).startsWith("a"))
          evt->events[eventnr].audio_file =request->arg(i).toInt();
        if (request->argName(i).startsWith("v"))
        {
          evt->events[eventnr].preani =request->arg(i);
          evt->events[eventnr].preani.trim();
        }
        if (request->argName(i).startsWith("n"))
        {
          evt->events[eventnr].postani =request->arg(i);
          evt->events[eventnr].postani.trim();
        }
        if (request->argName(i).startsWith("i"))
          evt->events[eventnr].intervall =request->arg(i).toInt();
      }
    }
#ifdef DEBUG_EVENTS
    for (uint8_t eventnr = 0; eventnr <= MAXEVENTS; eventnr++)
    {
      if (evt->events[eventnr].aktiv)
        Serial.printf("Save Event: %i , Jahr: %i Monat: %i Tag: %i AudioNr: %i\n", eventnr, evt->events[eventnr].year, evt->events[eventnr].month, evt->events[eventnr].day, evt->events[eventnr].audio_file);
      if (evt->events[eventnr].aktiv)
        Serial.println(evt->events[eventnr].text);
    }
#endif
    bool save = evt->saveEvents();
    callRoot(request);
  }
}
