/**
   MyWifi.cpp
   @autor    Bruno Merz
*/

#include "esp_wps.h"
#include <WiFi.h>

#include "ESPAsyncWebServer.h"
#include <ESPAsyncDNSServer.h>
#include <ElegantOTA.h>


#include "MyWifi.h"
#include "Configuration.h"
//#include "esp_task_wdt.h"

//#define myDEBUG
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



static IconRenderer *icor = IconRenderer::getInstance();
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
    delay(1000);
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
    case ARDUINO_EVENT_WIFI_STA_STOP:
      DEBUG_PRINTLN("STA Stop: " + String(WiFi.SSID()));
      break;
    case ARDUINO_EVENT_WPS_ER_SUCCESS:
      DEBUG_PRINTLN("WPS Successfull, stopping WPS and connecting to: " + String(WiFi.SSID()));
      wpsStop();
      vTaskDelay(pdMS_TO_TICKS(10));
      WiFi.begin();
      break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      DEBUG_PRINTLN("STA_CONNECTED");
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
    case ARDUINO_EVENT_WIFI_AP_STOP:
      DEBUG_PRINTLN("AP_STOP");
      break;
	  case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
      DEBUG_PRINTLN("AP_STACONNECTED");
      break;
	  case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
      DEBUG_PRINTLN("AP_STADISCONNECTED");
      break;
	  case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
      DEBUG_PRINTLN("AP_STAIPASSIGNED");
      break;
	  case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:
      DEBUG_PRINTLN("AP_PROBEREQRECVED");
      break;
    default:
      DEBUG_PRINTF("Nicht behandelter Event: %d\n", event);
      break;
  }
}


void handleRoot(AsyncWebServerRequest *request) {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Wi-Fi Konfiguration</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      background-color: #f3f3f3;
      text-align: center;
      padding: 20px;
    }
    .container {
      background-color: white;
      border-radius: 8px;
      padding: 20px;
      box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
      width: 300px;
      margin: 0 auto;
    }
    h1 {
      color: #4CAF50;
    }
    input[type="text"], input[type="password"] {
      width: 100%;
      padding: 10px;
      margin: 10px 0;
      border: 1px solid #ccc;
      border-radius: 4px;
    }
    button {
      background-color: #4CAF50;
      color: white;
      border: none;
      padding: 12px 20px;
      border-radius: 4px;
      cursor: pointer;
      width: 100%;
    }
    button:hover {
      background-color: #45a049;
    }
    ul {
      list-style-type: none;
      padding: 0;
    }
    li {
      padding: 1px;
      background-color: #f1f1f1;
      margin: 5px 0;
      cursor: pointer;
      border-radius: 4px;
    }
    li:hover {
      background-color: #ddd;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>Wi-Fi Konfiguration</h1>
    <form action="/save" method="POST">
      <h3>%LANG_AP_SEL%</h3>
      <ul id="item-list">
        %OPTIONS%
      </ul>
      <label for="ssid">%LANG_AP_SSID%</label>
      <input type="text" id="ssid" name="ssid" required placeholder="%LANG_AP_SSID_INP%">
      <label for="password">%LANG_AP_PWD%</label>
      <input type="password" id="pass" name="pass" required placeholder="%LANG_AP_PWD_INP%">
      <button type="submit">%LANG_AP_CONN%</button>
    </form>
  </div>
  <script>
    function setInputValue(value) {
      document.getElementById('ssid').value = value;
    }
  </script>
</body>
</html>
  )rawliteral";

  html.replace("%OPTIONS%", MyWifi::_scanResultHTML);
  html.replace("%LANG_AP_SEL%", LANG_AP_SEL);
  html.replace("%LANG_AP_SSID%", LANG_AP_SSID);
  html.replace("%LANG_AP_SSID_INP%", LANG_AP_SSID_INP);
  html.replace("%LANG_AP_PWD%", LANG_AP_PWD);
  html.replace("%LANG_AP_PWD_INP%", LANG_AP_PWD_INP);
  html.replace("%LANG_AP_CONN%", LANG_AP_CONN);
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


bool MyWifi::startConfigPortal(char *ssid) {
  DEBUG_PRINTLN("Starting Config Portal");
  IPAddress apIP(192, 168, 4, 1);
  const byte DNS_PORT = 53;

  DEBUG_PRINTLN("Suche nach WLAN-Netzen...");
  bool ret;
  int n = WiFi.scanNetworks();
  _scanResultHTML = "";

  if (n == 0) {
    _scanResultHTML = "<li>Keine Netzwerke gefunden</li>";
    ret = false;
  } else {
    for (int i = 0; i < n; ++i) {
      String ssid = WiFi.SSID(i);
      ssid.replace("\"", ""); // doppelte Anführungszeichen vermeiden
      if(_scanResultHTML.indexOf(ssid) < 0)
        _scanResultHTML += "<li onclick=\"setInputValue('"+ ssid + "')\">" + ssid + "</li>";
    }
    ret = true;
  }
  DEBUG_PRINTF("Suche nach WLAN-Netzen beendet: ret=%d, _scanResultHTML=%s\n", ret, _scanResultHTML.c_str());


  WiFi.disconnect(true);
  delay(100);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssid);
  delay(100);
 
  DEBUG_PRINTLN("adding handler");
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleRoot(request);
  });
  

  server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request) {
    handleSave(request);
  });

  server.onNotFound([](AsyncWebServerRequest *request) { 
    AsyncWebServerResponse *response = request->beginResponse(302, "text/plain");
    response->addHeader("Location", "/");
    request->send(response);
  });

  DEBUG_PRINTLN("Starting DNS Portal");
  dns.start(DNS_PORT, "*", apIP);

  delay(100);

  DEBUG_PRINTLN("Starting server");
  server.begin();
  
  delay(100);

  while(!_got_ip) {
    delay(10);
  }
  DEBUG_PRINTLN("server.end()");
  server.end();
  return true;
}


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
  WiFi.disconnect(true);
  WiFi.onEvent(WiFiEvent);
  WiFi.mode(WIFI_STA);
  DEBUG_PRINTF("systemname=%s\n", settings->mySettings.systemname);
  WiFi.setHostname(settings->mySettings.systemname);
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
      WiFi.disconnect(true, true);  // trennt, vergisst SSID & Hostname
      delay(100);
      // Hostname setzen **vor** WiFi.begin()
      WiFi.setHostname(settings->mySettings.systemname);
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
    _ssid = WiFi.SSID();
    _passwd = WiFi.psk();
    saveSSIDandPWD();
    ESP.restart();
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