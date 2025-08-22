
#include "Configuration.h"

#if defined(SENSOR_BME280)

#include "MyBME.h"
#include "OpenWeather.h"
#include "Settings.h"
#include "MyTime.h"

//#define myDEBUG
#include "MyDebug.h"

static Settings *settings = Settings::getInstance();
static OpenWeather *outdoorWeather = OpenWeather::getInstance();
static MyTime *mt = MyTime::getInstance();

MyBME* MyBME::instance = 0;

MyBME *MyBME::getInstance() {
  if (!instance)
  {
      instance = new MyBME();
  }
  DEBUG_PRINTLN("MyBME getInstance");
  return instance;
}

MyBME::MyBME(void) {
  DEBUG_PRINTLN(F("Setting up BME280."));
  tw.setPins(PIN_WIRE_SDA, PIN_WIRE_SCL);
  if (!bme.begin(0x76, &tw))
  {
    DEBUG_PRINTLN("Could not find a valid BME280 sensor, check wiring!");
  }
  else
  {
    DEBUG_PRINTLN("Setup BME280 OK");
  }
  lastMinute = 0;
}

void MyBME::getRoomConditions(void) {
  int stunde = mt->hour();
  String c_stunden;
  String c_minuten;

#if defined(RTC_BACKUP) && !defined(SENSOR_BME280)
  roomTemperature = RTC.temperature() / 4.0 + RTC_TEMPERATURE_OFFSET;
#ifdef DEBUG
  DEBUG_PRINTLN("Temperature (RTC): " + String(roomTemperature) + "C");
#endif
#endif
#ifdef SENSOR_BME280
  bme.takeForcedMeasurement();
  float luftdruckdiff = 0;
  float bmeTemperature = bme.readTemperature();
  float bmeHumidity = bme.readHumidity();
  float bmePressure = bme.readPressure() / 100.0F;

  DEBUG_PRINTF("BME SensorID was: %x\n", bme.sensorID());
  DEBUG_PRINTF("Temperature %f\n", bmeTemperature);
  DEBUG_PRINTF("Humidity %f\n", bmeHumidity);
  DEBUG_PRINTF("Pressure %f\n", bmePressure);

  if (bme.sensorID() != 255)
  {
    errorCounterBME = 0;
    roomTemperature = bmeTemperature + BME_TEMPERATURE_OFFSET;
    roomHumidity = bmeHumidity + BME_HUMIDITY_OFFSET;
    Pressure = bmePressure;
#ifdef APIKEY
    if (strlen(settings->mySettings.openweatherapikey) > 25)
    {
      Pressure_red = Pressure * pow(EULERSCHE_ZAHL, ((FALLBESCHLEUNIGUNG / (GASKONSTANTE * (KELVIN_0 + outdoorWeather->temperature + TEMPERATURGRADIENT * float(settings->mySettings.standort_hoehe)))) * float(settings->mySettings.standort_hoehe)));
    }
    else
    {
      Pressure_red = Pressure * pow(EULERSCHE_ZAHL, ((FALLBESCHLEUNIGUNG / (GASKONSTANTE * (KELVIN_0 + 20 + TEMPERATURGRADIENT * float(settings->mySettings.standort_hoehe)))) * float(settings->mySettings.standort_hoehe)));
    }
#else
    Pressure_red = Pressure * pow(EULERSCHE_ZAHL, ((FALLBESCHLEUNIGUNG / (GASKONSTANTE * (KELVIN_0 + 20 + TEMPERATURGRADIENT * float(settings->mySettings.standort_hoehe)))) * float(settings->mySettings.standort_hoehe)));
#endif
    // Historisierung Luftdruck
    lufdruck_hour[stunde % 3] = Pressure_red;
    luftdruck_hist[10] = (lufdruck_hour[0] + lufdruck_hour[1] + lufdruck_hour[2]) / 3; // Mittwelwert der letzten 3 Stunden

    if (mt->minute() == 1) // Historisierung der Luftdruckwerte
    {
      if (stunde % 3 == 0)
      {
        for (int hist = 0; hist <= 9; hist++)
          luftdruck_hist[hist] = luftdruck_hist[hist + 1];
        luftdruck_hist[10] = (lufdruck_hour[0] + lufdruck_hour[1] + lufdruck_hour[2]) / 3; // Mittwelwert der letzten 3 Stunden
      }
    }
    // Luftdrucktendenz für Web
    for (int i = 1; i < 11; i++)
    {
      luftdruckdiff = luftdruckdiff + (luftdruck_hist[i] - luftdruck_hist[i - 1]) * i;
    }
    if (luftdruckdiff <= LUFTDRUCK_DIFF_FALLEND)
      luftdrucktendenz_web = 1; // fallend
    if (luftdruckdiff > LUFTDRUCK_DIFF_FALLEND and luftdruckdiff <= LUFTDRUCK_DIFF_LEICHTFALLEND)
      luftdrucktendenz_web = 2; // leicht fallend
    if (luftdruckdiff > LUFTDRUCK_DIFF_LEICHTFALLEND and luftdruckdiff <= LUFTDRUCK_DIFF_LEICHTSTEIGEND)
      luftdrucktendenz_web = 3; // gleichbleibend
    if (luftdruckdiff > LUFTDRUCK_DIFF_LEICHTSTEIGEND and luftdruckdiff <= LUFTDRUCK_DIFF_STEIGEND)
      luftdrucktendenz_web = 4; // leicht steigend
    if (luftdruckdiff > LUFTDRUCK_DIFF_STEIGEND)
      luftdrucktendenz_web = 5; // steigend
    info_luftdruckdiff = luftdruckdiff;
    // Historisierung Temperatur
    c_stunden = String(stunde);
    if (stunde < 10)
      c_stunden = "0" + c_stunden;
    c_minuten = String(mt->minute());
    if (mt->minute() < 10)
      c_minuten = "0" + c_minuten;
    if ((mt->minute() % 20) - 1 == 0) // Werte alle 20 Minuten (01,21,41)
    {
#ifdef DEBUG
      DEBUG_PRINTLN("Historisierung Temp. Werte: " + c_stunden + ":" + c_minuten);
#endif
      for (int hist = 0; hist <= 71; hist++)
      {
        temperatur_hist[hist] = temperatur_hist[hist + 1];
      }
    }
    temperatur_hist[72].stundeminute = c_stunden + ":" + c_minuten;
    temperatur_hist[72].innentemp = roomTemperature;
    temperatur_hist[72].aussentemp = 0;
#ifdef APIKEY
    temperatur_hist[72].aussentemp = outdoorWeather->temperature;
#endif
#ifdef DEBUG
    DEBUG_PRINTLN("Temperature (BME): " + String(roomTemperature) + "C");
    DEBUG_PRINTLN("Humidity (BME): " + String(roomHumidity) + "%");
    DEBUG_PRINTLN("Pressure (BME): " + String(Pressure) + " hPa");
#endif
  }
  else
  {
    if (errorCounterBME < 250)
      errorCounterBME++;
#ifdef DEBUG
    DEBUG_PRINTLN(F("Could not find a valid BME280 sensor, check wiring, address, sensor ID!"));
    DEBUG_PRINTF("Error (BME): %u\r\n", errorCounterBME);
#endif
  }
#endif
}

#endif