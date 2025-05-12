#if defined(WITH_SECOND_BELL)
#pragma once

#include <Arduino.h>
#include "Configuration.h"
#include "Modes.h"
//#include <Adafruit_NeoPixel.h>
//#include "LedDriver_FastLED.h"
//#include "Colors.h"


class SecondBell {
  public:
    static SecondBell* getInstance();
    void blinkSecond();
    bool getStatus(void);
    void setStatus(bool status);
    uint8_t brightness;
    

  private:
    SecondBell();
    static SecondBell *instance;
    uint8_t _aktSecond;
    bool _status;
};
#endif