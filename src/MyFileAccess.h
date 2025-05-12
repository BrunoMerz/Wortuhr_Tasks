/**
   MyFileAccess.h
   @autor    Bruno Merz

   @version  2.0
   @created  30.12.2019
 
*/

#pragma once

#include <Arduino.h>

#define USE_LittleFS
#include <FS.h>
#include <LittleFS.h>
#include <SD.h>
#include <ArduinoJson.h>
#include "Configuration.h"

/**
 * Class definition
 */
class MyFileAccess {
public:
    static MyFileAccess* getInstance();
 
    File    openFile(String filename, char *mode);
    int     readFile(String filename, char *buffer);
    int     readFileS(String filename, String *buffer);
    size_t  readBytes(uint8_t *buffer, int len);
    void    writeFile(const uint8_t *buf, size_t len);
    void    writeFile(char *buf);
    void    closeFile(void);
    void    remove(String filename);
    void    format(void);
    bool    exists(String filename);
    int     fileSize(String filename);
    size_t  getFreeSpace(void);
    size_t  getTotalSpace(void);
    bool    readSettings(void);
    bool    readJson(String _filename, JsonDocument *_jsonDoc);
    String  getSetting(String settingName);
 
private:
    MyFileAccess(void);
    static MyFileAccess *instance;
    int     _fileSize;
    File    fsUploadFile;
    String  _settingValue;
};
