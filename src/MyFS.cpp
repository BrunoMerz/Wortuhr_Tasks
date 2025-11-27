#include <Arduino.h>

//#define myDEBUG
#include "MyDebug.h"

#include "html_content.h"

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
  extern ESP8266WebServer webServer;
#else
  #include <WiFi.h>
  #include "ESPAsyncWebServer.h"
  
#endif

#include <list>
#include <LittleFS.h>
#include "Configuration.h"
#include "Languages.h"


void initFS();
void formatFS();

void initFS() {
  if ( !LittleFS.begin() ) 
  {
    DEBUG_PRINTLN(F("LittleFS Mount fehlgeschlagen"));
    if ( !LittleFS.format() )
    {
      DEBUG_PRINTLN(F("Formatierung nicht möglich"));
    }
    else
    {
      DEBUG_PRINTLN(F("Formatierung erfolgreich"));
      if ( !LittleFS.begin() )
      {
        DEBUG_PRINTLN(F("LittleFS Mount trotzdem fehlgeschlagen"));
      }
      else 
      {
        DEBUG_PRINTLN(F("LittleFS Dateisystems erfolgreich gemounted!")); 
      }
    }
  }  
  else
  {
    DEBUG_PRINTLN(F("LittleFS erfolgreich gemounted!"));
  }
  
  if ( !LittleFS.exists("/web"))
  {
    if ( LittleFS.mkdir("/web") )
    {
      DEBUG_PRINTLN(F("Verzeichnis /web erstellt"));
    }
    else
    {
      DEBUG_PRINTLN(F("Verzeichnis /web konnte nicht erstellt werden"));
    }
  } 
#if defined(LILYGO_T_HMI)
  if ( !LittleFS.exists("/tft"))
  {
    if ( LittleFS.mkdir("/tft") )
    {
      DEBUG_PRINTLN(F("Verzeichnis /tft erstellt"));
    }
    else
    {
      DEBUG_PRINTLN(F("Verzeichnis /tft konnte nicht erstellt werden"));
    }
  } 
#endif
}


void formatFS() {                                                                      // Formatiert das Filesystem
  LittleFS.format();
  initFS();
}