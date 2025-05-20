#include <Arduino.h>
#include "Configuration.h"
#include "Settings.h"
#include "Renderer.h"
#include "IconRenderer.h"
#include "LedDriver_FastLED.h"
#include "MyWifi.h"
#include "MyTime.h"
#include "SecondBell.h"
#include "Modes.h"
#include "WebHandler.h"
#include "OpenWeather.h"
#include "Animation.h"
#include "Global.h"
#include "ElegantOTA.h"
#include "TaskStructs.h"



#define myDEBUG
#include "MyDebug.h"

static Global *glb = Global::getInstance();

s_taskParams taskParams;

unsigned long ota_progress_millis = 0;

extern void initFS();
extern bool ausperiode(void);

EventGroupHandle_t xEvent;
SemaphoreHandle_t xMutex;
uint16_t nightOffTime;
uint16_t dayOnTime;


uint16_t matrix[10] = {};

static AnimationFS *anifs = AnimationFS::getInstance();
static OpenWeather *ow = OpenWeather::getInstance();

void onOTAStart() {
  // Log when OTA has started
  Serial.println("OTA update started!");
  // <Add your own code here>
}

void onOTAProgress(size_t current, size_t final) {
  // Log every 1 second
  if (millis() - ota_progress_millis > 1000) {
    ota_progress_millis = millis();
    Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
  }
}

void onOTAEnd(bool success) {
  // Log when OTA has finished
  if (success) {
    Serial.println("OTA update finished successfully!");
  } else {
    Serial.println("There was an error during OTA update!");
  }
  // <Add your own code here>
}


