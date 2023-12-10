#include <EEPROM.h>
#include "LedControl.h"
#include <LiquidCrystal.h>
#include "arduinoPins.h"

enum joystickDirections {
  LEFT_OR_UP,
  CENTER,
  RIGHT_OR_DOWN
};

enum menuType {
  WELCOME_SCREEN,
  MAIN_MENU,
  SETTINGS,
  ABOUT,
  END_GAME,
  WIN_GAME
};

enum customChars {
  UP_ARROW,
  DOWN_ARROW
};

struct Coordinates {
  int row = 0;
  int col = 0;
};

struct Player {
  const byte jumpHeight = 3;
  const byte jumpIncrementTime = 150;
  long lastJumpIncrement = 0;
  const byte fallTime = 125;
  long lastFall = 0;
  bool onGround = 1;
  byte jumpNumberOfIncrements = 0;
  Coordinates pastPosition;
  Coordinates position;
};

int gameTimer = 0;
long lastTimerIncrement = 0;

const int playerBlinkInterval = 20;
long lastPlayerBlink = 0;
bool playerState = 1;
const int movementSpeed = 300;
long lastMove = 0;

Player player;

const byte displayLen = 16;
const byte displayHeight = 2;
const byte maxLcdBrightness = 5;
byte lcdBrightness = 5;

volatile bool selectButtonPressed = false;
volatile unsigned long lastInterruptTime = 0;
const unsigned long debounceDelay = 200;

enum menuType currentMenu = WELCOME_SCREEN;
bool inGame = false;
bool shouldRerender = false;

bool joyMoved = false;
const int minThreshold = 400;
const int maxThreshold = 600;

const byte mainMenuLen = 3;
const String mainMenu[mainMenuLen] = { "Start game", "Settings", "About" };
const byte settingsMenuLen = 3;
const String settingsMenu[settingsMenuLen] = { "LCD Bright", "Game Bright", "Back" };
const String aboutMenu = "Super Mario platformer game.  Github: danitns";
byte currentMenuIndex = 0;
byte currentMenuLen = 0;


