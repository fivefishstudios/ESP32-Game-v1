#include <Arduino.h>
#include <TFT_eSPI.h>                 // Include the graphics library (this includes the sprite functions)

// TFT specs
#define TFT_WIDTH  240
#define TFT_HEIGHT 320 

#define BKGD_COLOR TFT_BLACK
#define MARGIN_TOP 20

#define STEP_ITERATIONS 8   // only used during simulations

#define MIN_VELOCITY 2      // of enemy ships
#define MAX_VELOCITY 6      // of enemy ships
#define NUM_FIGHTERS 10     // current number of enemy ships

#define FIRE_LASER_PIN 26   // pin # for push button digital in
int LaserFireCounter;       // used by IRQ for Laser fire push button 


TFT_eSPI    tft = TFT_eSPI();            // Create object "tft"
TFT_eSprite buffer = TFT_eSprite(&tft);  // Create Sprite object "img" with pointer to "tft" object
TFT_eSprite enemy = TFT_eSprite(&tft);   // tie-fighter


// ---------- OBJECTS --------------
class TieFighter {
  public:
    int x;
    int y;
    int velocity_x;
    int velocity_y;
    int vector;   // ???
    void init();
    void move();
    // destructor
    // ????? 
  private:
    int direction;
};

void TieFighter::init(){
  x = random(1, TFT_WIDTH);
  y = random(1, TFT_HEIGHT);
  velocity_x = random(1,5);
  velocity_y = random(1,5);
  vector = 0;
}

void TieFighter::move(){
  x = x + velocity_x;
  y = y + velocity_y;
  
  if (y>TFT_HEIGHT){
    y = random(1, TFT_HEIGHT);
    velocity_y = random(MIN_VELOCITY, MAX_VELOCITY);
    if (random(1,3)==1){
      velocity_y = velocity_y * (-1);
    }
  }
  if (y<0){
    y = random(1, TFT_HEIGHT);
    velocity_y = random(MIN_VELOCITY, MAX_VELOCITY);
    if (random(1,3)==1){
      velocity_y = velocity_y * (-1);
    }
  }
  if (x>TFT_WIDTH){
    x = random(1, TFT_WIDTH);
    velocity_x = random(MIN_VELOCITY, MAX_VELOCITY);
    if (random(1,3)==1){
      velocity_x = velocity_x * (-1);
    }
  }
  if (x<0){
    x = random(1, TFT_WIDTH);
    velocity_x = random(MIN_VELOCITY, MAX_VELOCITY);
    if (random(1,3)==1){
      velocity_x = velocity_x * (-1);
    }
  }
}

// ----------- IRQ Handler ------------
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

// IRQ handler for Laser Fire
void IRAM_ATTR LaserFireHandler() {
  portENTER_CRITICAL_ISR(&mux);
  LaserFireCounter++;  
  portEXIT_CRITICAL_ISR(&mux);
}

// --------- PROGRAM VARIABLES ---------
int framendx = 0;       // increment for each frame 

// crosshair related variables. 
int x = 0;
int y = 0; 
int x_start = 0;
int x_end = 0;
int y_start = 0;
int y_end = 0;
int x_step = 2;
int y_step = 2;

// keep track of score 
int score = 0;

// enemy ships
TieFighter fighter[NUM_FIGHTERS];

// Stock font and GFXFF reference handle
// #define GFXFF 1
// #define FF18 &FreeSans12pt7b

// Custom are fonts added to library "TFT_eSPI\Fonts\Custom" folder
// a #include must also be added to the "User_Custom_Fonts.h" file
// in the "TFT_eSPI\User_Setups" folder. See example entries.
// #define CF_OL24 &Orbitron_Light_24
// #define CF_OL32 &Orbitron_Light_32
// #define CF_RT24 &Roboto_Thin_24
// #define CF_S24  &Satisfy_24
// #define CF_Y32  &Yellowtail_32

// forward declarations
void drawCrosshair(int x, int y, int color);
void displayScore(int x, int y, int color);
void fireLaser(int x, int y, int color);
void updateScreen();
bool detectBogeyKills(int x, int y);

// ------------ start of code ------------------
void drawCrosshair(int x, int y, int color){
  buffer.drawLine(0, y, TFT_WIDTH, y, color);   // draw horiz line
  buffer.drawLine(x, MARGIN_TOP, x, TFT_HEIGHT, color);  // draw vert line
}

void displayScore(int score, int color){
  buffer.drawLine(0, MARGIN_TOP,TFT_WIDTH, MARGIN_TOP, color);
  buffer.setTextSize(2);
  buffer.setTextColor(color);
  buffer.drawString("Player 1", 10, 1);
  buffer.drawString("SC:" + String(score), 150, 1);
}

void fireLaser(int x, int y, int color){
  // fire laser to x,y point from left and right bottom corner
  int bottomleftx = 0;
  int bottomlefty = TFT_HEIGHT;
  int bottomrightx = TFT_WIDTH;
  int bottomrighty = TFT_HEIGHT;

  // fire left laser to x,y point
  buffer.drawWideLine(bottomleftx, bottomlefty, x, y, 4.0, color);
  // fire right laser to x,y point
  buffer.drawWideLine(bottomrightx, bottomrighty, x, y, 4.0, color);

  // detect if we hit anything
  detectBogeyKills(x, y);
}

