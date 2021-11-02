
/* TODO LIST*/
/*
 ** Make const and move to flash 
 * URLs
 * Reducing SRAM size: https://create.arduino.cc/projecthub/john-bradnam/reducing-your-memory-usage-26ca05
 * Pointer error: https://stackoverflow.com/questions/27447220/invalid-conversion-from-const-int-to-int
 * 
 ** Change Accelerometer to ADXL
*/
// Accelerometer
#include <Wire.h>
#include <Adafruit_ADXL345_U.h>


// y orientation
#define DOWN         0
#define UP           1
#define FLAT         2

// x orientation
#define NOT_TILTED   0
#define TILTED_RIGHT 1 
#define TILTED_LEFT  2

// the tilt previously recorded from the last frame cycle
int prevTilt = NOT_TILTED;

// the threshold for the accelerometer measurements to break before considering it a tilt
int threshold = 500;

// accelerometer object
Adafruit_ADXL345_Unified adxl = Adafruit_ADXL345_Unified();

//ws2812b
#include <FastLED.h>
#define LED_PIN     13
#define NUM_LEDS    67
CRGB leds[NUM_LEDS];
CRGB sandColor = CRGB(255, 185, 0);
CRGB emptyColor = CRGB(0, 0, 0);

int delayBetweenSandFrames = 50; // delay between sand falling
int delayBetweenCycleFrames = 500; // delay before the next frame in the entire animation

int firstSandDropDistanceFromCenterIndex = 2; // distance from the center of the led to light for the first sand drop location
int secondSandDropDistanceFromCenterIndex = 10; // distance from the center of the led to light for the second sand drop location
int thirdSandDropDistanceFromCenterIndex = 22; // distance from the center of the led to light for the third sand drop location

// index of the start from the top of the hour glass
int topStartIndex = 0;

// index of the end from the top of the hour glass
int topEndIndex = 32;

// index of the center of the hour glass
int centerIndex = 33;

// index of the start from the bottom of the hour glass
int bottomStartIndex = 34;

// index of the end from the bottom of the hour glass
int bottomEndIndex = 66;

// current count of the leds on the bottom of the hour glass
int countLedsOnBottom = 0;

// current count of the leds on the top of the hour glass. Assume we start with the hour glass in proper orientation not tilted
int countLedsOnTop = 25;

