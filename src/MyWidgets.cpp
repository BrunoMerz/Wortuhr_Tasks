/**
   MyWidgets.cpp
   @autor    Bruno Merz

   @version  1.0
   @created  17.10.2023

*/

#if defined(LILYGO_T_HMI)

#define myDEBUG
#include "MyDebug.h"
#include "Configuration.h"
#include "MyWidgets.h"
#include "Settings.h"
#include "TFTImageRenderer.h"
#include "TFT_eWidget.h"
#include "Free_Fonts.h"


MyWidgets* MyWidgets::instance = 0;

MyWidgets *MyWidgets::getInstance() {
  if (!instance)
  {
      instance = new MyWidgets();
  }
  return instance;
}


MyWidgets::MyWidgets(void) {
  currentMode = (Mode)0;
}


void MyWidgets::init(void) {
  DEBUG_PRINTLN("MyWidgets::init");
  tft = MyTFT::getInstance();
  mt = MyTime::getInstance();
  ow = OpenWeather::getInstance();
  mfa = MyFileAccess::getInstance();

  W=tft->getMainCanvasWidth();
  H=tft->getMainCanvasHeight();
  DEBUG_PRINTF("W=%d, H=%d\n",W,H);


  
  DEBUG_PRINTLN("MyWidgets::init done");
}


void MyWidgets::drawWidget(Mode mode) {
  DEBUG_PRINTF("drawWidget mode=%d\n",mode);
  switch(mode) {
    case MODE_TIME:
      drawTime();
      break;
    case MODE_DATE:
    case MODE_WEEKDAY:
      drawDate();
      break;
    case MODE_MOONPHASE:
    case MODE_WETTER:
      drawWeather();
      break;
    case MODE_EXT_HUMIDITY:
    case MODE_EXT_TEMP:
      drawExtTempHumidity();
      break;
  }
  currentMode = mode;
  DEBUG_PRINTLN("drawMode end");
}

// Funktion zum Berechnen eines Zeigers
Line MyWidgets::computeHand(float centerX, float centerY, float length, float angleDegrees) {
    float backOffset = 20;
    float angle = -angleDegrees * DEG_TO_RAD + PI / 2;
    Line hand;

    hand.start.x = centerX - backOffset * cos(angle);
    hand.start.y = centerY + backOffset * sin(angle); // y-Achse nach unten
    hand.end.x = centerX + length * cos(angle);
    hand.end.y = centerY - length * sin(angle);
    return hand;
}

void MyWidgets::computeClockHands(uint8_t hour, uint8_t minute, uint8_t second, float ccx, float ccy, float R,
                       Line &hourHand, Line &minuteHand, Line &secondHand) {
    // Sekundenzeiger
    float secondAngle = second * 6; // 360° / 60
    secondHand = computeHand(ccx, ccy, R, secondAngle);

    // Minutenzeiger (berücksichtigt auch Sekunden für Zwischenposition)
    float minuteAngle = (minute + second / 60.0) * 6;
    minuteHand = computeHand(ccx, ccy, R * 0.77, minuteAngle);

    // Stundenzeiger (berücksichtigt Minuten)
    float hourAngle = ((hour % 12) + minute / 60.0) * 30; // 360° / 12
    hourHand = computeHand(ccx, ccy, R * 0.60, hourAngle);
}


