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
#define GROUND_COLOR        0xDAB2  ///< 221, 216, 148


#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

// For the Adafruit shield, these are the default.
#define TFT_CLK 13
#define TFT_MISO 12
#define TFT_MOSI 11
#define TFT_DC 9
#define TFT_CS 10
#define TFT_RST 8
#define BUTTON_PIN 2
// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
//Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
// If using the breakout, change pins as desired
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

int width = 0, height = 0;

int current_page = 0;

void setup() {
  Serial.begin(9600);
  tft.begin();
  DDRD &= ~(1 << BUTTON_PIN);
  width = tft.width();
  height = tft.height();

  initiateGame();
}

void loop(void) {

  if (current_page == 0) { // Main Menu
    if (PIND & (1 << BUTTON_PIN)) {
      Serial.println(F("Trec la urm pagina"));
      delay(500);  // Add some delay to prevent flooding the serial output
      current_page = 1;
    }
  }

  if (current_page == 1) { // Main Game
    tft.fillScreen(PINK);
  }
}

void initiateGame() {
  tft.drawRect(0, 0, 240, 240, BACKGROUND_COLOR);
  tft.fillRect(0, 0, 240, 240, BACKGROUND_COLOR);

  tft.drawRect(0, 240, 240, 100, GREEN);
  tft.fillRect(0, 240, 240, 100, GREEN);

  tft.drawRoundRect(0, 0, width, height, 8, WHITE);     //Border

  tft.fillRoundRect(30, 180, 180, 40, 8, RED);
  tft.drawRoundRect(30, 180, 180, 40, 8, WHITE);  //Background

  tft.setCursor(60, 100);
  tft.setTextSize(2);
  tft.setFont();
  tft.setTextColor(WHITE);
  tft.print("FLAPPY BIRD");
  tft.setTextColor(WHITE);
  tft.setTextSize(3);
  tft.setCursor(75, 190);
  tft.print("START");
}
