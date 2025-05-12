/**
   MyWifi.cpp
   @autor    Bruno Merz
*/

#define USE_EADNS

#include "esp_wps.h"
#include <WiFi.h>

#include "ESPAsyncWebServer.h"
#include "ESPAsyncWiFiManager.h"
#include <ESPAsyncDNSServer.h>
#include <ElegantOTA.h>


#include "MyWifi.h"
#include "Configuration.h"
//#include "esp_task_wdt.h"

#define myDEBUG
#include "MyDebug.h"

//#include "MyTFT.h"
//#include "TFTImageRenderer.h"
#include "IconRenderer.h"
#include "Settings.h"


#define ESP_WPS_MODE      WPS_TYPE_PBC
#define ESP_MANUFACTURER  "ESPRESSIF"
#define ESP_MODEL_NUMBER  "ESP32"
#define ESP_MODEL_NAME    "ESPRESSIF IOT"
#define ESP_DEVICE_NAME   "ESP STATION"

static boolean  _wpsSuccess;
static boolean  _got_ip;
static int      _wifi_stat;

#if defined(LILYGO_T_HMI)
static MyTFT *tft = MyTFT::getInstance();
static TFTImageRenderer *ir = TFTImageRenderer::getInstance();
#endif

static AsyncWebServer server(80);
static AsyncDNSServer dns;
static AsyncWiFiManager wifiManager(&server, &dns);


static IconRenderer *icor = IconRenderer::getInstance();
//static AsyncWiFiManager wifiManager(&server, &dns); // global wm instance
static Settings *settings = Settings::getInstance();


static esp_wps_config_t config;

String wpspin2string(uint8_t a[]) {
  char wps_pin[9];
  for(int i=0;i<8;i++){
    wps_pin[i] = a[i];
  }
  wps_pin[8] = '\0';
  return (String)wps_pin;
}

void wpsStop(void) {
    if(esp_wifi_wps_disable()){
      DEBUG_PRINTLN("WPS Disable Failed");
    }
}

boolean wpsStart(void) {
  _wpsSuccess=false;
  if(esp_wifi_wps_enable(&config)) {
    DEBUG_PRINTLN("WPS Enable Failed");
  } else if(esp_wifi_wps_start(0)) {
    DEBUG_PRINTLN("WPS Start Failed");
  } else
    DEBUG_PRINTLN("esp_wifi_wps_start OK");

  for (int i = 0; i < WPSTIMEOUT ; i++) {
    if (WiFi.status() == WL_CONNECTED)
    {
      DEBUG_PRINTLN("WPS WiFi connected");
      _wpsSuccess=true;
      break;
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
    DEBUG_PRINTF("Waiting for IP, i=%d\n",i);
  }
  if(!_wpsSuccess) {
    DEBUG_PRINTLN("WPS WiFi failed");
    wpsStop();
  }
  return _wpsSuccess;
}



void wpsInitConfig(){
  config.wps_type = ESP_WPS_MODE;
  strcpy(config.factory_info.manufacturer, ESP_MANUFACTURER);
  strcpy(config.factory_info.model_number, ESP_MODEL_NUMBER);
  strcpy(config.factory_info.model_name, ESP_MODEL_NAME);
  strcpy(config.factory_info.device_name, ESP_DEVICE_NAME);
}

void WiFiEvent(WiFiEvent_t event, arduino_event_info_t info){
  switch(event){
    case ARDUINO_EVENT_WIFI_STA_START:
      DEBUG_PRINTLN("Station Mode Started");
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      DEBUG_PRINTLN("Connected to :" + String(WiFi.SSID()));
      DEBUG_PRINT("GOT_IP: ");
      DEBUG_PRINTLN(WiFi.localIP().toString());
      _got_ip = true;
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      DEBUG_PRINTLN("Disconnected from station, attempting reconnection");
      WiFi.reconnect();
      break;
    case ARDUINO_EVENT_WPS_ER_SUCCESS:
      DEBUG_PRINTLN("WPS Successfull, stopping WPS and connecting to: " + String(WiFi.SSID()));
      wpsStop();
      vTaskDelay(pdMS_TO_TICKS(10));
      WiFi.begin();
      break;
    case ARDUINO_EVENT_WPS_ER_FAILED:
      DEBUG_PRINTLN("WPS Failed, retrying");
      wpsStop();
      wpsStart();
      break;
    case ARDUINO_EVENT_WPS_ER_TIMEOUT:
      DEBUG_PRINTLN("WPS Timedout, retrying");
      wpsStop();
      wpsStart();
      break;
    case ARDUINO_EVENT_WPS_ER_PIN:
      DEBUG_PRINTLN("WPS_PIN = " + wpspin2string(info.wps_er_pin.pin_code));
      break;
    default:
      break;
  }
}

void handleRoot(AsyncWebServerRequest *request) {
  String html = R"rawliteral(
    <html>
      <head><title>WiFi Konfiguration</title></head>
      <body>
        <h1>WLAN auswählen</h1>
        <form action="/save" method="GET">
          <label for="ssid">SSID:</label>
          <select name="ssid">%OPTIONS%</select><br><br>
          <label for="pass">Passwort:</label>
          <input name="pass" type="password"><br><br>
          <input type="submit" value="Verbinden">
        </form>
      </body>
    </html>
  )rawliteral";

  html.replace("%OPTIONS%", MyWifi::_scanResultHTML);
  request->send(200, "text/html", html);
}


