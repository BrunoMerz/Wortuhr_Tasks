#include <Arduino.h>
#include "Configuration.h"
#include "Settings.h"
#include "Renderer.h"
#include "IconRenderer.h"
#include "LedDriver_FastLED.h"
#include "MyWifi.h"
#include "MyTime.h"
#include "Modes.h"
#include "WebHandler.h"
#include "OpenWeather.h"
#include "Animation.h"
#include "Global.h"
#include "MzOTA.h"
#include "TaskStructs.h"
#include "DisplayModes.h"
#include "Events.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#if defined(WITH_ALEXA)
  #include "MyAlexa.h"
#endif

#ifdef WITH_SECOND_BELL
#include "SecondBell.h"
#endif

#ifdef WITH_SECOND_HAND
#include "SecondHand.h"
#endif



//#define myDEBUG
#include "MyDebug.h"

s_taskParams taskParams = {
    .feedPosition = 0,
    .feedBuzzer = 0,
    .feedColor = 0x000000,
    .updateScreen = false,
    .endless_loop = false,
    .feedText = "",
    .animation = "",
    .taskInfo = {
        [TASK_STARTUP]     = {NULL, 4096, true, STATE_INIT, "Startup"},
        [TASK_SCHEDULER]   = {NULL, 4096, true, STATE_INIT, "queueScheduler"},
        [TASK_TIME]        = {NULL, 3072, true, STATE_INIT, "DisplayTime"},
        [TASK_MODES]       = {NULL, 6144, true, STATE_INIT, "ModesQueueHandler"},
        [TASK_TEXT]        = {NULL, 2048, true, STATE_INIT, "showTextHandler"},
        [TASK_ANIMATION]   = {NULL, 5120, true, STATE_INIT, "animationQueueHandler"},
        [TASK_EVENT]       = {NULL, 5120, true, STATE_INIT, "eventQueueHandler"},
#if defined(WITH_SECOND_BELL)
        [TASK_SECOND_BELL] = {NULL, 2048, true, STATE_INIT, "DisplaySecondBell"},
#endif
#if defined(WITH_SECOND_HAND)
        [TASK_SECOND_HAND] = {NULL, 2048, true, STATE_INIT, "displaySecondHand"},
#endif
#if defined(LILYGO_T_HMI)
        [TASK_T_HMI]       = {NULL, 4096, true, STATE_INIT, "hmiHandler"},
        [TASK_DRAW]        = {NULL, 4096, true, STATE_INIT, "drawTask"},
#endif
    }
};

unsigned long ota_progress_millis = 0;

extern void initFS();


EventGroupHandle_t xEvent;
SemaphoreHandle_t xMutex;
uint16_t nightOffTime;
uint16_t dayOnTime;


uint16_t matrix[10] = {};

static Global *glb = Global::getInstance();
static OpenWeather *ow = OpenWeather::getInstance();
static DisplayModes *dm = DisplayModes::getInstance();
static AnimationFS *anifs = AnimationFS::getInstance();
static MyWifi *myWifi = MyWifi::getInstance();
static MyTime *mt = MyTime::getInstance();



static void displayWeekday(void) {
    dm->displayWeekday();
}
static void displayDate(void) {
    dm->displayDate();
}
static void displayMoonphase(void) {
    dm->displayMoonphase();
}
static void displayWeather(void) {
    dm->displayWeather();
}
static void displayExtTemp(void) {
    dm->displayExtTemp();
}
void displayExtHumidity(void) {
    dm->displayExtHumidity();
}
static void displaySeconds(void) {
    dm->displaySeconds();
}

static void wifiReset(void) {
  DEBUG_PRINTLN(F("WifiReset........."));
  delay(100);
  myWifi->doReset();
}

DPMODE dpm[] = {
  { 1, MODE_WEEKDAY,     "MODE_WEEKDAY",   3000, 0, displayWeekday    },
  { 1, MODE_DATE,        "MODE_DATE",      3000, 0, displayDate       },
  { 1, MODE_MOONPHASE,   "MODE_MOONPHASE", 3000, 0, displayMoonphase  },
  { 1, MODE_WETTER,      "MODE_WETTER",      10, 0, displayWeather    },
  { 1, MODE_EXT_TEMP,    "MODE_EXT_TEMP",  3000, 0, displayExtTemp    },
  { 1, MODE_EXT_HUMIDITY,"MODE_EXT_TEMP",  3000, 0, displayExtHumidity},
  { 0, MODE_SECONDS,     "MODE_SECONDS",   3000, 0, displaySeconds    },
};

uint32_t dpmSize = sizeof(dpm)/sizeof(dpMode);

void onOTAStart() {
  // Log when OTA has started
  DEBUG_PRINTLN("OTA update started!");
  // <Add your own code here>
}

