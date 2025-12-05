/**
   DisplayModes.cpp
   DisplayModes

   @autor    B. Merz

*/

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
#include "Animation.h"

//#define myDEBUG
#include "MyDebug.h"

static Renderer *renderer = Renderer::getInstance();
static Settings *settings = Settings::getInstance();
static LedDriver *ledDriver = LedDriver::getInstance();
static MyTime *mt = MyTime::getInstance();
static OpenWeather *ow = OpenWeather::getInstance();
static AnimationFS *anifs = AnimationFS::getInstance();

extern uint16_t matrix[10];

DisplayModes* DisplayModes::instance = 0;

DisplayModes *DisplayModes::getInstance() {
  if (!instance)
  {
      instance = new DisplayModes();
  }
  return instance;
}

DisplayModes::DisplayModes() {

}


void DisplayModes::displayAnsage(void) {
    // Ansage
    renderer->clearScreenBuffer(matrix);
    matrix[0] = 0b0000111000000000;
    matrix[1] = 0b0011111110000000;
    matrix[2] = 0b0111111111000000;
    matrix[3] = 0b0110111011000000;
    matrix[4] = 0b1110111011100000;
    matrix[5] = 0b1111111111100000;
    matrix[6] = 0b0101111101000000;
    matrix[7] = 0b0110000011000000;
    matrix[8] = 0b0011000110000000;
    matrix[9] = 0b0000111000000000;
    ledDriver->mode = MODE_ANSAGE;
    ledDriver->writeScreenBufferFade(matrix, colorArray[YELLOW]);
}


void DisplayModes::displayWeekday(void) {
    renderer->clearScreenBuffer(matrix);
    renderer->setSmallText(String((char)pgm_read_byte(&sWeekday[mt->weekday()][0])) + String((char)pgm_read_byte(&sWeekday[mt->weekday()][1])), TEXT_POS_MIDDLE, matrix);
    ledDriver->mode = MODE_WEEKDAY;
    ledDriver->writeScreenBufferFade(matrix, colorArray[YELLOW]);
}


void DisplayModes::displayDate(void) {
    uint8_t aktDay = mt->mytm.tm_mday;
    uint8_t aktMonth = mt->mytm.tm_mon;
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
    ledDriver->writeScreenBufferFade(matrix, colorArray[YELLOW]);
}


void DisplayModes::displayMoonphase(void) {
    int ani_moonphase;
    renderer->clearScreenBuffer(matrix);

    DEBUG_PRINTLN("moonphase: " + String(ow->moonphase));
    ledDriver->mode = MODE_MOONPHASE;

    for(int ModeSequenz=1; ModeSequenz<=5; ModeSequenz++) {
        ani_moonphase =  ow->moonphase - 5 + ModeSequenz;

        if ( ani_moonphase < 0 ) ani_moonphase = 22 + ani_moonphase;
    
        //DEBUG_PRINTLN("ani_moonphase: " + String(ani_moonphase));
    
        ledDriver->saveMatrix(matrix);
        for (uint8_t i = 0; i <= 9; i++)
        {
            matrix[i] = pgm_read_word(&MONDMATRIX[ani_moonphase][i]);
        }
        ledDriver->writeScreenBufferFade(matrix, colorArray[ORANGE]);
        vTaskDelay(pdMS_TO_TICKS(10)); // 1 Sekunde warten
    }
}


void DisplayModes::displayWeather(void) {
    ledDriver->saveMatrix(matrix, true);
    renderer->clearScreenBuffer(matrix);
    ow->getOutdoorConditions(String(settings->mySettings.openweatherlocation), String(settings->mySettings.openweatherapikey));
    if ( ow->retcodeOutdoorWeather > 0 )
    {
        ow->errorCounterOutdoorWeather++;
    }
    else
    {
        ledDriver->mode = MODE_WETTER;
        String animation = F("WETTER_");
    
         if (ow->WetterSequenz == 1)
            animation += ow->getWeatherIcon(ow->weathericon1);
          if (ow->WetterSequenz == 2001)
            animation += ow->getWeatherIcon(ow->weathericon2);

        animation.toUpperCase();
        if (anifs->loadAnimation(animation))
        {
            anifs->akt_aniframe = 0;
            anifs->akt_aniloop = 0;
            anifs->frame_fak = 0;
        
            while(anifs->showAnimation()) {
                vTaskDelay(pdMS_TO_TICKS(10));
            }
            vTaskDelay(pdMS_TO_TICKS(500));
        }
    }
}