// a int array that maps the number of lit leds in a half glass to an array of values indicating
// whether or not to light a certain index from the middle of the hour glass. Remember we are in a snaking formation
// these are the coordinates on the top of the hour glass
const int bottomPileLedsCoordinates[26][33] PROGMEM = {
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 0
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 1, 0, 0, 0, 0},
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 1, 1, 0, 0, 0},
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 
    0, 0, 0, 1, 0, 0, 0, 
    0, 0, 0, 1, 1, 0, 0, 0},
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 
    0, 0, 0, 1, 0, 0, 0, 
    0, 0, 0, 1, 1, 1, 0, 0},
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 
    0, 0, 1, 1, 0, 0, 0, 
    0, 0, 0, 1, 1, 1, 0, 0}, // 5
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 0, 1, 0, 0, 
    0, 0, 1, 1, 0, 0, 0, 
    0, 0, 0, 1, 1, 1, 0, 0},
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 0, 1, 0, 0, 
    0, 0, 1, 1, 0, 0, 0, 
    0, 0, 1, 1, 1, 1, 0, 0},
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 0, 1, 0, 0, 
    0, 0, 1, 1, 1, 0, 0, 
    0, 0, 1, 1, 1, 1, 0, 0},
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 1, 1, 0, 0, 
    0, 0, 1, 1, 1, 0, 0, 
    0, 0, 1, 1, 1, 1, 0, 0},
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 1, 1, 0, 0, 
    0, 0, 1, 1, 1, 0, 0, 
    0, 1, 1, 1, 1, 1, 0, 0}, // 10
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 1, 1, 0, 0, 
    0, 0, 1, 1, 1, 0, 0, 
    0, 1, 1, 1, 1, 1, 1, 0},
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 1, 1, 0, 0, 
    0, 1, 1, 1, 1, 0, 0, 
    0, 1, 1, 1, 1, 1, 1, 0},
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 1, 1, 0, 0, 
    0, 1, 1, 1, 1, 1, 0, 
    0, 1, 1, 1, 1, 1, 1, 0},
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 1, 0, 0, 
    0, 0, 1, 1, 0, 0, 
    0, 1, 1, 1, 1, 1, 0, 
    0, 1, 1, 1, 1, 1, 1, 0},
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 1, 1, 0, 0, 
    0, 1, 1, 1, 1, 1, 0, 
    1, 1, 1, 1, 1, 1, 1, 1}, // 15
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 1, 0, 0, 
    0, 0, 1, 1, 0, 0, 
    0, 1, 1, 1, 1, 1, 0, 
    1, 1, 1, 1, 1, 1, 1, 1},
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 1, 1, 0, 0, 
    1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1},
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 1, 0, 0, 
    0, 0, 1, 1, 0, 0, 
    1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1},
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 1, 0, 0, 
    0, 1, 1, 1, 0, 0, 
    1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1},
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 1, 0, 0, 
    0, 1, 1, 1, 1, 0, 
    1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1}, // 20
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 1, 1, 0, 
    0, 1, 1, 1, 1, 0, 
    1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1},
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 1, 1, 1, 0, 
    0, 1, 1, 1, 1, 0, 
    1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1},
  {
    0, 0, 0, 
    0, 1, 0, 0, 
    0, 1, 1, 1, 0, 
    0, 1, 1, 1, 1, 0, 
    1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1},
  {
    0, 0, 0, 
    0, 1, 1, 0, 
    0, 1, 1, 1, 0, 
    0, 1, 1, 1, 1, 0, 
    1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1},
  {
    0, 0, 0, 
    0, 1, 1, 0, 
    0, 1, 1, 1, 0, 
    1, 1, 1, 1, 1, 0, 
    1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1}, // 25
};

