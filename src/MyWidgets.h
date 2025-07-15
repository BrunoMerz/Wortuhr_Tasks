/**
   MyWidgets.h
   @autor    Bruno Merz

   @version  1.0
   @created  17.10.2023

*/

#pragma once

#ifdef LILYGO_T_HMI

#define FlashFS LittleFS


#include "MyTFT.h"
#include "TFTImageRenderer.h"

#define STD_FONT 2

class MyWidgets {
  public:
    static MyWidgets* getInstance();
    void init();
    void drawClock(void);
    void drawTime(uint8_t ss, uint8_t mm, uint8_t hh);
    void drawInfo(void);
    void setDrawState(boolean state);

  private:
    MyWidgets(void);
    static MyWidgets *instance;
    MyTFT *tft;
    uint16_t d,r,xm,ym;
    float sx = 0, sy = 1, mx = 1, my = 0, hx = -1, hy = 0;    // Saved H, M, S x & y multipliers
    float sdeg=0, mdeg=0, hdeg=0;
    uint16_t osx, osy, omx, omy, ohx, ohy;  // Saved H, M, S x & y coords
    uint16_t x0=0, x1=0, yy0=0, yy1=0;
    uint8_t lastMM=99;
    boolean doDrawClock=true;
    uint16_t W,H,D,R,RG,RT,RD,RL,RSTD,RH,RM,RS,OX,OY;
};

#endif