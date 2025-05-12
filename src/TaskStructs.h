#pragma once

#include <Arduino.h>

typedef struct {
    uint8_t feedBuzzer;
    uint32_t feedColor;
    String feedText;
    bool updateSceen;
} s_taskParams;
  