bool detectBogeyKills(int x, int y){
  // detect if we hit any bogeys
  // return true or false
  bool hit = false;

  for (int i=0; i<NUM_FIGHTERS; i++){
    // fighter[i].x, fighter[i].y 
    // size is 20,20
    // x, y is location where laser hit
    // check X tolerance
    if ((x > fighter[i].x-10) && (x < fighter[i].x+10) && (y>fighter[i].y-10) && (y<fighter[i].y+10)) {
      hit = true;
      score = score + 10;
      // TODO: do something about fighter if hit... show explosion, remove from screen, do something
      buffer.drawSmoothCircle(x, y, 15, TFT_RED, TFT_ORANGE);
      fighter[i].init();  // this will respawn this enemy to a new location (therefore, erasing current location)
    }
  }
  return hit;
}

void updateScreen(){
  // update screen with buffer
  buffer.pushSprite(0, 0);
}

void drawEnemyShip(int x, int y, int transparentcolor){
  // 
  enemy.setPivot(10,10);
  enemy.pushToSprite(&buffer, x, y, transparentcolor);
  // enemy.pushRotated(&buffer, 45, transparentcolor);
}

void drawStars(){
  int num_stars = 10;
  int star_x;
  int star_y;
  for (int i=0; i<num_stars; i++){
    star_x = random(TFT_WIDTH);
    star_y = random(MARGIN_TOP, TFT_HEIGHT);
    buffer.drawCircle(star_x, star_y, 1, TFT_LIGHTGREY);
  }
}


// ----------- SETUP -------------
void setup() {
  // setup PAD switch IRQ
  pinMode(FIRE_LASER_PIN, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(FIRE_LASER_PIN), LaserFireHandler, RISING);

  tft.init();
  tft.setRotation(0);
  buffer.createSprite(TFT_WIDTH, TFT_HEIGHT);
  enemy.createSprite(20, 20);

  // initialize stating point of crosshair
  x_start = random(TFT_WIDTH * 0.2, TFT_WIDTH * 0.8);
  y_start = random(TFT_HEIGHT * 0.2, TFT_HEIGHT * 0.8);

  // create X number of fighters. init x, y, vector values of fighter
  for (int i=0; i<NUM_FIGHTERS; i++){
    fighter[i].init();
  }

  // create a tie-fighter sprite
  enemy.fillSprite(TFT_BLACK);
  enemy.setPivot(10,10);
  enemy.drawLine(4, 0, 4, 20, TFT_GREEN);       // left wing
  enemy.drawLine(16, 0, 16, 20, TFT_GREEN);     // right wing
  enemy.drawCircle(10, 10, 3, TFT_GREEN);       // cockpit

}

void loop() {
  // TEST: Let's move the crosshair randomly
  x_end = random(TFT_WIDTH * 0.05, TFT_WIDTH * 0.95);
  y_end  = random(TFT_HEIGHT * 0.05, TFT_HEIGHT * 0.95);

  x_step = (x_end - x_start)/STEP_ITERATIONS;
  y_step = (y_end - y_start)/STEP_ITERATIONS;

  x = x_start;
  y = y_start;

  for (int i=0; i<STEP_ITERATIONS; i++){
    // tft.fillScreen(BKGD_COLOR);    // this will cause flickering of display
    // buffer.fillScreen(BKGD_COLOR); // no flicker :) 
    buffer.fillSprite(BKGD_COLOR);    // no flicker :) 

    // star field
    drawStars();

    // test draw enemy ships
    for (int i=0; i<NUM_FIGHTERS; i++){
      drawEnemyShip(fighter[i].x, fighter[i].y, TFT_BLACK);
      fighter[i].move();
    }

    framendx++;
    if (framendx > 30000) { framendx = 0;};
    
    if (score > 10000) score=0;
    displayScore(score, TFT_WHITE);

    buffer.setTextSize(2);
    buffer.setTextColor(TFT_WHITE);
    buffer.drawString("ESP32 GAME DEMO", (TFT_WIDTH/2)-90, TFT_HEIGHT-20);
    
    // update position of crosshair
    x = x + x_step;
    y = y + y_step;
  
    // moving text with crosshair
    buffer.setTextColor(TFT_LIGHTGREY);
    buffer.setTextSize(1);
    buffer.drawString("X=" + String(x), x+5, y-10);
    buffer.setTextSize(1);
    buffer.drawString("Y=" + String(y), x+5, y+6);

    drawCrosshair(x, y, TFT_CYAN);    // draw moving crosshair 

    if (LaserFireCounter > 0) { 
      fireLaser(x, y, TFT_RED);
      // clear LaserFireCounter
      portENTER_CRITICAL(&mux);
      // LaserFireCounter--;     
      LaserFireCounter = 0;
      portEXIT_CRITICAL(&mux);
    }   

    // TESTING: fire laser every 20 loops
    // if (framendx % 10 == 0){
    //   fireLaser(x, y, TFT_RED);
    // }
    // if (framendx % 15 == 0){
    //   fireLaser(x, y, TFT_RED);
    // }
    // if (framendx % 35 == 0){
    //   fireLaser(x, y, TFT_RED);
    // }    
    
    // update screen with buffer
    updateScreen();
    yield(); // Stop watchdog reset
  }
  // reset and start from last point
  x_start = x;
  y_start = y; 

}