void MyWidgets::drawWeather(void) {
  char tstr[34];
  DEBUG_PRINTLN("drawWeather start");
  tft->clearMainCanvas();
  tft->setFreeFont(FS9);
  // sunrise
  tft->ir->renderAndDisplayPNG((char *)("/tft/sunrise.png"), 0, 0, PIC1_X, PIC1_Y);
  
  strftime(tstr,sizeof(tstr),"%H:%M", localtime(&ow->sunrise));
  tft->drawString(tstr,TXT1_X,TXT1_Y,GFXFF);
  // sunset
  tft->ir->renderAndDisplayPNG((char *)("/tft/sunset.png"), 0, 0, PIC2_X, PIC2_Y);
  strftime(tstr,sizeof(tstr),"%H:%M", localtime(&ow->sunset));
  tft->drawString(tstr,TXT2_X,TXT2_Y,GFXFF);

  // act weather
  sprintf(tstr,"/tft/web_%s.png", ow->weathericon1.c_str());
  if(mfa->exists(tstr))
    tft->ir->renderAndDisplayPNG(tstr, 0, 0, PIC4_X, PIC4_Y);
  else
    tft->drawString(tstr,PIC4_X,PIC4_Y,2);
  tft->drawString(ow->description,TXT4_X,TXT4_Y,GFXFF);
  // temperature
  tft->drawString("Temp: "+String(ow->temperature,1),TXT5_X,TXT5_Y,GFXFF);
  // moonphase
  //  0         Neumond
  //  > 0 < 11  zunehmend
  //  11        Vollmond
  //  > 11 < 22 abnehmend
  sprintf(tstr,"/tft/web_moon%d.png", ow->moonphase);
  if(mfa->exists(tstr))
    tft->ir->renderAndDisplayPNG(tstr, 0, 0, PIC3_X, PIC3_Y);
  else
    tft->drawString(tstr,PIC3_X,PIC3_Y,2);
  if(!ow->moonphase)
    tft->drawString(LANG_NEWMOON,TXT3_X,TXT3_Y,GFXFF);
  else if(ow->moonphase > 0 && ow->moonphase < 11)
    tft->drawString(LANG_INCREASINGMOON,TXT3_X,TXT3_Y,GFXFF);
  else if(ow->moonphase == 11)
    tft->drawString(LANG_FULLMOON,TXT3_X,TXT3_Y,GFXFF);
  else if(ow->moonphase > 11 && ow->moonphase < 22)
    tft->drawString(LANG_WANINGMOON,TXT3_X,TXT3_Y,GFXFF);

  // draw lines
  tft->drawLine(PIC2_X,PIC1_Y,PIC2_X,PIC4_Y,TFT_WHITE);
  tft->drawLine(PIC3_X,PIC1_Y,PIC3_X,PIC4_Y,TFT_WHITE);

  // reset Font
  tft->setFreeFont(NULL);
  DEBUG_PRINTLN("drawWeather end");
}

void MyWidgets::drawDate(void) {
  DEBUG_PRINTLN("drawDate start");
  const char* days[] = {
    LANG_SUNDAY, LANG_MONDAY, LANG_TUESDAY, LANG_WEDNESDAY,
    LANG_THURSDAY, LANG_FRIDAY, LANG_SATURDAY
  };

  // Clear
  tft->clearMainCanvas();

  // Font setting
  tft->setFreeFont(FF36);
  int16_t fh = tft->fontHeight(GFXFF)+2;

  // new timestamp
  mt->getTime();

  // Day
  tft->setTextColor(TFT_GREEN);
  int16_t ypos = (H-(fh*3))/2;
  tft->drawCentreString(days[mt->mytm.tm_wday-1], W/2, ypos, GFXFF);

  // Date
  tft->setTextColor(TFT_SKYBLUE);
  char txt[20];
  sprintf(txt,"%.2d.%.2d.%4d", mt->mytm.tm_mday, mt->mytm.tm_mon, mt->mytm.tm_year);
  ypos += fh;
  tft->drawCentreString(txt, W/2, ypos, GFXFF);

  // Time
  tft->setTextColor(TFT_GOLD);
  sprintf(txt,"%.2d:%.2d", mt->mytm.tm_hour, mt->mytm.tm_min);
  ypos += fh;
  tft->drawCentreString(txt, W/2, ypos, GFXFF);

  // reset Font
  tft->setFreeFont(NULL);
  DEBUG_PRINTLN("drawDate end");
}


