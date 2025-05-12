#pragma once

#include <Arduino.h>
#include "LedDriver_FastLED.h"
#include "Renderer.h"
#if defined(ESP8266)
  #include <ESP8266WebServer.h>
  extern ESP8266WebServer webServer;
#else
  #include <WiFi.h>
  #include "WebServer.h"
  extern WebServer webServer;
#endif

#if defined(WITH_AUDIO)
#include "Audio.h"
#endif

#define  FIELD_WIDTH       11
#define  FIELD_HEIGHT      10
#define  DIR_UP    1
#define  DIR_DOWN  2
#define  DIR_LEFT  3
#define  DIR_RIGHT 4

#define  BTN_NONE  0
#define  BTN_UP    1
#define  BTN_DOWN  2
#define  BTN_LEFT  3
#define  BTN_RIGHT  4
#define  BTN_MIDDLE  5
#define  BTN_START  6
#define  BTN_STOP   7
#define  BTN_EXIT   8

#define SNAKE 1
#define TETRIS 2
#define BRICKS 3
#define VIERGEWINNT 4
#define TIERMEMORY 5
#define MUSIKMEMORY 6
#define ABBAMEMORY 7

