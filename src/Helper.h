#pragma once

#include <Arduino.h>
#include "Colors.h"

extern uint32_t string_to_num(String in_color);
extern color_s string_to_color(String in_color);
extern String color_to_string(color_s in_color);
extern color_s num_to_color(uint32_t in_color);
extern String num_to_string(uint32_t in_color);