void MyWidgets::drawExtTempHumidity(void) {
  DEBUG_PRINTLN("drawExtTempHumidity start");
  MyMeter   temp  = MyMeter(tft);
  MyMeter   hum = MyMeter(tft);
  // Clear
  tft->clearMainCanvas();
  temp.setZones(0, 25, 25, 50, 50, 75, 75, 100); // Example here red starts at 75% and ends at 100% of full scale
  // Meter is 239 pixels wide and 126 pixels high
  temp.analogMeter(0, 0, W-30, H/2, -12.0, 36.0, "Temperatur", "Grad", 4, "-12", "0", "12", "24", "36");    // Draw analogue meter at 0, 0
  temp.updateNeedle(ow->temperature, 0);
  // Colour draw order is red, orange, yellow, green. So red can be full scale with green drawn
  // last on top to indicate a "safe" zone.
  //             -1-   -2-  -3-  -4-
  hum.setZones(0, 25, 25, 50, 50, 75, 75, 100);
  hum.analogMeter(0, H/2, W-30, H/2, 0.0, 100.0, "Luftfeuchtigkeit", "in %", 5, "0", "25", "50", "75", "100"); // Draw analogue meter at 0, 128
  hum.updateNeedle(ow->humidity, 0);
}


void MyWidgets::drawClockFace(void) {
  tft->clearMainCanvas();
  tft->ir->renderAndDisplayPNG((char *)("/tft/Uhr_klein.png"), 0, 0, PIC1_X, PIC1_Y);
}

#define DREIECK 3.0
void MyWidgets::drawClockHands(uint8_t ss, uint8_t mm, uint8_t hh) {
  if(currentMode == MODE_TIME) {

    DEBUG_PRINTLN("drawClockHands start");
    tft->ir->renderAndDisplayPNG((char *)("/tft/Uhr_klein.png"), 0, 0, PIC1_X, PIC1_Y);
    uint32_t ccx=112;
    uint32_t ccy=112;
    uint32_t radius=65;
    Line h, m, s;
  
    computeClockHands(hh, mm, ss, ccx, ccy, radius, h, m, s);
    
    // Second
    float winkelInBogenmass = ss * 6 * (PI / 180);
    float ankathete = DREIECK * cos(winkelInBogenmass);
    float gegenkathete = DREIECK * sin(winkelInBogenmass);
    tft->fillTriangle(s.start.x+ankathete, s.start.y+gegenkathete, s.end.x, s.end.y, s.start.x-ankathete, s.start.y-gegenkathete, TFT_GOLD);

    // Minute
    winkelInBogenmass = (mm + ss / 60.0) * 6 * (PI / 180);
    ankathete = DREIECK * cos(winkelInBogenmass);
    gegenkathete = DREIECK * sin(winkelInBogenmass);
    tft->fillTriangle(m.start.x+ankathete, m.start.y+gegenkathete, m.end.x, m.end.y, m.start.x-ankathete, m.start.y-gegenkathete, TFT_VIOLET);

    // Hour
    winkelInBogenmass = ((hh % 12) + mm / 60.0) * 30 * (PI / 180);
    ankathete = DREIECK * cos(winkelInBogenmass);
    gegenkathete = DREIECK * sin(winkelInBogenmass);
    tft->fillTriangle(h.start.x+ankathete, h.start.y+gegenkathete, h.end.x, h.end.y, h.start.x-ankathete, h.start.y-gegenkathete, TFT_VIOLET);

    tft->drawCircle(ccx, ccy, 7, TFT_WHITE);
    tft->fillCircle(ccx, ccy, 6, TFT_DARKGREY);
    tft->fillCircle(ccx, ccy, 4, TFT_GREY);
    DEBUG_PRINTLN("drawClockHands end");
  }
}

void MyWidgets::drawTime(void) {
  DEBUG_PRINTF("drawTime start");
  if(currentMode != MODE_TIME)
    drawClockFace();

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
  tft->setFreeFont(NULL);
}


