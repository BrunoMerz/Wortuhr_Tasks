//******************************************************************************
// LedDriver_FastLED.h
//******************************************************************************

#pragma once


#include "Configuration.h"
#include <FastLED.h>
#include "Modes.h"


class LedDriver
{
public:
    static LedDriver* getInstance();

    void clear();
    void show();
    void setPixel(uint8_t x, uint8_t y, uint32_t color, uint8_t brightness);
    void setPixel(uint8_t num, uint32_t color, uint8_t brightness);
    void setPixelRGB(uint8_t x, uint8_t y, uint8_t red, uint8_t green, uint8_t blue);
    void setPixelRGB(uint8_t num, uint8_t red, uint8_t green, uint8_t blue );
    void setPixelRGB(uint8_t x, uint8_t y, uint8_t red, uint8_t green, uint8_t blue, uint16_t brightness);
    void setPixelRGB(uint8_t x, uint8_t y, uint32_t c);
    void setDegreeOffset(uint16_t offset);
    uint16_t getDegree(uint16_t y, uint16_t x);
    uint16_t getDegree(uint16_t sec);
    uint32_t getRGBFromDegRnd(uint16_t offset, uint16_t dg);
    uint32_t getRGBFromDegRnd(uint16_t dg);
    void     shiftMatrix(void);
    uint32_t getPixelRGB(uint8_t x, uint8_t y);
    void moveScreenBufferUp(uint16_t screenBufferNew[], uint32_t color);
    void moveScreenBufferDown(uint16_t screenBufferNew[], uint32_t color);
    void moveScreenBufferLeft(uint16_t screenBufferNew[], uint32_t color);
    void moveScreenBufferRight(uint16_t screenBufferNew[], uint32_t color);
    void moveScreenBufferLeftDown(uint16_t screenBufferNew[], uint32_t color);
    void moveScreenBufferRightDown(uint16_t screenBufferNew[], uint32_t color);
    void moveScreenBufferCenter(uint16_t screenBufferNew[], uint32_t color);
    void matrix_regen(uint16_t screenBufferNew[], uint32_t color);
    void moveSeriell(uint16_t screenBufferNew[], uint32_t color, uint8_t sertype);
    void farbenmeer(uint16_t screenBufferNew[], uint32_t color);
    void regenbogen(uint16_t screenBufferNew[], uint32_t color);
    bool kreistoBuffer(uint16_t kreislinie[], uint16_t kreisflaeche[], int8_t x0, int8_t y0, int8_t r);
    void kreise(uint16_t screenBufferNew[], uint32_t color);
    void quadrate(uint16_t screenBufferNew[], uint32_t color);
    void writeScreenBufferFade(uint16_t screenBufferNew[], uint32_t color);
    void writeScreenBuffer(uint16_t screenBuffer[], uint32_t color);
    
    void setBrightnessFromLdr(void);
    void setBrightness(uint8_t brightness);
    uint8_t getBrightness(void);
    void saveMatrix(uint16_t matrix[], bool clear=false);
    void setOnOff(void);
    
    int ldrValue;
    int lastLdrValue;
    int minLdrValue;
    int maxLdrValue;
    uint8_t abcBrightness;
    uint8_t enable_bg_color;
    uint32_t bg_color;
    uint32_t corcol;
    uint32_t colorold;
    uint32_t cornercolorold;
    Mode mode;
    Mode lastMode;

private:
    LedDriver();
    static LedDriver *instance;
    CRGB strip[NUMPIXELS];
    uint16_t _offset;
    uint16_t screenBufferOld[10];
    boolean transitionInProgress;
    uint8_t brightness;
};
