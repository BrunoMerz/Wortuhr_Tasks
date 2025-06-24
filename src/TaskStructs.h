#pragma once

#include <Arduino.h>

typedef enum eTask : uint8_t
{
    TASK_STARTUP=0,
    TASK_SCHEDULER,
    TASK_TIME,
    TASK_SECOND_BELL,
    TASK_SECOND_HAND,
    TASK_MODES,
    TASK_TEXT,
    TASK_ANIMATION,
    TASK_EVENT,
    TASK_MAX
} Task;


typedef enum eState : uint8_t
{
    STATE_INIT=0,
    STATE_PROCESSING,
    STATE_PROCESSED
} State;


typedef struct {
    TaskHandle_t taskHandle;
    uint32_t     stackSize;
    bool         handleEvent;
    eState       state;
} s_taskInfo;


typedef struct {
    uint8_t feedPosition;
    uint8_t feedBuzzer;
    uint32_t feedColor;
    bool updateSceen;
    String feedText;
    String animation;
    bool endless_loop;
    s_taskInfo taskInfo[TASK_MAX];
} s_taskParams;
  