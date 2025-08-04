//******************************************************************************
// LedDriver_FastLED.cpp
//******************************************************************************

#include <Arduino.h>
#include <driver/adc.h>
#include "LedDriver_FastLED.h"
#include "Colors.h"
#include "TaskStructs.h"
#ifdef WITH_SECOND_BELL
#include "SecondBell.h"
#endif
#include "Settings.h"

//#define myDEBUG
#include "MyDebug.h"

extern s_taskParams taskParams;

#ifdef LED_LAYOUT_HORIZONTAL_1
    const uint8_t ledMap[] PROGMEM = {
          0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,
         21,  20,  19,  18,  17,  16,  15,  14,  13,  12,  11,
         22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32,
         43,  42,  41,  40,  39,  38,  37,  36,  35,  34,  33,
         44,  45,  46,  47,  48,  49,  50,  51,  52,  53,  54,
         65,  64,  63,  62,  61,  60,  59,  58,  57,  56,  55,
         66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,
         87,  86,  85,  84,  83,  82,  81,  80,  79,  78,  77,
         88,  89,  90,  91,  92,  93,  94,  95,  96,  97,  98,
        109, 108, 107, 106, 105, 104, 103, 102, 101, 100,  99,
        111, 112, 113, 110, 114
    };
#endif

#ifdef LED_LAYOUT_HORIZONTAL_2
    const uint8_t ledMap[] PROGMEM = {
      109, 108, 107, 106, 105, 104, 103, 102, 101, 100, 99,
      88,  89,  90,  91,  92,  93,  94,  95,  96,  97,  98,
      87,  86,  85,  84,  83,  82,  81,  80,  79,  78,  77,
      66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,
      65,  64,  63,  62,  61,  60,  59,  58,  57,  56,  55,
      44,  45,  46,  47,  48,  49,  50,  51,  52,  53,  54,
      43,  42,  41,  40,  39,  38,  37,  36,  35,  34,  33,
      22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32,
      21,  20,  19,  18,  17,  16,  15,  14,  13,  12,  11,
       0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,
     110, 111, 112, 114, 113
    };
#endif

#ifdef LED_LAYOUT_HORIZONTAL_3
   const uint8_t ledMap[] PROGMEM = {
     112, 111, 110, 109, 108, 107, 106, 105, 104, 103, 102,
      90,  91,  92,  93,  94,  95,  96,  97,  98,  99, 100,
      89,  88,  87,  86,  85,  84,  83,  82,  81,  80,  79,
      68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,
      67,  66,  65,  64,  63,  62,  61,  60,  59,  58,  57,
      46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,
      45,  44,  43,  42,  41,  40,  39,  38,  37,  36,  35,
      24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,
      23,  22,  21,  20,  19,  18,  17,  16,  15,  14,  13,
       1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,
     113, 101, 12, 0, 114
    };
#endif

#ifdef LED_LAYOUT_VERTICAL_1
    const uint8_t ledMap[] PROGMEM = {
          1,  21,  22,  41,  42,  61,  62,  81,  82, 101, 103,
          2,  20,  23,  40,  43,  60,  63,  80,  83, 100, 104,
          3,  19,  24,  39,  44,  59,  64,  79,  84,  99, 105,
          4,  18,  25,  38,  45,  58,  65,  78,  85,  98, 106,
          5,  17,  26,  37,  46,  57,  66,  77,  86,  97, 107,
          6,  16,  27,  36,  47,  56,  67,  76,  87,  96, 108,
          7,  15,  28,  35,  48,  55,  68,  75,  88,  95, 109,
          8,  14,  29,  34,  49,  54,  69,  74,  89,  94, 110,
          9,  13,  30,  33,  50,  53,  70,  73,  90,  93, 111,
         10,  12,  31,  32,  51,  52,  71,  72,  91,  92, 112,
          0, 102, 113,  11, 114
    };
#endif

#ifdef LED_LAYOUT_VERTICAL_2
    const uint8_t ledMap[] PROGMEM = {
          9,  10,  29,  30,  49,  50,  69,  70,  89,  90, 109,
          8,  11,  28,  31,  48,  51,  68,  71,  88,  91, 108,
          7,  12,  27,  32,  47,  52,  67,  72,  87,  92, 107,
          6,  13,  26,  33,  46,  53,  66,  73,  86,  93, 106,
          5,  14,  25,  34,  45,  54,  65,  74,  85,  94, 105,
          4,  15,  24,  35,  44,  55,  64,  75,  84,  95, 104,
          3,  16,  23,  36,  43,  56,  63,  76,  83,  96, 103,
          2,  17,  22,  37,  42,  57,  62,  77,  82,  97, 102,
          1,  18,  21,  38,  41,  58,  61,  78,  81,  98, 101,
          0,  19,  20,  39,  40,  59,  60,  79,  80,  99, 100,
        112, 110, 114, 113, 111
    };
#endif

#ifdef LED_LAYOUT_VERTICAL_3
    const uint8_t ledMap[] PROGMEM = {
          10, 12,  31,  32,  51,  52,  71,  72,  91,  92, 112,
          9,  13,  30,  33,  50,  53,  70,  73,  90,  93, 111,
          8,  14,  29,  34,  49,  54,  69,  74,  89,  94, 110,
          7,  15,  28,  35,  48,  55,  68,  75,  88,  95, 109,
          6,  16,  27,  36,  47,  56,  67,  76,  87,  96, 108,
          5,  17,  26,  37,  46,  57,  66,  77,  86,  97, 107,
          4,  18,  25,  38,  45,  58,  65,  78,  85,  98, 106,
          3,  19,  24,  39,  44,  59,  64,  79,  84,  99, 105,
          2,  20,  23,  40,  43,  60,  63,  80,  83, 100, 104,
          1,  21,  22,  41,  42,  61,  62,  81,  82, 101, 103,
         11, 113, 102,   0, 114
    };
#endif

#ifdef LED_LAYOUT_VERTICAL_4
    uint8_t ledMap[] PROGMEM = {
        112,  92, 91, 72, 71, 52, 51, 32, 31, 12, 10,
        111,  93, 90, 73, 70, 53, 50, 33, 30, 13,  9,
        110,  94, 89, 74, 69, 54, 49, 34, 29, 14,  8,
        109,  95, 88, 75, 68, 55, 48, 35, 28, 15,  7,
        108,  96, 87, 76, 67, 56, 47, 36, 27, 16,  6,
        107,  97, 86, 77, 66, 57, 46, 37, 26, 17,  5,
        106,  98, 85, 78, 65, 58, 45, 38, 25, 18,  4,
        105,  99, 84, 79, 64, 59, 44, 39, 24, 19,  3,
        104, 100, 83, 80, 63, 60, 43, 40, 23, 20,  2,
        103, 101, 82, 81, 62, 61, 42, 41, 22, 21,  1,
        113,  11,  0, 102, 114
    };
