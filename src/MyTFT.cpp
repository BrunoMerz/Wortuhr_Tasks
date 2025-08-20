/**
   MyTft.cpp
   @autor    Bruno Merz

   @version  1.0
   @created  29.11.2021

*/

#if defined(LILYGO_T_HMI)

//#define myDEBUG
#include "MyDebug.h"
#include "Configuration.h"
#include "MyTFT.h"
#include "Languages.h"
#include "MyFileAccess.h"
#include "Modes.h"

static MyFileAccess *mfa = MyFileAccess::getInstance();

MyTFT* MyTFT::instance = 0;

MyTFT *MyTFT::getInstance() {
  if (!instance)
  {
      instance = new MyTFT();
  }
  return instance;
}


void MyTFT::init(void) {
  pinMode(PWR_EN_PIN, OUTPUT);
  pinMode(TFT_BL,  OUTPUT);
  //ledcSetup(0, 5000, 8);
  //ledcAttachPin(TFT_BL, 0);
  digitalWrite(PWR_EN_PIN, HIGH);
  TFT_eSPI::init();
  
  setRotation(3);
  setSwapBytes(true);
  fillScreen(TFT_BLACK);
  ir = TFTImageRenderer::getInstance();
  ir->renderAndDisplayPNG("/tft/wordclock.png",0,1);
  drawStateLine("Start....");
}


void MyTFT::clearMainCanvas(void) {
  DEBUG_PRINTF("clearMainCanvas: %d, %d\n",_mainCanvasWidth, _mainCanvasHeight);
  fillRect(0, 0, _mainCanvasWidth, _mainCanvasHeight, TFT_BLACK);
}


void MyTFT::clearStateCanvas(uint16_t _width) {
  fillRect(0, _stateLineYPos, _width?_width:_mainCanvasWidth, fontHeight(STD_FONT), TFT_BLACK);
}


void MyTFT::drawStateLine(String text, int x_pos) {
  drawString(text,x_pos,_stateLineYPos,STD_FONT);
}

void MyTFT::printStateLine(String txt, int x_pos, uint16_t clear) {
  if (clear)
    clearStateCanvas(160);
 drawStateLine(txt, x_pos);
}




#endif