void queueScheduler(void *p) {
  s_taskParams *tp = (s_taskParams*)p;
  Settings *settings=Settings::getInstance();
  MyTime *mt=MyTime::getInstance();
  OpenWeather *ow = OpenWeather::getInstance();
  LedDriver *ledDriver = LedDriver::getInstance();
  MyWifi *myWifi=MyWifi::getInstance();

  int lastHour=0;
  int lastMinute=0;
  int lastSecond=0;
  int aktDay;
  int aktHour;
  int aktMinute;
  int aktSecond;
  uint8_t akt_transition = 1;
  uint8_t akt_transition_old = 1;
  nightOffTime = settings->mySettings.nightOffTime;
  dayOnTime = settings->mySettings.dayOnTime;
  uint32_t aktts;
  bool isNightOff;
  uint16_t autoModeChangeTimer = settings->mySettings.auto_mode_change * 60;
  uint8_t autoMode = 0;
  uint32_t colorsaver;
  

  DEBUG_PRINTF("queueScheduler: Core=%d\n", xPortGetCoreID());

  while(true) {
    //Serial.println("displayTime loop");
    mt->getTime();
    aktDay = mt->mytm.tm_mday;
    aktHour = mt->mytm.tm_hour;
    aktMinute = mt->mytm.tm_min;
    aktSecond = mt->mytm.tm_sec;
    
    isNightOff=false;
    if(aktMinute != lastMinute) {
      lastMinute = aktMinute;

      isNightOff=false;
      aktts = aktHour * 3600 + aktMinute * 60;
      if (nightOffTime <= dayOnTime)
      {
        if (aktts >= nightOffTime && aktts < dayOnTime)
          isNightOff=true;
      }
      else
      {
        if ((aktts >= nightOffTime && aktts < 1440) || (aktts >= 0 && aktts < dayOnTime))
        isNightOff=true;
      }

      if(isNightOff && ledDriver->mode != MODE_BLANK) 
        ledDriver->setOnOff();

      if(!isNightOff && ledDriver->mode == MODE_BLANK)
        ledDriver->setOnOff();

      if (xEventGroupSetBits(xEvent, MODE_TIME)) {
        glb->stackSize = tp->taskInfo[1].stackSize;
        glb->highWaterMark = uxTaskGetStackHighWaterMark(NULL);
        glb->codeline = __LINE__;
        glb->codetab = __NAME__;
        DEBUG_PRINTF("Gesendet: MODE_TIME, aktMinute=%d, uxHighWaterMark=%d\n", aktMinute, glb->highWaterMark);
      }
    }

    autoModeChangeTimer--;
    if (!autoModeChangeTimer)
    {
      if (aktSecond > 45)
      {
        autoModeChangeTimer = autoModeChangeTimer + 30;
      }
      else if (aktSecond < 15)
      {
        autoModeChangeTimer = autoModeChangeTimer + 15;
      }
      else
      {

        DEBUG_PRINTF("autoMode=%d, aktSecond=%d\n", autoMode, aktSecond);

        autoModeChangeTimer = settings->mySettings.auto_mode_change * 60;
        //mode_ohne_sound = true;
        //single_mode = true;
        colorsaver = settings->mySettings.ledcol;
        switch (autoMode)
        {
        case 0:
#ifdef APIKEY
          if (myWifi->isConnected() && strlen(settings->mySettings.openweatherapikey) > 25) {
            if (xEventGroupSetBits(xEvent, MODE_EXT_TEMP)) {
                glb->stackSize = tp->taskInfo[1].stackSize;
                glb->highWaterMark = uxTaskGetStackHighWaterMark(NULL);
                glb->codeline = __LINE__;
                glb->codetab = __NAME__;
                DEBUG_PRINTF("Gesendet: MODE_EXT_TEMP, aktMinute=%d, uxHighWaterMark=%d\n", aktMinute, glb->highWaterMark);
              }
            }
#endif
          autoMode = 1;
          break;

        case 1:

#ifdef APIKEY
          if (myWifi->isConnected() && strlen(settings->mySettings.openweatherapikey) > 25) {
            if (xEventGroupSetBits(xEvent, MODE_EXT_TEMP)) {
                glb->stackSize = tp->taskInfo[1].stackSize;
                glb->highWaterMark = uxTaskGetStackHighWaterMark(NULL);
                glb->codeline = __LINE__;
                glb->codetab = __NAME__;
                DEBUG_PRINTF("Gesendet: MODE_EXT_TEMP, aktMinute=%d, uxHighWaterMark=%d\n", aktMinute, glb->highWaterMark);
              }
          }
#else
#ifdef SHOW_MODE_DATE
          if (xEventGroupSetBits(xEvent, MODE_DATE)) {
            glb->stackSize = tp->taskInfo[1].stackSize;
            glb->highWaterMark = uxTaskGetStackHighWaterMark(NULL);
            glb->codeline = __LINE__;
            glb->codetab = __NAME__;
            DEBUG_PRINTF("Gesendet: MODE_DATE, aktMinute=%d, uxHighWaterMark=%d\n", aktMinute, glb->highWaterMark);
          }
#endif
#endif

          autoMode = 2;
          break;

        case 2:
#ifdef SHOW_MODE_MOONPHASE
          if (xEventGroupSetBits(xEvent, MODE_MOONPHASE)) {
            glb->stackSize = tp->taskInfo[1].stackSize;
            glb->highWaterMark = uxTaskGetStackHighWaterMark(NULL);
            glb->codeline = __LINE__;
            glb->codetab = __NAME__;
            DEBUG_PRINTF("Gesendet: MODE_MOONPHASE, aktMinute=%d, uxHighWaterMark=%d\n", aktMinute, glb->highWaterMark);
          }
#endif
          autoMode = 3;
          break;

        case 3:
#ifdef SHOW_MODE_DATE
          if (xEventGroupSetBits(xEvent, MODE_DATE)) {
            glb->stackSize = tp->taskInfo[1].stackSize;
            glb->highWaterMark = uxTaskGetStackHighWaterMark(NULL);
            glb->codeline = __LINE__;
            glb->codetab = __NAME__;
            DEBUG_PRINTF("Gesendet: MODE_DATE, aktMinute=%d, uxHighWaterMark=%d\n", aktMinute, glb->highWaterMark);
          }
#endif
          autoMode = 4;
          break;

        case 4:
#ifdef APIKEY
#ifdef SHOW_MODE_WETTER
          if (strlen(settings->mySettings.openweatherapikey) > 25) {
            if (xEventGroupSetBits(xEvent, MODE_WETTER)) {
              glb->stackSize = tp->taskInfo[1].stackSize;
              glb->highWaterMark = uxTaskGetStackHighWaterMark(NULL);
              glb->codeline = __LINE__;
              glb->codetab = __NAME__;
              DEBUG_PRINTF("Gesendet: MODE_WETTER, aktMinute=%d, uxHighWaterMark=%d\n", aktMinute, glb->highWaterMark);
            } 
          }
#endif
#else
#ifdef SHOW_MODE_MOONPHASE
          if (xEventGroupSetBits(xEvent, MODE_MOONPHASE)) {
            glb->stackSize = tp->taskInfo[1].stackSize;
            glb->highWaterMark = uxTaskGetStackHighWaterMark(NULL);
            glb->codeline = __LINE__;
            glb->codetab = __NAME__;
            DEBUG_PRINTF("Gesendet: MODE_MOONPHASE, aktMinute=%d, uxHighWaterMark=%d\n", aktMinute, glb->highWaterMark);
          }
#endif
#endif
          autoMode = 0;
          break;
        }
      }
    }

    ElegantOTA.loop();
    vTaskDelay(pdMS_TO_TICKS(500)); // 1 Sekunde warten
  }
}