// a int array that maps the number of lit leds in a half glass to an array of values indicating
// whether or not to light a certain index from the middle of the hour glass. Remember we are in a snaking formation
// these are the coordinates when the glass is not tilted
const int topPileLedsCoordinates[26][33] PROGMEM = {
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 0
  {
    1, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0},
  {
    1, 0, 1, 
    0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0},
  {
    1, 1, 1, 
    0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0},
  {
    1, 1, 1, 
    0, 0, 0, 1, 
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0},
  {
    1, 1, 1, 
    1, 0, 0, 1, 
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0}, // 5
  {
    1, 1, 1, 
    1, 0, 1, 1, 
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0},
  {
    1, 1, 1, 
    1, 1, 1, 1, 
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0},
  {
    1, 1, 1, 
    1, 1, 1, 1, 
    0, 0, 0, 0, 1, 
    0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0},
  {
    1, 1, 1, 
    1, 1, 1, 1, 
    1, 0, 0, 0, 1, 
    0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0},
  {
    1, 1, 1, 
    1, 1, 1, 1, 
    1, 0, 0, 1, 1, 
    0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0}, // 10
  {
    1, 1, 1, 
    1, 1, 1, 1, 
    1, 1, 0, 1, 1, 
    0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0},
  {
    1, 1, 1, 
    1, 1, 1, 1, 
    1, 1, 0, 1, 1, 
    0, 0, 0, 0, 0, 1, 
    0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0},
  {
    1, 1, 1, 
    1, 1, 1, 1, 
    1, 1, 0, 1, 1, 
    1, 0, 0, 0, 0, 1, 
    0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0},
  {
    1, 1, 1, 
    1, 1, 1, 1, 
    1, 1, 1, 1, 1, 
    1, 0, 0, 0, 0, 1, 
    0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0},
  {
    1, 1, 1, 
    1, 1, 1, 1, 
    1, 1, 1, 1, 1, 
    1, 1, 0, 0, 0, 1, 
    0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0}, // 15
  {
    1, 1, 1, 
    1, 1, 1, 1, 
    1, 1, 1, 1, 1, 
    1, 1, 0, 0, 1, 1, 
    0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0},
  {
    1, 1, 1, 
    1, 1, 1, 1, 
    1, 1, 1, 1, 1, 
    1, 1, 0, 0, 1, 1, 
    0, 0, 0, 0, 0, 0, 1, 
    0, 0, 0, 0, 0, 0, 0, 0},
  {
    1, 1, 1, 
    1, 1, 1, 1, 
    1, 1, 1, 1, 1, 
    1, 1, 0, 0, 1, 1, 
    1, 0, 0, 0, 0, 0, 1, 
    0, 0, 0, 0, 0, 0, 0, 0},
  {
    1, 1, 1, 
    1, 1, 1, 1, 
    1, 1, 1, 1, 1, 
    1, 1, 0, 0, 1, 1, 
    1, 0, 0, 0, 0, 1, 1, 
    0, 0, 0, 0, 0, 0, 0, 0},
  {
    1, 1, 1, 
    1, 1, 1, 1, 
    1, 1, 1, 1, 1, 
    1, 1, 0, 0, 1, 1, 
    1, 1, 0, 0, 0, 1, 1, 
    0, 0, 0, 0, 0, 0, 0, 0}, // 20
  {
    1, 1, 1, 
    1, 1, 1, 1, 
    1, 1, 1, 1, 1, 
    1, 1, 0, 1, 1, 1, 
    1, 1, 0, 0, 0, 1, 1, 
    0, 0, 0, 0, 0, 0, 0, 0},
  {
    1, 1, 1, 
    1, 1, 1, 1, 
    1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 
    1, 1, 0, 0, 0, 1, 1, 
    0, 0, 0, 0, 0, 0, 0, 0},
  {
    1, 1, 1, 
    1, 1, 1, 1, 
    1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 
    1, 1, 0, 0, 1, 1, 1, 
    0, 0, 0, 0, 0, 0, 0, 0},
  {
    1, 1, 1, 
    1, 1, 1, 1, 
    1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 
    1, 1, 1, 0, 1, 1, 1, 
    0, 0, 0, 0, 0, 0, 0, 0},
  {
    1, 1, 1, 
    1, 1, 1, 1, 
    1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 
    0, 0, 0, 0, 0, 0, 0, 0}, // 25
};

