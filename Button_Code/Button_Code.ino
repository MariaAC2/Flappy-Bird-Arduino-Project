/***************************************************
  This is our GFX example for the Adafruit ILI9341 Breakout and Shield
  ----> http://www.adafruit.com/products/1651

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

/* Define Colors*/
#define BLACK       0x0000  ///<   0,   0,   0
#define NAVY        0x000F  ///<   0,   0, 123
#define DARKGREEN   0x03E0  ///<   0, 125,   0
#define DARKCYAN    0x03EF  ///<   0, 125, 123
#define MAROON      0x7800  ///< 123,   0,   0
#define PURPLE      0x780F  ///< 123,   0, 123
#define OLIVE       0x7BE0  ///< 123, 125,   0
#define LIGHTGREY   0xC618  ///< 198, 195, 198
#define DARKGREY    0x7BEF  ///< 123, 125, 123
#define BLUE        0x001F  ///<   0,   0, 255
#define GREEN       0x07E0  ///<   0, 255,   0
#define CYAN        0x07FF  ///<   0, 255, 255
#define RED         0xF800  ///< 255,   0,   0
#define MAGENTA     0xF81F  ///< 255,   0, 255
#define YELLOW      0xFFE0  ///< 255, 255,   0
#define WHITE       0xFFFF  ///< 255, 255, 255
#define ORANGE      0xFD20  ///< 255, 165,   0
#define GREENYELLOW 0xAFE5  ///< 173, 255,  41
#define PINK        0xFC18  ///< 255, 130, 198
#define BACKGROUND_COLOR    0x7399  ///< 114, 198, 206

/*Define Pillar Coordonates*/
#define REDBAR_MINX 50
#define GREENBAR_MINX 130
#define BLUEBAR_MINX 180
#define BAR_MINY 30
#define BAR_HEIGHT 250
#define BAR_WIDTH 30
#define DRAW_LOOP_INTERVAL 10

#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "EEPROM.h"
#include "SoftwareSerial.h"

// For the Adafruit shield, these are the default.
#define TFT_CLK 13
#define TFT_MISO 12
#define TFT_MOSI 11
#define TFT_DC 9
#define TFT_CS 10
#define TFT_RST 8
#define BUTTON_PIN 2
#define BLUETOOTH_RX 0
#define BLUETOOTH_TX 1

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
SoftwareSerial HC_05(BLUETOOTH_RX, BLUETOOTH_TX);

int width = 0, height = 0;
int current_page = 0, PILLAR_COLOR = DARKGREEN;
int currentWing;        //Wings animation
int flX, flY, fallRate; //Store the bird's position
int pillarPos, gapPosition;  //Position and gap for pillars
int score;              //Store the score
int highScore = 0;      //Store the highscore
bool running = false;   //Check if bird is flying or not
bool crashed = false;   //Check if bird crashed or not
long nextDrawLoopRunTime; // Next frame
bool startBird = false;

#define addr 0

