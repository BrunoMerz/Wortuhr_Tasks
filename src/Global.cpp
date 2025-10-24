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
  for(uint16_t i=0; i<TASK_MAX+1;i++)
    highWaterMark[i]=0;
}

void Global::setHighWaterMark(uint8_t t) {
  UBaseType_t hwm = uxTaskGetStackHighWaterMark(NULL);
  if(!highWaterMark[t] || hwm < highWaterMark[t])
    highWaterMark[t] = hwm;
  //Serial.printf("setHighWaterMark: t=%d, hwm=%d\n",t,hwm);
}