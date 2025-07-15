/**
   MyClock.cpp
   @autor    Bruno Merz

   @version  1.0
   @created  17.10.2023

*/

#if defined(LILYGO_T_HMI)

//#define myDEBUG
#include "MyDebug.h"
#include "Configuration.h"
#include "MyWidgets.h"
#include "Settings.h"



MyWidgets* MyWidgets::instance = 0;

MyWidgets *MyWidgets::getInstance() {
  if (!instance)
  {
      instance = new MyWidgets();
  }
  return instance;
}


MyWidgets::MyWidgets(void) {

}


void MyWidgets::init(void) {
  DEBUG_PRINTLN("MyWidgets::init");
  tft = MyTFT::getInstance();

  W=tft->getMainCanvasWidth();
  H=tft->getMainCanvasHeight();
  D=min(W, H);
  
  R=D/2;
  RG=R-2;    // 118
  RT=R-10;   // 110
  RD=R-18;   // 102 60 Punkte, 4 ViertelStunden Punkte
  RL=R-6;    // 114 Zwölf Striche
  RSTD=R-20;   // 100
  RH=R-58;   // 62 Stundenzeiger
  RM=R-36;   // 84 Minutenzeiger
  RS=R-30;   // 90 Sekundenzeiger

  W>D?OX=(W-D)/2:OX=0;
  H>D?OY=(H-D)/2:OY=0;
  DEBUG_PRINTLN("MyWidgets::init done");
}


void MyWidgets::drawClock(void) {
  DEBUG_PRINTLN("drawClock start");
 
   // Clear
  tft->clearMainCanvas();

  // Draw clock face
  tft->fillCircle(R+OX, R+OY, RG, TFT_GREEN);
  tft->fillCircle(R+OX, R+OY, RT, TFT_BLACK);

  // Draw 12 lines
  for(int i = 0; i<360; i+= 30) {
    sx = cos((i-90)*0.0174532925);
    sy = sin((i-90)*0.0174532925);
    x0 = sx*RL+R;
    yy0 = sy*RL+R;
    x1 = sx*RSTD+R;
    yy1 = sy*RSTD+R;

    tft->drawLine(x0+OX, yy0+OY, x1+OX, yy1+OY, TFT_GREEN);
  }

  // Draw 60 dots
  for(int i = 0; i<360; i+= 6) {
    sx = cos((i-90)*0.0174532925);
    sy = sin((i-90)*0.0174532925);
    x0 = sx*RD+R;
    yy0 = sy*RD+R;
    // Draw minute markers
    tft->drawPixel(x0+OX, yy0+OY, TFT_WHITE);
    
    // Draw main quadrant dots
    if(i==0 || i==180) tft->fillCircle(x0+OX, yy0+OY, 2, TFT_WHITE);
    if(i==90 || i==270) tft->fillCircle(x0+OX, yy0+OY, 2, TFT_WHITE);
  }

  tft->fillCircle(R+OX, R+OY+1, 3, TFT_WHITE);

  doDrawClock=false;
  DEBUG_PRINTLN("drawClock done");
}

void MyWidgets::drawTime(uint8_t ss, uint8_t mm, uint8_t hh) {
  DEBUG_PRINTF("drawTime start: ss=%d, mm=%d, hh=%d\n",ss,mm,hh);
  if(doDrawClock)
    drawClock();
  // Pre-compute hand degrees, x & y coords for a fast screen update
  sdeg = ss*6;                  // 0-59 -> 0-354
  mdeg = mm*6+sdeg*0.01666667;  // 0-59 -> 0-360 - includes seconds
  hdeg = hh*30+mdeg*0.0833333;  // 0-11 -> 0-360 - includes minutes and seconds
  hx = cos((hdeg-90)*0.0174532925);    
  hy = sin((hdeg-90)*0.0174532925);
  mx = cos((mdeg-90)*0.0174532925);    
  my = sin((mdeg-90)*0.0174532925);
  sx = cos((sdeg-90)*0.0174532925);    
  sy = sin((sdeg-90)*0.0174532925);

  if (mm != lastMM) {
    lastMM = mm;
    // Erase hour and minute hand positions every minute
    tft->drawLine(ohx+OX, ohy+OY, R+OX, R+OY+1, TFT_BLACK);
    ohx = hx*RH+R+1;    
    ohy = hy*RH+R+1;
    tft->drawLine(omx+OX, omy+OY, R+OX, R+OY+1, TFT_BLACK);
    omx = mx*RM+R;    
    omy = my*RM+R+1;
  }

  // Redraw new hand positions, hour and minute hands not erased here to avoid flicker
  tft->drawLine(osx+OX, osy+OY, R+OX, R+OY+1, TFT_BLACK);
  osx = sx*RS+R+1;    
  osy = sy*RS+R+1;
  tft->drawLine(osx+OX, osy+OY, R+OX, R+OY+1, TFT_RED);
  tft->drawLine(ohx+OX, ohy+OY, R+OX, R+OY+1, TFT_WHITE);
  tft->drawLine(omx+OX, omy+OY, R+OX, R+OY+1, TFT_WHITE);
  tft->drawLine(osx+OX, osy+OY, R+OX, R+OY+1, TFT_RED);

  tft->fillCircle(R+OX, R+OY+1, 3, TFT_RED);
  DEBUG_PRINTLN("drawTime done");
}


void MyWidgets::drawInfo(void) {
  Settings *settings = Settings::getInstance();
  uint16_t x=0, y=0, y1, x1=tft->getMainCanvasWidth()/2;
  tft->clearMainCanvas();

  // Überschrift
  tft->setFreeFont(&FreeMono12pt7b);
  
  uint16_t fh = tft->fontHeight();
  
  x=tft->drawString(settings->mySettings.systemname,0,y);
  tft->drawString(" Info",x,y);
  y += fh;
  tft->drawLine(0,y,tft->getMainCanvasWidth()-5,y,TFT_WHITE);
  y++;
  y1=y;

  // Font einstellen
  tft->setFreeFont(&FreeMono9pt7b);
  tft->setTextFont(STD_FONT);
  fh = tft->fontHeight();

  // Linke Spalte
  x=tft->drawString("WiFi/Netzwerk",0,y);
  
  // Zweite Spalte
  y=y1;
  x=tft->drawString("OpenWeather",x1,y);
}


void MyWidgets::setDrawState(boolean state) {
  doDrawClock = state;
}
#endif