void MyWidgets::drawDiagram(void) {
  GraphWidget gr = GraphWidget(tft); // Diagramm-Widget
  TraceWidget tr = TraceWidget(&gr);     // Graph trace tr with pointer to gr
  const float gxLow  = 0.0;
  const float gxHigh = 100.0;
  const float gyLow  = -512.0;
  const float gyHigh = 512.0;
  // Graph area is 200 pixels wide, 150 pixels high, dark grey background
  gr.createGraph(200, 150, tft->color565(5, 5, 5));

  // x scale units is from 0 to 100, y scale units is -512 to 512
  gr.setGraphScale(gxLow, gxHigh, gyLow, gyHigh);

  // X grid starts at 0 with lines every 20 x-scale units
  // Y grid starts at -512 with lines every 64 y-scale units
  // blue grid
  gr.setGraphGrid(gxLow, 20.0, gyLow, 64.0, TFT_BLUE);

  // Draw empty graph, top left corner at pixel coordinate 40,10 on TFT
  gr.drawGraph(40, 10);

  // Start a trace with using red, trace points are in x and y scale units
  // In this example a horizontal line is drawn
  tr.startTrace(TFT_RED);
  // Add a trace point at 0.0,0.0 on graph
  tr.addPoint(0.0, 0.0);
  // Add another point at 100.0, 0.0 this will be joined via line to the last point added
  tr.addPoint(100.0, 0.0);

  // Start a new trace with using white
  tr.startTrace(TFT_WHITE);
}


// #########################################################################
// Meter constructor
// #########################################################################
 MyMeter::MyMeter(TFT_eSPI* tft)
 {
  ltx = 0;              // Saved x coord of bottom of needle
  osx = 120, osy = 120; // Saved x & y coords
  old_analog =  -999.0;   // Value last displayed
  old_digital = -999.0;   // Value last displayed

  mx = 0;
  my = 0;

  nxs=nys=nxe=nye=0;

    // Defaults
  strncpy(ms0,   "0", 4);
  strncpy(ms1,  "25", 4);
  strncpy(ms2,  "50", 4);
  strncpy(ms3,  "75", 4);
  strncpy(ms4, "100", 4);

  redStart = 0;
  redEnd = 0;
  orangeStart = 0;
  orangeEnd = 0;
  yellowStart = 0;
  yellowEnd = 0;
  greenStart = 0;
  greenEnd = 0;

  ntft = tft;
 }

// #########################################################################
// Draw meter meter at x, y and define full scale range & the scale labels
// #########################################################################
void MyMeter::analogMeter(uint16_t x, uint16_t y, uint16_t w, uint16_t h, float fullScale, const char *title, const char *units, int div, const char *s0, const char *s1, const char *s2, const char *s3, const char *s4)
{
  analogMeter(x, y, w, h, 0.0, fullScale, title, units, div, s0, s1, s2, s3, s4);
}

