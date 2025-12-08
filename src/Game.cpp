#include "Game.h"
#include "TaskStructs.h"
#include "Animation.h"
#include "ESPAsyncWebServer.h"
#include "Renderer.h"
#include "LedDriver_FastLED.h"
#include "Settings.h"
#include "Animation.h"


#define myDEBUG
#include "MyDebug.h"

extern void callRoot(AsyncWebServerRequest *request);
extern void runSnake(void *p);
extern void runTetris(void *p);
extern void runBricks(void *p);
extern void runViergewinnt(void *p);
extern void runMemory(void *p);
extern bool ButtonIn(uint8_t buttonvalue);
extern void ButtonClear();

extern s_taskParams taskParams;
extern EventGroupHandle_t xEvent;

Game* Game::instance = 0;

Game *Game::getInstance() {
  if (!instance)
  {
      instance = new Game();
  }
  return instance;
}

Game::Game()
{
  gameTaskHandle = NULL;
}


// Starte Spiel
void Game::startGame(AsyncWebServerRequest *request)
{
 String webstring;
 String gamestring;

  DEBUG_PRINT("Start Game: ");
  DEBUG_PRINTLN(request->arg("game"));

  aktgame = request->arg("game").toInt();
  if ( aktgame == SNAKE )
    gamestring = F("SNAKE");
  if ( aktgame == TETRIS )
    gamestring = F("TETRIS");
  if ( aktgame == BRICKS )
    gamestring = F("BRICKS");
  if ( aktgame == VIERGEWINNT )
    gamestring = F("VIERGEWINNT");
  if ( aktgame == TIERMEMORY )
    gamestring = F("MEMORY");
  if ( aktgame == MUSIKMEMORY )
    gamestring = F("MEMORY");
  if ( aktgame == ABBAMEMORY )
    gamestring = F("MEMORY");
  aktscore = 0;
  webstring = F("<!doctype html><html><head><script>window.onload=function(){window.location.replace('/web/GameControl.html?game=");
  webstring += gamestring;
  webstring += F("&highscore=");
  webstring += String(highscore[aktgame]);

  if (aktgame == VIERGEWINNT) 
  {
    webstring += F("&level=");
    webstring += String(gamelevel);
  }
  if (aktgame == TIERMEMORY || aktgame == MUSIKMEMORY || aktgame == ABBAMEMORY ) 
  {
    webstring += F("&size=");
    webstring += String(gamesize);
  }
   
  webstring += F("');}</script></head></html>");
  request->send(200, "text/html",webstring);

  curControl= BTN_NONE;  
  delay(0);
  anzPlayer = 1;
}

