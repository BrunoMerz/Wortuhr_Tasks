//******************************************************************************
// Configuration.h
//******************************************************************************

#pragma once

#include <Arduino.h>

//******************************************************************************
// Software settings
//******************************************************************************
//die mit "//*" gekennzeichneten Zeilen nicht verändern!

#define WIFIHOSTNAME LANG_HOSTNAME             // Das ist der default Hostname/Systemname. Der eigentliche Systemname wird in Settings eingestellt.
#define WIFI_SETUP_TIMEOUT 300                 // So lange ist der AP (AccessPoint) zum eingeben der WiFi Parameter an
#define WIFI_AP_PASS "12345678"                // Wifi Pass für den AP
#define OTA_PASS "1234"                        // Passwort für den OTA (Over the Air) Update
#define NTP_SERVER "de.pool.ntp.org"           // Das ist der default Name des Zeitservers. Der eigentliche Zeitserver wird in Settings eingestellt.
#define SHOW_IP                                // Zeigt die IP-Adresse beim Start. Nach Neueingabe der Wlan Parameter wird die IP immer angezeigt!
#define WIFI_BEEPS                             // ob die WLAN Verbunden Meldung kommt
//#define ESP_LED                                // OnBoard LED blinkt im Sekundentakt

//#define NONE_TECHNICAL_ZERO                  // ob die Null bei Feed mit oder ohne Schrägstrich angezeigt wird.
#define AUTO_MODECHANGE_TIME 120               //* Default AutoMode Change Periode. Die eigentliche Periode wird in Settings eingestellt.
#define SHOW_MODE_TIMEOUT 3500                 //* Timer bis es im Mode Modus zum nächsten Mode geht
#define FEED_SPEED 120                         //* wie schnell die Buchstaben über die Uhr huschen
#define TRANSITION_SPEED 90                    //* wie schnell die Transitions laufen

#define MAXEVENTS  40                          //* wie viele Events maximal angelegt werden können                           
#define EVENT_TIME 1800                        //* Das ist die default Event Intervall. Der eigentliche Intervall wird am Event definiert

#define ALARM_LED_COLOR RED                    // Farbe der Alarm LED
//#define ABUSE_CORNER_LED_FOR_ALARM             // Rechte obere Minuten LED wird zur Alarm LED

//#define POWERON_SELFTEST                     // LEDs werden beim anschalten mit versch. Farben getestet. Der Test kann auch im Mode Modus bei nochmalige betätigen des ModeButtons während der Sekunden Ausgabe gestartet werden. 
//* welche Modes gibt es:
#define SHOW_MODE_ANSAGE                       
#define SHOW_MODE_SECONDS
#define SHOW_MODE_WEEKDAY
#define SHOW_MODE_DATE
#define SHOW_MODE_MOONPHASE
#define SHOW_MODE_WETTER
#define SHOW_MODE_TEST

//https://openweathermap.org/
// Test: http://api.openweathermap.org/data/2.5/weather?q=Baden-Baden,de&lang=de&units=metric&appid=<APIKEY>
#define APIKEY                                          // Zum abschalten von Openweather auskommentieren.
#define DEFAULTAPIKEY "OpenWeather APIKEY"              // Hier kann dein default APIKEY von openweathermap rein. Die eigentliche APIKey wird in Settings eingestellt.
#define LOCATION "Villingen-Schwenningen,de"            // Hier kann dein default openweathermap Location rein. Die eigentliche LOCATION wird in Settings eingestellt.
//#define LOCATION "Zurich, CH"
//#define LOCATION "Hong Kong, HK"
#define OPENWEATHER_PERIODE 1800                        //* wie oft in Sekunden openweather.org aufgerufen wird
#define OPENWEATHER_MAX_ERROR 20                        //* bei Fehler: wie oft es pro Tag versucht wird bis es aufgegeben wird