void onOTAProgress(size_t current, size_t final) {
  // Log every 1 second
  if (millis() - ota_progress_millis > 1000) {
    ota_progress_millis = millis();
    DEBUG_PRINTF("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
  }
}

void onOTAEnd(bool success) {
  // Log when OTA has finished
  if (success) {
    DEBUG_PRINTLN("OTA update finished successfully!");
  } else {
    DEBUG_PRINTLN("There was an error during OTA update!");
  }
  // <Add your own code here>
}


void queueScheduler(void *p) {
  s_taskParams *tp = (s_taskParams*)p;
  Settings *settings    = Settings::getInstance();
  OpenWeather *ow       = OpenWeather::getInstance();
  LedDriver *ledDriver  = LedDriver::getInstance();
  Events *evt            = Events::getInstance();

#if defined(WITH_ALEXA)
  MyAlexa *alexa = MyAlexa::getInstance();
  alexa->init(settings->mySettings.systemname, &ledDriver->mode, myWifi->getServer());
#endif

  int lastHour=0;
  int lastMinute=0;
  int lastSecond=0;
  int aktDay;
  int aktHour;
  int aktMinute;
  int aktSecond;
  
  nightOffTime = mt->hour(settings->mySettings.nightOffTime) * 60 + mt->minute(settings->mySettings.nightOffTime);
  dayOnTime = mt->hour(settings->mySettings.dayOnTime) * 60 + mt->minute(settings->mySettings.dayOnTime);
  uint32_t aktts;
  bool isNightOff;
  glb->autoModeChangeTimer = settings->mySettings.auto_mode_change * 60;
  uint8_t autoMode = 0;
  uint32_t colorsaver;
 

  DEBUG_PRINTF("queueScheduler: Core=%d\n", xPortGetCoreID());
  ow->getOutdoorConditions(String(settings->mySettings.openweatherlocation), String(settings->mySettings.openweatherapikey));

  while(true) {
    while(!tp->taskInfo[TASK_SCHEDULER].handleEvent)
      vTaskDelay(pdMS_TO_TICKS(500));

    mt->getTime();
    aktDay = mt->mytm.tm_mday;
    aktHour = mt->mytm.tm_hour;
    aktMinute = mt->mytm.tm_min;
    aktSecond = mt->mytm.tm_sec;
    
    isNightOff=false;

    String animation = "Z";
    if (aktHour <= 9)
      animation += "0";
    animation += String(aktHour);
    if (aktMinute <= 9)
      animation += "0";
    animation += String(aktMinute);

    if(aktHour != lastHour && aktMinute == 0 && animation!=anifs->myanimation.lastAnimation) {
      lastHour=aktHour;
      anifs->myanimation.lastAnimation = animation;
      DEBUG_PRINTF("animation=%s\n", animation.c_str());
      
      if (not(mt->month() == 1 && mt->day() == 1 && mt->hour() == 0 && mt->minute() <= 9)) // An Neujahr die ersten 9 Minuten keine Animation zulassen!
      {
        if (anifs->loadAnimation(animation))
        {
          anifs->akt_aniframe = 0;
          anifs->akt_aniloop = 0;
          anifs->frame_fak = 0;
        
          while(anifs->showAnimation(settings->mySettings.brightness)) {
              vTaskDelay(pdMS_TO_TICKS(10));
          }
          vTaskDelay(pdMS_TO_TICKS(500));
          tp->updateScreen = true;
        }     
      }
    }

    if (aktSecond != lastSecond)
    {
      lastSecond = aktSecond;
#if defined(LDR)
      // Set brightness from LDR
      if (settings->mySettings.useAbc)
        ledDriver->setBrightnessFromLdr();
#endif
    }

    if(aktMinute != lastMinute) {
      lastMinute = aktMinute;
      isNightOff=false;
      aktts = aktHour * 60 + aktMinute;
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

      DEBUG_PRINTF("isNightOff=%d, dayOnTime=%d, nightOffTime=%d, aktts=%d, mode=%d\n", isNightOff, dayOnTime, nightOffTime, aktts, ledDriver->mode);

      if(isNightOff && ledDriver->mode != MODE_BLANK) 
        ledDriver->setOnOff();

      if(!isNightOff && ledDriver->mode == MODE_BLANK) {
        evt->mustLoad = true;
        ledDriver->setOnOff();
      }

      if (xEventGroupSetBits(xEvent, MODE_TIME)) {
        glb->stackSize = tp->taskInfo[TASK_SCHEDULER].stackSize;
        glb->setHighWaterMark(TASK_SCHEDULER);
        glb->codeline = __LINE__;
        glb->codetab = __NAME__;
        DEBUG_PRINTF("Gesendet: MODE_TIME, aktMinute=%d, aktSecond=%d, uxHighWaterMark=%d\n", aktMinute, aktSecond, glb->highWaterMark);
      }
    }

    glb->autoModeChangeTimer--;
    if (!glb->autoModeChangeTimer)
    {
      if (aktSecond > 45)
      {
        glb->autoModeChangeTimer = glb->autoModeChangeTimer + 30;
      }
      else if (aktSecond < 15)
      {
        glb->autoModeChangeTimer = glb->autoModeChangeTimer + 15;
      }
      else
      {
        //DEBUG_PRINTF("autoMode=%d, aktSecond=%d\n", autoMode, aktSecond);

        glb->autoModeChangeTimer = settings->mySettings.auto_mode_change * 60;
        
        if (dpm[autoMode].event_automode && xEventGroupSetBits(xEvent, dpm[autoMode].event_type)) {
          glb->stackSize = tp->taskInfo[TASK_SCHEDULER].stackSize;
          glb->setHighWaterMark(TASK_SCHEDULER);
          glb->codeline = __LINE__;
          glb->codetab = __NAME__;
          DEBUG_PRINTF("Gesendet: %s, aktMinute=%d, aktSecond=%d, uxHighWaterMark=%d\n", dpm[autoMode].event_name.c_str(), aktMinute, aktSecond, glb->highWaterMark);
        }
        if(++autoMode >= dpmSize)
          autoMode = 0;
      }
    }

    MzOTA.loop();

#if defined(WITH_ALEXA)
    alexa->handle();
#endif
    vTaskDelay(pdMS_TO_TICKS(500)); // 500 ms warten
  }
}


void ModesQueueHandler(void *p) {
  s_taskParams *tp = (s_taskParams*)p;
  LedDriver *ledDriver=LedDriver::getInstance();
  Renderer *renderer=Renderer::getInstance();
  Settings *settings=Settings::getInstance();
  OpenWeather *outdoorWeather = OpenWeather::getInstance();
  DEBUG_PRINTF("temp: Core=%d\n", xPortGetCoreID());
  while(true) {
    for(uint8_t mode=0; mode < dpmSize; mode++) {
      uint32_t b = xEventGroupWaitBits(xEvent, dpm[mode].event_type, pdTRUE, pdFALSE, pdMS_TO_TICKS(10));
      //DEBUG_PRINTF("wait for=%x, b=%d\n",dpm[mode].event_type, b);
      if (tp->taskInfo[TASK_MODES].handleEvent && (b & dpm[mode].event_type)) {
        if (xSemaphoreTake(xMutex, portMAX_DELAY)) {
          if(ledDriver->mode == MODE_BLANK) { xSemaphoreGive(xMutex); vTaskDelay(pdMS_TO_TICKS(10)); continue; }

          glb->stackSize = tp->taskInfo[TASK_MODES].stackSize;
          glb->setHighWaterMark(TASK_MODES);
          glb->codeline = __LINE__;
          glb->codetab = __NAME__;
          DEBUG_PRINTF("Erhalten: %s, aktMinute=%d, aktSecond=%d, stackSize=%d, uxHighWaterMark=%d\n", dpm[mode].event_name.c_str(), mt->mytm.tm_min, mt->mytm.tm_sec, glb->stackSize, glb->highWaterMark);

          dpm[mode].event_fct();

          vTaskDelay(pdMS_TO_TICKS(dpm[mode].event_delay));

          xEventGroupClearBits(xEvent, dpm[mode].event_type);
          dpm[mode].event_state = STATE_PROCESSED;
          ledDriver->lastMode = dpm[mode].event_type;
          xSemaphoreGive(xMutex);
          xEventGroupSetBits(xEvent, MODE_TIME);
        }
      }
      vTaskDelay(pdMS_TO_TICKS(10));
    }
  } // while
}


void showTextHandler(void *p) {
  s_taskParams *tp = (s_taskParams*)p;
  LedDriver *ledDriver=LedDriver::getInstance();
  Renderer *renderer=Renderer::getInstance();
  Settings *settings=Settings::getInstance();
                                                                                                          ;
  DEBUG_PRINTF("showText: Core=%d\n", xPortGetCoreID());
  while(true) {
    if (tp->taskInfo[TASK_TEXT].handleEvent && xEventGroupWaitBits(xEvent, MODE_FEED, pdFALSE, pdFALSE, portMAX_DELAY)) {
      if (xSemaphoreTake(xMutex, portMAX_DELAY)) {
        if(ledDriver->mode == MODE_BLANK) { xSemaphoreGive(xMutex); vTaskDelay(pdMS_TO_TICKS(10)); continue; }

        glb->stackSize = tp->taskInfo[TASK_TEXT].stackSize;
        glb->setHighWaterMark(TASK_TEXT);
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
              matrix[2 + z] |= (b << 11 + y) & 0b1111111111100000;
              b = pgm_read_byte(&lettersBig[tp->feedText[i + 1] - 32][z]);
              matrix[2 + z] |= (b << 5 + y) & 0b1111111111100000;
              b = pgm_read_byte(&lettersBig[tp->feedText[i + 2] - 32][z]);
              matrix[2 + z] |= (b << y - 1) & 0b1111111111100000;
            }
            ledDriver->writeScreenBuffer(matrix, tp->feedColor);
            ledDriver->show();
            vTaskDelay(pdMS_TO_TICKS(FEED_SPEED));
          }
        }
        
        xEventGroupClearBits(xEvent, MODE_FEED);
        ledDriver->lastMode = MODE_FEED;
        xSemaphoreGive(xMutex);
        xEventGroupSetBits(xEvent, MODE_TIME); // Uhrzeit wieder anzeigen
        tp->taskInfo[TASK_TEXT].state = STATE_PROCESSED;
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  } // while
}