void Game::handleGameControl(AsyncWebServerRequest *request)
{
  Renderer  *renderer   = Renderer::getInstance();
  LedDriver *ledDriver  = LedDriver::getInstance();
  Settings  *settings   = Settings::getInstance();
  AnimationFS *anifs    = AnimationFS::getInstance();

  TaskFunction_t fptr;

  bool buttonret;
  String webreturn;
  uint8_t playerret;
  uint8_t playerbuttonadd = 0;

  // Check if game ended
  if (gameTaskHandle != NULL) {
    eTaskState state = eTaskGetState(gameTaskHandle);
    if (state == eDeleted) {
      gameTaskHandle = NULL;

      DEBUG_PRINTF("Game over, state=%d\n", state);

      gameisrunning = false;

      // sichern der Higshcore Werte
      if (  highscore[SNAKE] > settings->mySettings.highscore[SNAKE] || highscore[TETRIS] > settings->mySettings.highscore[TETRIS] 
        || highscore[BRICKS] > settings->mySettings.highscore[BRICKS] || highscore[VIERGEWINNT] > settings->mySettings.highscore[VIERGEWINNT] ) 
      {
        DEBUG_PRINTLN(F("sichere Highscore in EEPROM"));

        settings->mySettings.highscore[SNAKE] = highscore[SNAKE];
        settings->mySettings.highscore[TETRIS] = highscore[TETRIS];
        settings->mySettings.highscore[BRICKS] = highscore[BRICKS];
        settings->mySettings.highscore[VIERGEWINNT] = highscore[VIERGEWINNT];
        settings->saveToNVS();
      }
 
      if (curControl != BTN_STOP && aktgame != VIERGEWINNT && aktgame != TIERMEMORY && aktgame != MUSIKMEMORY && aktgame != ABBAMEMORY )
      {
        //vTaskDelay(pdMS_TO_TICKS(1000));
        uint8_t gor = random(3);
        if ( gor == 0 ) {
          DEBUG_PRINTLN(F("Game over Animation"));

          taskParams.animation = "GAME_OVER";
          taskParams.endless_loop = false;
          if(anifs->loadAnimation(taskParams.animation))
            xEventGroupSetBits(xEvent, MODE_SHOWANIMATION);
          taskParams.taskInfo[TASK_ANIMATION].handleEvent=true;
        }
        if ( gor == 1 ) {
          DEBUG_PRINTLN(F("Game over Pixel"));

          ledDriver->clear();
          ledDriver->setPixel(10,1,colorArray[YELLOW]);
          ledDriver->show();
          vTaskDelay(pdMS_TO_TICKS(200));
          ledDriver->setPixel(8,3,colorArray[YELLOW]);
          ledDriver->show();
          vTaskDelay(pdMS_TO_TICKS(200));
          ledDriver->setPixel(6,5,colorArray[YELLOW]);
          ledDriver->show();
          vTaskDelay(pdMS_TO_TICKS(200));
          ledDriver->setPixel(4,8,colorArray[YELLOW]);
          ledDriver->show();
          vTaskDelay(pdMS_TO_TICKS(200));
          ledDriver->setPixel(1,3,colorArray[RED]);
          ledDriver->show();
          vTaskDelay(pdMS_TO_TICKS(200));
          ledDriver->setPixel(4,2,colorArray[RED]);
          ledDriver->show();
          vTaskDelay(pdMS_TO_TICKS(200));
          ledDriver->setPixel(5,4,colorArray[RED]);
          ledDriver->show();
          vTaskDelay(pdMS_TO_TICKS(200));
          ledDriver->setPixel(7,2,colorArray[RED]);
          ledDriver->show();
          vTaskDelay(pdMS_TO_TICKS(3000));
        }
        if ( gor == 2 ) {
          DEBUG_PRINTLN(F("Game over Feed"));

          taskParams.feedColor = colorArray[MAGENTA];
          taskParams.feedText = F("  Game Over   ");
          taskParams.feedPosition = 0;
          xEventGroupSetBits(xEvent, MODE_FEED);
          taskParams.taskInfo[TASK_TEXT].handleEvent=true;
        }
      }
      anzPlayer = 1;
      DEBUG_PRINTLN(F("zurück zur Uhr..."));
    }
  }

  if ( gameisrunning) playerret = anzPlayer; else playerret = 0;
  
  if ( request->arg("button") == "start" ) 
  {

#ifdef myDEBUG
      webreturn += "1#" + String(aktgame) + "#" + String(anzPlayer) + "#" + String(highscore[aktgame]) + "#" + String(aktscore) + "#" + String(debugval);
#else
      webreturn += "1#" + String(aktgame) + "#" + String(anzPlayer) + "#" + String(highscore[aktgame]) + "#" + String(aktscore) + "#0";
#endif 

    request->send(200, "text/plain", webreturn); 
    delay(0);
    ButtonClear();

#if defined(WITH_SNAKE)
    if ( aktgame == SNAKE       ) fptr = runSnake;
#endif
#if defined(WITH_TETRIS)
    if ( aktgame == TETRIS      ) fptr = runTetris;
#endif
#if defined(WITH_BRICKS)
    if ( aktgame == BRICKS      ) fptr = runBricks;
#endif
#if defined(WITH_4GEWINNT)
    if ( aktgame == VIERGEWINNT ) fptr = runViergewinnt;
#endif

    // disable all other led actions
    for(uint8_t i=0; i < TASK_MAX; i++)
      taskParams.taskInfo[i].handleEvent=false;
 
    // create game task
    xTaskCreatePinnedToCore(fptr, "GameTask", 2048, NULL, 1, &gameTaskHandle, 0);
  }
  else
  {
    if ( request->arg(F("button")) == "stop" ) {
      curControl= BTN_STOP;
      buttonret = ButtonIn(BTN_STOP);
    }
    
    playerbuttonadd = 0;
   
    if ( request->arg(F("player")) == "2" ) playerbuttonadd = 10;
    if ( request->arg(F("player")) == "3" ) playerbuttonadd = 20;
    if ( request->arg(F("player")) == "4" ) playerbuttonadd = 30;
    if ( request->arg("gb") == F("up") ) buttonret = ButtonIn(BTN_UP + playerbuttonadd);
    if ( request->arg("gb") == F("down") ) buttonret = ButtonIn(BTN_DOWN + playerbuttonadd);
    if ( request->arg("gb") == F("left") ) buttonret = ButtonIn(BTN_LEFT + playerbuttonadd);
    if ( request->arg("gb") == F("right") ) buttonret = ButtonIn(BTN_RIGHT + playerbuttonadd);
    if ( request->arg("gb") == F("middle") ) buttonret = ButtonIn(BTN_MIDDLE + playerbuttonadd);
    
    if ( request->arg(F("level")).length() > 0) 
    {
      gamelevel = request->arg("level").toInt();
      DEBUG_PRINTF("GameLevel: %i\n",gamelevel);
    }
    
    if ( request->arg("size").length() > 0) 
    {
      gamesize = request->arg("size").toInt();
      DEBUG_PRINTF("GameSize: %i\n",gamesize);
    }
    
    if ( request->arg("button") == "back" )
    {
#ifdef myDEBUG
      webreturn += "0#" + String(aktgame) + "#" + String(anzPlayer) + "#" + String(highscore[aktgame]) + "#" + String(aktscore) + "#" + String(debugval);
#else
      webreturn += "0#" + String(aktgame) + "#" + String(anzPlayer) + "#" + String(highscore[aktgame]) + "#" + String(aktscore) + "#0";
#endif

      request->send(200, "text/plain", webreturn); 
      buttonret = ButtonIn(BTN_EXIT);
      curControl= BTN_NONE;
      callRoot(request);
    }
    
    if ( request->arg("poll") == "poll" ) 
    {
       playerret = anzPlayer;
#ifdef DEBUG_WEB
      Serial.println(F("Web Poll"));
      Serial.printf("Debugval: %i\n",debugval);
#endif
    }
    
//################################################################
    if ( request->arg(F("newplayer")) == F("init") ) 
    {  
       if (aktgame == TIERMEMORY || aktgame == MUSIKMEMORY || aktgame == ABBAMEMORY )
       {
         if ( gameisrunning ) 
         {
           playerret = 0; 
           for ( uint8_t pip=0;pip<4;pip++)
           {
             
             if (PlayerIP[pip] == request->client()->remoteIP().toString() )  // Spieler mit der gleichen IP bekommen wieder die gleiche Spielernummer/Farbe
             {
               playerret = pip+1;

               DEBUG_PRINT(F("Player refresh: "));
               DEBUG_PRINTLN(playerret);
               DEBUG_PRINT(F(" IP: "));
               DEBUG_PRINTLN(PlayerIP[pip]);

               if ( random(0,2) == 0 ) break;
             }
           
           }
         }
         else
         {
           if ( anzPlayer < 4 ) 
           {
             PlayerIP[anzPlayer] = request->client()->remoteIP().toString();
             anzPlayer++;            // Spieler hinzu
             playerret = anzPlayer;
  
             DEBUG_PRINT(F("Neuer Player: "));
             DEBUG_PRINTLN(anzPlayer);
             DEBUG_PRINT(F(" IP: "));
             DEBUG_PRINTLN(PlayerIP[anzPlayer-1]);
            }
            else
            {
              playerret = 0;          // kein neuer Spieler mehr möglich
            }
          }
       }
       else
       {
         playerret = 0;              // nur bei Mehrspieler Modus ist ein neuer Spieler möglich
       }
    }

    
    if ( gameisrunning ) 
    {
      webreturn = "1#"; 
    }
    else 
    {
      webreturn = "0#";
    }
#ifdef myDEBUG
    webreturn += String(aktgame) + "#" + String(playerret) + "#" + String(highscore[aktgame]) + "#" + String(aktscore)+ "#" + String(debugval);
#else
    webreturn += String(aktgame) + "#" + String(playerret) + "#" + String(highscore[aktgame]) + "#" + String(aktscore) + "#0";
#endif
    request->send(200, "text/plain", webreturn);
    
  }
}