#define SunRiseLib                                      // falls kein API Key vorhanden ist kann Sonnenaufgang/Sonnenuntergang über die Lib SunRise.h berechnet werden.
#define LONGITUDE 8.449                                 // Die RunRise Lib ist in der Bibliotheksverwaltung zu finden.
#define LATITUDE 48.056                                 // Dafür wird der Standort benötigt. Kann z.B. über google maps erfolgen. Die eigentliche Standort wird in Settings eingestellt.

// SunRise/SunSet Animationen
#define ANI_SUNRISE "SUNRISE"                           // Name der Sonnenaufgangsanimation
#define ANI_SUNSET "SUNSET"                             // Name der Sonnenuntergangsanimation

//#define FRONTCOVER_EN
//#define FRONTCOVER_DE_DE
//#define FRONTCOVER_DE_SW
//#define FRONTCOVER_DE_BA
//#define FRONTCOVER_DE_SA
//#define FRONTCOVER_DE_MKF_DE
//#define FRONTCOVER_DE_MKF_SW
//#define FRONTCOVER_DE_MKF_BA
//#define FRONTCOVER_DE_MKF_SA
//#define FRONTCOVER_D3
//#define FRONTCOVER_CH
//#define FRONTCOVER_CH_AG                              // Aargau ohne L
//#define FRONTCOVER_CH_AL                              // Aargau mit L
//#define FRONTCOVER_ES
//#define FRONTCOVER_FR
//#define FRONTCOVER_IT
//#define FRONTCOVER_NL
//#define FRONTCOVER_BINARY

typedef enum eFrontcover : uint16_t
{
  FC_EN=1,
  FC_DE_DE,
  FC_DE_SW,
  FC_DE_BA,
  FC_DE_SA,
  FC_DE_MKF_DE,
  FC_DE_MKF_SW,
  FC_DE_MKF_BA,
  FC_DE_MKF_SA,
  FC_D3,
  FC_CH,
  FC_CH_AG,
  FC_CH_AL,
  FC_ES,
  FC_FR,
  FC_IT,
  FC_NL,
  FC_AE_DE,
  FC_AE_EN,
  FC_BINARY
} FRONTCOVER; 

//******************************************************************************
// Timezone
//******************************************************************************

//#define TIMEZONE_IDLW  // IDLW  International Date Line West UTC-12
//#define TIMEZONE_SST   // SST   Samoa Standard Time UTC-11
//#define TIMEZONE_HST   // HST   Hawaiian Standard Time UTC-10
//#define TIMEZONE_AKST  // AKST  Alaska Standard Time UTC-9
//#define TIMEZONE_USPST // USPST Pacific Standard Time (USA) UTC-8
//#define TIMEZONE_USMST // USMST Mountain Standard Time (USA) UTC-7
//#define TIMEZONE_USAZ  // USAZ  Mountain Standard Time (USA) UTC-7 (no DST)
//#define TIMEZONE_USCST // USCST Central Standard Time (USA) UTC-6
//#define TIMEZONE_USEST // USEST Eastern Standard Time (USA) UTC-5
//#define TIMEZONE_AST   // AST   Atlantic Standard Time UTC-4
//#define TIMEZONE_BST   // BST   Eastern Brazil Standard Time UTC-3
//#define TIMEZONE_VTZ   // VTZ   Greenland Eastern Standard Time UTC-2
//#define TIMEZONE_AZOT  // AZOT  Azores Time UTC-1
//#define TIMEZONE_GMT   // GMT   Greenwich Mean Time UTC
#define TIMEZONE_CET     // CET   Central Europe Time UTC+1
//#define TIMEZONE_EST   // EST   Eastern Europe Time UTC+2
//#define TIMEZONE_MSK   // MSK   Moscow Time UTC+3 (no DST)
//#define TIMEZONE_GST   // GST   Gulf Standard Time UTC+4
//#define TIMEZONE_PKT   // PKT   Pakistan Time UTC+5
//#define TIMEZONE_BDT   // BDT   Bangladesh Time UTC+6
//#define TIMEZONE_JT    // JT    Java Time UTC+7
//#define TIMEZONE_CNST  // CNST  China Standard Time UTC+8
//#define TIMEZONE_HKT   // HKT   Hong Kong Time UTC+8
//#define TIMEZONE_PYT   // PYT   Pyongyang Time (North Korea) UTC+8.5
//#define TIMEZONE_CWT   // CWT   Central West Time (Australia) UTC+8.75
//#define TIMEZONE_JST   // JST   Japan Standard Time UTC+9
//#define TIMEZONE_ACST  // ACST  Australian Central Standard Time UTC+9.5
//#define TIMEZONE_AEST  // AEST  Australian Eastern Standard Time UTC+10
//#define TIMEZONE_LHST  // LHST  Lord Howe Standard Time UTC+10.5
//#define TIMEZONE_SBT   // SBT   Solomon Islands Time UTC+11
//#define TIMEZONE_NZST  // NZST  New Zealand Standard Time UTC+12

