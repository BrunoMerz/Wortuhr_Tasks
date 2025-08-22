/**
   MyButton.cpp
   @autor    Bruno Merz

   @version  1.0
   @created  29.09.2023

*/

#if defined(LILYGO_T_HMI)

//#define myDEBUG
#include "MyDebug.h"
#include "Configuration.h"
#include "MyButtons.h"
#include "MyTFT.h"
#include "Free_Fonts.h" // Include the header file attached to this sketch

#define BUTTON_W 100
#define BUTTON_H 50


MyButtons* MyButtons::instance = 0;


MyButtons::MyButtons() {
  buttonCount=0;
  aktBtnType = BTN_TIME;
}

MyButtons *MyButtons::getInstance() {
  if (!instance)
  {
      instance = new MyButtons();
  }
  return instance;
}

void btn1_pressAction(void) {
  DEBUG_PRINTLN("btn1_pressAction");
}

void btn1_releaseAction(void) {
  DEBUG_PRINTLN("btn1_releaseAction");
}


void MyButtons::init(void) {
  DEBUG_PRINTLN("MyButtons::init");
  tft = MyTFT::getInstance();
  for(uint8_t btnNr=0; btnNr<NUMBERBUTTONS; btnNr++) {
    btn[btnNr] = 0;
  }
  DEBUG_PRINTLN("MyButtons::init done");
}


uint8_t MyButtons::newButton(String fn, BTNType btnType, uint16_t y, actionCallback action) {
  DEBUG_PRINTLN("MyButtons::newButton");
  btn[buttonCount] = new MyButton(fn, btnType, y, action, tft);
  return buttonCount++;
}

void MyButtons::deleteButton(BTNType btnType) {
  DEBUG_PRINTLN("MyButtons::deleteButton");
  delete btn[buttonCount];
  buttonCount--;
}

void MyButtons::callAction(uint16_t x, uint16_t y) {
  for(uint8_t btnNr=0; btnNr<NUMBERBUTTONS; btnNr++) {
    MyButton *b = btn[btnNr];
    if(b && x >= b->_x1 && x <= b->_x2 && y >= b->_y1 && y <= b->_y2) {
      b->_action(b->_btnType);
    }
  }
}


MyButton::MyButton(String fn, BTNType btnType, uint16_t y, actionCallback action, MyTFT *tft) {
  String png("/tft/");
  png += fn;
  png += ".png";

  _x1     = 270;
  _x2     = _x1+50;
  _y1     = y;
  _y2     = _y1+50;
  _btnType  = btnType;
  _action = action;

  tft->ir->renderAndDisplayPNG((char *)png.c_str(),0,0,_x1,_y1);
}
#endif