#endif

const byte coorMap[][2] PROGMEM = {
    {5,0},
    {0,0},  {0,0},  {0,0},  {1,0},  {2,0},  {3,0},  {4,0},  {5,0},  {6,0},  {7,0},  {8,0},  {9,0},  {10,0},  {10,0},  {10,0},
    {10,0}, {10,0}, {10,0}, {10,1}, {10,2}, {10,3}, {10,4}, {10,5}, {10,6}, {10,7}, {10,8}, {10,9}, {10,10}, {10,10}, {10,10},
    {10,9}, {10,9}, {10,9}, {9,9},  {8,9},  {7,9},  {6,9},  {4,9},  {3,9},  {2,9},  {1,9},  {0,9},  {0,9},   {0,9},
    {0,9},  {0,9},  {0,9},  {0,8},  {0,7},  {0,6},  {0,5},  {0,4},  {0,3},  {0,2},  {0,1},  {0,0},  {0,0},   {0,0},   {0,0}
};

extern uint16_t matrix[];

LedDriver* LedDriver::instance = 0;

LedDriver *LedDriver::getInstance() {
  if (!instance)
  {
      instance = new LedDriver();
  }
  return instance;
}

LedDriver::LedDriver()
{
    FastLED.addLeds<WS2812B, PIN_LEDS_DATA, GRB>(strip, NUMPIXELS);
    brightness=10;
    enable_bg_color=0;
    bg_color=0;
    transitionInProgress=false;
    minLdrValue=MIN_LDR_STARTVALUE; // The ESP will crash if minLdrValue and maxLdrValue are equal due to an error in map();
    maxLdrValue=MAX_LDR_STARTVALUE;
    lastLdrValue=0;
    mode = MODE_TIME;
    lastMode = MODE_TIME;
}

uint16_t LedDriver::getDegree(uint16_t sec) {
  // sec == 0  ==> Sekunden blinken unter der Glocke
  // sec == 1-60  ==> Sekundenzeiger rund um die Uhr
  uint16_t x = pgm_read_byte(&(coorMap[sec][0]));
  uint16_t y = pgm_read_byte(&(coorMap[sec][1]));
  return getDegree(y, x);
}

uint16_t LedDriver::getDegree(uint16_t y, uint16_t x) {
  float degree;
  const double dx = 29.9;
  const double dy = 33.22;

  
  // Ankathete und Gegenkathete berechnen
  double ak = 149.5 - (x * dx);
  double gk = 149.5 - (y * dy);

  if(ak==0)
    if(gk==0)
      degree=0;
    else if(gk<0)
          degree=270;
         else
          degree=90;
  else {
    // atan() liefert radian und wird in Grad umgerechnet
    degree = atan(gk/ak) * 57.2957795131;
    if(ak<0 && gk<0)
      degree = 360-degree;
    if(ak>=0 && gk<0)
      degree = 180-degree;
    if(ak<0 && gk>=0)
      degree *= -1;
    if(ak>=0 && gk>=0)
      degree = 180-degree;
  }
  
  return degree;
}

void LedDriver::setDegreeOffset(uint16_t offset) {
  _offset = offset;
}

// Convert angle + randomvalue to a hsv value
uint32_t LedDriver::getRGBFromDegRnd(uint16_t dg) {
  return getRGBFromDegRnd(_offset, dg);
}

// Convert angle + randomvalue to a hsv value
uint32_t  LedDriver::getRGBFromDegRnd(uint16_t offset, uint16_t dg) {
  uint16_t deg = ((uint16_t)(dg+0.5)+offset)%360;
  uint8_t h = map(deg,0,359,0,255);

  CRGB rgb;
  rgb = CHSV(h, 255, 255);
  uint32_t curColor = ((long)rgb.r << 16L) | ((long)rgb.g << 8L) | (long)rgb.b;
  return curColor;
}

void LedDriver::clear()
{
  fill_solid(strip, NUMPIXELS, CRGB::Black); 
}

void LedDriver::show()
{
  if(mode != MODE_BLANK) {
    FastLED.show();
  }
}

void LedDriver::setPixel(uint8_t x, uint8_t y, uint32_t color)
{
  uint8_t red   = color >> 16;
  uint8_t green = (color >> 8) & 0xff;
  uint8_t blue  = color & 0xff;
  
  if(!color) {
    uint32_t c = getRGBFromDegRnd(_offset,getDegree(y,x));
    red = c >> 16;
    green = (c >> 8) & 0xff;
    blue = c & 0xff;
  }
  setPixelRGB(x, y, red, green, blue);
}

void LedDriver::setPixel(uint8_t num, uint32_t color)
{
    uint8_t red   = color >> 16;
    uint8_t green = (color >> 8) & 0xff;
    uint8_t blue  = color & 0xff;
    if(!color) {
      float deg[4] = {135.00,45.00,315.00,225.00};
      uint32_t c = getRGBFromDegRnd(_offset, deg[num-110]);
      red = c >> 16;
      green = (c >> 8) & 0xff;
      blue = c & 0xff;
    }

    setPixelRGB(num, red, green, blue);
}

void LedDriver::setPixelRGB(uint8_t x, uint8_t y, uint8_t red, uint8_t green, uint8_t blue)
{
    setPixelRGB(x + y * 11, red, green, blue);
}

void LedDriver::setPixelRGB(uint8_t num, uint8_t red, uint8_t green, uint8_t blue )
{
  strip[pgm_read_byte(&ledMap[num])] = CRGB(red, green, blue);
}

void LedDriver::setPixelRGB(uint8_t x, uint8_t y, uint32_t c) 
{
  uint16_t num = x + y * LED_COLS;
  strip[pgm_read_byte(&ledMap[num])]=c;
}

void LedDriver::setPixelRGB(uint8_t x, uint8_t y, CRGB c) 
{
  uint16_t num = x + y * LED_COLS;
  strip[pgm_read_byte(&ledMap[num])]=c;
}

void LedDriver::setPixelRGB(uint8_t num, uint32_t c) 
{
  strip[pgm_read_byte(&ledMap[num])]=c;
}

void LedDriver::setPixelRGB(uint8_t num, CRGB c) 
{
  strip[pgm_read_byte(&ledMap[num])]=c;
}

uint32_t LedDriver::getPixelRGB(uint8_t x, uint8_t y)
{
  uint16_t num = x + y * LED_COLS;
  CRGB color = strip[pgm_read_byte(&ledMap[num])];
  return ((uint32_t)color.r << 16) | ((uint32_t)color.g << 8) | color.b;
}