//******************************************************************************
// Hardware settings
//******************************************************************************

#define SERIAL_SPEED 115200

//#define ONOFF_BUTTON
//#define MODE_BUTTON
//#define SHOW_TIME_BUTTON


// Einstellen der min/max Helligkeiten
#define MIN_BRIGHTNESS 20                            // Minimale Helligkeit
#define MAX_BRIGHTNESS 255                           // Maximale Helligkeit
#define TEST_BRIGHTNESS 80                           // Helligkeit beim LED Test

// Ist ein BME280 an Board
#if defined(LILYGO_T_HMI)
#define SENSOR_BME280
#endif
#define SEALEVELPRESSURE_HPA (1013.25)               //* Luftdruck auf Meereshöhe
#define BME_TEMPERATURE_OFFSET -0.1                  // Temperaturkorrektur
#define BME_HUMIDITY_OFFSET +0.1                     // Luftfeuchtekorrektur


#define HOEHE_UEBER_0 720                            // Hier die default Höhe über NN des Wohnortes eintragen.Der eigentliche Wert wird in Settings eingestellt.

#define LUFTDRUCKMIN 975                             //* Hier den zu erwartenden minimalen Luftdruck eintragen (Baseline im Diagramm)
//* Parameter für die Berechnung des Luftdrucks auf Meeresniveau:
#define FALLBESCHLEUNIGUNG 9.80665
#define GASKONSTANTE 287.05
#define KELVIN_0 273.15
#define TEMPERATURGRADIENT 0.00325
#define EULERSCHE_ZAHL 2.7183

// Schwellen für steigend/fallender Luftdruck:
#define LUFTDRUCK_DIFF_STEIGEND 30
#define LUFTDRUCK_DIFF_LEICHTSTEIGEND 15
#define LUFTDRUCK_DIFF_LEICHTFALLEND -15
#define LUFTDRUCK_DIFF_FALLEND -30

//#define RTC_BACKUP
#define RTC_TEMPERATURE_OFFSET -0.1

// Lichtabhängiger Widerstand an Board?
//#define LDR
//#define LDR_IS_INVERSE                      // Falls LDR Wert gegen Masse angeschlossen wurde
#define MIN_LDR_STARTVALUE 30                 // LDR Startwert bei Dunkelheit (1-1023) (Wert siehe Infoseite)
#define MAX_LDR_STARTVALUE 70                // LDR Startwert bei Tageslicht (1-1023) (Wert muss größer als MIN_LDR_STARTVALUE sein!)

#define SMODE_SOUND "0"

#define NUMPIXELS 115                       // mit Alarm LED
#define NUMSECONDS 59
//#define NUMPIXELS 114                       // ohne Alarm LED

// Das LED Layout (Siehe in LedDriver.cpp):
//#define LED_LAYOUT_HORIZONTAL_1
//#define LED_LAYOUT_HORIZONTAL_2         // B
//#define LED_LAYOUT_HORIZONTAL_3          // C (260x260)
//#define LED_LAYOUT_VERTICAL_1
//#define LED_LAYOUT_VERTICAL_2
//#define LED_LAYOUT_VERTICAL_3           // A (390x390)
#define LED_LAYOUT_VERTICAL_4             // mz

