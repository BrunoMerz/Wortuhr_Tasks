/**
   IconRenderer.h
   @autor    Bruno Merz

   @version  1.0
   @created  09.01.2020
   @updated  09.01.2020

   Version history:
   V 1.0:  - Created.

*/

#pragma once


#include <Arduino.h>
#include "IconStructs.h"

#define COLOR_T uint32_t
#define LED_COLS 11
#define LED_ROWS 10

class IconRenderer {
  public:
    static IconRenderer* getInstance();
    
    void renderAndDisplay(String iconFile, int delayAfter, byte cleanUp, int16_t x_pos=-1, int16_t y_pos=-1);
    // delayAfter (ms)
    // cleanUp  0 - don't clear LED Matrix neither before nor after
    //          1 - clear LED Matrix before displaying icon
    //          2 - clear LED Matrix after displaying icon and delay
    //          3 - 1 + 2


  private:
    IconRenderer(void);
    static IconRenderer *instance;
    ICOHEADER *_icoHeader;
    ICONDIRENTRY _iconDirentry, *_pIconDirentry;
    BITMAPINFOHEADER _bitmapInfoHeader, *_pBitmapInfoHeader;
    ICON *_icon;
    uint16_t  _x_pos;
    uint16_t  _y_pos;
    uint8_t brightness;
};