// Matrix variables
Coordinates bottomLeftViewCoordinates;
const byte matrixSize = 8;
byte matrixBrightness = 5;
byte maxMatrixBrightness = 5;
const byte mapRows = 12;
const byte mapCols = 32;
byte easyLevel[mapRows][mapCols] = {
  { 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
  { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};

byte hardLevel[mapRows][mapCols] = {
  { 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
  { 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};

byte (*matrix)[mapCols] = hardLevel;

void setup() {
  getBrightnessFromEeprom();
  Serial.begin(9600);
  initLcd();
  initMatrix();
  pinMode(pinSW, INPUT_PULLUP);

  initGameVariables();

  attachInterrupt(digitalPinToInterrupt(pinSW), handleButtonPress, FALLING);
}

void loop() {
  if (inGame) {
    play();
  } else {
    renderMenu();
  }
}

// utils
void writeStringOnCenter(String str, int row = 0, bool displayAvailableDirections = false) {
  byte cursorPos = (displayLen / 2) - (str.length() / 2);
  lcd.setCursor(cursorPos, row);
  lcd.print(str);
  if (displayAvailableDirections) {
    displayMenuArrows();
  }
}

void displayMenuArrows() {
  if (currentMenuIndex > 0) {
    lcd.setCursor(15, 0);
    lcd.write(UP_ARROW);
  }
  if (currentMenuIndex < currentMenuLen - 1) {
    lcd.setCursor(15, 1);
    lcd.write(DOWN_ARROW);
  }
}

void handleButtonPress() {
  static unsigned long interruptTime = 0;  // Timestamp for the current interrupt, retains its value between ISR calls
  interruptTime = micros();                // Capture the current time in microseconds
  // Debounce logic: If interrupts come in faster succession than the debounce delay, they're ignored
  if (interruptTime - lastInterruptTime > debounceDelay * 1000) {  // Convert debounceDelay to microseconds for comparison
    selectButtonPressed = true;
  }
  // Update the last interrupt timestamp
  lastInterruptTime = interruptTime;
}

void displayImage(const uint8_t* image) {
  for (int row = 0; row < matrixSize; row++) {
    lc.setRow(0, row, image[matrixSize - row - 1]);
  }
}

void getBrightnessFromEeprom() {
  lcdBrightness = EEPROM.read(0);
  matrixBrightness = EEPROM.read(1);
}

void saveValuesToEeprom() {
  EEPROM.update(0, lcdBrightness);
  EEPROM.update(1, matrixBrightness);
}

joystickDirections getJoystickPosition(byte pin) {
  unsigned int value = analogRead(pin);
  if (value > maxThreshold) {
    return RIGHT_OR_DOWN;
  } else if (value < minThreshold) {
    return LEFT_OR_UP;
  } else return CENTER;
}

// init functions
void initLcd() {
  lcd.begin(16, 2);
  byte mappedValue = map(lcdBrightness, 0, maxLcdBrightness, 0, 254);

  lcd.createChar(UP_ARROW, customChar[UP_ARROW]);
  lcd.createChar(DOWN_ARROW, customChar[DOWN_ARROW]);

  analogWrite(backlightPin, mappedValue);
  writeStringOnCenter("SUPER MARIO GAME");
  writeStringOnCenter("press button", 1);
}

void initMatrix() {
  lc.shutdown(0, false);                 // turn off power saving, enables display
  lc.setIntensity(0, matrixBrightness);  // sets brightness (0~15 possible values)
  lc.clearDisplay(0);                    // clear screen
  displayImage(IMAGES[currentMenu]);
}

void initGameVariables() {
  player.position.row = 1;
  player.position.col = 2;
  gameTimer = 0;
}

void renderMenu() {
  handleJoystickMovementInMenu();
  if (selectButtonPressed) {
    executeChosenAction();
    currentMenuIndex = 0;
    shouldRerender = true;
    selectButtonPressed = false;
  }
  if (shouldRerender) {
    lcd.clear();
    displayMenu();
    shouldRerender = false;
  }
}

void executeChosenAction() {
  switch (currentMenu) {
    case WELCOME_SCREEN:
      currentMenu = MAIN_MENU;
      currentMenuLen = mainMenuLen;
      break;
    case MAIN_MENU:
      executeMainMenuAction();
      break;
    case SETTINGS:
      if (currentMenuIndex == 2) {
        currentMenu = MAIN_MENU;
        currentMenuLen = mainMenuLen;
      }
      break;
    case ABOUT:
      currentMenu = MAIN_MENU;
      currentMenuLen = mainMenuLen;
      break;
    default:
      currentMenu = MAIN_MENU;
      break;
  }
}

void executeMainMenuAction() {
  switch (currentMenuIndex) {
    case 0:
      inGame = true;
      break;
    case 1:
      currentMenu = SETTINGS;
      currentMenuLen = settingsMenuLen;
      break;
    case 2:
      currentMenu = ABOUT;
      currentMenuLen = ceil(aboutMenu.length() / float(((displayLen - 1) * displayHeight)));
      break;
    default:
      break;
  }
}

void displayMenu() {
  switch (currentMenu) {
    case MAIN_MENU:
      writeStringOnCenter(mainMenu[currentMenuIndex], 0, true);
      displayImage(IMAGES[currentMenuIndex + 1]);
      if (inGame == true) {
        displayMap();
      }
      break;
    case SETTINGS:
      displaySettings();
      break;
    case ABOUT:
      writeStringWithScroll(aboutMenu);
      break;
    case END_GAME:
      writeStringOnCenter("END GAME");
      writeStringOnCenter("press button", 1);
      displayImage(IMAGES[4]);
      break;
    case WIN_GAME:
      writeStringOnCenter("YOU WIN!");
      writeStringOnCenter("press button", 1);
      displayImage(IMAGES[5]);
      break;
    default:
      break;
  }
}

void displaySettings() {
  writeStringOnCenter(settingsMenu[currentMenuIndex], 0, true);
  switch (currentMenuIndex) {
    case 0:
      displayCurrentSettings(lcdBrightness, maxLcdBrightness, 1);
      break;
    case 1:
      displayCurrentSettings(matrixBrightness, maxMatrixBrightness, 1);
      break;
  }
}

void displayCurrentSettings(int currentSettingValue, int maxSettingValue, int screenRowIndex) {
  String str = "< ";
  for (int i = 0; i < maxSettingValue; i++) {
    if (i < currentSettingValue)
      str += 'X';
    else
      str += '-';
  }
  str += " >";
  writeStringOnCenter(str, screenRowIndex);
}

void handleJoystickMovementInMenu() {
  if (getJoystickPosition(pinY) == LEFT_OR_UP && joyMoved == false && currentMenuIndex > 0) {
    currentMenuIndex--;
    shouldRerender = true;
    joyMoved = true;
  }

  if (getJoystickPosition(pinY) == RIGHT_OR_DOWN && joyMoved == false && currentMenuIndex < (currentMenuLen - 1)) {
    currentMenuIndex++;
    shouldRerender = true;
    joyMoved = true;
  }

  if (currentMenu == SETTINGS) {
    adjustSettings();
  }

  if (getJoystickPosition(pinY) == CENTER && getJoystickPosition(pinX) == CENTER) {
    joyMoved = false;
  }
}

void adjustSettings() {
  if (getJoystickPosition(pinX) == LEFT_OR_UP && joyMoved == false) {
    if (currentMenuIndex == 0 && lcdBrightness > 0) {
      lcdBrightness--;
      byte mappedValue = map(lcdBrightness, 0, maxLcdBrightness, 0, 254);
      analogWrite(backlightPin, mappedValue);
      shouldRerender = true;
      joyMoved = true;
    }
    if (currentMenuIndex == 1 && matrixBrightness > 0) {
      matrixBrightness--;
      byte mappedValue = map(matrixBrightness, 0, maxMatrixBrightness, 0, 15);
      lc.setIntensity(0, mappedValue);
      shouldRerender = true;
      joyMoved = true;
    }

    saveValuesToEeprom();
  }

  if (getJoystickPosition(pinX) == RIGHT_OR_DOWN && joyMoved == false) {
    if (currentMenuIndex == 0 && lcdBrightness < maxLcdBrightness) {
      lcdBrightness++;
      byte mappedValue = map(lcdBrightness, 0, maxLcdBrightness, 0, 254);
      analogWrite(backlightPin, mappedValue);
      shouldRerender = true;
      joyMoved = true;
    }
    if (currentMenuIndex == 1 && matrixBrightness < maxMatrixBrightness) {
      matrixBrightness++;
      byte mappedValue = map(matrixBrightness, 0, maxMatrixBrightness, 0, 15);
      lc.setIntensity(0, mappedValue);
      shouldRerender = true;
      joyMoved = true;
    }
  }

  saveValuesToEeprom();
}

void writeStringWithScroll(String message) {
  byte numberOfCharsToPrint = (displayLen - 1) * displayHeight;
  for (int i = 0; i < numberOfCharsToPrint; i++) {
    if (i == displayLen - 1) {
      lcd.setCursor(0, 1);
    }
    byte index = (currentMenuIndex * numberOfCharsToPrint) + i;
    if (index >= message.length())
      break;
    lcd.print(message[index]);
  }
  displayMenuArrows();
}

void play() {
  handleLcdDisplayInGame();
  handleMapState();
  handlePlayerState();
}

void handleLcdDisplayInGame() {
  if (shouldRerender) {
    lcd.clear();
    writeStringOnCenter("Level 1");
    writeStringOnCenter(String(gameTimer), 1);
    shouldRerender = false;
  }
  if (millis() - lastTimerIncrement > 1000) {
    gameTimer++;
    shouldRerender = true;
    lastTimerIncrement = millis();
  }
}

void handleMapState() {
  if (player.position.col < (matrixSize / 2)) {
    bottomLeftViewCoordinates.col = 0;
  } else if (player.position.col > mapCols - (matrixSize / 2)) {
    bottomLeftViewCoordinates.col = mapCols - matrixSize;
  } else {
    bottomLeftViewCoordinates.col = player.position.col - 4;
  }

  if (player.position.row < (matrixSize / 2)) {
    bottomLeftViewCoordinates.row = 0;
  } else if (player.position.row > mapRows - (matrixSize / 2)) {
    bottomLeftViewCoordinates.row = mapRows - matrixSize;
  } else {
    bottomLeftViewCoordinates.row = player.position.row - 4;
  }

  displayMap();
}

void displayMap() {
  for (int row = 0; row < matrixSize; row++) {
    for (int col = 0; col < matrixSize; col++) {
      lc.setLed(0, row, col, matrix[row + bottomLeftViewCoordinates.row][col + bottomLeftViewCoordinates.col]);
    }
  }
}

void handlePlayerState() {
  displayPlayer();
  movePlayer();
  checkForGameResult();
}

void displayPlayer() {
  lc.setLed(0, player.pastPosition.row - bottomLeftViewCoordinates.row, player.pastPosition.col - bottomLeftViewCoordinates.col, LOW);
  if (millis() - lastPlayerBlink > playerBlinkInterval) {
    playerState = !playerState;
    lastPlayerBlink = millis();
  }
  lc.setLed(0, player.position.row - bottomLeftViewCoordinates.row, player.position.col - bottomLeftViewCoordinates.col, playerState);
  player.pastPosition = player.position;
}

void movePlayer() {
  applyHorizontalMoving();
  applyVerticalMoving();
}

void applyHorizontalMoving() {
  if (getJoystickPosition(pinX) == LEFT_OR_UP && matrix[player.position.row][player.position.col - 1] == 0) {
    if (millis() - lastMove > movementSpeed) {
      player.position.col--;
      lastMove = millis();
    }
  }
  if (getJoystickPosition(pinX) == RIGHT_OR_DOWN && matrix[player.position.row][player.position.col + 1] == 0) {
    if (millis() - lastMove > movementSpeed) {
      player.position.col++;
      lastMove = millis();
    }
  }
}

void applyVerticalMoving() {
  if (matrix[player.position.row - 1][player.position.col] == 1) {
    player.onGround = true;
  }

  if (getJoystickPosition(pinY) == LEFT_OR_UP && matrix[player.position.row - 1][player.position.col] == 1) {
    player.jumpNumberOfIncrements = player.jumpHeight;
  }
  
  // jump logic
  if (player.jumpNumberOfIncrements > 0) {
    if (millis() - player.lastJumpIncrement > player.jumpIncrementTime) {
      if(player.position.row + 1 < mapRows){
        player.position.row++;
      }
        
      player.lastJumpIncrement = millis();
      player.jumpNumberOfIncrements--;
    }
    if (player.jumpNumberOfIncrements == 0) {
      player.lastFall = millis();
    }
  } else if (matrix[player.position.row - 1][player.position.col] == 0) {
    // added onGround so the player wouldn t fall instantly from the current row to row-1
    if (player.onGround == true) {
      player.lastFall = millis();
      player.onGround = false;
    }
    if (millis() - player.lastFall > player.fallTime) {
      player.position.row--;
      player.lastFall = millis();
    }
    if (matrix[player.position.row - 1][player.position.col] == 0) {
      player.lastJumpIncrement = millis();
    }
  }
}

void checkForGameResult() {
  if (player.position.row <= 0) {
    inGame = false;
    currentMenu = END_GAME;
    shouldRerender = true;
    initGameVariables();
  } else if (player.position.col >= mapCols) {
    inGame = false;
    currentMenu = WIN_GAME;
    shouldRerender = true;
    initGameVariables();
  }
}
