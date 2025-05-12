#include <Arduino.h>
#include "Colors.h"
#include "WebHandler.h"
#include "Settings.h"
#include "MyTime.h"

#define myDEBUG
#include "MyDebug.h"



// Farbwert num -> s_color (z.B. 0xFF00DD) nach red, green, blue
color_s num_to_color(uint32_t in_color)
{
  color_s s_color;
  s_color.red = in_color >> 16 & 0xFF;
  s_color.green = in_color >> 8 & 0xFF;
  s_color.blue = in_color & 0xFF;
  return s_color;
}

// Farbwert (z.B. #FF00DD ) von HEX nach int wandeln
uint32_t string_to_num(String in_color)
{
  char buffer[9];
  in_color.substring(1, 7).toCharArray(buffer,7);
  return strtol(buffer, 0, 16);
}

// Farbwert (z.B. #FF00DD ) von HEX nach String wandeln
String num_to_string(uint32_t in_color)
{
  String farbe;
  farbe = "000000" + String(in_color,HEX);
  farbe = "#" + farbe.substring( farbe.length()-6,farbe.length());
  farbe.toUpperCase();
  return farbe;
}

// Farbwert (z.B. red, green, blue ) nach String wandeln
String color_to_string(color_s in_color)
{
  String farbe;
  uint32_t colbuf = 0;
  colbuf = in_color.red<<16;
  colbuf = colbuf | in_color.green<<8;
  colbuf = colbuf | in_color.blue;
  return num_to_string(colbuf);
}

// Farbwert string -> s_color
color_s string_to_color(String in_color)
{
  return num_to_color(string_to_num(in_color));
}

float getHeapFragmentation() {
    size_t total_free = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    size_t largest_free = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
    
    float fragmentation = 0.0;
    if (total_free > 0) {
        fragmentation = (1.0 - ((float)largest_free / total_free)) * 100.0;
    }
  
    return fragmentation;
  }

#if defined(WITH_AUDIO)
// finde den passenden Sound zur Wetter-Id
uint16_t getWeatherSound(uint16_t wetterid)
{
  uint8_t soundmapidx = 0;
  uint16_t soundidx = 0;
  uint16_t soundnr = 0;
  uint16_t wettermapid = 0;

  do
  {
    soundnr = pgm_read_word(&wettersoundmapping[soundmapidx][0]);
    wettermapid = pgm_read_word(&wettersoundmapping[soundmapidx][1]);
    if ( wettermapid == wetterid )
    {
      soundidx = WETTERSOUNDBASE - 1 + soundnr;
      soundnr = 0;
    }
    soundmapidx++;
  } while ( soundnr != 0 );

  if ( soundidx == WETTERSOUNDBASE - 1 || soundidx == 0 ) soundidx = 99; // kein Sound vorhanden
  return soundidx;
}
#else
uint16_t getWeatherSound(uint16_t wetterid)
{
  return 99;
}
#endif