void LedDriver::shiftMatrix(void) 
{
  for(uint16_t x_pos=0; x_pos < LED_COLS; x_pos++) {
    for(uint16_t y_pos=0; y_pos < LED_ROWS; y_pos++) {
      uint32_t c = getPixelRGB(x_pos+1,y_pos);
      setPixelRGB(x_pos,y_pos,c);
    }
  }
  for(uint16_t y_pos=0; y_pos < LED_ROWS; y_pos++)
    setPixelRGB(LED_COLS-1,y_pos,0);
}



void LedDriver::moveScreenBufferUp(uint16_t screenBufferNew[], uint32_t color)
{
  for (uint8_t z = 0; z <= 9; z++)
  {
    clear();
    for (uint8_t i = 0; i <= 9; i++)
    {
      if ( i == 9 )
      {
        screenBufferOld[i] = screenBufferNew[z] & 0b1111111111100000;
      }
      else
      {
        screenBufferOld[i] = screenBufferOld[i + 1] & 0b1111111111100000;
      }
      for (uint8_t x = 0; x <= 10; x++)
      {
        if ( 8 - z >= i ) {
          if (bitRead(screenBufferOld[i], 15 - x)) setPixel(x, i, colorold);
        }
        else
        {
          if (bitRead(screenBufferOld[i], 15 - x)) setPixel(x, i, color);
        }
      }
    }
    show();
    vTaskDelay(pdMS_TO_TICKS(TRANSITION_SPEED));
  }
  writeScreenBuffer(screenBufferNew, color);
}


void LedDriver::moveScreenBufferDown(uint16_t screenBufferNew[], uint32_t color)
{
  for (int z = 9; z >= 0; z--)
  {
    clear();
    for (int i = 9; i >= 0; i--)
    {
      if ( i == 0 )
      {
        screenBufferOld[i] = screenBufferNew[z] & 0b1111111111100000;
      }
      else
      {
        screenBufferOld[i] = screenBufferOld[i - 1] & 0b1111111111100000;
      }
      for (uint8_t x = 0; x <= 10; x++)
      {
        if ( 8 - z >= i ) {
          if (bitRead(screenBufferOld[i], 15 - x)) setPixel(x, i, color);
        }
        else
        {
          if (bitRead(screenBufferOld[i], 15 - x)) setPixel(x, i, colorold);
        }
      }
    }
    show();
    vTaskDelay(pdMS_TO_TICKS(TRANSITION_SPEED));
  }
  writeScreenBuffer(screenBufferNew, color);
}


void LedDriver::moveScreenBufferLeft(uint16_t screenBufferNew[], uint32_t color)
{
  uint16_t m_old[10] = {};
  uint16_t m_new[10] = {};
  for (uint8_t ze = 0; ze <= 9; ze++)
  {
    m_old[ze] = screenBufferOld[ze];
    m_new[ze] = screenBufferNew[ze];
  }
  for (uint8_t z = 0; z <= 10; z++)
  {
    clear();
    for (uint8_t i = 0; i <= 9; i++)
    {
      m_old[i] = (( m_old[i] & 0b1111111111100000 ) << 1  | (( m_new[i] >> 10 ) & 0b0000000000100000 )) & 0b1111111111100000;
      m_new[i] = (m_new[i] << 1) & 0b1111111111100000;

      for (uint8_t x = 0; x <= 10; x++)
      {
        if ( x > 9 - z  )
        {
          if (bitRead(m_old[i], 15 - x)) setPixel(x, i, color);
        }
        else
        {
          if (bitRead(m_old[i], 15 - x)) setPixel(x, i, colorold);
        }
      }
    }
    show();
     vTaskDelay(pdMS_TO_TICKS(TRANSITION_SPEED));
  }
  writeScreenBuffer(screenBufferNew, color);
}


void LedDriver::moveScreenBufferRight(uint16_t screenBufferNew[], uint32_t color)
{
  uint16_t m_old[10] = {};
  uint16_t m_new[10] = {};
  for (uint8_t ze = 0; ze <= 9; ze++)
  {
    m_old[ze] = screenBufferOld[ze];
    m_new[ze] = screenBufferNew[ze];
  }
  for (uint8_t z = 0; z <= 10; z++)
  {
    clear();
    for (uint8_t i = 0; i <= 9; i++)
    {
      m_old[i] = (m_old[i] >> 1 | m_new[i] << 10) & 0b1111111111100000;
      m_new[i] = (m_new[i] >> 1) & 0b1111111111100000;

      for (uint8_t x = 0; x <= 10; x++)
      {
        if ( x > z  )
        {
          if (bitRead(m_old[i], 15 - x)) setPixel(x, i, colorold);
        }
        else
        {
          if (bitRead(m_old[i], 15 - x)) setPixel(x, i, color);
        }
      }
    }
    show();
    vTaskDelay(pdMS_TO_TICKS(TRANSITION_SPEED));
  }
  writeScreenBuffer(screenBufferNew, color);
}


void LedDriver::moveScreenBufferLeftDown(uint16_t screenBufferNew[], uint32_t color)
{
  for (int z = 0; z <= 9; z++)
  {
    clear();
    for (int i = 0; i <= 8 ; i++)
    {
      screenBufferOld[9 - i] = (screenBufferOld[8 - i] & 0b1111111111100000) << 1;
    }
    for (int i = 0; i <= z ; i++)
    {
      screenBufferOld[i] = ((screenBufferNew[9 + i - z] >> (14 - z)) << 5) & 0b1111111111100000;
    }

    for (int y = 0; y <= 9; y++)
    {
      for (uint8_t x = 0; x <= 10; x++)
      {
        if ( y <= z ) {
          if (bitRead(screenBufferOld[y], 15 - x)) setPixel(x, y, color);
        }
        else
        {
          if (bitRead(screenBufferOld[y], 15 - x)) setPixel(x, y, colorold);
        }
      }
    }

    show();
    vTaskDelay(pdMS_TO_TICKS(TRANSITION_SPEED));
  }
  writeScreenBuffer(screenBufferNew, color);
}


void LedDriver::moveScreenBufferRightDown(uint16_t screenBufferNew[], uint32_t color)
{
  for (int z = 0; z <= 9; z++)
  {
    clear();
    for (int i = 0; i <= 8 ; i++)
    {
      screenBufferOld[9 - i] = screenBufferOld[8 - i] >> 1 & 0b1111111111100000;
    }
    for (int i = 0; i <= z ; i++)
    {
      screenBufferOld[i] = (screenBufferNew[9 + i - z] << (9 - z)) & 0b1111111111100000;
    }

    for (int y = 0; y <= 9; y++)
    {
      for (uint8_t x = 0; x <= 10; x++)
      {
        if ( y <= z ) {
          if (bitRead(screenBufferOld[y], 15 - x)) setPixel(x, y, color);
        }
        else
        {
          if (bitRead(screenBufferOld[y], 15 - x)) setPixel(x, y, colorold);
        }
      }
    }
    show();
    vTaskDelay(pdMS_TO_TICKS(TRANSITION_SPEED));
  }
  writeScreenBuffer(screenBufferNew, color);
}


