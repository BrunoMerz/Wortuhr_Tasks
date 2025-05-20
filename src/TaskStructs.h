#pragma once

#include <Arduino.h>

typedef struct {
    TaskHandle_t taskHandle;
    uint32_t     stackSize;

} s_taskInfo;

typedef struct {
    uint8_t feedBuzzer;
    uint32_t feedColor;
    String feedText;
    bool updateSceen;
    s_taskInfo taskInfo[8];
} s_taskParams;
  