void setup() {
  Serial.begin(9600); // LCD start
  HC_05.begin(9600); // Bluetooth start
  
  tft.begin();
  tft.setRotation(3); // 270 degrees rotation

  uint8_t x = tft.readcommand8(ILI9341_RDMODE);
  Serial.print("Display Power Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDMADCTL);
  Serial.print("MADCTL Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDPIXFMT);
  Serial.print("Pixel Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDIMGFMT);
  Serial.print("Image Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDSELFDIAG);
  Serial.print("Self Diagnostic: 0x"); Serial.println(x, HEX); 

  DDRD &= ~(1 << BUTTON_PIN);
  width = tft.width();
  height = tft.height();

  // Set highScore to 0
  int initFlag;
  EEPROM.get(addr, initFlag);

  // If EEPROM is not initialized, initialize it
  if (initFlag != 12345) {
    int defaultHighScore = 0;
    EEPROM.put(addr, 12345);
    EEPROM.put(addr + sizeof(int), defaultHighScore);
  }

  // Read the high score from EEPROM
  EEPROM.get(addr + sizeof(int), highScore);

  mainMenu();
}

void loop(void) {

  if (current_page == 0) { // Main Menu
    if (PIND & (1 << BUTTON_PIN)) {
      delay(500);  // Add some delay to prevent flooding the serial output
      current_page = 1;
      printBackground();

      nextDrawLoopRunTime = millis() + DRAW_LOOP_INTERVAL;
      crashed = false;
      running = false;

      startGame();
    }
  }

  if (current_page == 1) {
    if (millis() > nextDrawLoopRunTime && !crashed) { // check if we can animate next frame
      drawLoop();
      delay(15);
      checkCollision();
      delay(15);
      nextDrawLoopRunTime += DRAW_LOOP_INTERVAL;
      delay(15);
    }

    if (PIND & (1 << BUTTON_PIN)) {
      if (crashed) {
        // restart game
        current_page = 0;
        score = 0;
        mainMenu();
      }
      else if (!running) {
        // clear and restart
        tft.fillRect(0, 0, 320, 80, BACKGROUND_COLOR);
        running = true;
      }
      else
      {
        startBird = true;
        // Go up
        fallRate = -8;
      }
    }
  }
}

void mainMenu() {
  printBackground();
  initiateGame();
}

void printBackground() {
  // tft.drawRect(0, 0, 240, 240, BACKGROUND_COLOR);
  // tft.fillRect(0, 0, 240, 240, BACKGROUND_COLOR);

  tft.fillScreen(BACKGROUND_COLOR);
}

void initiateGame() {
  tft.setCursor(95, 60);
  tft.setTextSize(2);
  tft.setFont();
  tft.setTextColor(WHITE);
  tft.print("FLAPPY BIRD"); // Flappy Bird Logo

  tft.fillRoundRect(70, 150, 180, 40, 8, RED);
  tft.drawRoundRect(70, 150, 180, 40, 8, WHITE);  // Start Buttion

  tft.setTextColor(WHITE);
  tft.setTextSize(3);
  tft.setCursor(115, 160);
  tft.print("START"); // Start Text Button
}

void startGame() {
  // Default bird coordinates
  flX = 50;
  flY = 100;
  fallRate = 0;
  pillarPos = 320;
  gapPosition = 60;
  crashed = false;
  score = 0;

  if (!startBird) {
    tft.setCursor(110, 100);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.println("Tap to");

    tft.setCursor(110, 120);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.println("begin!!");
  }

  delay(1000);
  nextDrawLoopRunTime = millis() + DRAW_LOOP_INTERVAL;
}

void checkCollision() {
  // Collision with ground
  if (flY > 206) crashed = true;

  // Collision with pillar
  if (flX + 34 > pillarPos && flX < pillarPos + 50)
    if (flY < gapPosition || flY + 24 > gapPosition + 90)
      crashed = true;

  if (crashed) {      //Check if bird crashes
    if (score > highScore) {
      highScore = score;
      EEPROM.write(addr, highScore);
      tft.setCursor(80, 40);
      tft.setTextSize(3);
      tft.setTextColor(ORANGE);
      tft.print("NEW HIGH!");
    }
    tft.setTextColor(RED);
    tft.setTextSize(2);
    tft.setCursor(100, 75);
    tft.print("Game Over!");

    tft.setTextSize(2);
    tft.setCursor(115, 140);
    tft.print("Score:");
    tft.setTextSize(2);
    tft.setTextColor(WHITE);
    tft.print(score);
    HC_05.println(score);

    tft.fillRoundRect(130, 180, 60, 35, 8, GREEN);
    tft.drawRoundRect(130, 180, 60, 35, 8, WHITE);  // Start Buttion

    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.setCursor(150, 190);
    tft.print("OK"); // Start Text Button

    highScore = EEPROM.read(addr);
    
    // Display the high score on the screen
    tft.setTextColor(RED);
    tft.setTextSize(2);
    tft.setCursor(40, 260);
    tft.print("High Score: ");
    tft.print(highScore);

    // Stop animation
    running = false;

    // Delay to stop any last minute clicks from restarting immediately
    delay(1000);
  }
}

void drawLoop() { //Bird and Pillar Animations
  clearPillar(pillarPos, gapPosition);   //Clear pillar
  clearBird(flX, flY);            //Clear bird

  // Move items
  if (running) {
    flY += fallRate;
    fallRate++;

    pillarPos -= 5;
    if (pillarPos == 0) {
      score++;
    }
    else if (pillarPos < -50) {
      pillarPos = 200;
      gapPosition = random(20, 120);    //Random gap position
    }
  }

  // Draw pillar and bird
  drawPillar(pillarPos, gapPosition);
  drawBird(flX, flY);

  //Animate wings
  switch (currentWing) {
    case 0: case 1: drawWingDown(flX, flY); break;  //Wing down
    case 2: case 3: drawWingMiddle(flX, flY); break;  //Wing middle
    case 4: case 5: drawWingUp(flX, flY); break;  //Wing up
  }

  if (score == EEPROM.read(0) ) //Change the pillar colour when breaking highscore
    PILLAR_COLOR = YELLOW;
  else
    PILLAR_COLOR = DARKGREEN;

  currentWing++;   //flap the wing
  if (currentWing == 6) 
    currentWing = 0;  //reset the wing
}

void drawPillar(int x, int gap) {
  // Pillar Fill
  tft.fillRect(x + 2, 2, 46, gap - 4, PILLAR_COLOR);
  tft.fillRect(x + 2, gap + 92, 46, 150 - gap, PILLAR_COLOR);

  tft.drawRect(x, 0, 50, gap, BLACK);
  tft.drawRect(x + 1, 1, 48, gap - 2, BLACK);
  tft.drawRect(x, gap + 90, 50, 150 - gap, BLACK);
  tft.drawRect(x + 1, gap + 91 , 48, 149 - gap, BLACK);
}

void clearPillar(int x, int gap) {  //Clear the pillar
  tft.fillRect(x + 45, 0, 5, gap, BACKGROUND_COLOR);
  tft.fillRect(x + 45, gap + 90, 5, 140 - gap, BACKGROUND_COLOR);
}

void clearBird(int x, int y) {  //Clear the bird
  tft.fillRect(x, y, 34, 24, BACKGROUND_COLOR);
}

void drawBird(int x, int y) {  //Draw the bird
  // Upper and lower body
  tft.fillRect(x + 2, y + 8, 2, 10, BLACK);
  tft.fillRect(x + 4, y + 6, 2, 2, BLACK);
  tft.fillRect(x + 6, y + 4, 2, 2, BLACK);
  tft.fillRect(x + 8, y + 2, 4, 2, BLACK);
  tft.fillRect(x + 12, y, 12, 2, BLACK);
  tft.fillRect(x + 24, y + 2, 2, 2, BLACK);
  tft.fillRect(x + 26, y + 4, 2, 2, BLACK);
  tft.fillRect(x + 28, y + 6, 2, 6, BLACK);
  tft.fillRect(x + 10, y + 22, 10, 2, BLACK);
  tft.fillRect(x + 4, y + 18, 2, 2, BLACK);
  tft.fillRect(x + 6, y + 20, 4, 2, BLACK);

  // Body fill
  tft.fillRect(x + 12, y + 2, 6, 2, YELLOW);
  tft.fillRect(x + 8, y + 4, 8, 2, YELLOW);
  tft.fillRect(x + 6, y + 6, 10, 2, YELLOW);
  tft.fillRect(x + 4, y + 8, 12, 2, YELLOW);
  tft.fillRect(x + 4, y + 10, 14, 2, YELLOW);
  tft.fillRect(x + 4, y + 12, 16, 2, YELLOW);
  tft.fillRect(x + 4, y + 14, 14, 2, YELLOW);
  tft.fillRect(x + 4, y + 16, 12, 2, YELLOW);
  tft.fillRect(x + 6, y + 18, 12, 2, YELLOW);
  tft.fillRect(x + 10, y + 20, 10, 2, YELLOW);

  // Eye
  tft.fillRect(x + 18, y + 2, 2, 2, BLACK);
  tft.fillRect(x + 16, y + 4, 2, 6, BLACK);
  tft.fillRect(x + 18, y + 10, 2, 2, BLACK);
  tft.fillRect(x + 18, y + 4, 2, 6, WHITE);
  tft.fillRect(x + 20, y + 2, 4, 10, WHITE);
  tft.fillRect(x + 24, y + 4, 2, 8, WHITE);
  tft.fillRect(x + 26, y + 6, 2, 6, WHITE);
  tft.fillRect(x + 24, y + 6, 2, 4, BLACK);

  // Beak
  tft.fillRect(x + 20, y + 12, 12, 2, BLACK);
  tft.fillRect(x + 18, y + 14, 2, 2, BLACK);
  tft.fillRect(x + 20, y + 14, 12, 2, RED);
  tft.fillRect(x + 32, y + 14, 2, 2, BLACK);
  tft.fillRect(x + 16, y + 16, 2, 2, BLACK);
  tft.fillRect(x + 18, y + 16, 2, 2, RED);
  tft.fillRect(x + 20, y + 16, 12, 2, BLACK);
  tft.fillRect(x + 18, y + 18, 2, 2, BLACK);
  tft.fillRect(x + 20, y + 18, 10, 2, RED);
  tft.fillRect(x + 30, y + 18, 2, 2, BLACK);
  tft.fillRect(x + 20, y + 20, 10, 2, BLACK);
}

// First wing : down
void drawWingDown(int x, int y) {
  tft.fillRect(x, y + 14, 2, 6, BLACK);
  tft.fillRect(x + 2, y + 20, 8, 2, BLACK);
  tft.fillRect(x + 2, y + 12, 10, 2, BLACK);
  tft.fillRect(x + 12, y + 14, 2, 2, BLACK);
  tft.fillRect(x + 10, y + 16, 2, 2, BLACK);
  tft.fillRect(x + 2, y + 14, 8, 6, WHITE);
  tft.fillRect(x + 8, y + 18, 2, 2, BLACK);
  tft.fillRect(x + 10, y + 14, 2, 2, WHITE);
}

// Second wing: middle
void drawWingMiddle(int x, int y) {
  tft.fillRect(x + 2, y + 10, 10, 2, BLACK);
  tft.fillRect(x + 2, y + 16, 10, 2, BLACK);
  tft.fillRect(x, y + 12, 2, 4, BLACK);
  tft.fillRect(x + 12, y + 12, 2, 4, BLACK);
  tft.fillRect(x + 2, y + 12, 10, 4, WHITE);
}

// Third wing: up
void drawWingUp(int x, int y) {
  tft.fillRect(x + 2, y + 6, 8, 2, BLACK);
  tft.fillRect(x, y + 8, 2, 6, BLACK);
  tft.fillRect(x + 10, y + 8, 2, 2, BLACK);
  tft.fillRect(x + 12, y + 10, 2, 4, BLACK);
  tft.fillRect(x + 10, y + 14, 2, 2, BLACK);
  tft.fillRect(x + 2, y + 14, 2, 2, BLACK);
  tft.fillRect(x + 4, y + 16, 6, 2, BLACK);
  tft.fillRect(x + 2, y + 8, 8, 6, WHITE);
  tft.fillRect(x + 4, y + 14, 6, 2, WHITE);
  tft.fillRect(x + 10, y + 10, 2, 4, WHITE);
}