void MyMeter::analogMeter(uint16_t x, uint16_t y, uint16_t w, uint16_t h, float startScale, float endScale, const char *title, const char *units, int div, const char *s0, const char *s1, const char *s2, const char *s3, const char *s4)
{
  // Save offsets for needle plotting
  mx = x;
  my = y;
  mw = w;
  mh = h;

  float ticks = 25.0 / div;

  _startScale = startScale;
  _endScale = endScale;

  strncpy(mlabel, title, MAXTEXTLENGTH);

  strncpy(ms0, s0, 4);
  strncpy(ms1, s1, 4);
  strncpy(ms2, s2, 4);
  strncpy(ms3, s3, 4);
  strncpy(ms4, s4, 4);

  // Meter outline
  ntft->fillRect(x, y, mw, mh, TFT_GREY);
  ntft->fillRect(x + 5, y + 3, mw-9, mh-9, TFT_WHITE);

  ntft->setTextColor(TFT_BLACK);  // Text colour

  // Draw ticks every 5 degrees from -50 to +50 degrees (100 deg. FSD swing)
  for (float i = -50; i < 51; i += ticks) {
    // Long scale tick length
    int tl = 15;

    // Coordinates of tick to draw
    float sx = cos((i - 90) * 0.0174532925);
    float sy = sin((i - 90) * 0.0174532925);
    uint16_t x0 = x + sx * (100 + tl) + 120;
    uint16_t y0 = y + sy * (100 + tl) + 140;
    uint16_t x1 = x + sx * 100 + 120;
    uint16_t y1 = y + sy * 100 + 140;

    // Coordinates of next tick for zone fill
    float sx2 = cos((i + ticks - 90) * 0.0174532925);
    float sy2 = sin((i + ticks - 90) * 0.0174532925);
    int x2 = x + sx2 * (100 + tl) + 120;
    int y2 = y + sy2 * (100 + tl) + 140;
    int x3 = x + sx2 * 100 + 120;
    int y3 = y + sy2 * 100 + 140;

    // 1. zone limits
    if (redEnd > redStart) {
      if (i >= redStart && i < redEnd) {
        ntft->fillTriangle(x0, y0, x1, y1, x2, y2, TFT_VIOLET);
        ntft->fillTriangle(x1, y1, x2, y2, x3, y3, TFT_VIOLET);
      }
    }

    // 2 zone limits
    if (orangeEnd > orangeStart) {
      if (i >= orangeStart && i < orangeEnd) {
        ntft->fillTriangle(x0, y0, x1, y1, x2, y2, TFT_YELLOW);
        ntft->fillTriangle(x1, y1, x2, y2, x3, y3, TFT_YELLOW);
      }
    }

    // 3 zone limits
    if (yellowEnd > yellowStart) {
      if (i >= yellowStart && i < yellowEnd) {
        ntft->fillTriangle(x0, y0, x1, y1, x2, y2, TFT_ORANGE);
        ntft->fillTriangle(x1, y1, x2, y2, x3, y3, TFT_ORANGE);
      }
    }

    // 4. zone limits
    if (greenEnd > greenStart) {
      if (i >= greenStart && i < greenEnd) {
        ntft->fillTriangle(x0, y0, x1, y1, x2, y2, TFT_RED);
        ntft->fillTriangle(x1, y1, x2, y2, x3, y3, TFT_RED);
      }
    }

    // Short scale tick length
    
    if (fmod(i,25.0) != 0) tl = 8;

    // Recalculate coords in case tick length changed
    x0 = x + sx * (100 + tl) + 120;
    y0 = y + sy * (100 + tl) + 140;
    x1 = x + sx * 100 + 120;
    y1 = y + sy * 100 + 140;

    // Draw tick
    ntft->drawLine(x0, y0, x1, y1, TFT_BLACK);

    // Check if labels should be drawn, with position tweaks
    if (fmod(i,25.0) == 0) {
      // Calculate label positions
      x0 = x + sx * (100 + tl + 10) + 120;
      y0 = y + sy * (100 + tl + 10) + 140;
      switch (int(i / 25.0)) {
        case -2: ntft->drawCentreString(ms0, x0, y0 - 12, 2); break;
        case -1: ntft->drawCentreString(ms1, x0, y0 - 9, 2); break;
        case  0: ntft->drawCentreString(ms2, x0, y0 - 6, 2); break;
        case  1: ntft->drawCentreString(ms3, x0, y0 - 9, 2); break;
        case  2: ntft->drawCentreString(ms4, x0, y0 - 12, 2); break;
      }
    }

    // Now draw the arc of the scale
    sx = cos((i + ticks - 90) * 0.0174532925);
    sy = sin((i + ticks - 90) * 0.0174532925);
    x0 = x + sx * 100 + 120;
    y0 = y + sy * 100 + 140;
    // Draw scale arc, don't draw the last part
    if (i < 50) ntft->drawLine(x0, y0, x1, y1, TFT_BLACK);
  }

  DEBUG_PRINTF("analogMeter: units=%s, x=%d, y=%d\n",units,mx + mw - ntft->textWidth(units,2) - 15,my + mh - 30);
  ntft->drawString(units, mx + mw - ntft->textWidth(units, 2) - 15, my + mh - 30, 2); // Units at bottom right
  //ntft->drawCentreString(mlabel, x + xofs, y + 70, 2);          // Comment out to avoid font 4
  ntft->drawRect(x + 5, y + 3, mw-9, mh-9, TFT_BLACK);           // Draw bezel line
  
  //updateNeedle(0, 0);
}