void LedDriver::moveScreenBufferCenter(uint16_t screenBufferNew[], uint32_t color)
{
  uint16_t mask[5] = {};
  mask[0] = 0b1000000000100000;
  mask[1] = 0b1100000001100000;
  mask[2] = 0b1110000011100000;
  mask[3] = 0b1111000111100000;
  mask[4] = 0b1111101111100000;
  uint16_t m_old[10] = {};
  uint16_t m_new[10] = {};
  for (uint8_t ze = 0; ze <= 9; ze++)
  {
    m_old[ze] = screenBufferOld[ze] & 0b1111111111100000;
    m_new[ze] = screenBufferNew[ze];
  }
  //##################### Old zur Mitte #########################################
  for (int i = 0; i <= 4; i++)
  {
    for (int z = 0; z <= 4; z++)
    {
      m_old[z] = ((m_old[z] & 0b1111110000000000) >> 1) | ((m_old[z] & 0b0000001111110000) << 1);
      m_old[9 - z] = ((m_old[9 - z] & 0b1111110000000000) >> 1) | ((m_old[9 - z] & 0b0000001111110000) << 1);
    }
    for (int z = 0; z <= 3; z++)
    {
      m_old[z + 5] = m_old[z + 6];
      m_old[4 - z] = m_old[3 - z];
    }
    m_old[9] = 0;
    m_old[0] = 0;
    writeScreenBuffer(m_old, colorold);
    vTaskDelay(pdMS_TO_TICKS(TRANSITION_SPEED));
  }
  //###################### New aus der Mitte ########################################
  for (int i = 4; i >= 0 ; i--)
  {
    for (int z = 0; z <= 4 - i; z++)
    {
      m_old[i + z] =   ((m_new[z]  & mask[4 - i]) >> (i + 1) & 0b1111110000000000) | ((m_new[z]  & mask[4 - i]) << (i + 1) & 0b0000011111100000);
      m_old[9 - i - z] = ((m_new[9 - z] & mask[4 - i]) >> (i + 1) & 0b1111110000000000) | ((m_new[9 - z] & mask[4 - i]) << (i + 1) & 0b0000011111100000);
    }
    writeScreenBuffer(m_old, color);
    vTaskDelay(pdMS_TO_TICKS(TRANSITION_SPEED));
  }

  writeScreenBuffer(screenBufferNew, color);
}


void LedDriver::matrix_regen(uint16_t screenBufferNew[], uint32_t color)
{
  uint16_t mline[11] = {0};
  uint16_t wline[11] = {0};
  uint16_t sline[11] = {0};
  uint8_t aktline;
  uint16_t mleer = 0;  // zu prüfen ob wir fertig sind

  uint8_t mstep = 0;
  uint16_t mline_max[11] = {0};
  uint16_t brightnessBuffer[11][12] = {0};
  uint16_t brightness_16 = brightness;

  uint16_t zufallszeile[11] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  uint16_t zufallsbuffer;
  uint16_t zufallsindex;
  // Zufälliges Vertauschen von jeweils 2 Zeilenwerten:
  for (uint8_t i = 0; i <= 10; i++)
  {
    zufallsindex = random(0, 11);
    zufallsbuffer = zufallszeile[zufallsindex];
    zufallszeile[zufallsindex] = zufallszeile[i];
    zufallszeile[i] = zufallsbuffer;

  }
  // Variablen init
  for (uint8_t line = 0; line <= 10; line++)
  {
    sline[line] = (11 - (zufallszeile[line] % 11 )) * 5 / 2;
    mline_max[line] = 0;
    mline[line] = 0;
    wline[line] = 0;
  }

  for (uint16_t i = 0; i <= 1200; i++)
  {
    aktline = zufallszeile[i % 11];

    if ( sline[aktline] > 0 ) sline[aktline]--;
    if ( sline[aktline] == 0 )
    {
      sline[aktline] = 3 - (aktline % 2);

      if ( mline[aktline] == 0 && mline_max[aktline] < 10)
      {
        mline[aktline] = 1;
        mline_max[aktline]++;
      }
      else
      {
        if ( mline_max[aktline] < 11 )  // solange grün hinzu bis unten erreicht ist
        {
          if ( random(0, 6) == 0 && (mline[aktline] & 1) == 0 )
          {
            mline[aktline] = mline[aktline] << 1;
            mline[aktline] = mline[aktline] | 1;
            wline[aktline] = wline[aktline] << 1;
          }
          else
          {
            wline[aktline] = wline[aktline] << 1;
            wline[aktline] = wline[aktline] | 1;
            mline[aktline] = mline[aktline] << 1;
          }
          mline_max[aktline]++;
        }
        else
        {
          mline[aktline] = mline[aktline] << 1;
          wline[aktline] = wline[aktline] << 1;
          if ( (mline[aktline] & 0x3FF)  == 0 && (wline[aktline] & 0x3FF) == 0 ) mleer = mleer | 1 << aktline;
        }
      }
      clear();
      for ( uint16_t y = 0; y <= 9; y++ )
      {
        for ( uint16_t x = 0; x <= 10; x++ )
        {
          if ( y > mline_max[x] - 1 )
          {
            if (bitRead(screenBufferOld[y], 15 - x)) setPixel(x, y, colorold);
          }
          else
          {
            if (bitRead(screenBufferNew[y], 15 - x)) setPixel(x, y, color);
          }
          brightnessBuffer[y][x] = 0;
          if ( wline[x] & (1 << y) )
          {
            brightnessBuffer[y][x] = brightness_16 / 9;
            //             brightnessBuffer[y][x] = brightness_16 / 8 - ( brightness_16 / ( 10 * (mline_max[x]-1 - y))) ;
            if ( brightnessBuffer[y][x] < 3) brightnessBuffer[y][x] = 2;
          }
          if ( mline[x] & (1 << y) )
          {
            brightnessBuffer[y][x] =  brightness_16 * 10 / 9;
            if ( brightnessBuffer[y][x] > 254 ) brightnessBuffer[y][x] = 255;
          }

          if ( brightnessBuffer[y][x] > 0 )
          {
            CRGB c(CRGB::Green);
            setPixelRGB(x, y, c.nscale8(scale8(brightnessBuffer[y][x],brightness)));
          }
        } // x
      }  // y
      show();
      delay (int(TRANSITION_SPEED / 9));
      delay (1);
    }
    if ( i > 100 && mleer == 0x7FF) break;
  }

  writeScreenBuffer(screenBufferNew, color);
  colorold = color;
}