// a int array that maps the number of lit leds in a half glass to an array of values indicating
// whether or not to light a certain index from the middle of the hour glass. Remember we are in a snaking formation
// these are the coordinates when the glass is tilted left
const int tiltLeftBottomLedsCoordinates[26][33] PROGMEM = {
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 0
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 
    1, 0, 0, 0, 0, 0, 0, 0},
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 1, 
    1, 0, 0, 0, 0, 0, 0, 0},
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 1, 
    1, 1, 0, 0, 0, 0, 0, 0},
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 1, 
    1, 1, 1, 0, 0, 0, 0, 0},
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 1, 1, 
    1, 1, 1, 0, 0, 0, 0, 0}, // 5
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    1, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 1, 1, 
    1, 1, 1, 0, 0, 0, 0, 0},
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    1, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 1, 1, 
    1, 1, 1, 1, 0, 0, 0, 0},
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    1, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 1, 1, 1, 
    1, 1, 1, 1, 0, 0, 0, 0},
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    1, 1, 0, 0, 0, 0, 
    0, 0, 0, 0, 1, 1, 1, 
    1, 1, 1, 1, 0, 0, 0, 0},
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 0, 0, 1, 
    1, 1, 0, 0, 0, 0, 
    0, 0, 0, 0, 1, 1, 1, 
    1, 1, 1, 1, 0, 0, 0, 0}, // 10
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 0, 0, 1, 
    1, 1, 0, 0, 0, 0, 
    0, 0, 0, 0, 1, 1, 1, 
    1, 1, 1, 1, 1, 0, 0, 0},
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 0, 0, 1, 
    1, 1, 0, 0, 0, 0, 
    0, 0, 0, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 0, 0, 0},
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 0, 0, 1, 
    1, 1, 1, 0, 0, 0, 
    0, 0, 0, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 0, 0, 0},
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 0, 1, 1, 
    1, 1, 1, 0, 0, 0, 
    0, 0, 0, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 0, 0, 0},
  {
    0, 0, 0, 
    1, 0, 0, 0, 
    0, 0, 0, 1, 1, 
    1, 1, 1, 0, 0, 0, 
    0, 0, 0, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 0, 0, 0}, // 15
  {
    0, 0, 0, 
    1, 0, 0, 0, 
    0, 0, 0, 1, 1, 
    1, 1, 1, 0, 0, 0, 
    0, 0, 0, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 0, 0},
  {
    0, 0, 0, 
    1, 0, 0, 0, 
    0, 0, 0, 1, 1, 
    1, 1, 1, 0, 0, 0, 
    0, 0, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 0, 0},
  {
    0, 0, 0, 
    1, 0, 0, 0, 
    0, 0, 0, 1, 1, 
    1, 1, 1, 1, 0, 0, 
    0, 0, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 0, 0},
  {
    0, 0, 0, 
    1, 0, 0, 0, 
    0, 0, 1, 1, 1, 
    1, 1, 1, 1, 0, 0, 
    0, 0, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 0, 0},
  {
    0, 0, 0, 
    1, 1, 0, 0, 
    0, 0, 1, 1, 1, 
    1, 1, 1, 1, 0, 0, 
    0, 0, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 0, 0}, // 20
  {
    0, 0, 1, 
    1, 1, 0, 0, 
    0, 0, 1, 1, 1, 
    1, 1, 1, 1, 0, 0, 
    0, 0, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 0, 0},
  {
    0, 0, 1, 
    1, 1, 0, 0, 
    0, 0, 1, 1, 1, 
    1, 1, 1, 1, 0, 0, 
    0, 0, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 0},
  {
    0, 0, 1, 
    1, 1, 0, 0, 
    0, 0, 1, 1, 1, 
    1, 1, 1, 1, 0, 0, 
    0, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 0},
  {
    0, 0, 1, 
    1, 1, 0, 0, 
    0, 0, 1, 1, 1, 
    1, 1, 1, 1, 1, 0, 
    0, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 0},
  {
    0, 0, 1, 
    1, 1, 0, 0, 
    0, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 0, 
    0, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 0}, // 25
};

