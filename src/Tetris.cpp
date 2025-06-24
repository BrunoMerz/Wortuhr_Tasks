#if defined(WITH_TETRIS)
/* 
 * Main Tetris game
 */

#include "Tetris.h"
#include "Game.h"
#include "LedDriver_FastLED.h"


uint16_t brickSpeed;
uint8_t nbRowsThisLevel;
uint16_t nbRowsTotal;
uint8_t nextbrick = random(7);


boolean tetrisGameOver;

extern void clearField();
extern void newActiveBrick();
extern boolean checkFieldCollisionSound(struct Brick* brick);
extern boolean checkFieldCollision(struct Brick* brick);
extern void addActiveBrickToField();
extern void rotateActiveBrick();
extern void playerControlActiveBrick();
extern void shiftActiveBrick(int dir);
extern void checkFullLines();
extern void printField();


static Game *game = Game::getInstance();
static LedDriver *ledDriver = LedDriver::getInstance();

void tetrisInit(){
#ifdef DEBUG_GAME
  Serial.println(F("Start Tetris"));
#endif
  clearField();
  brickSpeed = INIT_SPEED;
  nbRowsThisLevel = 0;
  nbRowsTotal = 0;
  game->aktscore= 0;
  tetrisGameOver = false;
  pixel_aktiv = 0;
  newActiveBrick();
}

boolean tetrisRunning = false;
void runTetris(void *p) {
  tetrisInit();
  game->gamecount[TETRIS]++;
  game->gameisrunning = true;
  unsigned long prevUpdateTime = 0;
  
  tetrisRunning = true;
  while(tetrisRunning){
    unsigned long curTime;
    do{
      
      delay(5);
      if ( game->readButton() ) {
        if (game->curControl == BTN_STOP || game->curControl == BTN_EXIT){
          tetrisGameOver = true;
          break;
        }
        if (game->curControl != BTN_NONE ){
          playerControlActiveBrick();
          printField();
        }
      }
      if (tetrisGameOver) break;

      curTime = millis();
    } while ((curTime - prevUpdateTime) < brickSpeed); //Zeit bis der nächste Schritt kommt
    prevUpdateTime = curTime;
  
    if (tetrisGameOver){
      tetrisRunning = false;
      game->gameisrunning = false;
      break;
    }
  
    if (activeBrick.enabled){
      shiftActiveBrick(DIR_DOWN);
    } else {
      checkFullLines();
      newActiveBrick();
      prevUpdateTime = millis(); //Reset update time um doppelten Stein move zu verhindern
    }

    if ( checkFieldCollisionSound(&activeBrick) && activeBrick.enabled ) 
    {

    }
    printField();

  }
  // terminate task
  vTaskDelete(NULL);
}


void playerControlActiveBrick(){
  switch(game->curControl){
    case BTN_LEFT:
#ifdef DEBUG_GAME
    Serial.println(F("links(3)"));
#endif  
      shiftActiveBrick(DIR_LEFT);
      break;
    case BTN_RIGHT:
#ifdef DEBUG_GAME
    Serial.println(F("rechts(4)"));
#endif 
      shiftActiveBrick(DIR_RIGHT);
      break;
    case BTN_DOWN:
#ifdef DEBUG_GAME
    Serial.println(F("runter(2)"));
#endif 
      shiftActiveBrick(DIR_DOWN);
      break;
    case BTN_MIDDLE:   
    case BTN_UP:
#ifdef DEBUG_GAME
    Serial.println(F("drehen(1)"));
#endif  
      rotateActiveBrick();
      break;
    case BTN_EXIT:
      tetrisRunning = false;
      break;
  }
}