void handleSave(AsyncWebServerRequest *request) {
  String _ssid = request->arg("ssid");
  String _passwd = request->arg("pass");
  request->send(200, "text/plain", "SSID: " + _ssid + "\nPasswort: " + _passwd + "\nGespeichert!");
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(_ssid, _passwd);
}


MyWifi* MyWifi::instance = 0;
String MyWifi::_scanResultHTML = "";

MyWifi *MyWifi::getInstance() {
  if (!instance)
  {
      instance = new MyWifi();
  }
  return instance;
}

MyWifi::MyWifi(void) {

}


bool MyWifi::scanNetworks() {
  Serial.println("Suche nach WLAN-Netzen...");
  bool ret;
  int n = WiFi.scanNetworks();
  _scanResultHTML = "";

  if (n == 0) {
    _scanResultHTML = "<option value=\"\">Keine Netzwerke gefunden</option>";
    ret = false;
  } else {
    for (int i = 0; i < n; ++i) {
      String ssid = WiFi.SSID(i);
      ssid.replace("\"", ""); // doppelte Anführungszeichen vermeiden
      _scanResultHTML += "<option value=\"" + ssid + "\">" + ssid + "</option>\n";
    }
    ret = true;
  }
  Serial.printf("Suche nach WLAN-Netzen beendet: ret=%d, _scanResultHTML=%s\n", ret, _scanResultHTML.c_str());
  return ret;
}

bool MyWifi::startConfigPortal(char *ssid) {
  DEBUG_PRINTLN("Starting Config Portal");
  
  wifiManager.startConfigPortal("WORTUHR1");
  DEBUG_PRINTLN("Nach Config Portal");
  return true;
}

/*
bool MyWifi::startConfigPortal(char *ssid) {
  DEBUG_PRINTLN("Starting Config Portal");
  IPAddress apIP(192, 168, 4, 1);
  IPAddress netMsk(255, 255, 255, 0);
  const byte DNS_PORT = 53;
  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.softAP(ssid);
  WiFi.softAPConfig(apIP, apIP, netMsk);

  DEBUG_PRINTLN("Starting DNS Portal");
  dns.start(DNS_PORT, "*", apIP);
  DEBUG_PRINTLN("adding handler");
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleRoot(request);
  });
  

  server.on("/save", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleSave(request);
  });

  server.onNotFound([](AsyncWebServerRequest *request) { 
    AsyncWebServerResponse *response = request->beginResponse(302, "text/plain");
    response->addHeader("Location", "/");
    request->send(response);                                                                                      
  });
  DEBUG_PRINTLN("Starting server");
  server.begin();
  DEBUG_PRINTLN("Server started");
  while(!_got_ip) {
    DEBUG_PRINTLN("Waiting for IP");
    delay(1000);
  }
  return true;
}
*/

bool MyWifi::myStartWPS(void) {
  DEBUG_PRINTLN("WPS Konfiguration gestartet");
  _wpsSuccess = true;
  
  WiFi.mode(WIFI_MODE_STA);
  DEBUG_PRINTLN("Starting WPS");
  wpsInitConfig();
  DEBUG_PRINTLN("Config done");
  wpsStart();

  return _wpsSuccess;
}

/**
   Initialize
*/
void MyWifi::init(void) {
  WiFi.disconnect();
  delay(100);
  WiFi.onEvent(WiFiEvent);
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(settings->mySettings.systemname);
  scanNetworks();
  _connected = false;
}


/**
   resets network settings and restarts ESP
*/
void MyWifi::doReset(void) {
  DEBUG_PRINTLN("MyWifi::doReset called");
  settings->mySettings.ssid[0]='\0';
  settings->mySettings.passwd[0]='\0';
  settings->saveToNVS();
  //wifiManager.resetSettings();
  ESP.restart();
}