void LedDriver::moveSeriell(uint16_t screenBufferNew[], uint32_t color, uint8_t sertype)
{
  uint16_t pointerbuffer;
  uint16_t xold_dest;
  uint16_t yold_dest;
  uint16_t xold_source;
  uint16_t yold_source;
  uint16_t xnew_dest;
  uint16_t ynew_dest;
  uint16_t xnew_source;
  uint16_t ynew_source;
  int s_old;
  int c;
  int s_new;

  int aquad;
  int adelay;

  for (int a = 0; a <= 109; a++)
  {
    clear();
    for (int b = 0; b <= 109; b++)
    {
      s_old = a + b + 1;
      pointerbuffer = pgm_read_word(&seriellpointer[sertype][b]);
      xold_dest = pointerbuffer % 11;         //seriellpointer ist in Modes.h definiert
      yold_dest = pointerbuffer / 11;
      pointerbuffer = pgm_read_word(&seriellpointer[sertype][s_old]);
      xold_source = pointerbuffer % 11;
      yold_source = pointerbuffer / 11;

      c = 109 - b;
      s_new = a - c;

      xnew_dest = xold_dest;
      ynew_dest = yold_dest;
      pointerbuffer = pgm_read_word(&seriellpointer[sertype][s_new]);
      xnew_source = pointerbuffer % 11;
      ynew_source = pointerbuffer / 11;

      if (  c > a )
      {
        if (bitRead(screenBufferOld[yold_source], 15 - xold_source)) setPixel(xold_dest, yold_dest, colorold);
      }
      else
      {
        if (bitRead(screenBufferNew[ynew_source], 15 - xnew_source)) setPixel(xnew_dest, ynew_dest, color);
      }
    }
    show();
    aquad = (130 - a) * (130 - a); // damit wir am Ende quadratisch immer langsamer werden
    //    delay(2*TRANSITION_SPEED/(1+((120-a)/10)));
    adelay = 5 + ( 360 * TRANSITION_SPEED / aquad);
    vTaskDelay(pdMS_TO_TICKS(adelay));
  }
  writeScreenBuffer(screenBufferNew, color);
  colorold = color;
}


