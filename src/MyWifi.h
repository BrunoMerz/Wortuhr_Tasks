/**
   MyWifi.h
   @autor    Bruno Merz

   @version  2.0
   @created  30.12.2019
   @updated  31.03.2021

*/

#pragma once

#include "ESPAsyncWebServer.h"

#define WIFITIMEOUT 240
#define WPSTIMEOUT  30

#define USE_LITTLEFS
#define USING_CORS_FEATURE         false
#define USE_ESP_WIFIMANAGER_NTP    false
#define USE_CLOUDFLARE_NTP         false
#define USE_STATIC_IP_CONFIG_IN_CP false

//#define _ESPASYNC_WIFIMGR_LOGLEVEL_ 4


class MyWifi {
  public:
    static MyWifi* getInstance();
    void init(void);
    //static void saveConfigCallback (void);
    
    //static void configModeCallback (AsyncWiFiManager *myWiFiManager);

    bool myStartWPS(void);
    bool myBegin(char *ssid, char *passwd);
    String SSID(void) {return _ssid;};
    String PSK(void) {return _passwd;};
    IPAddress IP(void) {return _ip;};
    bool isConnected(void);
    void doReset(void);
    void saveSSIDandPWD(void);
    AsyncWebServer *getServer(void);
    static String _scanResultHTML;
  
  private:
    MyWifi(void);
    static MyWifi *instance;
    
    String _ssid;
    IPAddress _ip;
    String _passwd;
    
    bool  _connected;
};
