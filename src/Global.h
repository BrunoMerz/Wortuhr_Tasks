/**
   Global.h
   @autor    Bruno Merz

*/

#pragma once


#include <Arduino.h>
#include "Configuration.h"
#include "TaskStructs.h"



class Global {
    public:
        static Global* getInstance();
        void setHighWaterMark(uint8_t t);
        int Modecount = 0;
        uint32_t stackSize;
        UBaseType_t highWaterMark[TASK_MAX+1];
        uint16_t codeline = 0;
        String codetab;
        uint32_t autoModeChangeTimer;
        uint8_t randomHour = 0;
        uint8_t randomMinute = 0;
        uint8_t randomSecond = 0;
        uint8_t akt_transition = 1;
        uint8_t akt_transition_old = 1;

    private:
        Global(void);
        static Global *instance;
};