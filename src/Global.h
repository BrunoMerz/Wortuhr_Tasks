/**
   Global.h
   @autor    Bruno Merz

*/

#pragma once


#include <Arduino.h>
#include "Configuration.h"



class Global {
    public:
        static Global* getInstance();
        IPAddress myIP = { 0, 0, 0, 0 };
        int Modecount = 0;
        uint32_t stackSize;
        UBaseType_t highWaterMark;
        uint16_t codeline = 0;
        String codetab;
        uint32_t autoModeChangeTimer;
        int WLAN_reconnect = 0;
        uint8_t randomHour = 0;
        uint8_t randomMinute = 0;
        uint8_t randomSecond = 0;
        uint8_t akt_transition = 1;
        uint8_t akt_transition_old = 1;

    private:
        Global(void);
        static Global *instance;
};