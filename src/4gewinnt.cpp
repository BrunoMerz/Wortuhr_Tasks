#if defined(WITH_4GEWINNT)
/* 
  * 4 gewinnt
*/
#include "Game.h"
#include "LedDriver_FastLED.h"

#define VG_WIDTH 9
#define VG_HEIGHT 8
#define SPIELER_A_WINS  1000000
#define SPIELER_B_WINS -1000000
#define SPIELER_A 1
#define SPIELER_B -1
#define LEERES_FELD 0
#define STEINGESCHW VG_HEIGHT*VG_HEIGHT*4



uint8_t g_maxDepth = 5;
uint8_t g_maxLDepth = 0;
//int8_t board[VG_HEIGHT+1][VG_WIDTH] = {0};
boolean ViergewinntRunning;
unsigned long steinzeit;  //:-)
int8_t g_4g_posy = 0;
int8_t g_4g_posymax = 0;
int8_t g_4g_eingeworfen;
bool B_wirfStein = false;
bool A_wirfStein = false;

const int8_t pruefrichtung[4][4][2] =
{
  { { 0,0 } ,{ 0,1 }, {0,2}, {0,3}},
  { { 0,0 } ,{ 1,1 }, {2,2}, {3,3}},
  { { 0,0 } ,{ 1,-1 }, {2,-2}, {3,-3}},
  { { 0,0 } ,{ 1,0 }, {2,0}, {3,0}}
};

extern int ScoreBoard(int8_t scores[][9]);
extern int8_t dropDisk(int8_t board[][9], int8_t column, int8_t spieler);
extern void markiereGewinnSteine();
extern void abMinimax(int maximizeOrMinimize, int8_t spieler, uint8_t depth, int8_t board[][9], int8_t* move, int* score );
extern int8_t warteAufEinwurf();
extern bool wirfStein(int8_t &posx, int8_t w_spieler);
extern void ShowBoard();
extern void leereBoard();
extern void BoardInit();
extern bool innerhalb(int y, int x);
extern int8_t freieSpalteWahl(int8_t pos, bool lr, int8_t spieler);



/*
extern int8_t board[9][9];
extern uint8_t abcBrightness;
extern bool gameisrunning;
extern uint8_t curControl;
extern uint16_t highscore[9];
extern uint8_t gamecount[9];
extern uint8_t gamelevel;
extern uint8_t gamesize;
extern uint8_t lastMinute;
extern int gamesound;
extern uint8_t PABStatus;
extern int AUDIO_FILENR;
extern int ANSAGEBASE;
*/

static Game *game = Game::getInstance();
static LedDriver *ledDriver = LedDriver::getInstance();

void ViergewinntInit(){
  #ifdef DEBUG_GAME
    Serial.println(F("Start 4gewinnt"));
    Serial.println(game->abcBrightness);
  #endif
  BoardInit();
  game->ButtonClear();
  g_maxDepth = game->gamelevel + 3;
  game->gameisrunning = true;
}