void printField(){
  int x,y;
  for (x=0;x<FIELD_WIDTH;x++){
    for (y=0;y<FIELD_HEIGHT;y++){
      uint8_t activeBrickPix = 0;
      if (activeBrick.enabled){ //Nur Stein wenn er aktiv ist
        //Prüfen ob Stein sichtbar ist.
        if ((x>=activeBrick.xpos) && (x<(activeBrick.xpos+(activeBrick.siz)))
            && (y>=activeBrick.ypos) && (y<(activeBrick.ypos+(activeBrick.siz)))){
          activeBrickPix = (activeBrick.pix)[x-activeBrick.xpos][y-activeBrick.ypos];
        }
      }
      if (field.pix[x][y] == 1){
       ledDriver->setPixelRGB(x,y,field.color[x][y]);
      } else if (activeBrickPix == 1){
          ledDriver->setPixelRGB(x,y,activeBrick.color);
      } else {
         ledDriver->setPixelRGB(x,y,CRGB::Black);  // Pixel löschen!
      }
    }
  }
  switch (nextbrick) {
    case 0:
      ledDriver->setPixelRGB(110,colorArray[BLUE]);
      ledDriver->setPixelRGB(111,colorArray[BLUE]);
      ledDriver->setPixelRGB(112,colorArray[BLUE]);
      ledDriver->setPixelRGB(113,colorArray[BLUE]);
    break;
    case 1:
      ledDriver->setPixelRGB(110,CRGB::Black);
      ledDriver->setPixelRGB(111,CRGB::Black);
      ledDriver->setPixelRGB(112,colorArray[RED]);
      ledDriver->setPixelRGB(113,colorArray[RED]);
    break;
    case 2:
      ledDriver->setPixelRGB(110,colorArray[YELLOW]);
      ledDriver->setPixelRGB(111,CRGB::Black);
      ledDriver->setPixelRGB(112,colorArray[YELLOW]);
      ledDriver->setPixelRGB(113,colorArray[YELLOW]);
    break;
    case 3:
      ledDriver->setPixelRGB(110,CRGB::Black);
      ledDriver->setPixelRGB(111,colorArray[MAGENTA]);
      ledDriver->setPixelRGB(112,colorArray[MAGENTA]);
      ledDriver->setPixelRGB(113,colorArray[MAGENTA]);
    break;
    case 4:
      ledDriver->setPixelRGB(110,colorArray[GREEN]);
      ledDriver->setPixelRGB(111,CRGB::Black);
      ledDriver->setPixelRGB(112,CRGB::Black);
      ledDriver->setPixelRGB(113,colorArray[GREEN]);
    break;
    case 5:
      ledDriver->setPixelRGB(110,CRGB::Black);
      ledDriver->setPixelRGB(111,colorArray[ORANGE]);
      ledDriver->setPixelRGB(112,CRGB::Black);
      ledDriver->setPixelRGB(113,colorArray[ORANGE]);
    break;
    case 6:
      ledDriver->setPixelRGB(110,colorArray[CYAN]);
      ledDriver->setPixelRGB(111,CRGB::Black);
      ledDriver->setPixelRGB(112,colorArray[CYAN]);
      ledDriver->setPixelRGB(113,CRGB::Black);
    break;
  }
  
  ledDriver->show();
}

/* *** Game functions *** */

void newActiveBrick(){
//  uint8_t selectedBrick = 3;

  uint8_t selectedBrick = nextbrick;
  nextbrick = random(7);
  game->aktscore++;
  pixel_aktiv = pixel_aktiv + MAX_BRICK_SIZE;
  if ( game->aktscore > game->highscore[TETRIS] ) game->highscore[TETRIS] = game->aktscore;
#ifdef DEBUG_GAME
  Serial.printf("neuer Stein: %i\n",selectedBrick);
#endif 
  //Set properties of brick
  activeBrick.siz = brickLib[selectedBrick].siz;
  activeBrick.yOffset = brickLib[selectedBrick].yOffset;
  activeBrick.xpos = FIELD_WIDTH/2 - activeBrick.siz/2;
  activeBrick.ypos = BRICKOFFSET-activeBrick.yOffset;
  activeBrick.enabled = true;
  
  //Set color of brick
  activeBrick.color = brickLib[selectedBrick].color;
  //activeBrick.color = colorLib[1];
  
  //Copy pix array of selected Brick
  uint8_t x,y;
  for (y=0;y<MAX_BRICK_SIZE;y++){
    for (x=0;x<MAX_BRICK_SIZE;x++){
      activeBrick.pix[x][y] = (brickLib[selectedBrick]).pix[x][y];
    }
  }
  
  //Check collision, if already, then game is over
  if (checkFieldCollision(&activeBrick)){
    tetrisGameOver = true;
  }
}

//Check collision between bricks in the field and the specified brick
boolean checkFieldCollision(struct Brick* brick){
  uint8_t bx,by;
  uint8_t fx,fy;
  for (by=0;by<MAX_BRICK_SIZE;by++){
    for (bx=0;bx<MAX_BRICK_SIZE;bx++){
      fx = (*brick).xpos + bx;
      fy = (*brick).ypos + by;
      if (( (*brick).pix[bx][by] == 1) 
            && ( field.pix[fx][fy] == 1)){
        return true;
      }
    }
  }
  return false;
}

