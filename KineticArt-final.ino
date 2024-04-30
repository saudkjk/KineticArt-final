#include <FastLED.h>

#define NUM_LEDS 290          // Total number of LEDs
#define DATA_PIN 13           // Data pin where the LED strip is connected
#define LED_TYPE WS2812B      // Type of LED strip
#define COLOR_ORDER GRB       // Color order
#define BRIGHTNESS 100         // Brightness (0-255)


#define UPDATES_PER_SECOND 100

#define MAX_ROWS 9             // Maximum number of rows in the grid
#define MAX_COLS 30            // Maximum number of columns in the grid (highest column count)

CRGB leds[NUM_LEDS];

bool effectActive = false;
bool expanding = false;
int step = 0;
int effectRow = 0;
int effectCol = 0;
unsigned long effectStartTime = 0;
unsigned long  lastUpdate = 0;
const unsigned long effectDuration = 1000; 

CRGBPalette16 currentPalette;
TBlendType    currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

void setup() {
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  fill_solid(leds, NUM_LEDS, CRGB::Black);  // Initializes all LEDs to off
  FastLED.show();
  Serial.begin(9600);
    
  currentPalette = RainbowColors_p;
  currentBlending = LINEARBLEND;
}


#include <math.h>  // Ensure this include is at the top for mathematical functions

unsigned long currentTime = 0;
unsigned long previousTime = 0;

unsigned long elapsedTime = 0;

const unsigned long interval = 5000;


void loop() {

  currentTime = millis();
  elapsedTime = currentTime - previousTime;

  if (elapsedTime >= interval) {

    ChangePalettePeriodically();
    
    static uint8_t startIndex = 0;
    startIndex = startIndex + 1; /* motion speed */
    
    FillLEDsFromPaletteColors( startIndex);
    
    FastLED.show();
    FastLED.delay(1000 / UPDATES_PER_SECOND);
  }

  if (Serial.available() > 0) {
    previousTime = currentTime;
    String command = Serial.readStringUntil('\n');
    command.trim(); // Clean up the input string

    int commaIndex = command.lastIndexOf(',');
    String ledStates = command.substring(0, commaIndex);
    int row = command.substring(commaIndex + 1).toInt();

    for (int col = 0; col < MAX_COLS; col++) {
      if (col < ledStates.length()) {
        CRGB color = ledStates.charAt(col) == '1' ? CRGB::Blue : CRGB::Black;
        setLED(row, col, color);
      }
    }
    FastLED.show();
  }

}


// Function to map matrix coordinates to LED indices
int getLEDIndex(int row, int col) {
  int index = -1;  // Default to -1 for unused LEDs
  if (row < 0 || row >= MAX_ROWS || col < 0 || col >= MAX_COLS) {
    return index;  // Return -1 if row or column is out of range
  }
  if (row == 8) {
    index = col;
  } else if (row == 7) {
    index = 61 - col;
  } else if (row == 6) {
    index = 65 + col;
  } else if (row == 5) {
    index = 126 - col;
  } else if (row == 4) {
    index = 130 + col;
  } else if (row == 3) {
    index = 191 - col;
  } else if (row == 2) {
    index = 195 + col;
  } else if (row == 1) {
    index = 256 - col;
  } else if (row == 0) {
    index = 260 + col;
  }
  return index;
}

// Function to set an LED color at a given row and column
void setLED(int row, int col, CRGB color) {
  if (col < 0 || col >= MAX_COLS || row < 0 || row >= MAX_ROWS) {
    // Ignore the command if the row or column is out of bounds
    return;
  }
  int ledIndex = getLEDIndex(row, col);
  if (ledIndex != -1 && ledIndex < NUM_LEDS) {  // Check if the index is valid and within bounds
    leds[ledIndex] = color;
  }
}

void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    uint8_t brightness = 255;
    
    for( int i = 0; i < NUM_LEDS; ++i) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }
}


// There are several different palettes of colors demonstrated here.
//
// FastLED provides several 'preset' palettes: RainbowColors_p, RainbowStripeColors_p,
// OceanColors_p, CloudColors_p, LavaColors_p, ForestColors_p, and PartyColors_p.
//
// Additionally, you can manually define your own color palettes, or you can write
// code that creates color palettes on the fly.  All are shown here.

void ChangePalettePeriodically()
{
    uint8_t secondHand = (millis() / 1000) % 60;
    static uint8_t lastSecond = 99;
    
    if( lastSecond != secondHand) {
        lastSecond = secondHand;
        if( secondHand ==  0)  { currentPalette = RainbowColors_p;         currentBlending = LINEARBLEND; }
        if( secondHand == 10)  { currentPalette = RainbowStripeColors_p;   currentBlending = NOBLEND;  }
        if( secondHand == 15)  { currentPalette = RainbowStripeColors_p;   currentBlending = LINEARBLEND; }
        if( secondHand == 20)  { SetupPurpleAndGreenPalette();             currentBlending = LINEARBLEND; }
        if( secondHand == 25)  { SetupTotallyRandomPalette();              currentBlending = LINEARBLEND; }
        if( secondHand == 30)  { SetupBlackAndWhiteStripedPalette();       currentBlending = NOBLEND; }
        if( secondHand == 35)  { SetupBlackAndWhiteStripedPalette();       currentBlending = LINEARBLEND; }
        if( secondHand == 40)  { currentPalette = CloudColors_p;           currentBlending = LINEARBLEND; }
        if( secondHand == 45)  { currentPalette = PartyColors_p;           currentBlending = LINEARBLEND; }
        if( secondHand == 50)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = NOBLEND;  }
        if( secondHand == 55)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = LINEARBLEND; }
    }
}

// This function fills the palette with totally random colors.
void SetupTotallyRandomPalette()
{
    for( int i = 0; i < 16; ++i) {
        currentPalette[i] = CHSV( random8(), 255, random8());
    }
}

// This function sets up a palette of black and white stripes,
// using code.  Since the palette is effectively an array of
// sixteen CRGB colors, the various fill_* functions can be used
// to set them up.
void SetupBlackAndWhiteStripedPalette()
{
    // 'black out' all 16 palette entries...
    fill_solid( currentPalette, 16, CRGB::Black);
    // and set every fourth one to white.
    currentPalette[0] = CRGB::White;
    currentPalette[4] = CRGB::White;
    currentPalette[8] = CRGB::White;
    currentPalette[12] = CRGB::White;
    
}

// This function sets up a palette of purple and green stripes.
void SetupPurpleAndGreenPalette()
{
    CRGB purple = CHSV( HUE_PURPLE, 255, 255);
    CRGB green  = CHSV( HUE_GREEN, 255, 255);
    CRGB black  = CRGB::Black;
    
    currentPalette = CRGBPalette16(
                                   green,  green,  black,  black,
                                   purple, purple, black,  black,
                                   green,  green,  black,  black,
                                   purple, purple, black,  black );
}


// This example shows how to set up a static color palette
// which is stored in PROGMEM (flash), which is almost always more
// plentiful than RAM.  A static PROGMEM palette like this
// takes up 64 bytes of flash.
const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM =
{
    CRGB::Red,
    CRGB::Gray, // 'white' is too bright compared to red and blue
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Red,
    CRGB::Gray,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Blue,
    CRGB::Black,
    CRGB::Black
};