void animationQueueHandler(void *p) {
  s_taskParams *tp = (s_taskParams*)p;
  LedDriver *ledDriver=LedDriver::getInstance();
  Renderer *renderer=Renderer::getInstance();
  Settings *settings=Settings::getInstance();

  DEBUG_PRINTF("animation: Core=%d\n", xPortGetCoreID());
  while(true) {
    if (tp->taskInfo[TASK_ANIMATION].handleEvent && xEventGroupWaitBits(xEvent, MODE_SHOWANIMATION, pdFALSE, pdFALSE, portMAX_DELAY)) {
      if (xSemaphoreTake(xMutex, portMAX_DELAY)) {
        if(ledDriver->mode == MODE_BLANK) { xSemaphoreGive(xMutex); vTaskDelay(pdMS_TO_TICKS(10)); continue; }

        glb->stackSize = tp->taskInfo[TASK_ANIMATION].stackSize;
        glb->setHighWaterMark(TASK_ANIMATION);
        glb->codeline = __LINE__;
        glb->codetab = __NAME__;
        DEBUG_PRINTF("Erhalten: MODE_SHOWANIMATION, aktMinute=%d, aktSecond=%d, stackSize=%d, uxHighWaterMark=%d\n", mt->mytm.tm_min, mt->mytm.tm_sec, glb->stackSize, glb->highWaterMark);

        renderer->clearScreenBuffer(matrix);
 
        do {
          anifs->akt_aniframe = 0;
          anifs->akt_aniloop = 0;
          anifs->frame_fak = 0;
          while(anifs->showAnimation(settings->mySettings.brightness)) {
            vTaskDelay(pdMS_TO_TICKS(30));
          }
          DEBUG_PRINTF("nach showAnimation: frame=%d, loop=%d, fak=%d\n",anifs->akt_aniframe, anifs->akt_aniloop, anifs->frame_fak);
        } while(tp->endless_loop);
        

        xEventGroupClearBits(xEvent, MODE_SHOWANIMATION);
        ledDriver->lastMode = MODE_SHOWANIMATION;
        xSemaphoreGive(xMutex);
        xEventGroupSetBits(xEvent, MODE_TIME);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  } // while
}


void eventQueueHandler(void *p) {
  s_taskParams *tp = (s_taskParams*)p;
  Settings *settings    = Settings::getInstance();
  LedDriver *ledDriver  = LedDriver::getInstance();
  Events *evt           = Events::getInstance();

  int lastHour=0;
  int lastSecond=0;
  int aktMonth;
  int aktDay;
  int aktHour;
  int aktMinute;
  int aktSecond;
  time_t aktTime;
  time_t lastCall[MAXEVENTS+1];

  DEBUG_PRINTF("event: Core=%d\n", xPortGetCoreID());


  while(true) {
    while(!tp->taskInfo[TASK_EVENT].handleEvent || ledDriver->mode == MODE_BLANK)
      vTaskDelay(pdMS_TO_TICKS(500));

    mt->getTime();
    aktMonth = mt->mytm.tm_mon;
    aktDay = mt->mytm.tm_mday;
    aktHour = mt->mytm.tm_hour;
    aktMinute = mt->mytm.tm_min;
    aktSecond = mt->mytm.tm_sec;
    aktTime = mt->mytm.tm_loc;

    if(evt->mustLoad) {
      DEBUG_PRINTLN("mustLoad");
      evt->loadEvents();

      aktTime += (5-aktSecond);
      for(uint8_t i=0; i<=MAXEVENTS; i++) {
        lastCall[i] = aktTime;
      }
    }

    for (uint8_t i = 0; i <= MAXEVENTS; i++) {
      if (evt->events[i].aktiv && (evt->events[i].start == ST_DATE &&  aktDay == evt->events[i].day && aktMonth == evt->events[i].month) || evt->events[i].start == ST_ALWAYS) {
        if(aktTime - lastCall[i] >= evt->events[i].intervall*60) {
          // Damit keine Anzeige während der Eventbehandlung auftritt autoModeChangeTimer hochsetzen
          glb->autoModeChangeTimer = settings->mySettings.auto_mode_change * 60;
          DEBUG_PRINTF("event: found=%d, Sekunde=%d\n", i, aktSecond);
          evt->runEvent(i);
          DEBUG_PRINTF("event: %d processed\n", i);
          lastCall[i] = aktTime;
        }
      }
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  } // while
}

#ifdef WITH_SECOND_BELL
void displaySecondBell(void *) {
  SecondBell *secondBell=SecondBell::getInstance();
  LedDriver *ledDriver=LedDriver::getInstance();

  int lastSecond=0;
  int aktSecond;
  DEBUG_PRINTF("secondBell: Core=%d\n", xPortGetCoreID());
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
#endif

#ifdef WITH_SECOND_HAND
void displaySecondHand(void *) {
  SecondHand *secondHand=SecondHand::getInstance();
  LedDriver *ledDriver=LedDriver::getInstance();

  int lastSecond=0;
  int aktSecond;
  DEBUG_PRINTF("secondHand: Core=%d\n", xPortGetCoreID());
  while(true) {
    mt->getTime();
    aktSecond = mt->mytm.tm_sec;
    if(aktSecond!=lastSecond) {
      lastSecond=aktSecond;
      secondHand->drawSecond();
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
#endif


#if defined(LILYGO_T_HMI)

#include "MyTFT.h"
#include "MyButtons.h"
#include "MyWidgets.h"
#include "MyTouch.h"
#include "OneButton.h"

static void handleTFTScreen(BTNType btnType);

static void btn_action(BTNType btnType)
{
  DEBUG_PRINTF("btn_action called, btnNr=%d\n", btnType);
  handleTFTScreen(btnType);
}


static void handleTFTScreen(BTNType btnType)
{
  MyButtons *btns = MyButtons::getInstance();
  MyWidgets *widgets = MyWidgets::getInstance();

  DEBUG_PRINTF("handleTFTScreen: %d\n", btnType);
  BTNType oldBtnType = btns->aktBtnType;
  mt->getTime();

  if (btnType != BTN_UPDATE)
  {
    btns->aktBtnType = btnType;
  }
  switch (btns->aktBtnType)
  {
  case BTN_WEATHER:
    widgets->drawWidget(WIDGET_WEATHER);
    break;
  case BTN_TIME:
    widgets->drawWidget(WIDGET_TIME);
    break;
  case BTN_INFO:
    widgets->drawInfo();
    break;
  case BTN_OFF:
    analogWrite(TFT_BL,40);
    btns->deleteButton(BTN_OFF);
    btns->newButton("on", BTN_ON, 153, btn_action);
    btns->aktBtnType = oldBtnType;
    break;
  case BTN_ON:
    analogWrite(TFT_BL,255);
    btns->deleteButton(BTN_ON);
    btns->newButton("off", BTN_OFF, 153, btn_action);
    btns->aktBtnType = oldBtnType;
    break;
  }
}

void t_hmiHandler(void *) {
  uint16_t x, y;
  uint8_t aktHour;
  uint8_t aktMinute;
  uint8_t aktSecond;
  uint8_t lastSecond=100;
  WidgetMode aktMode = WIDGET_UNKNOWN;

  DEBUG_PRINTF("t_hmiHandler: Core=%d\n", xPortGetCoreID());

  MyTFT *tft = MyTFT::getInstance();
  MyTouch *touch = MyTouch::getInstance();
  MyButtons *btns = MyButtons::getInstance();
  MyWidgets *widgets = MyWidgets::getInstance();
  MyTime *mt = MyTime::getInstance();

  OneButton buttonWifiReset(WIFI_RESET, false, false);
  
  // Init Touch, Widgets and Buttons
  touch->init();
  buttonWifiReset.attachLongPressStop(wifiReset);
  btns->init();
  widgets->init();

  // Create Buttons
  btns->newButton("weather", BTN_WEATHER, 0, btn_action);
  btns->newButton("clock", BTN_TIME, 51, btn_action);
  btns->newButton("info", BTN_INFO, 102, btn_action);
  btns->newButton("off", BTN_OFF, 153, btn_action);

  uint16_t secWidth = tft->getMainCanvasWidth()/59;

  vTaskDelay(pdMS_TO_TICKS(5000));
  while(true) {

    aktHour = mt->mytm.tm_hour;
    aktMinute = mt->mytm.tm_min;
    aktSecond = mt->mytm.tm_sec;

    if(!aktSecond || aktMode == WIDGET_UNKNOWN) {
      if(taskParams.taskInfo[TASK_DRAW].state == STATE_PROCESSING) {
        taskParams.taskInfo[TASK_DRAW].state = STATE_PROCESSED;
        while(taskParams.taskInfo[TASK_DRAW].state == STATE_PROCESSED)
          vTaskDelay(pdMS_TO_TICKS(10));
      }
      if(++(*reinterpret_cast<uint8_t*>(&aktMode))==WIDGET_LAST)
        aktMode=WIDGET_TIME;
      widgets->drawWidget(aktMode);
      tft->clearStateCanvas(0);
      lastSecond = aktSecond;
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
    if(aktSecond != lastSecond) {
      if(aktMode==WIDGET_TIME)
        widgets->drawClockHands(aktSecond, aktMinute, aktHour);
      int16_t w = secWidth*aktSecond;
      if(aktSecond < 10)
        tft->fillRect(0, tft->getMainCanvasHeight()+2, w, tft->fontHeight(STD_FONT), TFT_YELLOW);
      else {
        tft->fillRect(w, tft->getMainCanvasHeight()+2, secWidth, tft->fontHeight(STD_FONT), TFT_YELLOW);
        tft->fillRect(secWidth*(aktSecond-1)/2, tft->getMainCanvasHeight()+2, 40, tft->fontHeight(STD_FONT), TFT_YELLOW);
      }
      tft->setTextColor(TFT_BLACK);
      tft->setTextDatum(TL_DATUM);
      tft->setTextFont(2);
      tft->drawNumber(aktSecond, w/2, tft->getMainCanvasHeight()+2);
      lastSecond = aktSecond;
    }

    if (touch->pressed(&x, &y))
      btns->callAction(x, y);
    buttonWifiReset.tick();

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
#endif

void displayTime(void *p) {
  s_taskParams *tp = (s_taskParams*)p;
  Settings *settings=Settings::getInstance();
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

  mt->mytm.tm_ntpserver = settings->mySettings.ntphost;
  mt->mytm.tm_timezone = settings->mySettings.timezone; //"CET-1CEST,M3.5.0,M10.5.0/3";
  mt->mytm.tm_lon = settings->mySettings.longitude;
  mt->mytm.tm_lat = settings->mySettings.latitude;
  DEBUG_PRINTLN(F("configure time"));
  mt->confTime();
  DEBUG_PRINTLN(F("get time"));
  DEBUG_PRINTF("time: Core=%d\n", xPortGetCoreID());
  while(true) {
    if (tp->taskInfo[TASK_TIME].handleEvent && xEventGroupWaitBits(xEvent, MODE_TIME, pdFALSE, pdFALSE, portMAX_DELAY)) {
      if (xSemaphoreTake(xMutex, portMAX_DELAY)) {
        if(ledDriver->mode == MODE_BLANK || !tp->taskInfo[TASK_TIME].handleEvent) { 
          xSemaphoreGive(xMutex); 
          vTaskDelay(pdMS_TO_TICKS(10)); 
          continue; 
        }

        glb->stackSize = tp->taskInfo[TASK_TIME].stackSize;
        glb->setHighWaterMark(TASK_TIME);
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

        if (aktDay != lastDay)
        {
          lastDay = aktDay;
          glb->Modecount = 0;

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

        if (aktMinute % 5 != 0 || tp->updateScreen)
        {
          //Serial.printf("displayTime: 1 aktMinute=%d, updateScreen=%d\n",aktMinute, tp->updateScreen);
          ledDriver->writeScreenBufferFade(matrix, settings->mySettings.ledcol);
        }
        else
        {
          glb->akt_transition = settings->mySettings.transition;
          if ( settings->mySettings.transition == TRANSITION_RANDOM )
          {
            glb->akt_transition_old = glb->akt_transition;
            for (uint8_t i = 0; i <= 20; i++)
            {
              glb->akt_transition = random(TRANSITION_NORMAL + 1, TRANSITION_MAX);
              if ( glb->akt_transition != glb->akt_transition_old ) break;
            }
          }
          if ( settings->mySettings.transition == TRANSITION_ALLE_NACHEINANDER )
          {
            glb->akt_transition_old++;
            if ( glb->akt_transition_old >= TRANSITION_MAX ) glb->akt_transition_old = TRANSITION_NORMAL + 1;
            glb->akt_transition = glb->akt_transition_old;
          }

          if (glb->akt_transition == TRANSITION_NORMAL)
            ledDriver->writeScreenBuffer(matrix, settings->mySettings.ledcol);
          else if (glb->akt_transition == TRANSITION_MOVEUP)
            ledDriver->moveScreenBufferUp(matrix, settings->mySettings.ledcol);
          else if (glb->akt_transition == TRANSITION_MOVEDOWN)
            ledDriver->moveScreenBufferDown(matrix, settings->mySettings.ledcol);
          else if (glb->akt_transition == TRANSITION_MOVELEFT)
            ledDriver->moveScreenBufferLeft(matrix, settings->mySettings.ledcol);
          else if (glb->akt_transition == TRANSITION_MOVERIGHT)
            ledDriver->moveScreenBufferRight(matrix, settings->mySettings.ledcol);
          else if (glb->akt_transition == TRANSITION_MOVELEFTDOWN)
            ledDriver->moveScreenBufferLeftDown(matrix, settings->mySettings.ledcol);
          else if (glb->akt_transition == TRANSITION_MOVERIGHTDOWN)
            ledDriver->moveScreenBufferRightDown(matrix, settings->mySettings.ledcol);
          else if (glb->akt_transition == TRANSITION_MOVECENTER)
            ledDriver->moveScreenBufferCenter(matrix, settings->mySettings.ledcol);
          else if (glb->akt_transition == TRANSITION_FADE)
            ledDriver->writeScreenBufferFade(matrix, settings->mySettings.ledcol);
          else if (glb->akt_transition == TRANSITION_FARBENMEER)
            ledDriver->farbenmeer(matrix, settings->mySettings.ledcol);
          else if (glb->akt_transition == TRANSITION_MATRIX)
            ledDriver->matrix_regen(matrix, settings->mySettings.ledcol);
          else if (glb->akt_transition == TRANSITION_SPIRALE_LINKS)
            ledDriver->moveSeriell(matrix, settings->mySettings.ledcol, SPIRALE_LINKS);
          else if (glb->akt_transition == TRANSITION_ZEILENWEISE)
            ledDriver->moveSeriell(matrix, settings->mySettings.ledcol, ZEILENWEISE);
          else if (glb->akt_transition == TRANSITION_SPIRALE_RECHTS)
            ledDriver->moveSeriell(matrix, settings->mySettings.ledcol, SPIRALE_RECHTS);
          else if (glb->akt_transition == TRANSITION_MITTE_LINKSHERUM)
            ledDriver->moveSeriell(matrix, settings->mySettings.ledcol, MITTE_LINKSHERUM);
          else if (glb->akt_transition == TRANSITION_REGENBOGEN)
            ledDriver->regenbogen(matrix, settings->mySettings.ledcol);
          else if (glb->akt_transition == TRANSITION_QUADRATE)
            ledDriver->quadrate(matrix, settings->mySettings.ledcol);
          else if (glb->akt_transition == TRANSITION_KREISE)
            ledDriver->kreise(matrix, settings->mySettings.ledcol);
          else
          {
            ledDriver->writeScreenBuffer(matrix, settings->mySettings.ledcol);
          }
          vTaskDelay(pdMS_TO_TICKS(500));
        } // else aktMinute % 5 != 0

        tp->updateScreen = false;
        xEventGroupClearBits(xEvent, MODE_TIME);
        ledDriver->lastMode = MODE_TIME;
        xSemaphoreGive(xMutex);
      }
      vTaskDelay(pdMS_TO_TICKS(500));
    } // if (xEventGroupWaitBits
    vTaskDelay(pdMS_TO_TICKS(10));
  } // while(true)
}



void startup(void *) {
  Settings *settings=Settings::getInstance();
  Renderer *renderer=Renderer::getInstance();
  IconRenderer *icor=IconRenderer::getInstance();
  LedDriver *ledDriver=LedDriver::getInstance();
  MyWifi *myWifi=MyWifi::getInstance();
  WebHandler *webHandler = WebHandler::getInstance();
  Global *glb = Global::getInstance();

#if defined(LILYGO_T_HMI)
  #include "MyTFT.h"
  MyTFT *tft = MyTFT::getInstance();
  tft->init();
#endif

  DEBUG_PRINTLN("startup called");
  DEBUG_PRINTF("startup: Core=%d\n", xPortGetCoreID());
  settings->init();

  ledDriver->setBrightness(settings->mySettings.brightness);

#ifdef LDR
  // Set brightness from LDR
  if (settings->mySettings.useAbc)
    ledDriver->setBrightnessFromLdr();
#endif



  renderer->language = settings->mySettings.language;
  renderer->itIs = settings->mySettings.itIs;
  
  renderer->clearScreenBuffer(matrix);
  DEBUG_PRINTLN("nach clearScreenBuffer");
  renderer->setSmallText("ST", TEXT_POS_TOP, matrix);
  renderer->setSmallText("AR", TEXT_POS_BOTTOM, matrix);
  DEBUG_PRINTLN("nach setSmallText");
  
  ledDriver->farbenmeer(matrix, colorArray[WHITE]);

  icor->renderAndDisplay("/ico/setup.ico",0,1);
  myWifi->init();
  DEBUG_PRINTLN(F("vor setup WiFi"));
#if defined(LILYGO_T_HMI)
  tft->printStateLine("Start WIFI...", 0, true);
#endif
  if(!myWifi->myBegin(settings->mySettings.ssid, settings->mySettings.passwd))
  {
    DEBUG_PRINTLN(F("WiFi failed"));
#if defined(LILYGO_T_HMI)
    tft->printStateLine("WIFI nicht verbunden", 0, true);
#endif
  } else {
    Serial.println(myWifi->IP().toString());
#if defined(LILYGO_T_HMI)
    tft->printStateLine(settings->mySettings.systemname, 0, true);
    tft->printStateLine(myWifi->IP().toString(), 160, false);
#endif
    webHandler->webRequests();

    // MzOTA
    MzOTA.begin(myWifi->getServer());

    // MzOTA callbacks
    MzOTA.onStart(onOTAStart);
    MzOTA.onProgress(onOTAProgress);
    MzOTA.onEnd(onOTAEnd);
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


  xTaskCreatePinnedToCore(
    &queueScheduler,   // Function name of the task
    taskParams.taskInfo[TASK_SCHEDULER].name,  // Name of the task (e.g. for debugging)
    taskParams.taskInfo[TASK_SCHEDULER].stackSize,       // Stack size (bytes)
    &taskParams,       // Parameter to pass
    1,          // Task priority
    &taskParams.taskInfo[TASK_SCHEDULER].taskHandle,// Task handle
    0
  );

  xTaskCreatePinnedToCore(
    &displayTime,   // Function name of the task
    taskParams.taskInfo[TASK_TIME].name,  // Name of the task (e.g. for debugging)
    taskParams.taskInfo[TASK_TIME].stackSize,           // Stack size (bytes)
    &taskParams,    // Parameter to pass
    1,              // Task priority
    &taskParams.taskInfo[TASK_TIME].taskHandle,// Task handle
    0
  );

#ifdef WITH_SECOND_BELL
  xTaskCreatePinnedToCore(
    &displaySecondBell,   // Function name of the task
    taskParams.taskInfo[TASK_SECOND_BELL].name,  // Name of the task (e.g. for debugging)
    taskParams.taskInfo[TASK_SECOND_BELL].stackSize,       // Stack size (bytes)
    &taskParams,       // Parameter to pass
    1,          // Task priority
    &taskParams.taskInfo[TASK_SECOND_BELL].taskHandle,// Task handle
    0
  );
#endif

#ifdef WITH_SECOND_HAND
  xTaskCreatePinnedToCore(
    &displaySecondHand,   // Function name of the task
    taskParams.taskInfo[TASK_SECOND_HAND].name,  // Name of the task (e.g. for debugging)
    taskParams.taskInfo[TASK_SECOND_HAND].stackSize,       // Stack size (bytes)
    &taskParams,       // Parameter to pass
    1,          // Task priority
    &taskParams.taskInfo[TASK_SECOND_HAND].taskHandle,// Task handle
    0
  );
#endif

  xTaskCreatePinnedToCore(
    &ModesQueueHandler,   // Function name of the task
    taskParams.taskInfo[TASK_MODES].name,  // Name of the task (e.g. for debugging)
    taskParams.taskInfo[TASK_MODES].stackSize,       // Stack size (bytes)
    &taskParams,       // Parameter to pass
    1,          // Task priority
    &taskParams.taskInfo[TASK_MODES].taskHandle,// Task handle
    0
  );

  xTaskCreatePinnedToCore(
    &showTextHandler,   // Function name of the task
    taskParams.taskInfo[TASK_TEXT].name,  // Name of the task (e.g. for debugging)
    taskParams.taskInfo[TASK_TEXT].stackSize,       // Stack size (bytes)
    &taskParams,       // Parameter to pass
    1,          // Task priority
    &taskParams.taskInfo[TASK_TEXT].taskHandle,// Task handle
    0
  );

  xTaskCreatePinnedToCore(
    &animationQueueHandler,   // Function name of the task
    taskParams.taskInfo[TASK_ANIMATION].name,  // Name of the task (e.g. for debugging)
    taskParams.taskInfo[TASK_ANIMATION].stackSize,       // Stack size (bytes)
    &taskParams,       // Parameter to pass
    1,          // Task priority
    &taskParams.taskInfo[TASK_ANIMATION].taskHandle,// Task handle
    0
  );

  xTaskCreatePinnedToCore(
    &eventQueueHandler,   // Function name of the task
    taskParams.taskInfo[TASK_EVENT].name,  // Name of the task (e.g. for debugging)
    taskParams.taskInfo[TASK_EVENT].stackSize,       // Stack size (bytes)
    &taskParams,       // Parameter to pass
    1,          // Task priority
    &taskParams.taskInfo[TASK_EVENT].taskHandle,// Task handle
    0
  );


#if defined(LILYGO_T_HMI)
  xTaskCreatePinnedToCore(
    &t_hmiHandler,   // Function name of the task
    taskParams.taskInfo[TASK_T_HMI].name,  // Name of the task (e.g. for debugging)
    taskParams.taskInfo[TASK_T_HMI].stackSize,       // Stack size (bytes)
    &taskParams,       // Parameter to pass
    1,          // Task priority
    &taskParams.taskInfo[TASK_T_HMI].taskHandle,// Task handle
    0
  );
#endif

  taskParams.feedText = "  " + String(settings->mySettings.systemname) + ":" + myWifi->IP().toString();
  taskParams.feedPosition = 0;
  taskParams.feedColor = colorArray[WHITE];
  taskParams.taskInfo[TASK_TEXT].state = STATE_INIT;
  xEventGroupSetBits(xEvent, MODE_FEED);
  while(!(taskParams.taskInfo[TASK_TEXT].state==STATE_PROCESSED))
    vTaskDelay(pdMS_TO_TICKS(100));

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

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
/*
  if(esp_reset_reason() != ESP_RST_PANIC) {
    Serial.println("erzeuge Panic nach 5 Sekunden");
    Serial.flush();
    delay(5000);
    *((int *)0) = 42;
  }
*/
  //--------------------------------------------------
  initFS();
  //--------------------------------------------------
  
  xTaskCreatePinnedToCore(
      &startup,   // Function name of the task
      taskParams.taskInfo[TASK_STARTUP].name,  // Name of the task (e.g. for debugging)
      taskParams.taskInfo[TASK_STARTUP].stackSize,       // Stack size (bytes)
      &taskParams,       // Parameter to pass
      1,          // Task priority
      &taskParams.taskInfo[TASK_STARTUP].taskHandle,       // Task handle
      1
  );
}

void loop() {
}
