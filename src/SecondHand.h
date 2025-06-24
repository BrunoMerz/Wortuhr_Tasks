#if defined(WITH_SECOND_HAND)
#pragma once

#include <Arduino.h>
#include "Configuration.h"
#include "LedDriver_FastLED.h"
#include "Colors.h"

class SecondHand {
  public:
    static SecondHand* getInstance();
    void init();
    void drawSecond(uint16_t sec);
    void drawSecond(void);
    void clearSecond(uint16_t sec);
    void clearAllSeconds(void);
    void drawQuarter(void);
    void drawQuarter(uint16_t quarter);
    void setQuarterWidth(uint16_t w);
    void show(uint16_t dly=0);
    boolean exists(void);
  
  private:
    SecondHand();
    static SecondHand *instance;

    CRGB _sec[NUMSECONDS];

    int mapSecondNo(uint16_t sec);
    uint8_t  _quarterWidth;
    uint8_t _lastSecond;
    uint8_t _aktSecond;
    boolean _doUpdate;
    boolean _exists;
};
#endif