void runViergewinnt(void *p)
{
  ViergewinntInit();
  uint8_t l_01_spieler = random (0,2);
  game->gamecount[VIERGEWINNT]++;
  ViergewinntRunning = true;
  ShowBoard();
  bool firstmove = true;
  
  while(ViergewinntRunning)
  {        
    if ( l_01_spieler ) 
    {
      g_4g_eingeworfen = warteAufEinwurf();
      if ( g_4g_eingeworfen == -1 ) 
      {
        #ifdef DEBUG_GAME
          Serial.println(F("Unendschieden!"));
        #endif    
        ViergewinntRunning = false;
        delay(200);
        #ifdef WITH_AUDIO
          AUDIO_FILENR = ANSAGEBASE + 198;   // unendschieden
          if (gamesound) Play_MP3(AUDIO_FILENR,false,33*gamesound);
        #endif
        for ( uint8_t d=0; d<10;d++)
        {
          delay(500);
        }
        leereBoard();
        break;
      }
      g_maxDepth = game->gamelevel + 3;
      
      #ifdef DEBUG_GAME
        Serial.print(F("Spieler wirft in Spalte "));
        Serial.println(g_4g_eingeworfen + 1 );
        Serial.printf("gamelevel: %i \n",gamelevel );
      #endif
      if ( !ViergewinntRunning ) break;
      g_4g_posymax = dropDisk(game->board, g_4g_eingeworfen, SPIELER_B);
      g_4g_posy = 0;
      steinzeit = millis() + STEINGESCHW - 4*g_4g_posy*g_4g_posy;
      B_wirfStein = wirfStein(g_4g_eingeworfen, SPIELER_B);
      //      ShowBoard();
    }
    if ( g_maxDepth > 5 ) g_maxLDepth = g_maxDepth-1; else g_maxLDepth = g_maxDepth;
    int scoreOrig = ScoreBoard(game->board);
    if (scoreOrig == SPIELER_B_WINS) 
    { 
      if ( B_wirfStein ) while ( wirfStein(g_4g_eingeworfen, SPIELER_B));
      #ifdef DEBUG_GAME
        Serial.println(F("Du hast 4gewinnt gewonnen!\n"));
      #endif
      #ifdef WITH_AUDIO
        AUDIO_FILENR = ANSAGEBASE + 192 + random(0,2);   // ich verloren/du gewonnen
        if (gamesound) Play_MP3(AUDIO_FILENR,false,33*gamesound);
      #endif
      markiereGewinnSteine();
      if ( game->highscore[VIERGEWINNT] < 999 ) game->highscore[VIERGEWINNT]++;
      break;
    }
    else 
    {
      int8_t move;
      int score;
      if ( firstmove ) 
      {
        firstmove = false;
        abMinimax(1,SPIELER_A,3,game->board,&move,&score);
      }
      else
      abMinimax(1,SPIELER_A,g_maxDepth,game->board,&move,&score);
      if ( B_wirfStein )
      while ( wirfStein(g_4g_eingeworfen, SPIELER_B));
      ShowBoard();
      if (move != -1) 
      {
        #ifdef DEBUG_GAME
          Serial.printf("ESP wirft in Spalte: %d\n",move + 1);
        #endif
        g_4g_posymax = dropDisk(game->board, move, SPIELER_A);
        g_4g_posy = 0;
        steinzeit =millis() + STEINGESCHW - 4*g_4g_posy*g_4g_posy;
        A_wirfStein = wirfStein(move, SPIELER_A);
        if ( A_wirfStein ) while ( wirfStein(move, SPIELER_A));
        ShowBoard();         
        scoreOrig = ScoreBoard(game->board);
        if (scoreOrig == SPIELER_A_WINS) 
        { 
          #ifdef DEBUG_GAME
            Serial.println(F("Ich habe 4gewinnt gewonnen!"));
          #endif
          #ifdef WITH_AUDIO
            AUDIO_FILENR = ANSAGEBASE + 190 + random(0,2);   // ich gewonnen/du verloren
            if (gamesound) Play_MP3(AUDIO_FILENR,false,33*gamesound);
          #endif
          markiereGewinnSteine();
          if ( game->highscore[VIERGEWINNT] > 0 ) game->highscore[VIERGEWINNT]--;
          break;
        }
      } 
      else 
      {
        #ifdef DEBUG_GAME
          Serial.println(F("Unendschieden!"));
        #endif     
        ViergewinntRunning = false;
        delay(200);
        #ifdef WITH_AUDIO
          AUDIO_FILENR = ANSAGEBASE + 198;   // unendschieden
          if (gamesound) Play_MP3(AUDIO_FILENR,false,33*gamesound);
        #endif
        for ( uint8_t d=0; d<10;d++)
        {
          delay(500);
        }
        leereBoard();
        break;
      }
    }
    l_01_spieler = 1;
  }
  // terminate task
  vTaskDelete(NULL);
}


int ScoreBoard(int8_t scores[][VG_WIDTH])
{
    int counters[9] = {0,0,0,0,0,0,0,0,0};
    int x,y;
  
    // Horizontal spans
    for(y=0; y<VG_HEIGHT; y++) {
        int score = scores[y][0] + scores[y][1] + scores[y][2];
        for(x=3; x<VG_WIDTH; x++) {
            score += scores[y][x];
            counters[score+4]++;
            score -= scores[y][x-3];
    }
  }
    // Vertical spans
    for(x=0; x<VG_WIDTH; x++) {
        int score = scores[0][x] + scores[1][x] + scores[2][x];
        for(y=3; y<VG_HEIGHT; y++) {
            score += scores[y][x];
            counters[score+4]++;
            score -= scores[y-3][x];
    }
  }
    // Down-right (and up-left) diagonals
    for(y=0; y<VG_HEIGHT-3; y++) {
        for(x=0; x<VG_WIDTH-3; x++) {
            int score=0, idx=0;
            for(idx=0; idx<4; idx++) {
                score += scores[y+idx][x+idx];
      }
            counters[score+4]++;
    }
  }
    // up-right (and down-left) diagonals
    for(y=3; y<VG_HEIGHT; y++) {
        for(x=0; x<VG_WIDTH-3; x++) {
            int score=0, idx=0;
            for(idx=0; idx<4; idx++) {
                score += scores[y-idx][x+idx];
      }
            counters[score+4]++;
    }
  }
    if (counters[0] != 0)
  return SPIELER_B_WINS;
    else if (counters[8] != 0)
  return SPIELER_A_WINS;
    else
  return
  counters[5] + 2*counters[6] + 5*counters[7] -
  counters[3] - 2*counters[2] - 5*counters[1];
}


