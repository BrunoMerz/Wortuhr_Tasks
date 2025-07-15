/**
   MyTft.h
   @autor    Bruno Merz

   @version  1.0
   @created  29.11.2021

*/

#pragma once

#ifdef LILYGO_T_HMI

#define FlashFS LittleFS

#include <TFT_eSPI.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include "TFTImageRenderer.h"
#include "OpenWeather.h"

class TFTIconRenderer;

#define STD_FONT 2



class MyTFT: public TFT_eSPI {
  public:
    static MyTFT* getInstance();
    void init(void);
    void drawStateLine(String text, int x_pos=0);
    void clearMainCanvas(void);
    void clearStateCanvas(uint16_t _width=0);
    void drawClock(void);
    void drawTime(void);
    uint16_t getMainCanvasWidth(void) {return _mainCanvasWidth;};
    uint16_t getMainCanvasHeight(void) {return _mainCanvasHeight;};
    TFTImageRenderer *ir;
    void printStateLine(String txt, int x_pos, uint16_t clear = 0);

  private:
    static MyTFT *instance;
      MyTFT(void): TFT_eSPI(TFT_WIDTH,TFT_HEIGHT) {
      _mainCanvasWidth = height()-50;
      _mainCanvasHeight = width() - fontHeight(STD_FONT);
      _stateLineYPos = _mainCanvasHeight;
      _buttonXPos = _mainCanvasWidth;
      };
    
    uint16_t _stateLineYPos;
    uint16_t _mainCanvasWidth;
    uint16_t _mainCanvasHeight;
    uint16_t _buttonXPos;
 
    
};

#endif
