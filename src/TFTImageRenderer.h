/**
   TFTIconRenderer.h
   @autor    Bruno Merz

   @version  1.0
   @created  09.01.2020
   @updated  09.01.2020

   Version history:
   V 1.0:  - Created.

*/
#pragma once

#if defined(LILYGO_T_HMI)

#include "IconStructs.h"

#define local
#include "PNGdec.h"

class TFTImageRenderer {
  public:
    static TFTImageRenderer* getInstance();
    void renderAndDisplay(char *iconFileName, int delayAfter, byte cleanUp, int16_t x_pos=-1, int16_t y_pos=-1);
    void renderAndDisplay(const __FlashStringHelper *iconFileName, int delayAfter, byte cleanUp, int16_t x_pos=-1, int16_t y_pos=-1);
    void renderAndDisplayICO(char *iconFileName, int delayAfter, byte cleanUp, int16_t x_pos=-1, int16_t y_pos=-1);
    void renderAndDisplayPNG(char *pngFileName, int delayAfter, byte cleanUp, int16_t x_pos=-1, int16_t y_pos=-1);
    // delayAfter (ms)
    // cleanUp  0 - don't clear display neither before nor after
    //          1 - clear display before displaying icon
    //          2 - clear display after displaying icon and delay
    //          3 - 1 + 2
    static void pngDraw(PNGDRAW *pDraw);

  private:
    TFTImageRenderer(void);
    static TFTImageRenderer *instance;
    //MyTFT *tft;
    //MyFileAccess *mfa;
    ICOHEADER *_icoHeader;
    ICONDIRENTRY _iconDirentry, *_pIconDirentry;
    BITMAPINFOHEADER _bitmapInfoHeader, *_pBitmapInfoHeader;
    ICON *_icon;
    uint16_t  _x_pos;
    uint16_t  _y_pos;
    
    //PNG png;
};


#endif