bool Game::ButtonIn(uint8_t buttonvalue)
{
  //DEBUG_PRINTF("Button write: value: %i, write %i read %i\n", buttonvalue, buttonbuffer.write, buttonbuffer.read);
  uint8_t next = ((buttonbuffer.write + 1) & CURCONTROL_BUFFER_MASK);

  if (buttonbuffer.read == next)
    return false; // voll

  buttonbuffer.data[buttonbuffer.write] = buttonvalue;
  // buttonbuffer.data[buttonbuffer.write & CURCONTROL_BUFFER_MASK] = buttonvalue; // absolut Sicher
  buttonbuffer.write = next;

  return true;
}

bool Game::readButton()
{
  if (buttonbuffer.read == buttonbuffer.write)
  {
    curControl = BTN_NONE;
    return false;
  }
  

  DEBUG_PRINTF("Button read: value: %i, write %i read %i\n", buttonbuffer.data[buttonbuffer.read], buttonbuffer.write, buttonbuffer.read);

  curControl = buttonbuffer.data[buttonbuffer.read];

  buttonbuffer.read = (buttonbuffer.read+1) & CURCONTROL_BUFFER_MASK;

  return true;
}

void Game::ButtonClear()
{
  for(uint8_t i=0; i<CURCONTROL_BUFFER_SIZE; i++) {
    buttonbuffer.data[i]=0;
  }
  buttonbuffer.read = 0;
  buttonbuffer.write = 0;
  curControl= BTN_NONE;
}