/**
   connects to a network
*/
bool MyWifi::myBegin(char *ssid, char *passwd) {
  int ret;
  int timeout;
  //station_config sc;
  _ssid=ssid;
  _passwd=passwd;
  DEBUG_PRINTF("_connected=%d, _ssid=%s, _passwd=%s\n", _connected, _ssid.c_str(), _passwd.c_str());
  if(_connected)
    return true;

  _got_ip=false;

  if(_ssid.length() && _passwd.length()) {
    icor->renderAndDisplay(F("/ico/wifi.ico"),0,1);
#if defined(LILYGO_T_HMI)
    ir->renderAndDisplayPNG("/tft/wifi.png",0,1);
#endif
    timeout = WIFITIMEOUT;
  } else {
    Serial.println(F("Starting WPS"));
    icor->renderAndDisplay("/ico/wps.ico",0,1);
#if defined(LILYGO_T_HMI)
    ir->renderAndDisplayPNG("/tft/wps.png",0,1);
#endif
    timeout = WPSTIMEOUT;
    WiFi.mode(WIFI_STA);
    if(myStartWPS()) {
      DEBUG_PRINTF("myBegin: WPS successfull, ssid=%s, passwd=%s\n",WiFi.SSID().c_str(), WiFi.psk().c_str());
      _ssid = WiFi.SSID();
      _passwd = WiFi.psk();
      saveSSIDandPWD();
      ESP.restart();
    }
  }

  int16_t i=0;
  DEBUG_PRINTF("pass=%s\n",_passwd.c_str());
  if (_passwd.length() > 0) {
    int j=0;
    _wifi_stat = -1;
    if(WiFi.isConnected()) {
      WiFi.disconnect();
      i=0;

      while(i++ < 10 && _wifi_stat != SYSTEM_EVENT_STA_DISCONNECTED) {
        DEBUG_PRINTLN("Warten auf disconnect");
        delay(100);
      }
    }

    while(!_got_ip) {
      _wifi_stat = -1;
      DEBUG_PRINTLN("vor WiFi.begin");

      WiFi.begin(_ssid, _passwd);
      i=0;
      while(i++ < timeout && !_got_ip) {
        // Warten auf connect oder timeout
#ifdef myDEBUG
        DEBUG_PRINTF("_wifi_stat=%d, i=%d\n", _wifi_stat, i);
#endif
        delay(1000);
        if(digitalRead(WIFI_RESET)) {
          doReset();
        }
      }
    }
  }
  else {
    timeout = WIFITIMEOUT;
    icor->renderAndDisplay("/ico/accesspoint.ico",0,1);
#if defined(LILYGO_T_HMI)
    ir->renderAndDisplayPNG("/tft/accesspoint.png",0,1);
#endif
    ret = startConfigPortal(settings->mySettings.systemname);
  }

  if (!_got_ip) {
    DEBUG_PRINTLN("Failed to connect, remove WiFi Settings");
    DEBUG_FLUSH();
    //system_restore();
    ESP.restart();
    return false;
  } else {
    DEBUG_PRINTLN("Got connection");
    //WiFi.config(INADDR_NONE,INADDR_NONE,INADDR_NONE,INADDR_NONE);
    
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
    icor->renderAndDisplay("/ico/check.ico",3000,1);

#if defined(LILYGO_T_HMI)
    ir->renderAndDisplayPNG("/tft/check.png",0,1);
#endif

    _ssid=WiFi.SSID();
    _passwd=WiFi.psk();
    saveSSIDandPWD();

    _ip = WiFi.localIP();
    DEBUG_PRINTF("myBegin: ssid=%s, passwd=%s, ip=%s\n",_ssid.c_str(), _passwd.c_str(),_ip.toString().c_str());
    _connected = true;
    return true;
  }
}

bool MyWifi::isConnected(void) {
  return _connected;
}

void MyWifi::saveSSIDandPWD(void) {
   DEBUG_PRINTLN("saveSSIDandPWD called");
   if(_ssid != String(settings->mySettings.ssid) || _passwd != String(settings->mySettings.passwd)) {
      strncpy(settings->mySettings.ssid, _ssid.c_str(), sizeof(settings->mySettings.ssid));
      strncpy(settings->mySettings.passwd, _passwd.c_str(), sizeof(settings->mySettings.passwd));
      settings->saveToNVS();
   }
}

AsyncWebServer *MyWifi::getServer(void) {
  return &server;
}