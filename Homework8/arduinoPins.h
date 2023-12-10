// LCD Pins
const byte rsPin = 9;
const byte enPin = 8;
const byte d4Pin = 7;
const byte d5Pin = 6;
const byte d6Pin = 5;
const byte d7Pin = 4;
const byte backlightPin = 3;
LiquidCrystal lcd(rsPin, enPin, d4Pin, d5Pin, d6Pin, d7Pin);

// Matrix Pins
const byte dinPin = 12;
const byte clockPin = 11;
const byte loadPin = 10;
LedControl lc = LedControl(dinPin, clockPin, loadPin, 1);  //DIN, CLK, LOAD, No. DRIVER

// Joystick Pins
const byte pinSW = 2;  // digital pin connected to switch output
const byte pinX = A0;  // A0 - analog pin connected to X output
const byte pinY = A1;  // A1 - analog pin connected to Y output

// matrix images
const uint8_t IMAGES[][8] = {
  { 0b00011000,
    0b00111100,
    0b00011000,
    0b00111100,
    0b01011010,
    0b00011000,
    0b00100100,
    0b00100100 },
  { 0b00111100,
    0b01000010,
    0b10011001,
    0b10010101,
    0b10011001,
    0b10010001,
    0b01000010,
    0b00111100 },
  { 0b01100110,
    0b10001111,
    0b10100111,
    0b01011010,
    0b00011000,
    0b00100100,
    0b01000010,
    0b10000001 },
  { 0b00011000,
    0b00100100,
    0b00100100,
    0b00000100,
    0b00011000,
    0b00010000,
    0b00000000,
    0b00010000 },
  { 0b00000000,
    0b10100101,
    0b01000010,
    0b10100101,
    0b00000000,
    0b00011000,
    0b00100100,
    0b01000010 },
  { 0b01111110,
    0b01111110,
    0b01111110,
    0b00111100,
    0b00011000,
    0b00011000,
    0b00011000,
    0b00111100 }
};

uint8_t customChar[][8] = {
  // up arrow
  { B00100,
    B01110,
    B10101,
    B00100,
    B00100,
    B00100,
    B00100,
    B00100 },
  // down arrow
  { B00100,
    B00100,
    B00100,
    B00100,
    B00100,
    B10101,
    B01110,
    B00100 }
};
