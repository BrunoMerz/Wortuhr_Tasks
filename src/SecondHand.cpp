#if defined(WITH_SECOND_HAND)
#include "SecondHand.h"
#include "SecondBell.h"
#include "Settings.h"
#include "MyTime.h"
#include "LedDriver_FastLED.h"


//#define DEBUG_SECONDHAND

static Settings *settings = Settings::getInstance();
static LedDriver *ledDriver = LedDriver::getInstance();
static MyTime *mytm=MyTime::getInstance();

SecondHand* SecondHand::instance = 0;

SecondHand *SecondHand::getInstance() {
  if (!instance)
  {
      instance = new SecondHand();
  }
  return instance;
}


SecondHand::SecondHand(void) {
  _quarterWidth=5;
  _lastSecond=0;
  _doUpdate = false;
}


void SecondHand::init(void) {
  FastLED.addLeds<WS2812B, PIN_SECONDS_DATA, GRB>(_sec, NUMSECONDS);
  _doUpdate = true;
  _exists = true;
}


boolean SecondHand::exists(void) {
  return _exists;
}

int SecondHand::mapSecondNo(uint16_t sec) {
    // LED Nummer berechnen
    // 1-37   wird 23 - 59 (immer +22)
    // 38-59  wird 1 - 22  (immer -37)
    // oder LED-Nr + 22 Modulo 59
    //      1  2  3  4  5  6  7  8  9  10 11 12 13 14 15  
    //      22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 
    // 59 21                                              37 16
    // 58 20                                              38 17
    // 57 19                                              39 18
    // 56 18                                              40 19
    // 55 17                                              41 20
    // 54 16                                              42 21
    // 53 15                                              43 22
    // 52 14                                              44 23
    // 51 13                                              45 24
    // 50 12                                              46 25
    // 49 11                                              47 26
    // 48 10                                              48 27
    // 47  9                                              49 28
    // 46  8                                              50 29
    // 45  7                                              51 30
    //       6  5  4  3  2  1  0     58 57 56 55 54 53 52
    //      44  43 42 41 40 39 38    37 36 35 34 33 32 31
    //Serial.printf("sec in=%d",sec);
    sec = (sec+22) % 59;
    if(!sec) sec=59;
    //Serial.printf(", sec out=%d\n",sec-1);
    return(sec-1);
}

void SecondHand::drawSecond(void) {
  
  _aktSecond = mytm->second();
  if(_doUpdate) {
    if(_aktSecond) {
      if(_aktSecond != _lastSecond) {
#if defined(DEBUG_SECONDHAND)
        Serial.printf("drawSecond:void: _aktSecond=%d, c=%d, last=%d, mode=%d\n",_aktSecond, settings->mySettings.ledcol,_lastSecond,mode);
#endif
        drawSecond(_aktSecond);
        show();
      }
    } else if(_lastSecond == 59) {
      if(settings->mySettings.secondsAll)
        clearAllSeconds();
      else
        clearSecond(59);
      _lastSecond = 0;
      show();
    }
  }
}

// sec 1-59   ==> Sekundenzeiger
void SecondHand::drawSecond(uint16_t sec) {
  color_s cs;
  uint32_t c;

  if(settings->mySettings.secondHand) {
    c = settings->mySettings.ledcol;
    cs.red = c >> 16;
    cs.green =  (c >> 8) & 0xff;
    cs.blue = c & 0xff;
  } else {
    //color = WHITE;
    cs.red = 0;
    cs.green = 0;
    cs.blue = 0;
  }
#ifdef DEBUG_SECONDHAND
  Serial.printf("drawSecond:COLOR_T: sec=%d, c=%#.6x, secondHand=%d\n", sec, color, settings->mySettings.secondHand);
  Serial.flush();
#endif
  if(!c) {
    c = ledDriver->getRGBFromDegRnd(ledDriver->getDegree(sec));
    cs.red = c >> 16;
    cs.green =  (c >> 8) & 0xff;
    cs.blue = c & 0xff;
  }
  
  if(sec) { // Sekundenzeiger
    uint16_t sec1 = mapSecondNo(sec);
#ifdef DEBUG_SECONDHAND
    Serial.printf("set   led# %d to %#2x %#2x %#2x\n", sec1, cs.red, cs.green, cs.blue);
#endif
    //cs.red *= ledDriver->getBrightness() * 0.00392157;
    //cs.green *= ledDriver->getBrightness() * 0.00392157; 
    //cs.blue *= ledDriver->getBrightness() * 0.00392157;
    if(_sec)
      _sec[sec1] = CRGB(cs.red, cs.green, cs.blue);

    if(_lastSecond && !settings->mySettings.secondsAll)
      clearSecond(_lastSecond);
    _lastSecond=sec;
  } 
}

void SecondHand::clearAllSeconds(void) {
#ifdef DEBUG_SECONDHAND
  Serial.println("clear all  ");
#endif
  if(_sec)
     fill_solid(_sec, NUMSECONDS, CRGB::Black);
  _lastSecond=0;
}


void SecondHand::clearSecond(uint16_t sec) {
  if(sec) {
    uint16_t sec1 = mapSecondNo(sec);
#ifdef DEBUG_SECONDHAND
    Serial.print("clear led# ");
    Serial.println(sec1);
#endif
    if(_sec)
      _sec[sec1] =  CRGB(0,0,0);
  } 
}


void SecondHand::setQuarterWidth(uint16_t w) {
  if(w>0 && w<60)
    _quarterWidth=w;
}


void SecondHand::drawQuarter(void) {
  uint16_t quarter;
  if(!settings->mySettings.secondHand)
    return;
  switch(mytm->minute()) {
    case 0:
      quarter=4;
      break;
    case 15:
      quarter=1;
      break;
    case 30:
      quarter=2;
      break;
    case 45:
      quarter=3;
      break;
    default:
      quarter=0;
      break;
  }
  if(quarter)
    drawQuarter(quarter);
}


void SecondHand::drawQuarter(uint16_t quarter) {
  _doUpdate = false;
  uint16_t dly = map(_quarterWidth,1,59,30,1);
  clearAllSeconds();
  for(uint16_t q=0; q<quarter ;q++) {
    for(uint16_t i=1; i<=59+_quarterWidth; i++) {
      if(i<60) {
        _lastSecond=0;
        drawSecond(i);
      }
      if(i>_quarterWidth)
        clearSecond(i-_quarterWidth);
      _doUpdate = false;
      show(dly);
    }
  }
  _doUpdate = true;
}


void SecondHand::show(uint16_t dly) {
#ifdef DEBUG_SECONDHAND
  if(modeExtraTime)
    Serial.printf("show: _lastSecond=%d, _aktSecond=%d\n",_lastSecond, _aktSecond);
#endif
  ledDriver->show();
  vTaskDelay(pdMS_TO_TICKS(dly));
}
#endif