/******************************************************************************
Events.h
******************************************************************************/
#pragma once

#include <Arduino.h>
#include "Configuration.h"
#include "Colors.h"

#define USE_LittleFS
#include <FS.h>
#include <LittleFS.h>
#include <SD.h>

#include <ArduinoJson.h>


#define EVENTFILE "/events.json"

typedef enum eSTType : uint16_t
{
  ST_DATE=1,
  ST_ALWAYS=2,
	ST_NEVER=3,
} STType; 

typedef struct
{
  bool aktiv;
	uint8_t month;
  uint8_t day;
	String text;
  uint16_t year;
  uint32_t color;
  uint16_t audio_file;
  String preani;
  String postani;
  uint16_t intervall;  // interval in Minuten
  STType start;
} s_event;

class Events {
    public:
      static Events* getInstance();
      bool loadEvents(void);
      bool saveEvents(void);
      void runEvent(uint8_t eventNo);
      s_event events[MAXEVENTS+1];
      bool mustLoad;
    private:
      Events(void);
        static Events *instance;
};