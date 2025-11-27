/**
   MyAlexa.h
   @autor    Bruno Merz

   @version  1.0
   @created  03.10.2023
 

*/

#pragma once

#if defined(WITH_ALEXA)

#include <Arduino.h>
#include "Modes.h"

#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#else
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>

#define ESPALEXA_MAXDEVICES 2
#define ESPALEXA_ASYNC
#define ESPALEXA_DEBUG
#include "Espalexa.h"

#include "LedDriver_FastLED.h"
#include "Settings.h"

class MyAlexa {
  public:
    static MyAlexa* getInstance();
    void init(char *systemname, Mode *mode, AsyncWebServer *webServer);
    void handle(void);
    static void colorLightChanged(uint8_t brightness, uint32_t rgb);
    Espalexa espalexa;
    EspalexaDevice *espalexadev;

  private:
    MyAlexa(void);
    static MyAlexa *instance;
};

#endif