void DisplayModes::displayExtTemp(void) {
    ledDriver->saveMatrix(matrix, true);
    renderer->clearScreenBuffer(matrix);
    ow->getOutdoorConditions(String(settings->mySettings.openweatherlocation), String(settings->mySettings.openweatherapikey));
    if ( ow->retcodeOutdoorWeather > 0 )
    {
        ow->errorCounterOutdoorWeather++;
    }
    else
    {

    }
    if (ow->temperature > 0)
    {
        matrix[1] = 0b0100000000000000;
        matrix[2] = 0b1110000000000000;
        matrix[3] = 0b0100000000000000;
    }
    if (ow->temperature < 0)
    {
        matrix[2] = 0b1110000000000000;
    }
    renderer->setSmallText(String(int(abs(ow->temperature) + 0.5)), TEXT_POS_BOTTOM, matrix);
    uint8_t Tempcolor;
    if (ow->temperature < -10)
        Tempcolor = VIOLET;
    else if (ow->temperature >= -10 && ow->temperature < -5)
        Tempcolor = BLUE;
    else if (ow->temperature >= -5 && ow->temperature < 0)
        Tempcolor = LIGHTBLUE;
    else if (ow->temperature >= 0 && ow->temperature < 5)
        Tempcolor = CYAN;
    else if (ow->temperature >= 5 && ow->temperature < 10)
        Tempcolor = MINTGREEN;
    else if (ow->temperature >= 10 && ow->temperature < 15)
        Tempcolor = GREEN;
    else if (ow->temperature >= 15 && ow->temperature < 20)
        Tempcolor = GREENYELLOW;
    else if (ow->temperature >= 20 && ow->temperature < 25)
        Tempcolor = YELLOW;
    else if (ow->temperature >= 25 && ow->temperature < 30)
        Tempcolor = ORANGE;
    else if (ow->temperature >= 30 && ow->temperature < 40)
        Tempcolor = RED;
    else if (ow->temperature >= 40)
        Tempcolor = RED_50;

    ledDriver->mode = MODE_EXT_TEMP;
    ledDriver->writeScreenBufferFade(matrix, colorArray[Tempcolor]);
}



void DisplayModes::displayExtHumidity(void) {
    ledDriver->saveMatrix(matrix, true);
    renderer->clearScreenBuffer(matrix);
    if (ow->humidity < 100)
    {
        renderer->setSmallText(String(ow->humidity), TEXT_POS_TOP, matrix);
    }
    else
    {
        matrix[0] = 0b0010111011100000;
        matrix[1] = 0b0110101010100000;
        matrix[2] = 0b0010101010100000;
        matrix[3] = 0b0010101010100000;
        matrix[4] = 0b0010111011100000;
    }
    matrix[6] = 0b0100100000000000;
    matrix[7] = 0b0001000000000000;
    matrix[8] = 0b0010000000000000;
    matrix[9] = 0b0100100000000000;

    ledDriver->mode = MODE_EXT_HUMIDITY;
    ledDriver->writeScreenBufferFade(matrix, colorArray[VIOLET]);
}


void DisplayModes::displaySeconds(void) {
    for(uint8_t sek=0; sek<10; sek++) {
        mt->getTime();
        uint8_t aktMinute = mt->mytm.tm_min;
        uint8_t aktSecond = mt->mytm.tm_sec;
        renderer->clearScreenBuffer(matrix);
        renderer->setCorners(aktMinute, matrix);
        for (uint8_t i = 0; i <= 6; i++)
        {
            uint8_t b = pgm_read_byte(&numbersBig[aktSecond / 10][i]);
            matrix[1 + i] |= b << 11;
            b = pgm_read_byte(&numbersBig[aktSecond % 10][i]);
            matrix[1 + i] |= b << 5;
         }
         ledDriver->mode = MODE_SECONDS;
         ledDriver->writeScreenBuffer(matrix, colorArray[WHITE]);
         vTaskDelay(pdMS_TO_TICKS(1000));
    }
}