void LedDriver::farbenmeer(uint16_t screenBufferNew[], uint32_t color)
{
  uint16_t zufallszeile[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  uint16_t zufallsbuffer;
  uint16_t zufallsspalte[11] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  //  uint16_t zufallsspalte[11] = {0x20,0x40,0x80,0x100,0x200,0x400,0x800,0x1000,0x2000,0x4000,0x8000};
  uint16_t zufallsindex;
  // Zufälliges Vertauschen von jeweils 2 Zeilenwerten:
  for (uint8_t i = 0; i <= 9; i++)
  {
    zufallsindex = random(0, 10);
    zufallsbuffer = zufallszeile[zufallsindex];
    zufallszeile[zufallsindex] = zufallszeile[i];
    zufallszeile[i] = zufallsbuffer;
  }
  // Zufälliges Vertauschen von jeweils 2 Spaltennwerten:
  for (uint8_t i = 0; i <= 10; i++)
  {
    zufallsindex = random(0, 11);
    zufallsbuffer = zufallsspalte[zufallsindex];
    zufallsspalte[zufallsindex] = zufallsspalte[i];
    zufallsspalte[i] = zufallsbuffer;
  }
  // Farben füllen
  for (uint8_t x = 0; x <= 10; x++)
  {
    for (uint8_t y = 0; y <= 9; y++)
    {
      zufallsindex = y + x;
      if ( zufallsindex > 10) zufallsindex = zufallsindex - 11;
      screenBufferOld[zufallszeile[y]] = screenBufferOld[zufallszeile[y]] | zufallsspalte[zufallsindex];
      setPixel(zufallsspalte[zufallsindex], zufallszeile[y], colorArray[random(1, COLOR_COUNT + 1)] );
      show();
      vTaskDelay(pdMS_TO_TICKS (11 - x));
    }
  }
  // Farben leeren
  for (uint8_t x = 0; x <= 10; x++)
  {
    for (uint8_t y = 0; y <= 9; y++)
    {
      zufallsindex = y + x;
      if ( zufallsindex > 10) zufallsindex = zufallsindex - 11;
      if ( bitRead(screenBufferNew[zufallszeile[y]], 15 - zufallsspalte[zufallsindex]))
      {
        setPixel(zufallsspalte[zufallsindex], zufallszeile[y], color );
      }
      else
      {
        setPixelRGB(zufallsspalte[zufallsindex], zufallszeile[y], CRGB::Black);
      }
      show();
      vTaskDelay(pdMS_TO_TICKS (1 + x));
    }
  }
  writeScreenBuffer(screenBufferNew, color);
  colorold = color;
}


void LedDriver::regenbogen(uint16_t screenBufferNew[], uint32_t color)
{
  int startbogen = 0;
  int bogenidx = 0;

  uint8_t bogen[10][12] = {
    {0      , 0      , 0      , 0      , RED    , RED    , RED    , 0      , 0      , 0      , 0     },
    {0      , RED    , RED    , RED    , ORANGE , ORANGE , ORANGE , RED    , RED    , RED    , 0     },
    {RED    , ORANGE , ORANGE , ORANGE , YELLOW , YELLOW , YELLOW , ORANGE , ORANGE , ORANGE , RED   },
    {ORANGE , YELLOW , YELLOW , YELLOW , GREEN  , GREEN  , GREEN  , YELLOW , YELLOW , YELLOW , ORANGE},
    {YELLOW , GREEN  , GREEN  , GREEN  , BLUE   , BLUE   , BLUE   , GREEN  , GREEN  , GREEN  , YELLOW},
    {GREEN  , BLUE   , BLUE   , BLUE   , MAGENTA, MAGENTA, MAGENTA, BLUE   , BLUE   , BLUE   , GREEN },
    {BLUE   , MAGENTA, MAGENTA, MAGENTA, 99     , 99     , 99     , MAGENTA, MAGENTA, MAGENTA, BLUE  },
    {MAGENTA, 99     , 99     , 99     , 99     , 99     , 99     , 99     , 99     , 99     , MAGENTA},
    {99     , 99     , 99     , 99     , 99     , 99     , 99     , 99     , 99     , 99     , 99},
    {99     , 99     , 99     , 99     , 99     , 99     , 99     , 99     , 99     , 99     , 99    }
  };

  for (uint8_t d = 0; d < 19; d++)
  {
    clear();
    for (uint8_t y = 0; y <= 9; y++)
    {
      startbogen = 9 - d;
      bogenidx = y - startbogen;
      if ( startbogen < 0 ) startbogen = 0;
      for (uint8_t x = 0; x <= 10; x++)
      {
        if (bitRead(screenBufferOld[y], 15 - x)) setPixel(x, y, colorold);
        if ( y >= startbogen && bogenidx >= 0 && bogenidx <= 9)                                                                // Regenbogenbereich
        {
          if ( bogen[bogenidx][x] != 0 && bogen[bogenidx][x] != 99 ) setPixel(x, y, colorArray[bogen[bogenidx][x]]); // Farbe aus Regenbogen
          if ( bogen[bogenidx][x] == 99 )                                                                                      // Prüfen ob hier kein Regenbogen ist
          {
            if (bitRead(screenBufferNew[y], 15 - x)) setPixel(x, y, color);                              // Farbe aus color (neue Farbe)
            else setPixelRGB(x, y, CRGB::Black); //setPixel(x, y, color, 0);                                                                           // Pixel löschen (brightness = 0)
          }
        }
        if ( y >= startbogen && bogenidx > 9 )                                                                         // Regenbogen ist vorbei
        {
          if (bitRead(screenBufferNew[y], 15 - x)) setPixel(x, y, color);                        // Neues Pixel gesetzt dann neue Farbe
          else setPixelRGB(x, y, CRGB::Black); //setPixel(x, y, color, 0);                                                                     // ansonsten Pixel löschen (brightness = 0)
        }
      }
    }
    show();
    vTaskDelay(pdMS_TO_TICKS(TRANSITION_SPEED * 15 / 10));
  }
  writeScreenBuffer(screenBufferNew, color);
  colorold = color;
}

//#########################
// Kreise
//
// kreistoBuffer
// legt einen Kreis und die innere Kreisfläche jeweils in eine matrix
// x0: x-Koordinate 0-10 (0=links 10=rechts)
// y0: y-Koordinate 0-9 (0=oben 9=unten)
// r: Radius
// Rückgabe: true wenn Kreis innerhalb der Matrix ist. false wenn nicht.
bool LedDriver::kreistoBuffer(uint16_t kreislinie[], uint16_t kreisflaeche[], int8_t x0, int8_t y0, int8_t r)
{
  int8_t maxy = 10;
  int8_t maxx = 11;
  int8_t f = 1 - r;
  int8_t ddF_x = 1;
  int8_t ddF_y = -2 * r;
  int8_t x = 0;
  int8_t y = r;
  uint16_t matrix[10] = {};
  uint16_t matrixf[10] = {};
  uint16_t mask = 0b1111111111100000;
  uint16_t tst = 0;

  bool retval = false;

  for (uint8_t ze = 0; ze <= 9; ze++)
  {
    matrix[ze] = 0;
    matrixf[ze] = mask;
  }

  if ( y0 >= 0 && y0 < maxy && x0 + r >= 0 && x0 + r < maxx) matrix[y0]   = (matrix[y0] | 1 << (15 - x0 - r))& mask;
  if ( y0 >= 0 && y0 < maxy && x0 - r >= 0 && x0 - r < maxx) matrix[y0]   = (matrix[y0] | 1 << (15 - x0 + r))& mask;
  if ( y0 + r >= 0 && y0 + r < maxy && x0 >= 0 && x0 < maxx) matrix[y0 + r] = (matrix[y0 + r] | 1 << (15 - x0))& mask;
  if ( y0 - r >= 0 && y0 - r < maxy && x0 >= 0 && x0 < maxx) matrix[y0 - r] = (matrix[y0 - r] | 1 << (15 - x0))& mask;

  if ( y0 >= 0 && y0 < maxy )
  {
    matrixf[y0] = mask >> (x0 + r);
    matrixf[y0] = matrixf[y0] | mask << (maxx - 1 - x0 + r);
  }

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;

    if ( y0 + y >= 0 && y0 + y < maxy && x0 + x >= 0 && x0 + x < maxx) matrix[y0 + y] = (matrix[y0 + y] | 1 << (15 - x0 - x))&mask;
    if ( y0 + y >= 0 && y0 + y < maxy && x0 - x >= 0 && x0 - x < maxx) matrix[y0 + y] = (matrix[y0 + y] | 1 << (15 - x0 + x))&mask;
    if ( y0 - y >= 0 && y0 - y < maxy && x0 + x >= 0 && x0 + x < maxx) matrix[y0 - y] = (matrix[y0 - y] | 1 << (15 - x0 - x))&mask;
    if ( y0 - y >= 0 && y0 - y < maxy && x0 - x >= 0 && x0 - x < maxx) matrix[y0 - y] = (matrix[y0 - y] | 1 << (15 - x0 + x))&mask;
    if ( y0 + x >= 0 && y0 + x < maxy && x0 + y >= 0 && x0 + y < maxx) matrix[y0 + x] = (matrix[y0 + x] | 1 << (15 - x0 - y))&mask;
    if ( y0 + x >= 0 && y0 + x < maxy && x0 - y >= 0 && x0 - y < maxx) matrix[y0 + x] = (matrix[y0 + x] | 1 << (15 - x0 + y))&mask;
    if ( y0 - x >= 0 && y0 - x < maxy && x0 + y >= 0 && x0 + y < maxx) matrix[y0 - x] = (matrix[y0 - x] | 1 << (15 - x0 - y))&mask;
    if ( y0 - x >= 0 && y0 - x < maxy && x0 - y >= 0 && x0 - y < maxx) matrix[y0 - x] = (matrix[y0 - x] | 1 << (15 - x0 + y))&mask;

    if ( y0 + x >= 0 && y0 + x < maxy )
    {
      matrixf[y0 + x] = mask >> (x0 + y);
      matrixf[y0 + x] = matrixf[y0 + x] | mask << (maxx - 1 - x0 + y);
    }
    if ( y0 - x >= 0 && y0 - x < maxy )
    {
      matrixf[y0 - x] = mask >> (x0 + y);
      matrixf[y0 - x] = matrixf[y0 - x] | mask << (maxx - 1 - x0 + y);
    }

    if ( y0 + y >= 0 && y0 + y < maxy )
    {
      matrixf[y0 + y] = mask >> (x0 + x);
      matrixf[y0 + y] = matrixf[y0 + y] | mask << (maxx - 1 - x0 + x);
    }
    if ( y0 - y >= 0 && y0 - y < maxy )
    {
      matrixf[y0 - y] = mask >> (x0 + x);
      matrixf[y0 - y] = matrixf[y0 - y] | mask << (maxx - 1 - x0 + x);
    }
  }

  for (uint8_t ze = 0; ze <= 9; ze++)
  {
    kreislinie[ze] = matrix[ze];
    if ( kreislinie[ze] > 0 ) retval = true;
    kreisflaeche[ze] = (~(matrixf[ze] | matrix[ze])) & mask;
  }
  return retval;
}


