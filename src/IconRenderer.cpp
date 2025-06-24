/**
   IconRenderer.cpp
   Class outputs icons stored in LittleFS

   @autor    B. Merz
   @version  1.1
   @created  09.01.2020
   @updated  19.03.2020

*/


#include "IconRenderer.h"
#include "LedDriver_FastLED.h"
#include "MyFileAccess.h"

//#define myDEBUG
#include "MyDebug.h"

static LedDriver *ledDriver = LedDriver::getInstance();
static MyFileAccess *fa = MyFileAccess::getInstance();

IconRenderer* IconRenderer::instance = 0;

IconRenderer *IconRenderer::getInstance() {
  if (!instance)
  {
      instance = new IconRenderer();
  }
  return instance;
}

IconRenderer::IconRenderer() {
  brightness=10;
}


void IconRenderer::renderAndDisplay(String iconFileName, int delayAfter, byte cleanUp, int16_t x_pos, int16_t y_pos) {
  char *fb;
  String tmp=iconFileName;
  char TFTfn[40] = {"/TFT"};
  if(!tmp.startsWith("/")) {
    strcpy(TFTfn+4,tmp.c_str());
    tmp = "/"+tmp;
  } else {
    strcpy(TFTfn+4, tmp.substring(1).c_str());
  }
  DEBUG_PRINTF("Icon to render='%s', delayAfter=%d, cleanUp=%d\n",tmp.c_str(), delayAfter, cleanUp);
  vTaskDelay(pdMS_TO_TICKS(5));
  DEBUG_PRINTLN(tmp);

  int fs = fa->fileSize(tmp);
  DEBUG_PRINTF("Icon size=%d\n",fs);
  if(fs <= 0)
    return;
  fb = (char *)malloc(fs+10);
  if(!fb) {
    DEBUG_PRINTF("Malloc %d bytes failed\n",fs);
    return;
  }
  int lng = fa->readFile(tmp, fb);
  if(!lng){
    DEBUG_PRINTLN("renderAndDisplay: file not found -> "+iconFileName);
     return;
  }

  uint16_t _width = LED_COLS;
  uint16_t _height = LED_ROWS;

  // set _icoHeader to beginning of icon file
  _icoHeader = (ICOHEADER*)fb;
  // set _pIconDirentry 
  _pIconDirentry = (ICONDIRENTRY*)(fb+sizeof(ICOHEADER));
  // _pIconDirentry must be copied due to alignment of integer values
  memcpy((void*)&_iconDirentry,(const void*) _pIconDirentry, sizeof(ICONDIRENTRY));
  // set Bitmap Infoheader 40 Byte
  _pBitmapInfoHeader = (BITMAPINFOHEADER*) (fb+sizeof(ICOHEADER)+(sizeof(ICONDIRENTRY)*_icoHeader->wNumImages));
  // _pBitmapInfoHeader must be copied due to alignment of integer values
  memcpy((void*)&_bitmapInfoHeader,(const void*) _pBitmapInfoHeader, sizeof(BITMAPINFOHEADER));

  // setup all necessary values
  byte r,g,b;
  int paddedW       = 0;
  int Mapsize  = 0;
  int wBitCount     = _bitmapInfoHeader.biBitCount;
  int width         = _bitmapInfoHeader.biWidth;
  int height        = _bitmapInfoHeader.biHeight/2;
  int wPlanes       = _bitmapInfoHeader.biPlanes;
  int Count    = 1 << (wBitCount * wPlanes);                 // Count = 2 ^ number of  bits, Bits=1->2 s, 4->16 s, 24->‭16.777.216‬ s
  byte bit_m        = Count - 1;

  // check if x_pos/y_pos are given, if no place ico midth of display
  if(x_pos >= 0) 
    _x_pos = x_pos;
  else
    if(_width>width)
      _x_pos=(_width-width)/2;
    else
      _x_pos = 0;

  if(y_pos >= 0) 
    _y_pos = y_pos;
  else
    if(_height>height)
      _y_pos=(_height-height)/2;
    else
      _y_pos = 0;

  DEBUG_PRINTF("_width=%d, width=%d, _x_pos=%d, _height=%d, height=%d, _y_pos=%d\n",_width, width, _x_pos, _height, height, _y_pos);
  // due to different bitmap width, i.e 11 bits width, we need some padding bytes
  if(wBitCount==1) {
    paddedW = (32 -_bitmapInfoHeader.biWidth)%32;
  } else if(wBitCount==4){
    paddedW = (16 -_bitmapInfoHeader.biWidth)%16;
  } else if(wBitCount==24){
    paddedW = 4 - ((_bitmapInfoHeader.biWidth*3)%4);
  }

  // calculate size of  table for icons with a bitCount less or equal than 8
  // for icons with bitCount=24 we d  on't have a  table
  if(wBitCount <= 8) Mapsize = (Count*4); // biBitCount=1->8, 4->64, 8->1024, 24->no  map

  char *XorMask = ((char *)_pBitmapInfoHeader + _bitmapInfoHeader.biSize + Mapsize);
  char *AndMask = XorMask + (((width+paddedW)*height*wBitCount)/8);
  
  DEBUG_PRINTF("Mapsize=%#x\nCount=%#x\npaddedW=%d\nXorMask=%#x\nAndMask=%#x\n",Mapsize, Count, paddedW, XorMask-fb, AndMask-fb);
  DEBUG_PRINTF("height=%d, width=%d, wBitCount=%d, paddedW=%d\n",height,width,wBitCount,paddedW);
  
  // icons with compressed bitmaps or icons > display size are not suppoerted
  if(_bitmapInfoHeader.biCompression || height > _height || width > _width) { 
    DEBUG_PRINTLN("Compressed icons or icons greater than LED Matrix not supported");
    if(fb) free(fb);
    ledDriver->show();
    return;
  }

  // clear led matrix
  if(cleanUp & 1)
    ledDriver->clear();

  if(wBitCount <= 8) {
    // Icon with  map    

    _icon = (ICON*) ((char *)_pBitmapInfoHeader + _bitmapInfoHeader.biSize);

    if(Count==2) {
      for(int y=height-1; y>=0; y--) {
        for(int x=0; x<width; x++) {
          int bit_n = x+(y*(width+paddedW));// bit_n: n-th bit within XorMask
          int byte_n = bit_n/8;             // byte_n: n-th byte within XorMask
          int bit_r = bit_n%8;              // bit_r: remainder 
          byte b = AndMask[byte_n];         // byte with necessary bit
          byte _i = b >> (7-bit_r);    // shift bit to right into rightmost position
          _i &= bit_m;                 // mask out all other bits
  
          if(!x) {
            DEBUG_PRINTLN();
          }
          if(x<_width && y>=(height-_height) && !_i) {
            DEBUG_PRINT("o ");
            ledDriver->setPixelRGB(x+_x_pos, _height-(y+_y_pos), 0xff, 0xff, 0xff);
          } else {
            DEBUG_PRINT("  ");
          }
        }
        DEBUG_FLUSH();
      }
      DEBUG_PRINTLN();
    } else {  
      // 16 or 256 s
      DEBUG_PRINTLN("16 or 256 s");
      for(int y=height-1; y>=0; y--) {
        for(int x=0; x<width; x++) {
          int bit_n = (x*wBitCount)+(y*(width+paddedW)*wBitCount);// bit_n: n-th bit within XorMask
          int byte_n = bit_n/8;             // byte_n: n-th byte within XorMask
          int bit_r = bit_n%8;              // bit_r: remainder 
          byte b = XorMask[byte_n];         // byte with necessary bit
          byte _i = b >> (8-wBitCount-bit_r);    // shift bit to right into rightmost position
          _i &= bit_m;                 // mask out all other bits
  
          if(x<_width && y>=(height-_height)) {
            if(!x) {
              DEBUG_PRINTLN();
            }
            r = _icon[_i].r;
            g = _icon[_i].r;
            b = _icon[_i].r;
            
            if(r || g || b) {
              DEBUG_PRINT("o ");
              ledDriver->setPixelRGB(x+_x_pos, _height-1-(y+_y_pos), r, g, b);
            } else {
              DEBUG_PRINT("  ");
            }
          }
        }
      }
      DEBUG_PRINTLN();
    }
  } else {
    // Icon without  map, s defined directly in XorMask (each pixel in BGR)
    DEBUG_PRINTLN("Icon without  map");
    for(int y=height-1, offset=0; y>=0; y--) {
      for(int x=0; x<width; x++) {
        offset = (x*3) + (y*3*width)+(paddedW*y);
        
        if(x<_width && y>=(height-_height)) {
          r = XorMask[offset+2];
          g = XorMask[offset+1];
          b = XorMask[offset];
          uint16_t xg = x+_x_pos;
          uint16_t yg = y+_y_pos;

          //DEBUG_PRINTF("x=%d, y=%d, c=%#.6x\n",xg,yg,cg);
          if(r || g || b) {
            ledDriver->setPixelRGB(xg, _height-1-yg, r, g, b);
          }
        }
      }
    }
  }


  ledDriver->show();
  vTaskDelay(pdMS_TO_TICKS(delayAfter));

  // clear led matrix
  if(cleanUp & 2)
    ledDriver->clear();

  if(fb)
    free(fb);
}