void weatherQueueHandler(void *p) {
  s_taskParams *tp = (s_taskParams*)p;
  LedDriver *ledDriver=LedDriver::getInstance();
  Renderer *renderer=Renderer::getInstance();
  Settings *settings=Settings::getInstance();
  MyTime *mt=MyTime::getInstance();
  OpenWeather *outdoorWeather = OpenWeather::getInstance();

  String animation;
  DEBUG_PRINTF("weather: Core=%d\n", xPortGetCoreID());
  while(true) {
    if (xEventGroupWaitBits(xEvent, MODE_WETTER, pdFALSE, pdFALSE, portMAX_DELAY)) {
      if (xSemaphoreTake(xMutex, portMAX_DELAY)) {
        if(ledDriver->mode == MODE_BLANK) { xSemaphoreGive(xMutex); vTaskDelay(pdMS_TO_TICKS(10)); continue; }

        glb->stackSize = tp->taskInfo[6].stackSize;
        glb->highWaterMark = uxTaskGetStackHighWaterMark(NULL);
        glb->codeline = __LINE__;
        glb->codetab = __NAME__;
        DEBUG_PRINTF("Erhalten: MODE_Wetter, aktMinute=%d, aktSecond=%d, uxHighWaterMark=%d\n", mt->mytm.tm_min, mt->mytm.tm_sec, glb->highWaterMark);

        renderer->clearScreenBuffer(matrix);
        outdoorWeather->getOutdoorConditions(String(settings->mySettings.openweatherlocation), String(settings->mySettings.openweatherapikey));
        if ( outdoorWeather->retcodeOutdoorWeather > 0 )
        {
          outdoorWeather->errorCounterOutdoorWeather++;
        }
        else
        {
          animation = F("WETTER_");
          //if (WetterSequenz == 1)
            animation += outdoorWeather->getWeatherIcon(outdoorWeather->weathericon1);
          //if (WetterSequenz == 2001)
          //  animation += outdoorWeather->getWeatherIcon(outdoorWeather->weathericon2);
          animation.toUpperCase();
          if (anifs->loadAnimation(animation))
          {
            //WetterSequenz++;
            anifs->akt_aniframe = 0;
            anifs->akt_aniloop = 0;
            anifs->frame_fak = 0;
          }
          while(anifs->showAnimation(settings->mySettings.brightness)) {
            vTaskDelay(pdMS_TO_TICKS(500));
          }
        }
        xEventGroupClearBits(xEvent, MODE_WETTER);
        ledDriver->lastMode = MODE_WETTER;
        xSemaphoreGive(xMutex);
        xEventGroupSetBits(xEvent, MODE_TIME);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  } // while
}


void tempQueueHandler(void *p) {
  s_taskParams *tp = (s_taskParams*)p;
  LedDriver *ledDriver=LedDriver::getInstance();
  Renderer *renderer=Renderer::getInstance();
  Settings *settings=Settings::getInstance();
  MyTime *mt=MyTime::getInstance();
  OpenWeather *outdoorWeather = OpenWeather::getInstance();
  DEBUG_PRINTF("temp: Core=%d\n", xPortGetCoreID());
  while(true) {
    if (xEventGroupWaitBits(xEvent, MODE_EXT_TEMP, pdFALSE, pdFALSE, portMAX_DELAY)) {
      if (xSemaphoreTake(xMutex, portMAX_DELAY)) {
        if(ledDriver->mode == MODE_BLANK) { xSemaphoreGive(xMutex); vTaskDelay(pdMS_TO_TICKS(10)); continue; }

        glb->stackSize = tp->taskInfo[5].stackSize;
        glb->highWaterMark = uxTaskGetStackHighWaterMark(NULL);
        glb->codeline = __LINE__;
        glb->codetab = __NAME__;
        DEBUG_PRINTF("Erhalten: MODE_EXT_TEMP, aktMinute=%d, aktSecond=%d, stackSize=%d, uxHighWaterMark=%d\n", mt->mytm.tm_min, mt->mytm.tm_sec, glb->stackSize, glb->highWaterMark);

        renderer->clearScreenBuffer(matrix);
        outdoorWeather->getOutdoorConditions(String(settings->mySettings.openweatherlocation), String(settings->mySettings.openweatherapikey));
        if ( outdoorWeather->retcodeOutdoorWeather > 0 )
        {
          outdoorWeather->errorCounterOutdoorWeather++;
        }
        else
        {

        }
        if (outdoorWeather->temperature > 0)
        {
          matrix[1] = 0b0100000000000000;
          matrix[2] = 0b1110000000000000;
          matrix[3] = 0b0100000000000000;
        }
        if (outdoorWeather->temperature < 0)
        {
          matrix[2] = 0b1110000000000000;
        }
        renderer->setSmallText(String(int(abs(outdoorWeather->temperature) + 0.5)), TEXT_POS_BOTTOM, matrix);
        uint8_t Tempcolor;
        if (outdoorWeather->temperature < -10)
          Tempcolor = VIOLET;
        else if (outdoorWeather->temperature >= -10 && outdoorWeather->temperature < -5)
          Tempcolor = BLUE;
        else if (outdoorWeather->temperature >= -5 && outdoorWeather->temperature < 0)
          Tempcolor = LIGHTBLUE;
        else if (outdoorWeather->temperature >= 0 && outdoorWeather->temperature < 5)
          Tempcolor = CYAN;
        else if (outdoorWeather->temperature >= 5 && outdoorWeather->temperature < 10)
          Tempcolor = MINTGREEN;
        else if (outdoorWeather->temperature >= 10 && outdoorWeather->temperature < 15)
          Tempcolor = GREEN;
        else if (outdoorWeather->temperature >= 15 && outdoorWeather->temperature < 20)
          Tempcolor = GREENYELLOW;
        else if (outdoorWeather->temperature >= 20 && outdoorWeather->temperature < 25)
          Tempcolor = YELLOW;
        else if (outdoorWeather->temperature >= 25 && outdoorWeather->temperature < 30)
          Tempcolor = ORANGE;
        else if (outdoorWeather->temperature >= 30 && outdoorWeather->temperature < 40)
          Tempcolor = RED;
        else if (outdoorWeather->temperature >= 40)
          Tempcolor = RED_50;
        ledDriver->writeScreenBufferFade(matrix, colorArray[Tempcolor]);
        vTaskDelay(pdMS_TO_TICKS(3000));

        xEventGroupClearBits(xEvent, MODE_EXT_TEMP);
        ledDriver->lastMode = MODE_EXT_TEMP;
        xSemaphoreGive(xMutex);
        xEventGroupSetBits(xEvent, MODE_TIME);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  } // while
}


void moonQueueHandler(void *p) {
  s_taskParams *tp = (s_taskParams*)p;
  LedDriver *ledDriver=LedDriver::getInstance();
  Renderer *renderer=Renderer::getInstance();
  Settings *settings=Settings::getInstance();
  MyTime *mt=MyTime::getInstance();
  OpenWeather *ow=OpenWeather::getInstance();

  //int moonphase;
  int ani_moonphase;                                                                                                                ;
  DEBUG_PRINTF("moon: Core=%d\n", xPortGetCoreID());
  while(true) {
    if (xEventGroupWaitBits(xEvent, MODE_MOONPHASE, pdFALSE, pdFALSE, portMAX_DELAY)) {
      if (xSemaphoreTake(xMutex, portMAX_DELAY)) {
        if(ledDriver->mode == MODE_BLANK) { xSemaphoreGive(xMutex); vTaskDelay(pdMS_TO_TICKS(10)); continue; }

        glb->stackSize = tp->taskInfo[2].stackSize;
        glb->highWaterMark = uxTaskGetStackHighWaterMark(NULL);
        glb->codeline = __LINE__;
        glb->codetab = __NAME__;
        DEBUG_PRINTF("Erhalten: MODE_MOONPHASE, aktMinute=%d, aktSecond=%d, uxHighWaterMark=%d\n",mt->mytm.tm_min, mt->mytm.tm_sec, glb->highWaterMark);

        renderer->clearScreenBuffer(matrix);

        DEBUG_PRINTLN("moonphase: " + String(ow->moonphase));
        
        for(int ModeSequenz=1; ModeSequenz<=5; ModeSequenz++) {
          ani_moonphase =  ow->moonphase - 5 + ModeSequenz;

          if ( ani_moonphase < 0 ) ani_moonphase = 22 + ani_moonphase;
        
          DEBUG_PRINTLN("ani_moonphase: " + String(ani_moonphase));
        
          ledDriver->saveMatrix(matrix);
          for (uint8_t i = 0; i <= 9; i++)
          {
            matrix[i] = pgm_read_word(&MONDMATRIX[ani_moonphase][i]);
          }
          ledDriver->writeScreenBufferFade(matrix, colorArray[ORANGE]);
          vTaskDelay(pdMS_TO_TICKS(10)); // 1 Sekunde warten
        }
        xEventGroupClearBits(xEvent, MODE_MOONPHASE);
        ledDriver->lastMode = MODE_MOONPHASE;
        xSemaphoreGive(xMutex);
        xEventGroupSetBits(xEvent, MODE_TIME); // Uhrzeit wieder anzeigen
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  } // while
}


void showTextHandler(void *p) {
  s_taskParams *tp = (s_taskParams*)p;
  LedDriver *ledDriver=LedDriver::getInstance();
  Renderer *renderer=Renderer::getInstance();
  Settings *settings=Settings::getInstance();
  MyTime *mt=MyTime::getInstance();
                                                                                                          ;
  DEBUG_PRINTF("showText: Core=%d\n", xPortGetCoreID());
  while(true) {
    if (xEventGroupWaitBits(xEvent, MODE_FEED, pdFALSE, pdFALSE, portMAX_DELAY)) {
      if (xSemaphoreTake(xMutex, portMAX_DELAY)) {
        if(ledDriver->mode == MODE_BLANK) { xSemaphoreGive(xMutex); vTaskDelay(pdMS_TO_TICKS(10)); continue; }

        glb->stackSize = tp->taskInfo[7].stackSize;
        glb->highWaterMark = uxTaskGetStackHighWaterMark(NULL);
        glb->codeline = __LINE__;
        glb->codetab = __NAME__;
        DEBUG_PRINTF("Erhalten: MODE_FEED, aktMinute=%d, aktSecond=%d, uxHighWaterMark=%d\n", mt->mytm.tm_min, mt->mytm.tm_sec, glb->highWaterMark);

        ledDriver->mode = MODE_FEED;
        DEBUG_PRINTLN(tp->feedText);

        tp->feedText.replace("\xC3\x84", "\x7F"); // Ä
        tp->feedText.replace("\xC3\xA4", "\x80"); // ä
        tp->feedText.replace("\xC3\x9C", "\x81"); // Ü
        tp->feedText.replace("\xC3\xBC", "\x82"); // ü
        tp->feedText.replace("\xC3\x96", "\x83"); // Ö
        tp->feedText.replace("\xC3\xB6", "\x84"); // ö
        tp->feedText.replace("\xC3\x9F", "\x85"); // ß

        for(int i=0; i<tp->feedText.length()-2; i++) {
          if (tp->feedText[i] < 32 || tp->feedText[i] > 133)
            tp->feedText[i] = '.';
          if (tp->feedText[i + 1] < 32 || tp->feedText[i + 1] > 133)
            tp->feedText[i + 1] = '.';
          if (tp->feedText[i + 2] < 32 || tp->feedText[i + 2] > 133)
            tp->feedText[i + 2] = '.';
    
          for (uint8_t y = 0; y <= 5; y++)
          {
            renderer->clearScreenBuffer(matrix);
            for (uint8_t z = 0; z <= 6; z++)
            {
              uint8_t b = pgm_read_byte(&lettersBig[tp->feedText[i] - 32][z]);
              //					matrix[2 + z] |= (lettersBig[tp->feedText[tp->feedPosition] - 32][z] << 11 + y) & 0b1111111111100000;
              matrix[2 + z] |= (b << 11 + y) & 0b1111111111100000;
              b = pgm_read_byte(&lettersBig[tp->feedText[i + 1] - 32][z]);
              //					matrix[2 + z] |= (lettersBig[tp->feedText[tp->feedPosition + 1] - 32][z] << 5 + y) & 0b1111111111100000;
              matrix[2 + z] |= (b << 5 + y) & 0b1111111111100000;
              b = pgm_read_byte(&lettersBig[tp->feedText[i + 2] - 32][z]);
              //					matrix[2 + z] |= (lettersBig[tp->feedText[tp->feedPosition + 2] - 32][z] << y - 1) & 0b1111111111100000;
              matrix[2 + z] |= (b << y - 1) & 0b1111111111100000;
            }
            ledDriver->writeScreenBuffer(matrix, tp->feedColor);
            ledDriver->show();
            vTaskDelay(pdMS_TO_TICKS(FEED_SPEED));
            /*
            if (mode == MODE_IP)
            {
              delay(FEED_SPEED / 2);
    #ifdef WITH_AUDIO
              if (PABStatus > 0)
                PABStatus = PlayAudioBuffer(0);
    #endif
            }
            */
          }


          /*
          if (post_event_ani)
          {
            post_event_ani = false;
            while (showAnimation(brightness))
            {

            }
          }
          */
  #ifdef WITH_AUDIO
          if (audio_stop_nach_feed)
            Play_MP3(99, false, 0); // Stop
          audio_stop_nach_feed = true;
  #endif
        }

        
        xEventGroupClearBits(xEvent, MODE_FEED);
        ledDriver->lastMode = MODE_FEED;
        xSemaphoreGive(xMutex);
        xEventGroupSetBits(xEvent, MODE_TIME); // Uhrzeit wieder anzeigen
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  } // while
}


void dateQueueHandler(void *p) {
  s_taskParams *tp = (s_taskParams*)p;
  LedDriver *ledDriver=LedDriver::getInstance();
  Renderer *renderer=Renderer::getInstance();
  Settings *settings=Settings::getInstance();
  MyTime *mt=MyTime::getInstance();

  uint8_t aktDay;
  uint8_t aktMonth;
  DEBUG_PRINTF("date: Core=%d\n", xPortGetCoreID());
  while(true) {
    if (xEventGroupWaitBits(xEvent, MODE_DATE, pdFALSE, pdFALSE, portMAX_DELAY)) {
      if (xSemaphoreTake(xMutex, portMAX_DELAY)) {
        if(ledDriver->mode == MODE_BLANK) { xSemaphoreGive(xMutex); vTaskDelay(pdMS_TO_TICKS(10)); continue; }

        glb->stackSize = tp->taskInfo[0].stackSize;
        glb->highWaterMark = uxTaskGetStackHighWaterMark(NULL);
        glb->codeline = __LINE__;
        glb->codetab = __NAME__;
        DEBUG_PRINTF("Erhalten: MODE_DATE, aktMinute=%d, aktSecond=%d, glb->stackSize=%duxHighWaterMark=%d\n", mt->mytm.tm_min, mt->mytm.tm_sec, glb->highWaterMark);

        aktDay = mt->mytm.tm_mday;
        aktMonth = mt->mytm.tm_mon;
        renderer->clearScreenBuffer(matrix);
        if (aktDay < 10)
          renderer->setSmallText("0"+String(aktDay), TEXT_POS_TOP, matrix);
        else
          renderer->setSmallText(String(aktDay), TEXT_POS_TOP, matrix);

        if (aktMonth < 10)
          renderer->setSmallText("0"+String(aktMonth), TEXT_POS_BOTTOM, matrix);
        else
          renderer->setSmallText(String(aktMonth), TEXT_POS_BOTTOM, matrix);
  
        ledDriver->mode = MODE_DATE;
        ledDriver->writeScreenBuffer(matrix, colorArray[YELLOW]);
        vTaskDelay(pdMS_TO_TICKS(3000));

        xEventGroupClearBits(xEvent, MODE_DATE);
        ledDriver->lastMode = MODE_DATE;
        xSemaphoreGive(xMutex);
        xEventGroupSetBits(xEvent, MODE_TIME); // Uhrzeit wieder anzeigen
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  } // while
}


void displaySecondBell(void *) {
  MyTime *mt=MyTime::getInstance();
  SecondBell *secondBell=SecondBell::getInstance();
  LedDriver *ledDriver=LedDriver::getInstance();

  int lastSecond=0;
  int aktSecond;
  DEBUG_PRINTF("second: Core=%d\n", xPortGetCoreID());
  while(true) {
    mt->getTime();
    aktSecond = mt->mytm.tm_sec;
    if(aktSecond!=lastSecond) {
      lastSecond=aktSecond;
      secondBell->blinkSecond();
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}


void displayTime(void *p) {
  s_taskParams *tp = (s_taskParams*)p;
  Settings *settings=Settings::getInstance();
  MyTime *mt=MyTime::getInstance();
  LedDriver *ledDriver=LedDriver::getInstance();
  Renderer *renderer=Renderer::getInstance();

  int lastHour=0;
  int lastMinute=0;
  int lastSecond=0;
  int aktDay;
  int aktHour;
  int aktMinute;
  int aktSecond;
  uint8_t lastDay = 0;
  uint8_t akt_transition = 1;
  uint8_t akt_transition_old = 1;
  mt->mytm.tm_ntpserver = settings->mySettings.ntphost;
  mt->mytm.tm_timezone = settings->mySettings.timezone; //"CET-1CEST,M3.5.0,M10.5.0/3";
  mt->mytm.tm_lon = settings->mySettings.longitude;
  mt->mytm.tm_lat = settings->mySettings.latitude;
  DEBUG_PRINTLN(F("configure time"));
  mt->confTime();
  DEBUG_PRINTLN(F("get time"));
  DEBUG_PRINTF("time: Core=%d\n", xPortGetCoreID());
  while(true) {
    if (xEventGroupWaitBits(xEvent, MODE_TIME, pdFALSE, pdFALSE, portMAX_DELAY)) {
      if (xSemaphoreTake(xMutex, portMAX_DELAY)) {
        if(ledDriver->mode == MODE_BLANK) { xSemaphoreGive(xMutex); vTaskDelay(pdMS_TO_TICKS(10)); continue; }

        glb->stackSize = tp->taskInfo[3].stackSize;
        glb->highWaterMark = uxTaskGetStackHighWaterMark(NULL);
        glb->codeline = __LINE__;
        glb->codetab = __NAME__;
        DEBUG_PRINTF("Erhalten: MODE_TIME, aktMinute=%d, aktSecond=%d, stackSize=%d, uxHighWaterMark=%d\n", mt->mytm.tm_min, mt->mytm.tm_sec, glb->stackSize, glb->highWaterMark);

        ledDriver->mode = MODE_TIME;
        mt->getTime();
        aktDay = mt->mytm.tm_mday;
        aktHour = mt->mytm.tm_hour;
        aktMinute = mt->mytm.tm_min;
        aktSecond = mt->mytm.tm_sec;
        
        ledDriver->saveMatrix(matrix);
        DEBUG_PRINTF("New Minute: %d\n",aktMinute);
        lastMinute=aktMinute;
        ledDriver->setDegreeOffset((aktHour*15+aktMinute*6)%360);
        ledDriver->corcol=settings->mySettings.corcol;
        renderer->clearScreenBuffer(matrix);
        renderer->setTime(aktHour, aktMinute, matrix);
        renderer->setCorners(aktMinute, matrix);
        if (!settings->mySettings.itIs && ((aktMinute / 5) % 6))
          renderer->clearEntryWords(matrix);

        if (aktSecond != lastSecond)
        {
          lastSecond = aktSecond;
#ifdef LDR
          // Set brightness from LDR
          if (settings->mySettings.useAbc)
            ledDriver->setBrightnessFromLdr();
#endif
        }

        if (aktDay != lastDay)
        {
          lastDay = aktDay;
      
#ifdef SHOW_MODE_MOONPHASE
          ow->moonphase = ow->getMoonphase(mt->mytm.tm_year, mt->mytm.tm_mon, mt->mytm.tm_mday);
#endif
      
// SunRise
#ifdef SunRiseLib
          ow->sr.calculate(settings->mySettings.latitude, settings->mySettings.longitude, mt->mytm.tm_loc);
          ow->sunRiseTime = ow->sr.riseTime;
          ow->sunSetTime = ow->sr.setTime;
          ow->sunriseset();
          DEBUG_PRINTF("Sunrise: %02u:%02u:%02u %02u.%02u.%04u\r\n", ow->sunriseHour, ow->sunriseMinute, ow->sunriseSecond, mt->day(ow->sunRiseTime), mt->month(ow->sunRiseTime), mt->year(ow->sunRiseTime));
          DEBUG_PRINTF("Sunset : %02u:%02u:%02u %02u.%02u.%04u\r\n", ow->sunsetHour, ow->sunsetMinute, ow->sunsetSecond, mt->day(ow->sunSetTime), mt->month(ow->sunSetTime), mt->year(ow->sunSetTime));
#endif
        }

        if (aktMinute % 5 != 0 || tp->updateSceen)
        {
          ledDriver->writeScreenBufferFade(matrix, settings->mySettings.ledcol);
        }
        else
        {
          akt_transition = settings->mySettings.transition;
          if ( settings->mySettings.transition == TRANSITION_RANDOM )
          {
            akt_transition_old = akt_transition;
            for (uint8_t i = 0; i <= 20; i++)
            {
              akt_transition = random(TRANSITION_NORMAL + 1, TRANSITION_MAX);
              if ( akt_transition != akt_transition_old ) break;
            }
          }
          if ( settings->mySettings.transition == TRANSITION_ALLE_NACHEINANDER )
          {
            akt_transition_old++;
            if ( akt_transition_old >= TRANSITION_MAX ) akt_transition_old = TRANSITION_NORMAL + 1;
            akt_transition = akt_transition_old;
          }

          if (akt_transition == TRANSITION_NORMAL)
            ledDriver->writeScreenBuffer(matrix, settings->mySettings.ledcol);
          else if (akt_transition == TRANSITION_MOVEUP)
            ledDriver->moveScreenBufferUp(matrix, settings->mySettings.ledcol);
          else if (akt_transition == TRANSITION_MOVEDOWN)
            ledDriver->moveScreenBufferDown(matrix, settings->mySettings.ledcol);
          else if (akt_transition == TRANSITION_MOVELEFT)
            ledDriver->moveScreenBufferLeft(matrix, settings->mySettings.ledcol);
          else if (akt_transition == TRANSITION_MOVERIGHT)
            ledDriver->moveScreenBufferRight(matrix, settings->mySettings.ledcol);
          else if (akt_transition == TRANSITION_MOVELEFTDOWN)
            ledDriver->moveScreenBufferLeftDown(matrix, settings->mySettings.ledcol);
          else if (akt_transition == TRANSITION_MOVERIGHTDOWN)
            ledDriver->moveScreenBufferRightDown(matrix, settings->mySettings.ledcol);
          else if (akt_transition == TRANSITION_MOVECENTER)
            ledDriver->moveScreenBufferCenter(matrix, settings->mySettings.ledcol);
          else if (akt_transition == TRANSITION_FADE)
            ledDriver->writeScreenBufferFade(matrix, settings->mySettings.ledcol);
          else if (akt_transition == TRANSITION_FARBENMEER)
            ledDriver->farbenmeer(matrix, settings->mySettings.ledcol);
          else if (akt_transition == TRANSITION_MATRIX)
            ledDriver->matrix_regen(matrix, settings->mySettings.ledcol);
          else if (akt_transition == TRANSITION_SPIRALE_LINKS)
            ledDriver->moveSeriell(matrix, settings->mySettings.ledcol, SPIRALE_LINKS);
          else if (akt_transition == TRANSITION_ZEILENWEISE)
            ledDriver->moveSeriell(matrix, settings->mySettings.ledcol, ZEILENWEISE);
          else if (akt_transition == TRANSITION_SPIRALE_RECHTS)
            ledDriver->moveSeriell(matrix, settings->mySettings.ledcol, SPIRALE_RECHTS);
          else if (akt_transition == TRANSITION_MITTE_LINKSHERUM)
            ledDriver->moveSeriell(matrix, settings->mySettings.ledcol, MITTE_LINKSHERUM);
          else if (akt_transition == TRANSITION_REGENBOGEN)
            ledDriver->regenbogen(matrix, settings->mySettings.ledcol);
          else if (akt_transition == TRANSITION_QUADRATE)
            ledDriver->quadrate(matrix, settings->mySettings.ledcol);
          else if (akt_transition == TRANSITION_KREISE)
            ledDriver->kreise(matrix, settings->mySettings.ledcol);
          else
          {
            ledDriver->writeScreenBuffer(matrix, settings->mySettings.ledcol);
          }
          vTaskDelay(pdMS_TO_TICKS(500));
        } // else aktMinute % 5 != 0

        xEventGroupClearBits(xEvent, MODE_TIME);
        ledDriver->lastMode = MODE_TIME;
        xSemaphoreGive(xMutex);
      }
      vTaskDelay(pdMS_TO_TICKS(500));
    } // if (xEventGroupWaitBits
    vTaskDelay(pdMS_TO_TICKS(10));
  } // while(true)
}


void startup(void *){
  Settings *settings=Settings::getInstance();
  Renderer *renderer=Renderer::getInstance();
  IconRenderer *icor=IconRenderer::getInstance();
  LedDriver *ledDriver=LedDriver::getInstance();
  MyWifi *myWifi=MyWifi::getInstance();
  WebHandler *webHandler = WebHandler::getInstance();
  Global *glb = Global::getInstance();

  DEBUG_PRINTLN("startup called");
  DEBUG_PRINTF("startup: Core=%d\n", xPortGetCoreID());
  settings->init();
  renderer->language = settings->mySettings.language;
  renderer->itIs = settings->mySettings.itIs;
  
  //autoModeChngeTimer = settings.mySettings.auto_mode_change * 60;
  renderer->clearScreenBuffer(matrix);
  DEBUG_PRINTLN("nach clearScreenBuffer");
  renderer->setSmallText("ST", TEXT_POS_TOP, matrix);
  renderer->setSmallText("AR", TEXT_POS_BOTTOM, matrix);
  DEBUG_PRINTLN("nach setSmallText");
  ledDriver->setBrightness(settings->mySettings.brightness);
  ledDriver->farbenmeer(matrix, colorArray[WHITE]);

  icor->renderAndDisplay("/ico/setup.ico",0,1);
  myWifi->init();
  DEBUG_PRINTLN(F("vor setup WiFi"));
  if(!myWifi->myBegin(settings->mySettings.ssid, settings->mySettings.passwd))
  {
    DEBUG_PRINTLN(F("WiFi failed"));
  } else {
    String myIP = myWifi->IP().toString();
    Serial.println(myIP);
    webHandler->webRequests();
    ElegantOTA.begin(myWifi->getServer());    // Start ElegantOTA
    // ElegantOTA callbacks
    ElegantOTA.onStart(onOTAStart);
    ElegantOTA.onProgress(onOTAProgress);
    ElegantOTA.onEnd(onOTAEnd);
  }

  for(uint16_t i=0;i<10;i++)
    matrix[i]=0;
  
  ledDriver->saveMatrix(matrix);

  // Define a random time
  randomSeed(analogRead(ANALOG_PIN));
  glb->randomHour = random(9, 16);
  for (uint8_t i = 0; i <= 20; i++)
  {
    glb->randomMinute = random(23, 37); // eine Zufallsminute die nicht auf eine volle 5 Minute fällt
    if (glb->randomMinute % 5 != 0)
      break;
  }
  glb->randomSecond = random(5, 56);
  
  xMutex = xSemaphoreCreateBinary();
  xSemaphoreGive(xMutex);  // Anfangszustand: freigegeben

  xEvent = xEventGroupCreate();
  if (xEvent == NULL) {
    printf("Fehler: xEvent konnte nicht erstellt werden.\n");
    return;
  }


  taskParams.taskInfo[0].stackSize=2048;
  xTaskCreatePinnedToCore(
    &dateQueueHandler,   // Function name of the task
    "dateQueueHandler",  // Name of the task (e.g. for debugging)
    taskParams.taskInfo[0].stackSize,       // Stack size (bytes)
    &taskParams,       // Parameter to pass
    1,          // Task priority
    &taskParams.taskInfo[0].taskHandle,// Task handle
    0
  );

  taskParams.taskInfo[1].stackSize=2048;
  xTaskCreatePinnedToCore(
    &queueScheduler,   // Function name of the task
    "queueScheduler",  // Name of the task (e.g. for debugging)
    taskParams.taskInfo[1].stackSize,       // Stack size (bytes)
    &taskParams,       // Parameter to pass
    1,          // Task priority
    &taskParams.taskInfo[1].taskHandle,// Task handle
    0
  );

  taskParams.taskInfo[2].stackSize=2048;
  xTaskCreatePinnedToCore(
    &moonQueueHandler,   // Function name of the task
    "moonQueueHandler",  // Name of the task (e.g. for debugging)
    taskParams.taskInfo[2].stackSize,       // Stack size (bytes)
    &taskParams,       // Parameter to pass
    1,          // Task priority
    &taskParams.taskInfo[2].taskHandle,// Task handle
    0
  );

  taskParams.taskInfo[3].stackSize=2048;
  xTaskCreatePinnedToCore(
    &displayTime,   // Function name of the task
    "DisplayTime",  // Name of the task (e.g. for debugging)
    taskParams.taskInfo[3].stackSize,           // Stack size (bytes)
    &taskParams,    // Parameter to pass
    1,              // Task priority
    &taskParams.taskInfo[3].taskHandle,// Task handle
    0
  );

  taskParams.taskInfo[4].stackSize=2048;
  xTaskCreatePinnedToCore(
    &displaySecondBell,   // Function name of the task
    "DisplaySecondBell",  // Name of the task (e.g. for debugging)
    taskParams.taskInfo[4].stackSize,       // Stack size (bytes)
    &taskParams,       // Parameter to pass
    1,          // Task priority
    &taskParams.taskInfo[4].taskHandle,// Task handle
    0
  );

  taskParams.taskInfo[5].stackSize=2560;
  xTaskCreatePinnedToCore(
    &tempQueueHandler,   // Function name of the task
    "tempQueueHandler",  // Name of the task (e.g. for debugging)
    taskParams.taskInfo[5].stackSize,       // Stack size (bytes)
    &taskParams,       // Parameter to pass
    1,          // Task priority
    &taskParams.taskInfo[5].taskHandle,// Task handle
    0
  );

  taskParams.taskInfo[6].stackSize=2816;
  xTaskCreatePinnedToCore(
    &weatherQueueHandler,   // Function name of the task
    "weatherQueueHandler",  // Name of the task (e.g. for debugging)
    taskParams.taskInfo[6].stackSize,       // Stack size (bytes)
    &taskParams,       // Parameter to pass
    1,          // Task priority
    &taskParams.taskInfo[6].taskHandle,// Task handle
    0
  );

  taskParams.taskInfo[7].stackSize=1800;
  xTaskCreatePinnedToCore(
    &showTextHandler,   // Function name of the task
    "showTextHandler",  // Name of the task (e.g. for debugging)
    taskParams.taskInfo[7].stackSize,       // Stack size (bytes)
    &taskParams,       // Parameter to pass
    1,          // Task priority
    &taskParams.taskInfo[7].taskHandle,// Task handle
    0
  );


  DEBUG_PRINTLN("Task beendet sich selbst...");
  vTaskDelay(pdMS_TO_TICKS(1000));
  vTaskDelete(NULL); // Löscht die eigene Task
}


void setup() {
  delay(1000);
  Serial.begin(SERIAL_SPEED);
  int i=0;
  while(!Serial) {
    delay(100);
    if(i++ == 20) break;
  }
  Serial.println("\n\nStarting....");
  Serial.flush();
  //--------------------------------------------------
  initFS();
  //--------------------------------------------------
  
  xTaskCreatePinnedToCore(
      &startup,   // Function name of the task
      "Startup",  // Name of the task (e.g. for debugging)
      4096,       // Stack size (bytes)
      &taskParams,       // Parameter to pass
      1,          // Task priority
      NULL,       // Task handle
      1
  );
}

void loop() {
}

