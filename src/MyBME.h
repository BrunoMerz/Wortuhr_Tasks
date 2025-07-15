/**
   MyBME.h
   @autor    Bruno Merz
*/
#pragma once

#if defined(SENSOR_BME280)
#include <Arduino.h>
#include <Adafruit_BME280.h>
#include <Wire.h>
#include "Configuration.h"


class MyBME {
  public:
    static MyBME* getInstance();
    void getRoomConditions(void);
   
    Adafruit_BME280 bme;
    TwoWire tw = TwoWire(0); ;
    bool bme_status;
    float luftdruck_hist[15] = {};
    float lufdruck_hour[3] = {};
    struct temperatur_hist_struct
    {
        String stundeminute;
        float innentemp;
        float aussentemp;
    };
    temperatur_hist_struct temperatur_hist[74];

    float roomTemperature = 0;
    float roomHumidity = 0;
    float Pressure = 0;
    float Pressure_red = 0;
    float info_luftdruckdiff = 0;
    int luftdrucktendenz_soundbase;
    int luftdrucktendenz_web = 0;
    uint8_t errorCounterBME = 0;
    uint8_t Tempcolor;
    int lastMinute;

  private:
    MyBME(void);
    static MyBME *instance;
 };

#endif