int8_t dropDisk(int8_t board[][VG_WIDTH], int8_t column, int8_t spieler)
{
    int8_t y;
    for (y=VG_HEIGHT-1; y>=0; y--)
  if (board[y][column] == LEERES_FELD) {
    board[y][column] = spieler;
    return y;
  }
    return -1;
}

void abMinimax(int maximizeOrMinimize, int8_t spieler, uint8_t depth, int8_t board[][VG_WIDTH], int8_t* move, int* score )
{
    int bestScore=maximizeOrMinimize?-10000000:10000000;
    int8_t bestMove=-1, column;
    for (column=0; column<VG_WIDTH; column++) 
    {
      if (board[0][column]!=LEERES_FELD) continue;
      int8_t rowFilled = dropDisk(board, column, spieler);
      if (rowFilled == -1) continue;
      int s = ScoreBoard(board);
      if (s == (maximizeOrMinimize?SPIELER_A_WINS:SPIELER_B_WINS)) 
      {
        bestMove = column;
        bestScore = s;
        board[rowFilled][column] = LEERES_FELD;
        break;
      }
      int8_t moveInner;
      int scoreInner;
      if (depth>1)
        abMinimax(!maximizeOrMinimize, spieler==SPIELER_A?SPIELER_B:SPIELER_A, depth-1, board, &moveInner, &scoreInner);
      else 
      {
        moveInner = -1;
        scoreInner = s;
      }
      board[rowFilled][column] = LEERES_FELD;
        
      if (scoreInner == SPIELER_A_WINS || scoreInner == SPIELER_B_WINS) scoreInner -= depth * (int)spieler;
      if (depth == g_maxDepth-1 && B_wirfStein ) B_wirfStein = wirfStein(g_4g_eingeworfen, spieler);
    
      if (depth == g_maxLDepth && !B_wirfStein )
      {
        if ( column % 4 == 0 ) ledDriver->setPixel(110,colorArray[RED],game->abcBrightness); else ledDriver->setPixel(110,colorArray[RED],0);
        if ( column % 4 == 1 ) ledDriver->setPixel(111,colorArray[RED],game->abcBrightness); else ledDriver->setPixel(111,colorArray[RED],0);
        if ( column % 4 == 2 ) ledDriver->setPixel(112,colorArray[RED],game->abcBrightness); else ledDriver->setPixel(112,colorArray[RED],0);
        if ( column % 4 == 3 ) ledDriver->setPixel(113,colorArray[RED],game->abcBrightness); else ledDriver->setPixel(113,colorArray[RED],0);
        ledDriver->show(); 
        delay(0);     
      }
        
      #ifdef WITH_AUDIO
        if ( depth == g_maxDepth && column == 4 && !B_wirfStein && g_4g_posymax > 2) 
        {
          AUDIO_FILENR = ANSAGEBASE + 196 + random(0,2);   // ich überlege / was mach ich als nächstes
          if (gamesound) Play_MP3(AUDIO_FILENR,false,33*gamesound);
        }
      #endif    
      if (depth == g_maxDepth)
      {
        #ifdef DEBUG_GAME
          printf("Depth %d, placing on %d, score:%d MaxFree %d Heap %d\n", depth, column+1, scoreInner, ESP.getMaxFreeBlockSize() , ESP.getFreeHeap());
        #endif
      }
      
      if (maximizeOrMinimize) 
      {
        if (scoreInner==bestScore && (random(0,2) == 1 || bestMove == -1) ) 
        {
          bestMove = column;
        }
        if (scoreInner>bestScore) 
        {
          bestScore = scoreInner;
          bestMove = column;
        }
      } else 
      {
        if (scoreInner==bestScore && (random(0,2) == 1 || bestMove == -1) ) 
        {
          bestMove = column;
        }
        if (scoreInner<bestScore) 
        {
          bestScore = scoreInner;
          bestMove = column;
        }
      }
    }
    *move = bestMove;
    *score = bestScore;
}

