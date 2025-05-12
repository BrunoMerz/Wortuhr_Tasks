//******************************************************************************
// Animation.h
//******************************************************************************

#pragma once

#include <Arduino.h>
#include "Configuration.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "Colors.h"

#define HOSTNAME LANG_HOSTNAME

#define MAXFRAMES 25
#define MAXANIMATION 80
#define ANIMATIONSLISTE "/animationsliste.json"

typedef struct
  {
    color_s color[11][10];
    uint16_t delay;
  } s_frame;

typedef struct
  {
    char name[20];
    uint8_t loops;
    uint8_t laufmode;
    s_frame frame[MAXFRAMES+1];
  } s_myanimation;

  static s_frame copyframe;

  class AnimationFS
  {
    public:
      static AnimationFS* getInstance();

      bool loadAnimation(String aniname );
      void saveAnimationsListe();
      bool saveAnimation(String aniname );
      void getAnimationList();
      void makeAnimationmenue();
      bool showAnimation(uint8_t brightness);
      bool loadjsonarry( int8_t frame, uint8_t zeile, String &jsonBuffer);

      uint8_t akt_aniframe;
      uint8_t akt_aniloop;
      uint8_t frame_fak;        // Animationsrichtung
      s_myanimation myanimation;
      String myanimationslist[MAXANIMATION + 1];

    private:
      AnimationFS();
      static AnimationFS *instance;
      
      // uint32_t anipalette[] = { 0xFF0000, 0xFFAA00, 0xFFFF00, 0x00FF00, 0x00FFFF, 0x0000FF, 0xAA00FF, 0xFF00FF, 0x000000, 0xFFFFFF };
      uint32_t anipalette[10];
      
  };
  