void LedDriver::kreise(uint16_t screenBufferNew[], uint32_t color)
{
  uint16_t xbuff;
  uint16_t m_old[10] = {};
  uint16_t m_new[10] = {};

  uint16_t kreis1[10] = {};
  uint16_t kreis1fl[10] = {};
  bool work1 = true;
  uint8_t mp1x = random(2, 6);
  uint8_t mp1y = random(2, 6);
  uint8_t r1 = 0;

  uint16_t kreis2[10] = {};
  uint16_t kreis2fl[10] = {};
  bool work2 = false;
  uint8_t mp2x = random(5, 10);
  uint8_t mp2y = random(5, 9);
  uint8_t r2 = 0;

  uint16_t kreis3[10] = {};
  uint16_t kreis3fl[10] = {};
  bool work3 = false;
  uint8_t mp3x = random(3, 8);
  uint8_t mp3y = random(2, 7);
  uint8_t r3 = 0;

  uint32_t uhrcolor = colorold;

  // Init
  for (uint8_t ze = 0; ze <= 9; ze++)
  {
    m_old[ze] = screenBufferOld[ze];
    m_new[ze] = screenBufferNew[ze];
  }
  while (work1 || work2 || work3)
  {
    clear();
    if ( work1) work1 = kreistoBuffer(kreis1, kreis1fl, mp1x, mp1y, r1);
    if ( work2) work2 = kreistoBuffer(kreis2, kreis2fl, mp2x, mp2y, r2);
    if ( work3) work3 = kreistoBuffer(kreis3, kreis3fl, mp1x, mp3y, r3);


    for (uint8_t y = 0; y <= 9; y++)
    {
      if (work3)
      {
        xbuff = (m_new[y] & kreis3fl[y]);
        uhrcolor = color;
      }
      else
      {
        xbuff = (m_old[y] & ~kreis1fl[y]);
      }
      if (!(work1 || work2 || work3) ) xbuff = m_new[y];

      for (uint8_t x = 0; x <= 10; x++)
      {
        if (bitRead( xbuff, 15 - x )) setPixel(x, y, uhrcolor);
        if (work1 && bitRead( kreis1[y], 15 - x )) setPixel(x, y, colorArray[WHITE]);
        if (work2 && bitRead( kreis2[y], 15 - x )) setPixel(x, y, colorArray[WHITE]);
        if (work3 && bitRead( kreis3[y], 15 - x )) setPixel(x, y, colorArray[WHITE]);
      }
    }

    if ( work1) r1++;
    if ( work2) r2++;
    if ( work3) r3++;
    if ( r2 == 0 && r1 > abs(mp1x - mp2x) + 1 && r1 > abs(mp1y - mp2y) + 1)
    {
      work2 = true;
     }
    if ( r3 == 0 && r2 > abs(mp2x - mp3x) + 1 && r2 > abs(mp2y - mp3y) + 1)
    {
      work3 = true;
    }
    show();
    vTaskDelay(pdMS_TO_TICKS(TRANSITION_SPEED + 10));
  }
  writeScreenBuffer(screenBufferNew, color);
  colorold = color;
}


//#########################
// Quadrate
void LedDriver::quadrate(uint16_t screenBufferNew[], uint32_t color)
{
  uint8_t lpx = random(3, 9);
  uint8_t lpy = random(3, 8);
  uint8_t spx = random(3, 9);
  uint8_t spy = random(3, 8);
  uint8_t dl = 0;
  uint16_t xbuff;
  uint16_t m_old[10] = {};
  uint16_t m_new[10] = {};
  uint16_t lquad_old[10] = {};
  uint16_t lquad_new[10] = {};
  uint16_t lquad_disp[10] = {};
  uint16_t squad_old[10] = {};
  uint16_t squad_new[10] = {};
  uint16_t squad_disp[10] = {};
  uint16_t mask = 0b1111111111100000;
  bool q2 = false;
  bool work = true;

  // Init
  for (uint8_t ze = 0; ze <= 9; ze++)
  {
    m_old[ze] = screenBufferOld[ze];
    m_new[ze] = screenBufferNew[ze];
    if (ze == lpy) lquad_old[ze] = 1 << (16 - lpx);
    lquad_new[ze] = lquad_old[ze];
    lquad_disp[ze] = lquad_old[ze];

    if (ze == spy) squad_old[ze] = 1 << (16 - spx);
    squad_new[ze] = squad_old[ze];
    squad_disp[ze] = squad_old[ze];
  }

  //  Work
  while (work)
  {
    dl++;
    work = false;
    clear();
    for (uint8_t y = 0; y <= 9; y++)
    {
      lquad_old[y] = lquad_new[y];
      m_old[y] = m_old[y] & ~lquad_disp[y];
      for (uint8_t x = 0; x <= 10; x++)
      {
        if (bitRead( lquad_disp[y], 15 - x )) setPixel(x, y, colorArray[RED]);
        if (bitRead( m_old[y], 15 - x )) setPixel(x, y, colorold);
        if ( q2 )
        {
          if (bitRead( squad_disp[y], 15 - x )) setPixel(x, y, colorArray[GREEN]);
          if (bitRead( m_new[y] & squad_old[y], 15 - x )) setPixel(x, y, color);
        }
      }
      if ( q2 ) squad_old[y] = squad_new[y];
      if ( squad_disp[y] > 0 ) work = true;
    }
    show();
    vTaskDelay(pdMS_TO_TICKS(TRANSITION_SPEED));
    // Löschquadrat nach oben und unten kopieren
    for (uint8_t yc = 0 ; yc <= lpy - 1; yc++)
    {
      lquad_new[yc] = lquad_old[yc + 1];
    }
    for (uint8_t yc = 9 ; yc >= lpy + 1; yc--)
    {
      lquad_new[yc] = lquad_old[yc - 1];
    }
    // Löschquadrat nach links und rechts erweitern
    for (uint8_t ye = 0; ye <= 9; ye++)
    {
      xbuff = lquad_new[ye];
      lquad_new[ye] = lquad_new[ye] * 2 | lquad_new[ye];
      xbuff = xbuff / 2;
      lquad_new[ye] = lquad_new[ye] | xbuff;
      lquad_disp[ye] = lquad_new[ye] & ~lquad_old[ye] & mask;
    }
    // Start Setzquadrat
    if ( q2 )
    {
      // Setzquadrat nach oben und unten kopieren
      for (uint8_t yc = 0 ; yc <= spy - 1; yc++)
      {
        squad_new[yc] = squad_old[yc + 1];
      }
      for (uint8_t yc = 9 ; yc >= spy + 1; yc--)
      {
        squad_new[yc] = squad_old[yc - 1];
      }
      // Setzquadrat nach links und rechts erweitern
      for (uint8_t ye = 0; ye <= 9; ye++)
      {
        xbuff = squad_new[ye];
        squad_new[ye] = squad_new[ye] * 2 | squad_new[ye];
        xbuff = xbuff / 2;
        squad_new[ye] = squad_new[ye] | xbuff;
        squad_disp[ye] = squad_new[ye] & ~squad_old[ye] & mask;
      }
    }
    // Prüfen ob Setzquadrat starten kann ( xdiff und ydiff müssen größer als die Anzahl der Durchläufe +1 sein)
    if ( dl > abs(lpx - spx) + 1 && dl > abs(lpy - spy) + 1) q2 = true;
  }
  writeScreenBuffer(screenBufferNew, color);
  colorold = color;
}