//******************************************************************************
// Einstellungen LED Type
//******************************************************************************
//#define LED_LIBRARY_LPD8806RGBW              // https://github.com/bracci/LPD8806RGBW for download Lib
//-------------------------------

#define LED_LIBRARY_NEOPIXEL

#define NEOPIXEL_TYPE NEO_GRB + NEO_KHZ800     // see Adafruit_NeoPixel.h for help

//#define NEOPIXEL_TYPE NEO_GRBW + NEO_KHZ800

//#define NEOPIXEL_TYPE NEO_WRGB + NEO_KHZ800

// Um den Weißanteil bzw. die Farbtemperatur zu verändern (nur für sk6812 LEDs mit weißer LED)
//#define NEOPIXEL_FARBTEMPERATUR

#ifdef NEOPIXEL_FARBTEMPERATUR
#define whiteboost 3 //1-10 default 4 
//Je kleiner der whiteboost, desto höher der Weißanteil (heller aber weniger starke Farben). 
//Je größer der whiteboost, desto kleiner der Weißanteil (dunkler aber intensivere Farben).

//Ändern der Farbtemperatur. Nur für sk6812 LEDs
//Laut Datenblatt = RGBW (WW 2700-3000K; CW 6000-6500K; NW 4000-4500K)
//https://andi-siess.de/rgb-to-color-temperature/
// WW K2800: (255, 173, 94)
// CW K6300: (255, 246, 247)
// NW K4200: (255, 213, 173)
//Beispiel K4500: (255, 219, 186),
#define temp_red 255 //default 255 
#define temp_green 255 //default 255
#define temp_blue 255 //default 255
#endif

#define LED_COLS  11
#define LED_ROWS  10

//******************************************************************************
// Misc
//******************************************************************************

// Debug Schalter:
// Im Normalbetrieb immer alle DEBUG Schalter aus!

//#define DEBUG 1
//#define DEBUG_EVENTS
//#define DEBUG_ANIMATION
//#define DEBUG_GAME
//#define DEBUG_WEB
//#define DEBUG_IR
//#define DEBUG_MATRIX
//#define DEBUG_FPS
//#define DEBUG_OW
//#define DEBUG_AUDIO
//#define DEBUG_WPS
//#define DEBUG_SECONDHAND


#define __NAME__ (strrchr(__FILE__,'\\')?strrchr(__FILE__,'\\')+1:__FILE__)
//******************************************************************************
//#define SYSLOGSERVER_SERVER "192.168.178.193"
//#define SYSLOGSERVER_PORT 514
//see platformio.ini


#define JSON_CONFIG F("/Wortuhr.json")  // Name of LittleFS JSON filename

#if defined(ESP8266)
//******************************************************************************
// ESP8266 / WeMos Anschlüsse:
//******************************************************************************

#define WIFI_RESET       D8
#define ANALOG_PIN       A0
#define PIN_MODE_BUTTON  D7     // D7      D3,mode/flash
#define PIN_LED          D4     // D4      D4,LED Data        LED_BUILTIN
#define PIN_AUDIO_RX     D5     // D5      D5,14
#define PIN_AUDIO_TX     D7     // D3      D7,13
#define PIN_AUDIO_BUSY   D0     // D0      D0,16              kein Interrupt!
//#define PIN_BUZZER       D4     //
//#define PIN_LEDS_CLOCK   D6     //       D6,12              wird für LPD8806RGBW benötigt D6
#define PIN_LEDS_DATA    D4     // D8      D8,Data            (z.B. WS2812B)
#define PIN_SECONDS_DATA D3     // D8      D8,Data            (z.B. WS2812B)
#define PIN_LDR          A0     // A0      A0,LDR             ADC
#define PIN_SHOW_TIME_BUTTON RX //         RX,time
#define PIN_ONOFF_BUTTON TX     //         TX,on/off
// GPIO 06 to GPIO 11 are
// used for flash memory databus
#endif

#if defined(WEMOS_D1_MINI32)
//******************************************************************************
// WEMOS_D1_MINI32 / WeMos Anschlüsse:
//******************************************************************************

