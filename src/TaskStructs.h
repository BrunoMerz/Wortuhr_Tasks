#pragma once

#include <Arduino.h>

typedef enum eTask : uint8_t
{
    TASK_STARTUP=0,     // 0
    TASK_SCHEDULER,     // 1
    TASK_TIME,          // 2
    TASK_SECOND_BELL,   // 3
    TASK_SECOND_HAND,   // 4
    TASK_MODES,         // 5
    TASK_TEXT,          // 6
    TASK_ANIMATION,     // 7
    TASK_EVENT,         // 8
#if defined(LILYGO_T_HMI)
    TASK_T_HMI,
    TASK_DRAW,
#endif
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
    bool updateScreen;
    String feedText;
    String animation;
    bool endless_loop;
    s_taskInfo taskInfo[TASK_MAX];
} s_taskParams;
  