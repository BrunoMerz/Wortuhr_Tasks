/**
   MyTft.h
   @autor    Bruno Merz

   @version  1.0
   @created  29.11.2021

*/

#pragma once

#ifdef LILYGO_T_HMI

#include "MyTFT.h"
#include "Modes.h"

typedef void (*actionCallback)(BTNType btnNr);

#define NUMBERBUTTONS 10


class MyButton {
  public:
    MyButton(String fn, BTNType btnType, uint16_t y, actionCallback action, MyTFT *tft);
    actionCallback _action;
    uint16_t _x1, _x2, _y1, _y2;
    BTNType _btnType;
};


class MyButtons {
  public:
    static MyButtons* getInstance();
    void init(void);
    uint8_t newButton(String fn, BTNType btnType, uint16_t y, actionCallback action);
    void deleteButton(BTNType btnType);
    void callAction(uint16_t x, uint16_t y);
    BTNType aktBtnType;

  private:
    static MyButtons *instance;
    MyTFT *tft;
    MyButtons(void);
    MyButton *btn[NUMBERBUTTONS];
    uint8_t buttonCount;
};

#endif