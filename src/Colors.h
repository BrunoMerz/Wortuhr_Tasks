/******************************************************************************
Colors.h
******************************************************************************/

#ifndef COLORS_H
#define COLORS_H

enum eColorChange : uint8_t
{
	COLORCHANGE_NO,                      // 0
	COLORCHANGE_FIVE,                    // 1
	COLORCHANGE_HOUR,                    // 2
	COLORCHANGE_DAY,                     // 3
  	COLORCHANGE_MAIN,                    // 4
	COLORCHANGE_COUNT = COLORCHANGE_DAY
};

enum eColor : uint8_t
{
	WHITE,
	YELLOW_50,
	YELLOW_25,
	YELLOW,
	MAGENTA_50,
	RED_50,
	ORANGE,
	MAGENTA_25,
	RED_25,
	MAGENTA,
	PINK,
	RED,
	CYAN_50,
	GREEN_50,
	BLUE_50,
	GREENYELLOW,
	VIOLET,
	CYAN_25,
	GREEN_25,
	BLUE_25,
	BLUE,
	CYAN,
	MINTGREEN,
	GREEN,
	LIGHTBLUE,
	RAINBOW,
	COLOR_COUNT = RAINBOW
};

struct color_s
{
	uint8_t red;
	uint8_t green;
	uint8_t blue;
};

const color_s defaultColors[] =
{
	{ 0xFF, 0xFF, 0xFF }, // 00 WHITE
	{ 0xFF, 0xFF, 0x80 }, // 01 YELLOW_50
	{ 0xFF, 0xFF, 0x40 }, // 02 YELLOW_25
	{ 0xFF, 0xFF, 0x00 }, // 03 YELLOW
	{ 0xFF, 0x80, 0xFF }, // 04 MAGENTA_50
	{ 0xFF, 0x80, 0x80 }, // 05 RED_50
	{ 0xFF, 0x70, 0x00 }, // 06 ORANGE
	{ 0xFF, 0x40, 0xFF }, // 07 MAGENTA_25
	{ 0xFF, 0x40, 0x40 }, // 08 RED_25
	{ 0xFF, 0x00, 0xFF }, // 09 MAGENTA
	{ 0xFF, 0x00, 0x7F }, // 10 PINK
	{ 0xFF, 0x00, 0x00 }, // 11 RED
	{ 0x80, 0xFF, 0xFF }, // 12 CYAN_50
	{ 0x80, 0xFF, 0x80 }, // 13 GREEN_50
	{ 0x80, 0x80, 0xFF }, // 14 BLUE_50
	{ 0x7F, 0xFF, 0x00 }, // 15 GREENYELLOW
	{ 0x7F, 0x00, 0xFF }, // 16 VIOLET
	{ 0x40, 0xFF, 0xFF }, // 17 CYAN_25
	{ 0x40, 0xFF, 0x40 }, // 18 GREEN_25
	{ 0x40, 0x40, 0xFF }, // 19 BLUE_25
	{ 0x0F, 0x0F, 0xFF }, // 20 BLUE
	{ 0x00, 0xFF, 0xFF }, // 21 CYAN
	{ 0x00, 0xFF, 0x7F }, // 22 MINTGREEN
	{ 0x00, 0xFF, 0x00 }, // 23 GREEN
	{ 0x00, 0x7F, 0xFF }, // 24 LIGHTBLUE
	{ 0x00, 0x00, 0x00 }, // 25 RAINBOW
};

const uint32_t colorArray[] =
{
	0xFFFFFF, // 00 WHITE
	0xFFFF80, // 01 YELLOW_50
	0xFFFF40, // 02 YELLOW_25
	0xFFFF00, // 03 YELLOW
	0xFF80FF, // 04 MAGENTA_50
	0xFF8080, // 05 RED_50
	0xFF7000, // 06 ORANGE
	0xFF40FF, // 07 MAGENTA_25
	0xFF4040, // 08 RED_25
	0xFF00FF, // 09 MAGENTA
	0xFF007F, // 10 PINK
	0xFF0000, // 11 RED
	0x80FFFF, // 12 CYAN_50
	0x80FF80, // 13 GREEN_50
	0x8080FF, // 14 BLUE_50
	0x7FFF00, // 15 GREENYELLOW
	0x7F00FF, // 16 VIOLET
	0x40FFFF, // 17 CYAN_25
	0x40FF40, // 18 GREEN_25
	0x4040FF, // 19 BLUE_25
	0x0F0FFF, // 20 BLUE
	0x00FFFF, // 21 CYAN
	0x00FF7F, // 22 MINTGREEN
	0x00FF00, // 23 GREEN
	0x007FFF, // 24 LIGHTBLUE
	0x000000, // 25 RAINBOW
};


#endif