// a int array that maps the number of lit leds in a half glass to an array of values indicating
// whether or not to light a certain index from the middle of the hour glass. Remember we are in a snaking formation
// these are the coordinates when the glass is tilted right
const int tiltRightBottomLedsCoordinates[26][33] PROGMEM = {
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 0
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 1},
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 
    1, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 1},
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 
    1, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 1, 1},
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 
    1, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 1, 1, 1},
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 
    1, 1, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 1, 1, 1}, // 5
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 1, 
    1, 1, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 1, 1, 1},
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 1, 
    1, 1, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 1, 1, 1, 1},
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 1, 
    1, 1, 1, 0, 0, 0, 0, 
    0, 0, 0, 0, 1, 1, 1, 1},
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    0, 0, 0, 0, 0, 
    0, 0, 0, 0, 1, 1, 
    1, 1, 1, 0, 0, 0, 0, 
    0, 0, 0, 0, 1, 1, 1, 1},
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    1, 0, 0, 0, 0, 
    0, 0, 0, 0, 1, 1, 
    1, 1, 1, 0, 0, 0, 0, 
    0, 0, 0, 0, 1, 1, 1, 1}, // 10
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    1, 0, 0, 0, 0, 
    0, 0, 0, 0, 1, 1, 
    1, 1, 1, 0, 0, 0, 0, 
    0, 0, 0, 1, 1, 1, 1, 1},
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    1, 0, 0, 0, 0, 
    0, 0, 0, 0, 1, 1, 
    1, 1, 1, 1, 0, 0, 0, 
    0, 0, 0, 1, 1, 1, 1, 1},
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    1, 0, 0, 0, 0, 
    0, 0, 0, 1, 1, 1, 
    1, 1, 1, 1, 0, 0, 0, 
    0, 0, 0, 1, 1, 1, 1, 1},
  {
    0, 0, 0, 
    0, 0, 0, 0, 
    1, 1, 0, 0, 0, 
    0, 0, 0, 1, 1, 1, 
    1, 1, 1, 1, 0, 0, 0, 
    0, 0, 0, 1, 1, 1, 1, 1},
  {
    0, 0, 0, 
    0, 0, 0, 1, 
    1, 1, 0, 0, 0, 
    0, 0, 0, 1, 1, 1, 
    1, 1, 1, 1, 0, 0, 0, 
    0, 0, 0, 1, 1, 1, 1, 1}, // 15
  {
    0, 0, 0, 
    0, 0, 0, 1, 
    1, 1, 0, 0, 0, 
    0, 0, 0, 1, 1, 1, 
    1, 1, 1, 1, 0, 0, 0, 
    0, 0, 1, 1, 1, 1, 1, 1},
  {
    0, 0, 0, 
    0, 0, 0, 1, 
    1, 1, 0, 0, 0, 
    0, 0, 0, 1, 1, 1, 
    1, 1, 1, 1, 1, 0, 0, 
    0, 0, 1, 1, 1, 1, 1, 1},
  {
    0, 0, 0, 
    0, 0, 0, 1, 
    1, 1, 0, 0, 0, 
    0, 0, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 0, 0, 
    0, 0, 1, 1, 1, 1, 1, 1},
  {
    0, 0, 0, 
    0, 0, 0, 1, 
    1, 1, 1, 0, 0, 
    0, 0, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 0, 0, 
    0, 0, 1, 1, 1, 1, 1, 1},
  {
    0, 0, 0, 
    0, 0, 1, 1, 
    1, 1, 1, 0, 0, 
    0, 0, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 0, 0, 
    0, 0, 1, 1, 1, 1, 1, 1}, // 20
  {
    1, 0, 0, 
    0, 0, 1, 1, 
    1, 1, 1, 0, 0, 
    0, 0, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 0, 0, 
    0, 0, 1, 1, 1, 1, 1, 1},
  {
    1, 0, 0, 
    0, 0, 1, 1, 
    1, 1, 1, 0, 0, 
    0, 0, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 0, 0, 
    0, 1, 1, 1, 1, 1, 1, 1},
  {
    1, 0, 0, 
    0, 0, 1, 1, 
    1, 1, 1, 0, 0, 
    0, 0, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 0, 
    0, 1, 1, 1, 1, 1, 1, 1},
  {
    1, 0, 0, 
    0, 0, 1, 1, 
    1, 1, 1, 0, 0, 
    0, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 0, 
    0, 1, 1, 1, 1, 1, 1, 1},
  {
    1, 0, 0, 
    0, 0, 1, 1, 
    1, 1, 1, 1, 0, 
    0, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 0, 
    0, 1, 1, 1, 1, 1, 1, 1}, // 25
};

void setup(void) {
  Serial.begin(9600);

  // setup accelerometer
  setupAdxl();

  // init fast led
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  clearLeds();
}

void loop() {
  int16_t x = adxl.getX();
  int16_t y = adxl.getY();
  dropSand(x,y);
}

void setupAdxl() {
  if (!adxl.begin()) {
    Serial.println("Couldn't start");
    while (1);
  }
  Serial.println("ADXL345 found!");
  adxl.setRange(ADXL345_RANGE_2_G);
}

void dropSand(int16_t x,int16_t y){
  int tilt = NOT_TILTED;
  if(x > threshold) {
    tilt = TILTED_RIGHT;
  }else if(x < -threshold) {
    tilt = TILTED_LEFT;
  }

  int dir = FLAT;
  if(y > threshold) {
    dir = UP;
    Serial.print(y); 
    Serial.println(" | UP");
  
  }else if(y < -threshold) {
    dir = DOWN;
    Serial.print(y); 
    Serial.println(" | DOWN");
  }
  dropSandPileAnimation(dir, tilt);
  prevTilt = tilt;
}

