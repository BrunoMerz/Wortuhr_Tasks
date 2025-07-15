/**
   Global.cpp
   Global variables

   @autor    B. Merz

*/


#include "Global.h"

//#define myDEBUG
#include "MyDebug.h"

Global* Global::instance = 0;

Global *Global::getInstance() {
  if (!instance)
  {
      instance = new Global();
  }
  return instance;
}

Global::Global() {

}

void Global::setHighWaterMark(uint8_t t) {
  UBaseType_t hwm = uxTaskGetStackHighWaterMark(NULL);
  if(!highWaterMark[t] || hwm < highWaterMark[t])
    highWaterMark[t] = hwm;
  //Serial.printf("setHighWaterMark: t=%d, hwm=%d\n",t,hwm);
}