/*
   MyTouch.cpp
   @autor    Bruno Merz

   @version  1.0
   @created  29.11.2022

*/

#if defined(LILYGO_T_HMI)

//#define myDEBUG
#include "MyDebug.h"
#include "Configuration.h"
#include "MyTouch.h"

#include <SPI.h>
#include <Arduino.h>
#include <xpt2046.h>

MyTouch* MyTouch::instance = 0;


MyTouch::MyTouch() {
    _lastPressed = 0;
}

MyTouch *MyTouch::getInstance() {
  if (!instance)
  {
      instance = new MyTouch();
  }
  return instance;
}


void MyTouch::init(void) {
    SPI.begin(TOUCHSCREEN_SCLK_PIN, TOUCHSCREEN_MISO_PIN, TOUCHSCREEN_MOSI_PIN);
    mt.begin(240, 320);

    mt.setCal(180, 1841, 135, 1788, 240, 320); // Raw xmin, xmax, ymin, ymax, width, height
    //mt.setCal(285, 1788, 311, 1877, 240, 320);
    Serial.println("Use default calibration data");

    mt.setRotation(3);    
}
//#define DEBUG_TOUCH
bool MyTouch::pressed(uint16_t *x, uint16_t *y) {
    String s;
    if (mt.pressed() && _lastPressed+2000 < millis()) {
#if defined(DEBUG_TOUCH)
        s = "RAW X: ";
        s += mt.RawX();
        s += " RAW Y: ";
        s += mt.RawY();
        Serial.println(s);
        s = "X: ";
        s += mt.X();
        s += " Y: ";
        s += mt.Y();
        Serial.println(s);
#endif
        *x = mt.X();
        *y = mt.Y();
        _lastPressed = millis();
        return true;
    } else
        return false;
}

#endif