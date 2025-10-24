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
#include "Modes.h"
#include "MyTime.h"
#include "OpenWeather.h"
#include "MyFileAccess.h"

#define STD_FONT 2


// Meter boarder colour
//#define TFT_GREY 0xA944
#define MAXTEXTLENGTH 20

#define XOFFSET 40
#define YOFFSET 0

#define PIC1_X 0
#define PIC1_Y 0

#define PIC2_X 90
#define PIC2_Y 0

#define PIC3_X 180
#define PIC3_Y 0

#define PIC4_X 0
#define PIC4_Y 106

#define TXT1_X 30
#define TXT1_Y 90

#define TXT2_X 120
#define TXT2_Y 90

#define TXT3_X 190
#define TXT3_Y 90

#define TXT4_X 0
#define TXT4_Y 196

#define TXT5_X 120
#define TXT5_Y 196

struct Point {
    float x;
    float y;
};

struct Line {
    Point start;
    Point end;
};


class MyWidgets {
  public:
    static MyWidgets* getInstance();
    void init();
    void drawWidget(WidgetMode mode);
    void drawExtTempHumidity(void);
    void drawWeather(void);
    void drawClockFace(void);
    void drawClockHands(uint8_t ss, uint8_t mm, uint8_t hh);
    Line computeHand(float centerX, float centerY, float length, float angleDegrees);
    void computeClockHands(uint8_t hour, uint8_t minute, uint8_t second, float ccx, float ccy, float R,
                       Line &hourHand, Line &minuteHand, Line &secondHand);
    void drawTime(void);
    void drawDate(void);
    void drawInfo(void);
    void drawDiagram(void);

  private:
    MyWidgets(void);
    void drawThermometerLevel(int x, int value, int minVal, int maxVal, const char* unit, uint16_t color);

    void drawThermometerFrame(int x, const char* label, bool isTemperature);
    void drawHumidityIcon(int x, int y);
    void drawThermometerIcon(int x, int y);

    int tubeHeight;
    int tubeWidth;
    int centerY;
    static MyWidgets *instance;
    MyTFT *tft;
    MyTime *mt;
    OpenWeather *ow;
    MyFileAccess *mfa;
    
    // Width und Height of main canvas
    uint16_t W,H;
    
    // Current Widget on TFT
    WidgetMode currentMode;
};


// Meter class
class MyMeter
{
  public:
    MyMeter(TFT_eSPI* tft);

    // Set red, orange, yellow and green start+end zones as a percentage of full scale
    void setZones(uint16_t rs, uint16_t re, uint16_t os, uint16_t oe, uint16_t ys, uint16_t ye, uint16_t gs, uint16_t ge);
    // Draw meter meter at x, y and define full scale range plus the scale labels
    void analogMeter(uint16_t x, uint16_t y, uint16_t w, uint16_t h, float fullScale, const char *title, const char *units, int div, const char *s0, const char *s1, const char *s2, const char *s3, const char *s4);
    // Draw meter meter at x, y and define full scale range plus the scale labels
    void analogMeter(uint16_t x, uint16_t y, uint16_t w, uint16_t h, float startScale, float endScale, const char *title, const char *units, int div, const char *s0, const char *s1, const char *s2, const char *s3, const char *s4);
    // Move needle to new position
    void updateNeedle(float value, uint32_t ms_delay);

  private:
    // Pointer to TFT_eSPI class functions
    TFT_eSPI* ntft;

    float ltx;         // x delta of needle start
    uint16_t osx, osy; // Saved x & y coords of needle end
    float old_analog;    // Value last displayed
    float old_digital;   // Value last displayed

    // x, y coord of top left corner of meter graphic
    uint16_t mx;
    uint16_t my;

    // needle coordinates
    uint16_t nxs, nys, nxe, nye;

    // w, h of meter
    uint16_t mw;
    uint16_t mh;
    
    // Scale factor
    float _startScale;
    float _endScale;

    // Scale label
    char     mlabel[MAXTEXTLENGTH+1];

    // Scale values
    char     ms0[5];
    char     ms1[5];
    char     ms2[5];
    char     ms3[5];
    char     ms4[5];

    // Scale colour zone start end end values
    int16_t redStart;
    int16_t redEnd;
    int16_t orangeStart;
    int16_t orangeEnd;
    int16_t yellowStart;
    int16_t yellowEnd;
    int16_t greenStart;
    int16_t greenEnd;
};

#endif