void LedDriver::writeScreenBufferFade(uint16_t screenBufferNew[], uint32_t color)
{

  uint16_t blendProgress = 0;  // 0-255 (Fortschritt der Überblendung)
  unsigned long st = millis();
  transitionInProgress = true;

  for(uint16_t i=0; i<NUMPIXELS-1; i++) {
    oldPattern[i]=strip[i];
    newPattern[i]=CRGB::Black;
  }

  for (uint8_t y = 0; y <= 9; y++)
  {
    if ( mode == MODE_DATE && y > 4 ) color = colorArray[LIGHTBLUE];
    for (uint8_t x = 0; x <= 10; x++)
    {
      if (bitRead(screenBufferNew[y], 15 - x)) newPattern[pgm_read_byte(&ledMap[x + y * LED_COLS])]=color?color:getRGBFromDegRnd(_offset,getDegree(y,x));
    }
  }

  // Corner LEDs
  float deg[4] = {135.00,45.00,315.00,225.00};
  for (uint8_t y = 0; y <= 3; y++)
  {
    if (bitRead(screenBufferNew[y], 4)) newPattern[pgm_read_byte(&ledMap[110 + y])]=corcol?corcol:getRGBFromDegRnd(_offset,deg[y]);
  }


  
  while (blendProgress <= 255) {
    // Überblende jedes Pixels einzeln
    uint8_t easedProgress = ease8InOutQuad(blendProgress);
    for (int i = 0; i < NUMPIXELS-1; i++) {
      CRGB fadedOld = oldPattern[i];
      fadedOld.nscale8_video(255 - easedProgress);  // Helligkeit reduzieren

      // Neues Muster einblenden (Alpha = progress)
      CRGB fadedNew = newPattern[i];
      fadedNew.nscale8_video(easedProgress);  // Helligkeit erhöhen

      // Beide Anteile addieren (keine Überblendung, da Helligkeiten komplementär)
      strip[i] = fadedOld + fadedNew;
    }
    show();
    blendProgress++;  // Fortschritt erhöhen
    vTaskDelay(pdMS_TO_TICKS(2));
  }

  transitionInProgress = false;
  colorold = color;
  cornercolorold = corcol;
  if ( lastMode == MODE_DATE && mode == MODE_TIME )
  {
    lastMode = MODE_TIME;
  }
  
  //Serial.printf("zeit(ms)=%ld, HighWater=%d, mode=%d\n",millis()-st, uxTaskGetStackHighWaterMark(NULL), mode);
}


void LedDriver::writeScreenBuffer(uint16_t screenBuffer[], uint32_t color)
{
  clear();
  for (uint8_t y = 0; y <= 9; y++)
  {
    if ( mode == MODE_DATE && y > 4 ) color = colorArray[LIGHTBLUE];
    for (uint8_t x = 0; x <= 10; x++)
    {
      if (bitRead(screenBuffer[y], 15 - x)) setPixel(x, y, color);
    }
  }

  // Corner LEDs
  for (uint8_t y = 0; y <= 3; y++)
  {
    if (bitRead(screenBuffer[y], 4)) setPixel(110 + y, corcol);
  }

  show();
  colorold = color;
  //cornercolorold = settings->mySettings.corcol;
}

uint8_t LedDriver::getBrightness(void)
{
  return brightness;
}

#ifdef LDR
//******************************************************************************
// get brightness from LDR
//******************************************************************************
void LedDriver::setBrightnessFromLdr(void)
{
#ifdef LDR_IS_INVERSE
  ldrValue = 1024 - adc1_get_raw(ADC1_CHANNEL_1);
#else
  //mz ldrValue = adc1_get_raw(PIN_LDR);
#endif
  if (ldrValue < minLdrValue)
    minLdrValue = ldrValue;
  if (ldrValue > maxLdrValue)
    maxLdrValue = ldrValue;
  if ((!lastLdrValue) || (ldrValue >= (lastLdrValue + 30)) || (ldrValue <= (lastLdrValue - 30))) // Hysteresis ist 30 (vorher 40)
  {
    lastLdrValue = ldrValue;
    brightness = map(ldrValue, minLdrValue, maxLdrValue, MIN_BRIGHTNESS, abcBrightness);
    FastLED.setBrightness(brightness);
    show();
  }
}
#endif

void LedDriver::setBrightness(uint8_t _brightness)
{
  brightness = _brightness;
  FastLED.setBrightness(brightness);
  show();
}


void LedDriver::saveMatrix(uint16_t matrix[], bool clear) {
  for (uint8_t i = 0; i <= 9; i++) {
    screenBufferOld[i] = matrix[i];
    if(clear)
      matrix[i]=0;
  }
}

void LedDriver::setOnOff(void) {

  DEBUG_PRINTF("setOnOff: mode=%d\n", mode);
#ifdef WITH_SECOND_BELL
  SecondBell *secondBell = SecondBell::getInstance();
#endif
  Settings *settings = Settings::getInstance();
  if(mode != MODE_BLANK) {
    saveMatrix(matrix, true);
    lastMode = mode;
#ifdef WITH_SECOND_BELL
    secondBell->setStatus(false);
#endif
    writeScreenBufferFade(matrix, settings->mySettings.ledcol);
    mode = MODE_BLANK;
#if defined(LILYGO_T_HMI)
    analogWrite(TFT_BL,0);
#endif
    taskParams.updateScreen=false;
  } else {
    mode = MODE_TIME;
#if defined(LILYGO_T_HMI)
    analogWrite(TFT_BL,255);
#endif
    taskParams.updateScreen=true;
#ifdef WITH_SECOND_BELL
    secondBell->setStatus(true);
#endif
  }
}
