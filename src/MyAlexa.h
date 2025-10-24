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

#define ESPALEXA_MAXDEVICES 5
#define ESPALEXA_ASYNC
#define ESPALEXA_DEBUG
#include "LedDriver_FastLED.h"
#include "ESPAsyncWebServer.h"
#include "Settings.h"
#include "Espalexa.h"

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