//                                 V1      KELLY
//#define PIN_IR_RECEIVER  D6     //                          wird für IR-Receiver benötigt
#define WIFI_RESET      (05)
#define ANALOG_PIN      (34)
#define PIN_WIRE_SCL     SCL    // D1,SCL  D1,SCL             SCL
#define PIN_WIRE_SDA     SDA    // D2,SDA  D2,SDA             SDA
#define PIN_MODE_BUTTON  D7     // D7      D3,mode/flash
#define PIN_LED          D4     // D4      D4,LED Data        LED_BUILTIN
#define PIN_AUDIO_RX     D5     // D5      D5,14
#define PIN_AUDIO_TX     D7     // D3      D7,13
#define PIN_AUDIO_BUSY   D0     // D0      D0,16              kein Interrupt!
//#define PIN_BUZZER       D4     //
//#define PIN_LEDS_CLOCK   D6     //       D6,12              wird für LPD8806RGBW benötigt D6
#define PIN_LEDS_DATA    D4     // D8      D8,Data            (z.B. WS2812B)
#define PIN_SECONDS_DATA D3     // D8      D8,Data            (z.B. WS2812B)
#define PIN_LDR          A0     // A0      A0,LDR             ADC
#define PIN_SHOW_TIME_BUTTON RX //         RX,time
#define PIN_ONOFF_BUTTON TX     //         TX,on/off
// GPIO 06 to GPIO 11 are
// used for flash memory databus
#endif

#if defined(ARDUINO_LOLIN_S2_MINI)
//******************************************************************************
// ARDUINO_LOLIN_S2_MINI / WeMos Anschlüsse:
//******************************************************************************

//
#define ANALOG_PIN       ADC1_CHANNEL_1     // ADC1_0
#define PIN_WIRE_SCL     SCL    // D1,SCL  D1,SCL             SCL
#define PIN_WIRE_SDA     SDA    // D2,SDA  D2,SDA             SDA
#define PIN_MODE_BUTTON  D7     // D7      D3,mode/flash
#define PIN_LED          D4     // D4      D4,LED Data        LED_BUILTIN
#define PIN_AUDIO_RX     D5     // D5      D5,14
#define PIN_AUDIO_TX     D7     // D3      D7,13
#define PIN_AUDIO_BUSY   D0     // D0      D0,16              kein Interrupt!
//#define PIN_BUZZER       D4     //
//#define PIN_LEDS_CLOCK   D6     //       D6,12              wird für LPD8806RGBW benötigt D6
#define PIN_LEDS_DATA    16     //
#if defined(WITH_SECOND_HAND)
#define PIN_SECONDS_DATA 17     //
#endif
#define PIN_LDR          ADC1_CHANNEL_1     // ADC1_1
#define WIDTH_LDR        ADC_WIDTH_BIT_13
#define PIN_SHOW_TIME_BUTTON RX //         RX,time
#define PIN_ONOFF_BUTTON TX     //         TX,on/off
#define WIFI_RESET       (14)
#endif

#if defined(ARDUINO_LOLIN_S3_MINI)
//******************************************************************************
// ARDUINO_LOLIN_S3_MINI / WeMos Anschlüsse:
//******************************************************************************

//                                 V1      KELLY
//#define PIN_IR_RECEIVER  D6     //                          wird für IR-Receiver benötigt
#define ANALOG_PIN       ADC1_CHANNEL_1     // ADC1_0
#define PIN_WIRE_SCL     SCL    // D1,SCL  D1,SCL             SCL
#define PIN_WIRE_SDA     SDA    // D2,SDA  D2,SDA             SDA
#define PIN_MODE_BUTTON  D7     // D7      D3,mode/flash
#define PIN_LED          D4     // D4      D4,LED Data        LED_BUILTIN
#define PIN_AUDIO_RX     D5     // D5      D5,14
#define PIN_AUDIO_TX     D7     // D3      D7,13
#define PIN_AUDIO_BUSY   D0     // D0      D0,16              kein Interrupt!
//#define PIN_BUZZER       D4     //
//#define PIN_LEDS_CLOCK   D6     //       D6,12              wird für LPD8806RGBW benötigt D6
#define PIN_LEDS_DATA    16     //
#define PIN_SECONDS_DATA 18     //
#define PIN_LDR          ADC1_CHANNEL_1     // ADC1_0
#define WIDTH_LDR        ADC_WIDTH_BIT_12
#define PIN_SHOW_TIME_BUTTON RX //         RX,time
#define PIN_ONOFF_BUTTON TX     //         TX,on/off
#define WIFI_RESET       (14)
#endif

