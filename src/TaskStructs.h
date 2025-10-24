#pragma once

#include <Arduino.h>

typedef enum eTask : uint8_t
{
    TASK_STARTUP=0,     // 0
    TASK_SCHEDULER,     // 1
    TASK_TIME,          // 2
    TASK_MODES,         // 3
    TASK_TEXT,          // 4
    TASK_ANIMATION,     // 5
    TASK_EVENT,         // 6
#if defined(WITH_SECOND_BELL)
    TASK_SECOND_BELL,   // 7
#endif
#if defined(WITH_SECOND_HAND)
    TASK_SECOND_HAND,   // 8
#endif
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
    char         name[24];
} s_taskInfo;


typedef struct {
    uint8_t feedPosition;
    uint8_t feedBuzzer;
    uint32_t feedColor;
    bool updateScreen;
    bool endless_loop;
    String feedText;
    String animation;
    s_taskInfo taskInfo[TASK_MAX];
} s_taskParams;
  