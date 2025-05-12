/******************************************************************************
Spiel_main.h
******************************************************************************/
#pragma once

#include <Arduino.h>
#include "Configuration.h"
#include "Colors.h"
#include "Spiel_externals.h"

int8_t board[9][9] = {0};

uint8_t curControl = BTN_NONE;
uint8_t aktgame = 0;

bool gameisrunning = false;
uint16_t highscore[9] = {0,0,0,0,0,0,0,0,0}; //Snake, Tetris, Bricks,4gewinnt...
uint8_t gamecount[9]  = {0,0,0,0,0,0,0,0,0}; 

uint16_t aktscore;
uint8_t gamelevel = 2;
uint8_t gamesize = 2;
int8_t aktivPlayer = 0;
uint8_t anzPlayer = 0;

String PlayerIP[4]={"","","",""};

int debugval;
int gamesound = 2;