#if defined(LILYGO_T_HMI)
//******************************************************************************
// LILYGO_T_HMI / WeMos Anschlüsse:
//******************************************************************************

//                                 V1      KELLY
//#define PIN_IR_RECEIVER  D6     //                          wird für IR-Receiver benötigt
#define ANALOG_PIN      (13)     // IO15

#define PIN_WIRE_SCL     15    // D1,SCL  D1,SCL             SCL
#define PIN_WIRE_SDA     16    // D2,SDA  D2,SDA             SDA
//#define PIN_MODE_BUTTON  15     // D7      D3,mode/flash
#define PIN_LED          LED_BUILTIN     // D4      D4,LED Data        LED_BUILTIN
#define PIN_AUDIO_RX     RX     // IO44 RX
#define PIN_AUDIO_TX     TX     // IO43 TX
#define PIN_AUDIO_BUSY   11     // IO18 Busy
//#define PIN_BUZZER       D4     //
//#define PIN_LEDS_CLOCK   D6     //       D6,12              wird für LPD8806RGBW benötigt D6
#define PIN_LEDS_DATA    17     // D8      D8,Data            (z.B. WS2812B)
#define PIN_SECONDS_DATA 18     // D8      D8,Data            (z.B. WS2812B)
#define PIN_LDR          ANALOG_PIN     // A0      A0,LDR             ADC
//#define PIN_SHOW_TIME_BUTTON RX //         RX,time
//#define PIN_ONOFF_BUTTON TX     //         TX,on/off
// GPIO 06 to GPIO 11 are
// used for flash memory databus
#define PWR_EN_PIN       (10)
#define WIFI_RESET       (21)

// touch screen
#define TOUCHSCREEN_SCLK_PIN (1)
#define TOUCHSCREEN_MISO_PIN (4)
#define TOUCHSCREEN_MOSI_PIN (3)
#define TOUCHSCREEN_CS_PIN   (2)
#define TOUCHSCREEN_IRQ_PIN  (9)


#endif

#if defined(LILYGO_T7_S3)
//******************************************************************************
// LILYGO_T7_S3 / WeMos Anschlüsse:
//******************************************************************************

//                                 V1      KELLY
//#define PIN_IR_RECEIVER  D6     //                          wird für IR-Receiver benötigt
#define WIFI_RESET      (05)
#define ANALOG_PIN      (01)
#define PIN_WIRE_SCL     SCL    // D1,SCL  D1,SCL             SCL
#define PIN_WIRE_SDA     SDA    // D2,SDA  D2,SDA             SDA
#define PIN_MODE_BUTTON  15     // D7      D3,mode/flash
#define PIN_LED          12     // D4      D4,LED Data        LED_BUILTIN
#define PIN_AUDIO_RX     16     // D5      D5,14
#define PIN_AUDIO_TX     17     // D3      D7,13
#define PIN_AUDIO_BUSY   18     // D0      D0,16              kein Interrupt!
//#define PIN_BUZZER       D4     //
//#define PIN_LEDS_CLOCK   D6     //       D6,12              wird für LPD8806RGBW benötigt D6
#define PIN_LEDS_DATA    44     // D8      D8,Data            (z.B. WS2812B)
#define PIN_SECONDS_DATA 43     // D8      D8,Data            (z.B. WS2812B)
#define PIN_LDR          A0     // A0      A0,LDR             ADC
#define PIN_SHOW_TIME_BUTTON RX //         RX,time
#define PIN_ONOFF_BUTTON TX     //         TX,on/off
// GPIO 06 to GPIO 11 are
// used for flash memory databus
#endif