void dropSandPileAnimation(int dir, int tilt) {
  if(dir == DOWN) {
    dropSandFromTopPile(tilt);
  }else if(dir == UP) {
    dropSandFromBottomPile(tilt);
  }
}

void dropSandFromBottomPile(int tilt) {
  // if we are tilting the glass or we have sand that can drop, run the animation
  if(countLedsOnBottom > 0 || tilt != prevTilt) {
    if(countLedsOnBottom > 0) {
      countLedsOnTop += 1;
      countLedsOnBottom -= 1;
    }
    dropSandFromBottomPileAnimation(tilt);
  }
}

void dropSandFromTopPile(int tilt) {
  // if we are tilting the glass or we have sand that can drop, run the animation
  if(countLedsOnTop > 0 || tilt != prevTilt) {
    if(countLedsOnTop > 0) {
      countLedsOnTop -= 1;
      countLedsOnBottom += 1;
    }
    dropSandFromTopPileAnimation(tilt);
  }
}

void dropSandFromBottomPileAnimation(int tilt) {
  if(countLedsOnBottom > 0){
    /// ----- START FIRST FRAME ----- ///
    // drop sand from top and set center index
    setLedsForBottomHalf(topPileLedsCoordinates[countLedsOnBottom]);
    leds[centerIndex] = sandColor;
    FastLED.show();
    delay(delayBetweenSandFrames);
  
    /// ----- START SECOND FRAME ----- ///
    // keep dropping sand
    leds[centerIndex] = emptyColor;
    boolean isFirstLedAlreadyLit = leds[centerIndex - firstSandDropDistanceFromCenterIndex] == sandColor;
    leds[centerIndex - firstSandDropDistanceFromCenterIndex] = sandColor;
    FastLED.show();
    delay(delayBetweenSandFrames);
  
    /// ----- START THIRD FRAME ----- ///
    // keep dropping sand if the led is not lit that we are going to fall into
    boolean isSecondLedAlreadyLit = leds[centerIndex - secondSandDropDistanceFromCenterIndex] == sandColor;
    if(!isFirstLedAlreadyLit) {
      leds[centerIndex - firstSandDropDistanceFromCenterIndex] = emptyColor;
      leds[centerIndex - secondSandDropDistanceFromCenterIndex] = sandColor;
      FastLED.show();
      delay(delayBetweenSandFrames);
    }
  
    /// ----- START FOURTH FRAME ----- ///
    // keep dropping sand if the led is not lit that we are going to fall into
    boolean isThirdLedAlreadyLit = leds[centerIndex - thirdSandDropDistanceFromCenterIndex] == sandColor;
    if(!isSecondLedAlreadyLit) {
      leds[centerIndex - secondSandDropDistanceFromCenterIndex] = emptyColor;
      leds[centerIndex - thirdSandDropDistanceFromCenterIndex] = sandColor;
      FastLED.show();
      delay(delayBetweenSandFrames);
    }
  
    /// ----- START FIFTH FRAME ----- ///
    // clear out the last sand drop location if it is not already lit
    if(!isThirdLedAlreadyLit) {
      leds[centerIndex - secondSandDropDistanceFromCenterIndex] = emptyColor;
      FastLED.show();
      delay(delayBetweenSandFrames);
    }
  }else if(countLedsOnBottom == 0){
    leds[bottomStartIndex] = emptyColor;
  }
  
  /// ----- START SIXTH FRAME ----- ///
  // depending on tilt, draw sand on the bottom
  if(tilt == TILTED_LEFT){
    setLedsForTopHalf(tiltLeftBottomLedsCoordinates[countLedsOnTop]);
  }else if(tilt == TILTED_RIGHT) {
    setLedsForTopHalf(tiltRightBottomLedsCoordinates[countLedsOnTop]);
  }else{
    setLedsForTopHalf(bottomPileLedsCoordinates[countLedsOnTop]);
  }
  FastLED.show();
  delay(delayBetweenCycleFrames);
}