float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
// #########################################################################
// Update needle position
// This function is blocking while needle moves, time depends on ms_delay
// 10ms minimises needle flicker if text is drawn within needle sweep area
// Smaller values OK if text not in sweep area, zero for instant movement but
// does not look realistic... (note: 100 increments for full scale deflection)
// #########################################################################
void verschiebeStartpunkt(float &x, float &y, float x1, float y1, float yd) {
  // Berechne die Steigung der Geraden (Winkel in Bogenmaß)
  float deltaX = x1 - x;
  float deltaY = y1 - y;
  
  // Berechne den Winkel der Geraden
  float theta = atan2(deltaY, deltaX);  // atan2 gibt den Winkel in Bogenmaß zurück
  
  // Berechne den neuen y-Wert
  y += yd;
  
  // Berechne den neuen x-Wert so, dass der Winkel erhalten bleibt
  float dx = yd / tan(theta);
  x += dx;
}
void MyMeter::updateNeedle(float val, uint32_t ms_delay)
{
  //int value = (val - scaleStart) * factor;
  float value = val;

  DEBUG_PRINTF("%s: mx=%d, my=%d, mw=%d, mh=%d\n",mlabel, mx, my, mw, mh);
  
  ntft->setTextColor(TFT_BLACK, TFT_WHITE);
  char buf[MAXTEXTLENGTH]; 
  dtostrf(val, 5, 1, buf);
  
  if (value < _startScale) value = _startScale; // Limit value to emulate needle end stops
  if (value > _endScale) value = _endScale;

  // Move the needle until new value reached
  while (value != old_analog) {
    if (old_analog < value) old_analog++;
    else old_analog--;

    if (ms_delay == 0) 
      old_analog = value; // Update immediately id delay is 0

    float sdeg = mapFloat(value, _startScale, _endScale, -140.0, -40.0);
    // Calculate tip of needle coords
    float sx = cos(sdeg * 0.0174532925);
    float sy = sin(sdeg * 0.0174532925);

    // Calculate x delta of needle start (does not start at pivot point)
    float tx = tan((sdeg + 90) * 0.0174532925);

    

    // Erase old needle image
    if(nxs) {
      ntft->drawLine(nxs - 1, nys, nxe - 1, nye, TFT_WHITE);
      ntft->drawLine(nxs    , nys, nxe    , nye, TFT_WHITE);
      ntft->drawLine(nxs + 1, nys, nxe + 1, nye, TFT_WHITE);
    }
    
    // Re-plot text under needle
    ntft->setTextColor(TFT_BLACK);
    ntft->drawCentreString(mlabel, mx + (mw/2), my + 70, 2);
    ntft->drawCentreString(buf,    mx + (mw/2), my + 85, 2);

    // Store new needle end coords for next erase
    ltx = tx;
    osx = sx * 98 + 120;
    osy = sy * 98 + 140;

    // Draw the needle in the new position, magenta makes needle a bit bolder
    // draws 3 lines to thicken needle
    float fxs = mx + 120 + 20 * ltx;
    float fys = my + 140 - 20;
    float fxe = mx + osx;
    float fye = my + osy;

    DEBUG_PRINTF("vor nxs=%f, ys=%f, xe=%f, ye=%f\n",fxs,fys,fxe,fye);
    verschiebeStartpunkt(fxs, fys, fxe, fye, my+mh-6-fys);
    nxs = fxs;
    nys = fys;
    nxe = fxe;
    nye = fye;
    DEBUG_PRINTF("nach nxs=%d, nys=%d, nxe=%d, nye=%d\n",nxs,nys,nxe,nye);

    ntft->drawLine(nxs-1, nys, nxe-1, nye, TFT_RED);
    ntft->drawLine(nxs, nys, nxe, nye, TFT_MAGENTA);
    ntft->drawLine(nxs-1, nys,nxe-1, nye, TFT_RED);

    // Slow needle down slightly as it approaches new position
    if (abs(old_analog - value) < 10) 
      ms_delay += ms_delay / 5;

    // Wait before next update
    delay(ms_delay);
  }
}

// #########################################################################
// Set red, orange, yellow and green start+end zones as a % of full scale
// #########################################################################
void MyMeter::setZones(uint16_t rs, uint16_t re, uint16_t os, uint16_t oe, uint16_t ys, uint16_t ye, uint16_t gs, uint16_t ge)
{
  // Meter scale is -50 to +50
  redStart = rs - 50;
  redEnd = re - 50;
  orangeStart = os - 50;
  orangeEnd = oe - 50;
  yellowStart = ys - 50;
  yellowEnd = ye - 50;
  greenStart = gs - 50;
  greenEnd = ge - 50;
}
#endif