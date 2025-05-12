#pragma once

#define BYTE  uint8_t
#define WORD  uint16_t
#define DWORD uint32_t
#define LONG  uint32_t


struct ICOHEADER {
   WORD wReserved;  // Always 0
   WORD wResID;     // Always 1
   WORD wNumImages; // Number of icon images/directory entries
};


struct ICONDIRENTRY {
    BYTE  bWidth;    // Breite des Bildes
    BYTE  bHeight;    // Die doppelte Höhe des Bildes in Pixeln.
    BYTE  bCount;    // Anzahl der Farben im Bild (2 oder 16, bzw. 0 falls mehr als 
    BYTE  bReserved;    // 256 Farben). immer 0
    WORD  wPlanes;    // Anzahl der Farbebenen in der Regel 1 u. 0
    WORD  wBitCount;      // Anzahl der Bits pro Pixel( Anzahl der Farben) 1= Schwarz-
        // Weiß = 8 Byte, 4 = 16 Farben = 64 Byte, 
        // 8 = 256 Farben = 1024 Byte, 24 = 16 Milion
        // Farben (keine Farbtabelle) 
    DWORD dwBytesInRes;   // Bildgröße in Bytes ab diesem Record. Das bedeutet 
                          // BITMAPINFOHEADER, Farbtabelle, XOR- und 
                          // AND-Bild zusammen gerechnet.
    DWORD dwImageOffset;  // Offset ab wo der BITMAPINFOHEADER beginnt.
};


struct BITMAPINFOHEADER {
    DWORD   biSize;      // Länge des Info-Headers(dieser Record) = 40 Byte in Hex 28
    LONG    biWidth;      // Breite des Bildes 
    LONG    biHeight;     // Höhe des Bildes 
    WORD    biPlanes;     // Anzahl der Farbebenen in der Regel 1 u. 0
    WORD    biBitCount;   // Anzahl der Bits pro Pixel( Anzahl der Farben) 1= Schwartz-
        // Weiß = 8 Byte, 4 = 16 Farben = 64 Byte, 
        // 8 = 256 Farben = 1024 Byte, 24 = 16 Milion 
        // Farben (keine Farbtabelle)  
    DWORD   biCompression;  // Komprimierungstyp, 0 = Unkomprimiert, 1 = 8-Bit 
          // RLE =Run-Length-Encording-Verfahren, 2 = 4-Bit Run-Encording-
          // Verfahren
    DWORD   biSizeImage;    // Bildgröße ohne Farbtabelle. Aber XOR und AND Bild zusammen 
             // gerechnet bei Icon u. Cursor.
    DWORD   biXPelsPerMeter;   // Horizontale Auflösung
    DWORD   biYPelsPerMeter;   // Vertikale Auflösung
    DWORD   biClrUsed;         // Die Zahl der im Bild vorkommenden Farben zB. Bei 256 Farben
             //  müssen es nicht unbedingt 256 sein, es könne auch 
             // zB. 206 sein.
    DWORD   biClrImportant;    // die Anzahl der wichtigen Farben
} ;

struct ICON {
  BYTE  b;  // blue
  BYTE  g;  // green
  BYTE  r;  // red
  BYTE  a;  // alpha
};
