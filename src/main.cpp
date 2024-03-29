#include <Arduino.h>
#include <TFT_eSPI.h>                 // Include the graphics library (this includes the sprite functions)

// TFT specs
#define TFT_WIDTH  240
#define TFT_HEIGHT 320 
#define BKGD_COLOR TFT_BLACK
#define MARGIN_TOP 20

#define MIN_VELOCITY 2      // of enemy ships
#define MAX_VELOCITY 6      // of enemy ships
#define NUM_FIGHTERS 10     // current number of enemy ships

#define FIRE_LASER_PIN 26   // pin # for push button digital in
#define ANALOG_X_PIN   36   // X analog input for joystick
#define ANALOG_Y_PIN   39   // Y analog input for joystick

int LaserFireCounter;       // used by IRQ for Laser fire push button 
float analog_X_Value;
float analog_Y_Value;

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
    void init();
    void move();
  private:
    int direction;
};

void TieFighter::init(){
  x = random(1, TFT_WIDTH);
  y = random(1, TFT_HEIGHT);
  velocity_x = random(1,5);
  velocity_y = random(1,5);
}

void TieFighter::move(){
  x = x + velocity_x;
  y = y + velocity_y;
  
  if (y>TFT_HEIGHT){
    y = random(1, TFT_HEIGHT);
    velocity_y = random(MIN_VELOCITY, MAX_VELOCITY);
    if (random(1,3)==1) velocity_y = velocity_y * (-1);
  }
  if (y<0){
    y = random(1, TFT_HEIGHT);
    velocity_y = random(MIN_VELOCITY, MAX_VELOCITY);
    if (random(1,3)==1) velocity_y = velocity_y * (-1);
  }
  if (x>TFT_WIDTH){
    x = random(1, TFT_WIDTH);
    velocity_x = random(MIN_VELOCITY, MAX_VELOCITY);
    if (random(1,3)==1) velocity_x = velocity_x * (-1);
  }
  if (x<0){
    x = random(1, TFT_WIDTH);
    velocity_x = random(MIN_VELOCITY, MAX_VELOCITY);
    if (random(1,3)==1) velocity_x = velocity_x * (-1);
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
// crosshair related variables. 
int x = 0;
int y = 0; 

// keep track of score 
int score = 0;

// enemy ships
TieFighter fighter[NUM_FIGHTERS];

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
  // moving text with crosshair
  buffer.setTextColor(TFT_LIGHTGREY);
  buffer.setTextSize(1);
  buffer.drawString("X=" + String(x), x+5, y-10);
  buffer.setTextSize(1);
  buffer.drawString("Y=" + String(y), x+5, y+6);  
}

void displayScore(int score, int color){
  if (score > 1000) score=0;
  buffer.drawLine(0, MARGIN_TOP,TFT_WIDTH, MARGIN_TOP, color);
  buffer.setTextSize(2);
  buffer.setTextColor(color);
  buffer.drawString("@owel.codes", 2, 1);
  buffer.drawString("SC:" + String(score), 160, 1);
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
    // fighter[i].x, fighter[i].y, sprite size is 20,20
    if ((x > fighter[i].x-10) && (x < fighter[i].x+10) && (y>fighter[i].y-10) && (y<fighter[i].y+10)) {
      hit = true;
      score = score + 10;
      // do something about fighter if hit... show explosion, remove from screen, do something
      buffer.drawSmoothCircle(x, y, 15, TFT_RED, TFT_ORANGE);
      fighter[i].init();  // this will respawn this enemy to a new location (therefore, erasing current location that was hit)
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

void drawEnemyShips(){
  for (int i=0; i<NUM_FIGHTERS; i++){
    drawEnemyShip(fighter[i].x, fighter[i].y, TFT_BLACK);
    fighter[i].move();
  }
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

void displayGameName(){
  // game name at bottom of screen
  buffer.setTextSize(2);
  buffer.setTextColor(TFT_WHITE);
  buffer.drawString("ESP-ar Wars 32", (TFT_WIDTH/2)-80, TFT_HEIGHT-20);
}

void readJoystick(){
  // read the joystick
  analog_X_Value = analogRead(ANALOG_X_PIN);  // 0 - 4095 ---> scale to 0 - TFT_WIDTH-1
  analog_Y_Value = analogRead(ANALOG_Y_PIN);  // 0 - 4095 ---> scale to 0 - TFT_HEIGHT-1
  // Serial.print("\nX="); Serial.println(analog_X_Value);
  // Serial.print("Y="); Serial.println(analog_Y_Value);
  // proportionally scale to TFT screen
  x = map(analog_X_Value, 0, 4095, 0, TFT_WIDTH-1);
  y = map(analog_Y_Value, 0, 4095, MARGIN_TOP, TFT_HEIGHT-1);
}

// ----------- SETUP -------------
void setup() {
  // Serial.begin(115200);

  // setup FireLaser Button Interrupt
  pinMode(FIRE_LASER_PIN, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(FIRE_LASER_PIN), LaserFireHandler, RISING);

  tft.init();
  tft.setRotation(0);
  
  buffer.createSprite(TFT_WIDTH, TFT_HEIGHT);

  // create a tie-fighter sprite
  enemy.createSprite(20, 20);
  enemy.fillSprite(TFT_BLACK);
  enemy.setPivot(10,10);
  enemy.drawLine(4, 0, 4, 20, TFT_GREEN);       // left wing
  enemy.drawLine(16, 0, 16, 20, TFT_GREEN);     // right wing
  enemy.drawCircle(10, 10, 3, TFT_GREEN);       // cockpit

  // create X number of fighters. init x, y, vector values of fighter
  for (int i=0; i<NUM_FIGHTERS; i++){
    fighter[i].init();
  }
}

void loop() {
    buffer.fillSprite(BKGD_COLOR);    // no flicker :) 

    drawStars();
    drawEnemyShips();  
    displayScore(score, TFT_WHITE);
    displayGameName();
    readJoystick();
    drawCrosshair(x, y, TFT_CYAN);    

    // check interrupt if Push button was pressed
    if (LaserFireCounter > 0) { 
      fireLaser(x, y, TFT_RED);
      portENTER_CRITICAL(&mux);
      // LaserFireCounter--;     
      LaserFireCounter = 0;   // reset 
      portEXIT_CRITICAL(&mux);
    }   
    
    updateScreen();
}