boolean checkFieldCollisionSound(struct Brick* brick){
  uint8_t bx,by;
  uint8_t fx,fy;
  for (by=0;by<MAX_BRICK_SIZE;by++){
    for (bx=0;bx<MAX_BRICK_SIZE;bx++){
      fx = (*brick).xpos + bx;
      fy = (*brick).ypos + by + 1;
      if (( (*brick).pix[bx][by] == 1) 
            && ( field.pix[fx][fy] == 1)){
        return true;
      }
    }
  }
  return false;
}

//Check Kollision zwischen Stein und den Seiten
boolean checkSidesCollision(struct Brick* brick){
  //Check vertical collision with sides of field
  uint8_t bx,by;
  uint8_t fx,fy;
  for (by=0;by<MAX_BRICK_SIZE;by++){
    for (bx=0;bx<MAX_BRICK_SIZE;bx++){
      if ( (*brick).pix[bx][by] == 1){
        fx = (*brick).xpos + bx; //Ermitteln der aktiven Position des Steins
        fy = (*brick).ypos + by;
        if (fx<0 || fx>=FIELD_WIDTH){
          return true;
        }
      }
    }
  }
  return false;
}

Brick tmpBrick;

void rotateActiveBrick(){
  // Kopiere den aktiven Stein 
  uint8_t x,y;
  for (y=0;y<MAX_BRICK_SIZE;y++){
    for (x=0;x<MAX_BRICK_SIZE;x++){
      tmpBrick.pix[x][y] = activeBrick.pix[x][y];
    }
  }
  tmpBrick.xpos = activeBrick.xpos;
  tmpBrick.ypos = activeBrick.ypos;
  tmpBrick.siz = activeBrick.siz;
  
  // Abhänging von der Steingröße wird anders gedreht
  if (activeBrick.siz == 3){
    //Rotation um den Zentralen Pixel
    tmpBrick.pix[0][0] = activeBrick.pix[0][2];
    tmpBrick.pix[0][1] = activeBrick.pix[1][2];
    tmpBrick.pix[0][2] = activeBrick.pix[2][2];
    tmpBrick.pix[1][0] = activeBrick.pix[0][1];
    tmpBrick.pix[1][1] = activeBrick.pix[1][1];
    tmpBrick.pix[1][2] = activeBrick.pix[2][1];
    tmpBrick.pix[2][0] = activeBrick.pix[0][0];
    tmpBrick.pix[2][1] = activeBrick.pix[1][0];
    tmpBrick.pix[2][2] = activeBrick.pix[2][0];
    // Alle anderen Teile leeren
    tmpBrick.pix[0][3] = 0;
    tmpBrick.pix[1][3] = 0;
    tmpBrick.pix[2][3] = 0;
    tmpBrick.pix[3][3] = 0;
    tmpBrick.pix[3][2] = 0;
    tmpBrick.pix[3][1] = 0;
    tmpBrick.pix[3][0] = 0;
    
  } else if (activeBrick.siz == 4){
    // Rotation um Kreuz
    tmpBrick.pix[0][0] = activeBrick.pix[0][3];
    tmpBrick.pix[0][1] = activeBrick.pix[1][3];
    tmpBrick.pix[0][2] = activeBrick.pix[2][3];
    tmpBrick.pix[0][3] = activeBrick.pix[3][3];
    tmpBrick.pix[1][0] = activeBrick.pix[0][2];
    tmpBrick.pix[1][1] = activeBrick.pix[1][2];
    tmpBrick.pix[1][2] = activeBrick.pix[2][2];
    tmpBrick.pix[1][3] = activeBrick.pix[3][2];
    tmpBrick.pix[2][0] = activeBrick.pix[0][1];
    tmpBrick.pix[2][1] = activeBrick.pix[1][1];
    tmpBrick.pix[2][2] = activeBrick.pix[2][1];
    tmpBrick.pix[2][3] = activeBrick.pix[3][1];
    tmpBrick.pix[3][0] = activeBrick.pix[0][0];
    tmpBrick.pix[3][1] = activeBrick.pix[1][0];
    tmpBrick.pix[3][2] = activeBrick.pix[2][0];
    tmpBrick.pix[3][3] = activeBrick.pix[3][0];
  } else {
    Serial.println("Brick size error");
  }
  
  //Prüfen der Kollisionen
  // Möglichkeiten:
  // - Stein klebt außerhalb
  // - Stein klebt innerhalb
  // Bei Kollision ist kein drehen mehr möglich
  if ((!checkSidesCollision(&tmpBrick)) && (!checkFieldCollision(&tmpBrick))){
    // Kopiere den Temp. Stein zurück
    for (y=0;y<MAX_BRICK_SIZE;y++){
      for (x=0;x<MAX_BRICK_SIZE;x++){
        activeBrick.pix[x][y] = tmpBrick.pix[x][y];
      }
    }
  }
}

