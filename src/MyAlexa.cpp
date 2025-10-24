/**
   MyAlexa.cpp
   @autor    Bruno Merz

   @version  1.0
   @created  03.10.2023

*/

#if defined(WITH_ALEXA)

#define myDEBUG
#include "MyDebug.h"
#include "Configuration.h"
#include "MyAlexa.h"

#if defined(WITH_MQTT)
  #include "MyMqtt.h"
#endif

//extern void setLedsOff();
//extern void setLedsOn();

static Mode *_mode;
static Settings *settings;
static LedDriver *ledDriver;

MyAlexa* MyAlexa::instance = 0;

MyAlexa *MyAlexa::getInstance() {
  if (!instance)
  {
      instance = new MyAlexa();
  }
  return instance;
}

MyAlexa::MyAlexa() {
  
}

void wordclockChanged(EspalexaDevice* d)
{
  if (d == nullptr) return;
  bool state = d->getState();
  uint32_t c = d->getRGB();
  uint8_t  p = d->getPercent();

  DEBUG_PRINTF("percent=%d, color=0x%06x, state=%d, value=%d\n",p,c,state,d->getValue());
  
  if(c == 0xffffff)
    c=0;

  if(!state && ledDriver->mode != MODE_BLANK)
    ledDriver->setOnOff();
  else if(state) {
    if(ledDriver->mode == MODE_BLANK)
      ledDriver->setOnOff();
    settings->mySettings.brightness = p;
    settings->mySettings.ledcol = c;
    settings->mySettings.corcol = c;
    for(uint16_t y=0; y<LED_ROWS; y++)
      for(uint16_t x=0; x<LED_COLS; x++) {
        if(ledDriver->getPixelRGB(x,y)) {
          ledDriver->setPixel(x, y, c);
        }
      }
  }
  ledDriver->show();
}


void MyAlexa::init(char *systemname, Mode *mode, AsyncWebServer *webServer) {
  settings = Settings::getInstance();
  ledDriver = LedDriver::getInstance();

  _mode = mode;
  uint32_t c=settings->mySettings.ledcol;

  DEBUG_PRINTF("init: percent=%d, color=0x%06x\n",settings->mySettings.brightness, c);

  espalexadev = new EspalexaDevice(systemname, wordclockChanged, EspalexaDeviceType::extendedcolor); //color + color temperature
  espalexadev->setPercent(settings->mySettings.brightness); //set value "brightness" in percent
  espalexadev->setColor((uint8_t)(c>>16 & 0xFF),(c>>8 & 0xFF),(c & 0xFF));

#if defined(WITH_MQTT)
  extern MyMqtt *mqtt;
  char buf[30];
  sprintf(buf,"{\"name\":\"%s\"}", systemname);
  mqtt->publish("addDevice",buf);
#endif
  espalexa.addDevice(espalexadev);
  espalexa.begin(webServer);
}

void MyAlexa::handle(void) {
  espalexa.loop();
}


#endif
