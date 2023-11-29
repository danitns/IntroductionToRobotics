#include "LedControl.h"  // need the library

const int dinPin = 12;
const int clockPin = 11;
const int loadPin = 10;
const int rows = 8;
const int cols = 8;

// declare all the joystick pins
const int pinSW = 2;  // digital pin connected to switch output
const int pinX = A0;  // A0 - analog pin connected to X output
const int pinY = A1;  // A1 - analog pin connected to Y output

struct Coordinates {
  int x = 0;
  int y = 0;
};

struct Bomb {
  Coordinates coordinates;
  long createdAt = millis();
  bool isActive = false;
};

Bomb bomb;

const byte bombBlinkInterval = 100;
const int bombExplosionTime = 2000;
long lastBombBlink = 0;
bool bombState = true;

int xValue = 0;
int yValue = 0;

bool joyMoved = false;
const int minThreshold = 400;
const int maxThreshold = 600;

Coordinates playerCurrentPosition;
Coordinates playerPastPosition;

const int playerBlinkInterval = 400;
long lastPlayerBlink = 0;
bool playerState = 1;

byte swState = LOW;
byte lastSwState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned int debounceDelay = 50;
volatile bool possiblePress = false;
volatile bool shouldStartCounter = false;

LedControl lc = LedControl(dinPin, clockPin, loadPin, 1);  //DIN, CLK, LOAD, No. DRIVER

const byte matrixSize = 8;
byte matrixBrightness = 2;
byte matrix[matrixSize][matrixSize] = {
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 }
};

void setup() {
  randomSeed(analogRead(2));  // init seed value with a random value from an unconnected pin
  initMatrix();

  pinMode(pinSW, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pinSW), handleInterrupt, FALLING);
}
void loop() {
  displayMatrix();
  displayPlayer();

  if (possiblePress) {
    readButton();
  }

  handleBombState();
}

void initMatrix() {
  // the zero refers to the MAX7219 number, it is zero for 1 chip
  lc.shutdown(0, false);  // turn off power saving, enables display
  lc.setIntensity(0, 5);  // sets brightness (0~15 possible values)
  lc.clearDisplay(0);     // clear screen
  generateRandomMatrix();
  generatePlayerPosition();
}

void generateRandomMatrix() {
  for (int row = 0; row < matrixSize; row++)
    for (int col = 0; col < matrixSize; col++) {
      matrix[row][col] = random(2); // 0 or 1 random value
    }
}

void generatePlayerPosition() {
  playerCurrentPosition.x = random(matrixSize);  // random value between 0 and 7
  playerCurrentPosition.y = random(matrixSize); // random value between 0 and 7
  matrix[playerCurrentPosition.x][playerCurrentPosition.y] = 0;
}

void displayMatrix() {
  for (int row = 0; row < matrixSize; row++) {
    for (int col = 0; col < matrixSize; col++) {
      // skip player position and bomb when drawing matrix
      if ((row != playerCurrentPosition.x || col != playerPastPosition.y)  
        && (!bomb.isActive || row != bomb.coordinates.x || col != bomb.coordinates.y)) 
        lc.setLed(0, row, col, matrix[row][col]);
    }
  }
}

void displayPlayer() {
  lc.setLed(0, playerPastPosition.x, playerPastPosition.y, LOW);

  readJoystickAndMovePlayer();
  if (millis() - lastPlayerBlink > playerBlinkInterval) {
    playerState = !playerState;
    lastPlayerBlink = millis();
  }
  lc.setLed(0, playerCurrentPosition.x, playerCurrentPosition.y, playerState);
  playerPastPosition = playerCurrentPosition;
}


void readJoystickAndMovePlayer() {
  int xValue = analogRead(pinX);
  int yValue = analogRead(pinY);
  // check for walls 
  if (xValue < minThreshold && joyMoved == false && matrix[playerCurrentPosition.x - 1][playerCurrentPosition.y] == 0) {
    playerCurrentPosition.x--;
    if (playerCurrentPosition.x < 0) {
      playerCurrentPosition.x = 0;
    }
    joyMoved = true;
  }
  if (xValue > maxThreshold && joyMoved == false && matrix[playerCurrentPosition.x + 1][playerCurrentPosition.y] == 0) {
    playerCurrentPosition.x++;
    if (playerCurrentPosition.x >= matrixSize) {
      playerCurrentPosition.x = matrixSize - 1;
    }
    joyMoved = true;
  }

  if (yValue < minThreshold && joyMoved == false && matrix[playerCurrentPosition.x][playerCurrentPosition.y - 1] == 0) {

    playerCurrentPosition.y--;
    if (playerCurrentPosition.y < 0) {
      playerCurrentPosition.y = 0;
    }
    joyMoved = true;
  }
  if (yValue > maxThreshold && joyMoved == false && matrix[playerCurrentPosition.x][playerCurrentPosition.y + 1] == 0) {

    playerCurrentPosition.y++;
    if (playerCurrentPosition.y >= matrixSize) {
      playerCurrentPosition.y = matrixSize - 1;
    }
    joyMoved = true;
  }

  if (xValue >= minThreshold && xValue <= maxThreshold && yValue >= minThreshold && yValue <= maxThreshold) {
    joyMoved = false;
  }
}

void handleInterrupt() {
  possiblePress = true;
  shouldStartCounter = true;
}

void readButton() {
  if (shouldStartCounter) {
    lastDebounceTime = millis();
    shouldStartCounter = false;
  }

  if (millis() - lastDebounceTime > debounceDelay) {
    swState = digitalRead(pinSW);
    if (swState != lastSwState && swState == LOW) {
      placeBomb();
    }
    possiblePress = false;
  }
}

void placeBomb() {
  if (bomb.isActive == false) {
    bomb.isActive = true;
    bomb.coordinates = playerCurrentPosition;
    bomb.createdAt = millis();
  }
}

void handleBombState() {
  if (bomb.isActive) {
    if (millis() - bomb.createdAt > bombExplosionTime) {
      explode(bomb.coordinates.x, bomb.coordinates.y);
    }
    displayBomb();
  }
}

void explode(byte x, byte y) {
  // cross shape bomb
  if (x > 0) {
    matrix[x - 1][y] = 0;
  }
  if (x < 7) {
    matrix[x + 1][y] = 0;
  }
  if (y > 0) {
    matrix[x][y - 1] = 0;
  }
  if (y < 7) {
    matrix[x][y + 1] = 0;
  }

  bomb.isActive = false;
}

void displayBomb() {
  if (millis() - lastBombBlink > bombBlinkInterval) {
    bombState = !bombState;
    lastBombBlink = millis();
  }
  lc.setLed(0, bomb.coordinates.x, bomb.coordinates.y, bombState);
}