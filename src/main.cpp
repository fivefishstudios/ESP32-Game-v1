#include <Arduino.h>
#include <TFT_eSPI.h>                 // Include the graphics library (this includes the sprite functions)

// TFT specs
#define TFT_WIDTH  240
#define TFT_HEIGHT 320 
#define BKGD_COLOR TFT_BLACK
#define MARGIN_TOP 20
#define STEP_ITERATIONS 8

TFT_eSPI    tft = TFT_eSPI();         // Create object "tft"
TFT_eSprite buffer = TFT_eSprite(&tft);  // Create Sprite object "img" with pointer to "tft" object

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
}

void updateScreen(){
  // update screen with buffer
  buffer.pushSprite(0, 0);
}

int angle = 0;
int x = 0;
int y = 0; 
int x_start = 0;
int x_end = 0;
int y_start = 0;
int y_end = 0;
int x_step = 2;
int y_step = 2;
int score = 0;
int loopndx = 0;

void setup() {
  tft.init();
  tft.setRotation(0);
  buffer.createSprite(TFT_WIDTH, TFT_HEIGHT);

  // initialize stating point of crosshair
  x_start = random(TFT_WIDTH * 0.2, TFT_WIDTH * 0.8);
  y_start = random(TFT_HEIGHT * 0.2, TFT_HEIGHT * 0.8);
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
    
    // drawCrosshair(TFT_WIDTH/2, TFT_HEIGHT/2, TFT_ORANGE);     // draw new crosshair on center screen 

    loopndx++;
    if (loopndx > 30000) { loopndx = 0;};
    
    if (score > 10000) score=0;
    displayScore(score, TFT_MAGENTA);

    buffer.setTextSize(2);
    buffer.setTextColor(TFT_WHITE);
    buffer.drawString("ESP32 GAME DEMO", (TFT_WIDTH/2)-90, TFT_HEIGHT/2);
    
    // update position of crosshair
    x = x + x_step;
    y = y + y_step;
  
    // moving text with crosshair
    buffer.setTextColor(TFT_YELLOW);
    buffer.setTextSize(1);
    buffer.drawString("X=" + String(x), x+5, y-10);
    buffer.setTextSize(1);
    buffer.drawString("Y=" + String(y), x+5, y+6);

    drawCrosshair(x, y, TFT_CYAN);    // draw moving crosshair 

    // TESTING: fire laser every 20 loops
    if (loopndx % 20 == 0){
      fireLaser(x, y, TFT_RED);
      score = score + 10;
    }
    if (loopndx % 50 == 0){
      fireLaser(x, y, TFT_RED);
      score = score + 10;
    }
    
    // update screen with buffer
    updateScreen();
    yield(); // Stop watchdog reset
  }
  // reset and start from last point
  x_start = x;
  y_start = y; 

}