void dropSandFromTopPileAnimation(int tilt) {
  if(countLedsOnTop > 0){
    /// ----- START FIRST FRAME ----- ///
    // drop sand from top and set center index
    setLedsForTopHalf(topPileLedsCoordinates[countLedsOnTop]);
    leds[centerIndex] = sandColor;
    FastLED.show();
    delay(delayBetweenSandFrames);
  
    /// ----- START SECOND FRAME ----- ///
    // keep dropping sand
    leds[centerIndex] = emptyColor;
    boolean isFirstLedAlreadyLit = leds[centerIndex + firstSandDropDistanceFromCenterIndex] == sandColor;
    leds[centerIndex + firstSandDropDistanceFromCenterIndex] = sandColor;
    FastLED.show();
    delay(delayBetweenSandFrames);
  
    /// ----- START THIRD FRAME ----- ///
    // keep dropping sand if the led is not lit that we are going to fall into
    boolean isSecondLedAlreadyLit = leds[centerIndex + secondSandDropDistanceFromCenterIndex] == sandColor;
    if(!isFirstLedAlreadyLit) {
      leds[centerIndex + firstSandDropDistanceFromCenterIndex] = emptyColor;
      leds[centerIndex + secondSandDropDistanceFromCenterIndex] = sandColor;
      FastLED.show();
      delay(delayBetweenSandFrames);
    }
  
    /// ----- START FOURTH FRAME ----- ///
    // keep dropping sand if the led is not lit that we are going to fall into
    boolean isThirdLedAlreadyLit = leds[centerIndex + thirdSandDropDistanceFromCenterIndex] == sandColor;
    if(!isSecondLedAlreadyLit) {
      leds[centerIndex + secondSandDropDistanceFromCenterIndex] = emptyColor;
      leds[centerIndex + thirdSandDropDistanceFromCenterIndex] = sandColor;
      FastLED.show();
      delay(delayBetweenSandFrames);
    }
  
    /// ----- START FIFTH FRAME ----- ///
    // clear out the last sand drop location if it is not already lit
    if(!isThirdLedAlreadyLit) {
      leds[centerIndex + secondSandDropDistanceFromCenterIndex] = emptyColor;
      FastLED.show();
      delay(delayBetweenSandFrames);
    }
  }else if(countLedsOnTop == 0){
    leds[topEndIndex] = emptyColor;
  }
  
  /// ----- START SIXTH FRAME ----- ///
  // depending on tilt, draw sand on the bottom
  if(tilt == TILTED_RIGHT){
    setLedsForBottomHalf(tiltLeftBottomLedsCoordinates[countLedsOnBottom]);
  }else if(tilt == TILTED_LEFT) {
    setLedsForBottomHalf(tiltRightBottomLedsCoordinates[countLedsOnBottom]);
  }else{
    setLedsForBottomHalf(bottomPileLedsCoordinates[countLedsOnBottom]);
  }
  FastLED.show();
  delay(delayBetweenCycleFrames);
}

void clearLeds() {
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = emptyColor;
  }
}

// ledsToLight - the coordinates of the leds to light (1 for on, 0 for off)
void setLedsForBottomHalf(const int ledsToLight[]) {
  uint16_t ms = millis();  // rainbow pattern
  for(int i = 0; i < centerIndex; i++) {
    leds[bottomStartIndex + i] = ledsToLight[i] == 1 ? CHSV( ms / 41, 200, 255) : emptyColor;
  }
}

// ledsToLight - the coordinates of the leds to light (1 for on, 0 for off)
void setLedsForTopHalf(const int ledsToLight[]) {
  uint16_t ms = millis();  // rainbow pattern
  for(int i = 0; i < centerIndex; i++) {
    leds[topEndIndex - i] = ledsToLight[i] == 1 ? CHSV( ms / 53, 200, 255) : emptyColor;
  }
}
