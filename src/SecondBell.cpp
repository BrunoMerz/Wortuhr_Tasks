#if defined(WITH_SECOND_BELL)
#include "SecondBell.h"
#include "Settings.h"
#include "MyTime.h"
#include "LedDriver_FastLED.h"

static Settings *settings=Settings::getInstance();
static LedDriver *ledDriver=LedDriver::getInstance();
static MyTime *mytm=MyTime::getInstance();

#ifdef SYSLOGSERVER_SERVER
#include "Syslog.h"
extern Syslog syslog;
#endif
 
//#define DEBUG_SECONDBELL

SecondBell* SecondBell::instance = 0;

SecondBell *SecondBell::getInstance() {
  if (!instance)
  {
      instance = new SecondBell();
  }
  return instance;
}

SecondBell::SecondBell(void) {
    brightness=10;
    _status=true;
}

void SecondBell::blinkSecond(void) {
    static MyTime *mytm=MyTime::getInstance();
    if(_status) {
        uint32_t c = settings->mySettings.ledcol;
        color_s cs;
        cs.red = c >> 16;
        cs.green =  (c >> 8) & 0xff;
        cs.blue = c & 0xff;
        _aktSecond = mytm->second();

    #ifdef DEBUG_SECONDBELL
        Serial.printf("blinkSecond: sec=%d, secondsBell=%d\n",_aktSecond, settings->mySettings.secondsBell);
    #endif
        if(!c) {
            c = ledDriver->getRGBFromDegRnd(ledDriver->getDegree(0));
            cs.red = c >> 16;
            cs.green =  (c >> 8) & 0xff;
            cs.blue = c & 0xff;
        }
    
        if((_aktSecond % 2 == 0) || !settings->mySettings.secondsBell) {
            cs.red  = 0;
            cs.green= 0;
            cs.blue = 0;
        } else {
            //cs.red *= brightness * 0.0039;
            //cs.green *= brightness * 0.0039;
            //cs.blue *= brightness * 0.0039;
        }
    #ifdef DEBUG_SECONDBELL
        Serial.printf("blinkSecond: r=%d, g=%d, b=%d\n",cs.red,cs.green,cs.blue);
    #endif

        ledDriver->setPixelRGB(114, cs.red, cs.green, cs.blue);
        ledDriver->show();
    }
}

bool SecondBell::getStatus(void) {
    return _status;
}

void SecondBell::setStatus(bool status) {
    _status = status;
    if(!_status) {
        ledDriver->setPixelRGB(114, 0, 0, 0);
        ledDriver->show();
    }
}

#endif