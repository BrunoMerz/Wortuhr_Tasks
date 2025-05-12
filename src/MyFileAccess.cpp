/**
   MyFileAccess.cpp
   @autor    Bruno Merz

   @version  2.0
   @created  30.12.2019
 
*/


#include "MyFileAccess.h"

//#define myDEBUG
#include "MyDebug.h"

JsonDocument jsonDoc;
MyFileAccess* MyFileAccess::instance = 0;

MyFileAccess *MyFileAccess::getInstance() {
  if (!instance)
  {
      instance = new MyFileAccess();
  }
  return instance;
}


/**
   Constructor, sets webpage to nothing
*/
MyFileAccess::MyFileAccess(void) {
  //LittleFS.begin();
}


/**
   removes a file from LittleFS
*/
void MyFileAccess::remove(String filename) {
  LittleFS.remove(filename);
}

/**
   read file size from LittleFS
*/
int MyFileAccess::fileSize(String filename) {
  int fs=-1;
  if (LittleFS.exists(filename)) {
    File fileObj = LittleFS.open(filename, "r");
    if (fileObj) {
      fs=fileObj.size();
      fileObj.close();
    }
  }
  return fs;
}


/**
   open LittleFS file
*/
File MyFileAccess::openFile(String filename, char *mode) {
  DEBUG_PRINTLN("openFile: " + filename);
  fsUploadFile = LittleFS.open(filename, mode);
  if (fsUploadFile)
    DEBUG_PRINTLN("openFile: ok");
  return fsUploadFile;
}


/**
   write LittleFS file
*/
void MyFileAccess::writeFile(const uint8_t *buf, size_t len) {
  //DEBUG_PRINTLN("writeFile: len"+len);
  if (fsUploadFile)
    fsUploadFile.write(buf, len);
}

/**
   write LittleFS file
*/
void MyFileAccess::writeFile(char *buf) {
  //DEBUG_PRINTLN("writeFile: len"+len);
  if (fsUploadFile)
    fsUploadFile.write((const uint8_t*)buf, strlen(buf));
}


/**
   close LittleFS file
*/
void MyFileAccess::closeFile(void) {
  DEBUG_PRINTLN("closeFile:");
  if (fsUploadFile) {
    fsUploadFile.flush();
    fsUploadFile.close();
    fsUploadFile = (File)NULL;
   }
}


/**
   read file from LittleFS
*/
size_t MyFileAccess::readBytes(uint8_t *buffer, int len) {
  DEBUG_PRINT("readBytes: len=");
  DEBUG_PRINTLN(len);
  size_t l=0;
  if (fsUploadFile) {
    l = fsUploadFile.readBytes((char *)buffer, len);
  }
  return l;
}


/**
   read file from LittleFS
*/
int MyFileAccess::readFile(String filename, char *buffer) {
  DEBUG_PRINTLN("readFile: start '"+filename+"'");
  if (LittleFS.exists(filename)) {
    //file exists, reading and loading
    DEBUG_PRINTLN("readFile: reading "+filename);
    File fileObj = LittleFS.open(filename, "r");
    if (fileObj) {
      DEBUG_PRINTLN("readFile: File opened!");
      _fileSize = fileObj.size();
      DEBUG_PRINT("#bytes=");
      DEBUG_PRINTLN(_fileSize);
      DEBUG_FLUSH();
      int x=fileObj.readBytes(buffer,_fileSize);
      DEBUG_PRINT("readFile: readBytes result=");
      DEBUG_PRINTLN(x);
      *(buffer+_fileSize)='\0';
      fileObj.close();
    }
  } else {
    DEBUG_PRINTLN("readFile: no file in FS");
    *buffer   = '\0';      // file not found
    _fileSize = 0;
  }
  return _fileSize;
}

int MyFileAccess::readFileS(String filename, String *buffer) {
  DEBUG_PRINTLN("readFileS: start '"+filename+"'");
  if (LittleFS.exists(filename)) {
    //file exists, reading and loading
    DEBUG_PRINTLN("readFileS: reading "+filename);
    File fileObj = LittleFS.open(filename, "r");
    if (fileObj) {
      DEBUG_PRINTLN("readFileS: File opened!");
      *buffer = fileObj.readString();
      fileObj.close();
    }
  } else {
    DEBUG_PRINTLN("readFileS: no file in FS");
    *buffer="";   // file not found
  }
  return buffer->length();
}


/**
   format LittleFS
*/
void MyFileAccess::format(void) {
  bool formatted = LittleFS.format();
  if (formatted) {
    DEBUG_PRINTLN("\n\nSuccess formatting");
  } else {
    DEBUG_PRINTLN("\n\nError formatting");
  }
}


/**
   file exists?
*/
bool MyFileAccess::exists(String filename) {
  return LittleFS.exists(filename);
}


size_t MyFileAccess::getFreeSpace(void) {
#if defined(ESP8266)
  FSInfo fs_info;
  LittleFS.info(fs_info);
  return fs_info.totalBytes-fs_info.usedBytes;
#else
  return LittleFS.totalBytes()-LittleFS.usedBytes();
#endif
}

size_t MyFileAccess::getTotalSpace(void) {
#if defined(ESP8266)
  FSInfo fs_info;
  LittleFS.info(fs_info);
  return fs_info.totalBytes;
#else
  return LittleFS.totalBytes();
#endif
}

bool MyFileAccess::readJson(String _filename, JsonDocument *_jsonDoc) {
  //read Json File
  if (LittleFS.exists(_filename)) {
    //file exists, reading and loading
    File jsonFile = LittleFS.open(_filename, "r");
    if (jsonFile) {
      DeserializationError error = deserializeJson(*_jsonDoc, jsonFile);
      jsonFile.close();
      if (!error) {
        return true;
      } else {
        DEBUG_PRINT("Failed to load json config: ");
        DEBUG_PRINTLN(error.c_str());
      }
    } else {
      DEBUG_PRINTLN("Failed to open " + _filename);
    }
  } else {
    DEBUG_PRINTLN("Coudnt find " + _filename);
  }

  return false;
}


bool MyFileAccess::readSettings(void) {
  //read settings from File
  return readJson(JSON_CONFIG, &jsonDoc);
}

String MyFileAccess::getSetting(String settingName) {
  JsonVariant jv = jsonDoc[settingName];
  if (jv.isNull())
    _settingValue = "";
  else
    _settingValue = jv.as<String>();
    
  DEBUG_PRINTLN("getSetting spiffs: "+settingName+"='"+_settingValue+"'");
  return _settingValue;
}