int8_t warteAufEinwurf()
{
  int8_t pos = 0;
  bool wae = true;
  uint8_t dbdw = 1;

  delay(10);
  game->ButtonClear();
  if ( !game->gameisrunning )
  {
    for (int8_t j = 1; j < VG_WIDTH ; j++)
    {
      ledDriver->setPixelRGB(j,0,0,0,0);  // Eingabespalte löschen
    }
    ledDriver->show();
  }
  else
  {
    pos = freieSpalteWahl(-1,true,SPIELER_B);
    if ( pos == -1) return -1; //alles voll
  }
  
  steinzeit = millis(); // wird hier für die Ansage "du bist dran" benutzt.
  do {

    if (game->curControl == BTN_STOP || game->curControl == BTN_EXIT){
      ViergewinntRunning = false;
      game->gameisrunning = false;
      wae = false;
      break;
    }
    if ( game->readButton() )
    {
      if (game->curControl != BTN_NONE && game->gameisrunning )
      {
        if ( game->curControl == BTN_RIGHT ) pos=freieSpalteWahl(pos,true,SPIELER_B);
        if ( game->curControl == BTN_LEFT )  pos=freieSpalteWahl(pos,false,SPIELER_B);
        if ( game->curControl == BTN_DOWN ) wae = false;
        
        steinzeit = millis()+dbdw*5000;
        dbdw++;
      }
    }
    #ifdef WITH_AUDIO
      if ( millis() > steinzeit + 10000 )
      {
        AUDIO_FILENR = ANSAGEBASE + 195;   // du bist dran
        if (gamesound) Play_MP3(AUDIO_FILENR,false,33*gamesound);
        steinzeit = millis()+ dbdw*5000;
        dbdw++;
        if ( dbdw > 250 ) dbdw = 250;
      }
    #endif

    delay(10);            
  } while ( wae );
  return pos;
}

int8_t freieSpalteWahl(int8_t pos, bool lr, int8_t spieler)
{
  int8_t retval = -1;
  for (int8_t j=0; j<VG_WIDTH; j++) {
    if (game->board[0][j]==LEERES_FELD)
    {
      retval = 0;
      break;
    }
  }
  if ( retval == -1 ) return retval;
  
  retval = pos;
  
  if ( lr ) 
  {
    if ( pos == VG_WIDTH-1) retval = pos;
    for (int8_t j = pos+1; j < VG_WIDTH ; j++)
    {
      ledDriver->setPixelRGB(j,0,0,0,0);
      if (game->board[0][j] == LEERES_FELD) 
      {
        retval = j;
        break;
      }
    }
  }
  else
  {
    if ( pos == 0) retval = pos;
    for (int8_t j = pos-1; j >= 0 ; j--)
    {
      ledDriver->setPixelRGB(j+2,0,0,0,0);
      if (game->board[0][j] == LEERES_FELD) 
      {
        retval = j;
        break;
      }
    }
  }
  if ( spieler == SPIELER_A ) ledDriver->setPixel(retval+1,0,colorArray[RED],game->abcBrightness);
  if ( spieler == SPIELER_B ) ledDriver->setPixel(retval+1,0,colorArray[YELLOW],game->abcBrightness);  
  ledDriver->show();
  return retval;
}



bool wirfStein(int8_t &posx, int8_t w_spieler)
{
  if ( millis() < steinzeit) return true; 
  
  steinzeit = millis() + STEINGESCHW - 4*g_4g_posy*g_4g_posy;
  //  printf("posx %i, g_4g_posy %i, g_4g_posymax: %i \n", posx,g_4g_posy,g_4g_posymax);
  if ( g_4g_posy <= g_4g_posymax )
  {
    if ( g_4g_posy == 0 ) ledDriver->setPixelRGB(posx+1,g_4g_posy,0,0,0);
    else ledDriver->setPixel(posx+1,g_4g_posy,colorArray[BLUE],game->abcBrightness/5);
    g_4g_posy++;
    if ( w_spieler == SPIELER_A ) ledDriver->setPixel(posx+1,g_4g_posy,colorArray[RED],game->abcBrightness);
    if ( w_spieler == SPIELER_B ) ledDriver->setPixel(posx+1,g_4g_posy,colorArray[YELLOW],game->abcBrightness);
    ledDriver->show(); 
    delay(10);
    return true;
  }
  else
  {
    return false;
  }
}

