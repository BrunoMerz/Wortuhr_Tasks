#include <Arduino.h>

//#define myDEBUG
#include "MyDebug.h"

#include "html_content.h"

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
  extern ESP8266WebServer webServer;
#else
  #include <WiFi.h>
  #include "WebServer.h"
  extern WebServer webServer;
#endif

#include <list>
#include <LittleFS.h>
#include "Configuration.h"
#include "Languages.h"

#if defined(WITH_ALEXA)
#include "MyAlexa.h"
static MyAlexa *alexa = MyAlexa::getInstance();
#endif

void initFS();

//void handleContent(const uint8_t * image, size_t size, const char * mime_type);

void handleUpload();

void formatFS();

void initFS() {
  if ( !LittleFS.begin() ) 
  {
    DEBUG_PRINTLN(F("LittleFS Mount fehlgeschlagen"));
    if ( !LittleFS.format() )
    {
      DEBUG_PRINTLN(F("Formatierung nicht möglich"));
    }
    else
    {
      DEBUG_PRINTLN(F("Formatierung erfolgreich"));
      if ( !LittleFS.begin() )
      {
        DEBUG_PRINTLN(F("LittleFS Mount trotzdem fehlgeschlagen"));
      }
      else 
      {
        DEBUG_PRINTLN(F("LittleFS Dateisystems erfolgreich gemounted!")); 
      }
    }
  }  
  else
  {
    DEBUG_PRINTLN(F("LittleFS erfolgreich gemounted!"));
  }
  
  if ( !LittleFS.exists("/web"))
  {
    if ( LittleFS.mkdir("/web") )
    {
      DEBUG_PRINTLN(F("Verzeichnis /web erstellt"));
    }
    else
    {
      DEBUG_PRINTLN(F("Verzeichnis /web konnte nicht erstellt werden"));
    }
  } 
#if defined(LILYGO_T_HMI)
  if ( !LittleFS.exists("/tft"))
  {
    if ( LittleFS.mkdir("/tft") )
    {
      DEBUG_PRINTLN(F("Verzeichnis /tft erstellt"));
    }
    else
    {
      DEBUG_PRINTLN(F("Verzeichnis /tft konnte nicht erstellt werden"));
    }
  } 
#endif
}


void handleUpload() {      
                                                   // Dateien ins Filesystem schreiben
  static File fsUploadFile;
  HTTPUpload& upload = webServer.upload();
 
  if (upload.status == UPLOAD_FILE_START) {
    if (upload.filename.length() > 31) {  // Dateinamen kürzen
      upload.filename = upload.filename.substring(upload.filename.length() - 31, upload.filename.length());
    }
    fsUploadFile = LittleFS.open(webServer.arg(0) + "/" + webServer.urlDecode(upload.filename), "w");
  } else if (upload.status == UPLOAD_FILE_WRITE) {
     fsUploadFile.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
     fsUploadFile.close();
  }

}

void formatFS() {                                                                      // Formatiert das Filesystem
  LittleFS.format();
  initFS();
//  sendResponse();
}


/*
void handleContent(const uint8_t * image, size_t size, const char * mime_type) {
  uint8_t buffer[512];
  size_t buffer_size = sizeof(buffer);
  size_t sent_size = 0;

  webServer.setContentLength(size);
  webServer.send(200, mime_type, "");
//  WiFiClient client = webServer.client();

  while (sent_size < size) {
    size_t chunk_size = min(buffer_size, size - sent_size);
    memcpy_P(buffer, image + sent_size, chunk_size);
    webServer.client().write(buffer, chunk_size);
    sent_size += chunk_size;
    delay(0);
#ifdef DEBUG_WEB
    DEBUG_PRINTF("sendContent: %i byte : %i byte of %i byte\n", chunk_size, sent_size,size );
#endif
  }
  webServer.sendContent("");
  delay(0);
}
*/