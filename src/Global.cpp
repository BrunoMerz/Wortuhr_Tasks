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
