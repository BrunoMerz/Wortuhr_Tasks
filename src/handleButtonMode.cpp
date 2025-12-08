#include "Arduino.h"
#include "Configuration.h"
#include "Languages.h"
#include "Settings.h"
#include "Html_content.h"
#include "Helper.h"
#include "TaskStructs.h"
#include "Events.h"
#include "Renderer.h"
#include "LedDriver_FastLED.h"
#include "MyTime.h"
#include "OpenWeather.h"
#include "DisplayModes.h"

//#define myDEBUG
#include "MyDebug.h"

#include "ESPAsyncWebServer.h"
#include <LittleFS.h>

static Renderer *renderer = Renderer::getInstance();
static Settings *settings = Settings::getInstance();
static LedDriver *ledDriver = LedDriver::getInstance();
static MyTime *mt = MyTime::getInstance();
static OpenWeather *outdoorWeather = OpenWeather::getInstance();
static DisplayModes *dm = DisplayModes::getInstance();

extern EventGroupHandle_t xEvent;
extern s_taskParams taskParams;
extern DPMODE dpm[];
extern uint32_t dpmSize;

void infoAnzeige(void *p) {
    s_taskParams *tp = (s_taskParams*)p;
 
    DEBUG_PRINTLN("Start infoAnzeige");

    // disable TASK_SCHEDULER an TASK_TIME
    tp->taskInfo[TASK_SCHEDULER].handleEvent = false;
    tp->taskInfo[TASK_TIME].handleEvent = false;

    // clear all events
    xEventGroupClearBits(xEvent, 0x00FFFFFF);

    // set state to initialized
    uint32_t modes=0;
    for(uint8_t mode=0; mode < dpmSize; mode++) {
          modes += dpm[mode].event_type;
          dpm[mode].event_state = STATE_INIT;
    }

    // start all events
    xEventGroupSetBits(xEvent, modes);

    // wait until all events are processed
    while(true) {
      uint8_t processed=0;
      for(uint8_t mode=0; mode < dpmSize; mode++) {
        if(dpm[mode].event_state == STATE_PROCESSED)
          processed++;
      }
       if(processed==dpmSize)
        break;
      vTaskDelay(pdMS_TO_TICKS(10));
    }

    // enable TASK_SCHEDULER an TASK_TIME
    tp->taskInfo[TASK_TIME].handleEvent = true;
    tp->taskInfo[TASK_SCHEDULER].handleEvent = true;
    DEBUG_PRINTLN("Ende Infoanzeige");

    // terminate task
    vTaskDelete(NULL);
    return;
}


/******************************************************************************
  "Mode" pressed
******************************************************************************/

void buttonModePressed(AsyncWebServerRequest *request)
{
  DEBUG_PRINTLN(F("Mode pressed."));

  xTaskCreatePinnedToCore(
      &infoAnzeige,   // Function name of the task
      NULL,  // Name of the task (e.g. for debugging)
      2800,       // Stack size (bytes)
      &taskParams,       // Parameter to pass
      1,          // Task priority
      NULL,// Task handle
      0
  );
}

/******************************************************************************
  "Time" pressed
******************************************************************************/

void buttonTimePressed(AsyncWebServerRequest *request)
{
  DEBUG_PRINTLN(F("Time pressed."));
  // clear all events
  xEventGroupClearBits(xEvent, 0x00FFFFFF);
  // enable Events
  taskParams.taskInfo[TASK_SCHEDULER].handleEvent = true;
  taskParams.taskInfo[TASK_TIME].handleEvent = true;
  // display Time
  xEventGroupSetBits(xEvent, MODE_TIME);
}