// Verschiebe Stein links/rechts/runter (wenn möglich)
void shiftActiveBrick(int dir){
  // Ändere die Position des Steins
  if (dir == DIR_LEFT){
    activeBrick.xpos--;
  } else if (dir == DIR_RIGHT){
    activeBrick.xpos++;
  } else if (dir == DIR_DOWN){
    activeBrick.ypos++;
  }
  
  // Prüfen der Stein Postition
  // 2 Möglichkeiten bei Kollision
  // - Richtung Links/Rechts Postion zurücksetzen
  // - Richtung Runter, Position zurücksetzen und Stein festsetzen
  // Wenn keine Kollision, Stein Position unverändert
  if ((checkSidesCollision(&activeBrick)) || (checkFieldCollision(&activeBrick))){
    //Serial.println("coll");
    if (dir == DIR_LEFT){
      activeBrick.xpos++;
    } else if (dir == DIR_RIGHT){
      activeBrick.xpos--;
    } else if (dir == DIR_DOWN){
      activeBrick.ypos--;//gehe eins zurück nach oben
      addActiveBrickToField();
      activeBrick.enabled = false;//Disable brick, Stein ist nicht mehr zu bewegen!
    }
  }
}

// Kopiere den aktiven Stein ins Feld
void addActiveBrickToField(){
  uint8_t bx,by;
  uint8_t fx,fy;
  for (by=0;by<MAX_BRICK_SIZE;by++){
    for (bx=0;bx<MAX_BRICK_SIZE;bx++){
      fx = activeBrick.xpos + bx;
      fy = activeBrick.ypos + by;
      
      if (fx>=0 && fy>=0 && fx<FIELD_WIDTH && fy<FIELD_HEIGHT && activeBrick.pix[bx][by]){//check ob innerhalb vom Feld
        //field.pix[fx][fy] = field.pix[fx][fy] || activeBrick.pix[bx][by];
        field.pix[fx][fy] = activeBrick.pix[bx][by];
        field.color[fx][fy] = activeBrick.color;
      }
    }
  }
}

//Bewege alle Pixel ab. Startzeile wird überschrieben
void moveFieldDownOne(uint8_t startRow){
  if (startRow == 0){//Topmost row has nothing on top to move...
    return;
  }
  uint8_t x,y;
  for (y=startRow-1; y>0; y--){
    for (x=0;x<FIELD_WIDTH; x++){
      field.pix[x][y+1] = field.pix[x][y];
      field.color[x][y+1] = field.color[x][y];
    }
  }
}

void checkFullLines(){
  int x,y;
  int minY = 0;
  for (y=(FIELD_HEIGHT-1); y>=minY; y--){
    uint8_t rowSum = 0;
    for (x=0; x<FIELD_WIDTH; x++){
      rowSum = rowSum + (field.pix[x][y]);
    }
    if (rowSum>=FIELD_WIDTH){
       // Ganze Zeile gefunden 
      pixel_aktiv = pixel_aktiv - FIELD_WIDTH;
     
      delay(200);
      //starte Löschanimation
      for (x=0;x<FIELD_WIDTH; x++){
        field.pix[x][y] = 0;
        printField();
        delay(100);
      }

      // Bewege alle Zeilen oberhalb eins runter
      delay(450);
      moveFieldDownOne(y);
      y++; minY++;
      printField();
      
      nbRowsThisLevel++; nbRowsTotal++;
      if (nbRowsThisLevel >= LEVELUP){
        nbRowsThisLevel = 0;
        brickSpeed = brickSpeed - SPEED_STEP;
        if (brickSpeed<200){
          brickSpeed = 200;
        }
      }
    }
  }
}

void clearField(){
  uint8_t x,y;
  for (y=0;y<FIELD_HEIGHT;y++){
    for (x=0;x<FIELD_WIDTH;x++){
      field.pix[x][y] = 0;
      field.color[x][y] = 0;
    }
  }
  for (x=0;x<FIELD_WIDTH;x++){// Die letzte Zeile ist nur virtuelle und wird zum Kollisionscheck benötigt
    field.pix[x][FIELD_HEIGHT] = 1; 
  }
}
#endif