void markiereGewinnSteine()
{
  int8_t wert = 0;
  int8_t pruefx;
  int8_t pruefy;
  int8_t ind=0; 
  uint8_t gewinnerfarbe;
  struct GewinnStartSteine {
    int8_t x;
    int8_t y; 
    uint8_t r; 
  };
  
  GewinnStartSteine gewinnstartsteine[6]{};
  #ifdef DEBUG_GAME
    Serial.println(F("GewinnSteine:"));
  #endif
  for(uint8_t x=0;x<VG_WIDTH;x++)
  {
    for(uint8_t y=0;y<VG_HEIGHT;y++)
    { 
      for( uint8_t r=0;r<4;r++ )          // Prüfe 4 Richtungen
      {
        wert = 0;
        for( uint8_t i=0;i<4;i++ )        // Jeweils die nächsten 3 Nachbarn
        {
          pruefx = x+pruefrichtung[r][i][0];
          pruefy = y+pruefrichtung[r][i][1];
          if ( innerhalb(pruefy,pruefx )) wert = wert + game->board[pruefy][pruefx];
        }
        //        Serial.printf("x: %i y: %i r: %i wert: %i\n",x,y,r,wert);
        if ( wert == 4 || wert == -4) 
        {
          if ( wert > 0 ) gewinnerfarbe = RED; else gewinnerfarbe = YELLOW;
          #ifdef DEBUG_GAME
            Serial.printf("Index: %i X: %i , Y: %i, R: %i\n",ind,x,y,r);
          #endif
          gewinnstartsteine[ind].x = x;
          gewinnstartsteine[ind].y = y;
          gewinnstartsteine[ind].r = r;
          if ( ind < 5 ) ind++;
        }
      }
    }
  }

  for ( uint8_t bl=0; bl < 50;bl++)
  {
    for ( uint8_t g=0; g < ind;g++)
    {
      for( uint8_t i=0;i<4;i++ )
      {
        pruefx = gewinnstartsteine[g].x + pruefrichtung[gewinnstartsteine[g].r][i][0];
        pruefy = gewinnstartsteine[g].y + pruefrichtung[gewinnstartsteine[g].r][i][1];
        if ( bl%2 == 0 ) ledDriver->setPixel(pruefx+1,pruefy+1,colorArray[gewinnerfarbe],10);
        else ledDriver->setPixel(pruefx+1,pruefy+1,colorArray[gewinnerfarbe],game->abcBrightness);
      }
    }
    ledDriver->show();
    delay (120);
  }

  delay (100);
  game->ButtonClear();
  leereBoard();
  ViergewinntRunning = false;
}

bool innerhalb(int y, int x)
{
    return y>=0 && y<VG_HEIGHT && x>=0 && x<VG_WIDTH;
}

void ShowBoard()
{
  // Hintergrund magenta
  uint8_t rahmen = MAGENTA;
  if ( g_maxDepth <5 ) rahmen = GREEN;
  else if ( g_maxDepth >5 ) rahmen = ORANGE;
  ledDriver->clear();
  for(uint8_t x=0;x<FIELD_WIDTH;x++)
  {
    for(uint8_t y=1;y<FIELD_HEIGHT;y++)
    {
      ledDriver->setPixel(x,y,colorArray[rahmen],game->abcBrightness/4);
    }
  }
  
  //board
  for (uint8_t i = 0; i < VG_HEIGHT ; i++){
    for (uint8_t j = 0; j < VG_WIDTH ; j++){
      if (game->board[i][j] == LEERES_FELD)
      ledDriver->setPixel(j+1,i+1,colorArray[BLUE],game->abcBrightness/5);
      if (game->board[i][j] == SPIELER_A)
      ledDriver->setPixel(j+1,i+1,colorArray[RED],game->abcBrightness);
      if (game->board[i][j] == SPIELER_B)
      ledDriver->setPixel(j+1,i+1,colorArray[YELLOW],game->abcBrightness);
    }
  }  
  ledDriver->show();
}

// Leert Board nach unten
void leereBoard()
{
   for ( int d=0;d<VG_HEIGHT;d++)
   {
     for (int y=VG_HEIGHT-1;y>=d;y--)
     { 
       if ( y > d) 
       { 
         for ( int x=0;x<VG_WIDTH;x++)
         {
           game->board[y][x] = game->board[y-1][x];
         }
       }
       else
       {
         for ( int x=0;x<VG_WIDTH;x++)
         {
           game->board[y][x] = LEERES_FELD;
         }
       }
     }
     ShowBoard();
     delay (VG_HEIGHT*20-d*15);
   }
}

void BoardInit()
{
  for (uint8_t i = 0; i < VG_HEIGHT ; i++)
  {
    for (uint8_t j = 0; j < VG_WIDTH ; j++)
    {
      game->board[i][j] = 0;
    }
  }  
}
#endif