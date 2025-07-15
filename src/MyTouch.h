/**
   MyTouch.h
   @autor    Bruno Merz

   @version  1.0
   @created  29.11.2022

*/

#pragma once

#ifdef LILYGO_T_HMI

#include <Arduino.h>
#include <xpt2046.h>

class MyTouch {
  public:
    static MyTouch* getInstance();
    void init(void);
    bool pressed(uint16_t *x, uint16_t *y);

  private:
    MyTouch();
    static MyTouch *instance;
    XPT2046 mt = XPT2046(SPI, TOUCHSCREEN_CS_PIN, TOUCHSCREEN_IRQ_PIN);
    uint32_t _lastPressed;
};

#endif
