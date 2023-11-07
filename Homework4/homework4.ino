struct LedSegment {
  int pin;
  int ledState;
  LedSegment* topSegment;
  LedSegment* bottomSegment;
  LedSegment* leftSegment;
  LedSegment* rightSegment;

  LedSegment() = default;
  LedSegment(int pin)
    : pin(pin), ledState(LOW), topSegment(nullptr), bottomSegment(nullptr), leftSegment(nullptr), rightSegment(nullptr) {}

  void setSegments(LedSegment* top, LedSegment* bottom, LedSegment* left, LedSegment* right) {
    topSegment = top;
    bottomSegment = bottom;
    leftSegment = left;
    rightSegment = right;
  }
};

// declare all the joystick pins
const int pinSW = 2;  // digital pin connected to switch output
const int pinX = A0;  // A0 - analog pin connected to X output
const int pinY = A1;  // A1 - analog pin connected to Y output

// declare all the segments pins
const int pinA = 12;
const int pinB = 10;
const int pinC = 9;
const int pinD = 8;
const int pinE = 7;
const int pinF = 6;
const int pinG = 5;
const int pinDP = 4;

LedSegment aSegment(pinA);
LedSegment bSegment(pinB);
LedSegment cSegment(pinC);
LedSegment dSegment(pinD);
LedSegment eSegment(pinE);
LedSegment fSegment(pinF);
LedSegment gSegment(pinG);
LedSegment dpSegment(pinDP);
LedSegment* currentSegment;

byte swState = LOW;
byte lastSwState = HIGH;
int xValue = 0;
int yValue = 0;

bool joyMoved = false;
int minThreshold = 400;
int maxThreshold = 600;

int currentLedState = LOW;
const long ledBlinkInterval = 200;
long lastBlink = 0;

const unsigned int resetTime = 2000;
unsigned long lastResetTime = 0;

unsigned long lastDebounceTime = 0;
const unsigned int debounceDelay = 50;
volatile bool possiblePress = false;
volatile bool shouldStartCounter = false;
volatile bool possibleReset = false;
volatile bool startCounterForReset = false;

void setup() {
  // initialize all the pins
  initSegments();
  currentSegment = &dpSegment;

  pinMode(pinSW, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pinSW), handleInterrupt, FALLING);
}

void loop() {
  digitalWrite(currentSegment->pin, currentLedState);

  if (millis() - lastBlink > ledBlinkInterval) {
    currentLedState = !currentLedState;
    lastBlink = millis();
  }

  readJoystickAndMoveCurrentSegment();

  if (possiblePress) {
    readButton();
  }

  if (possibleReset) {
    checkForReset();
  }

  writeLedStates();
}

void handleInterrupt() {
  possiblePress = true;
  shouldStartCounter = true;
  possibleReset = true;
  startCounterForReset = true;
}

void initSegments() {
  aSegment.setSegments(nullptr, &gSegment, &fSegment, &bSegment);
  pinMode(aSegment.pin, OUTPUT);

  bSegment.setSegments(&aSegment, &gSegment, &fSegment, nullptr);
  pinMode(bSegment.pin, OUTPUT);

  cSegment.setSegments(&gSegment, &dSegment, &eSegment, &dpSegment);
  pinMode(cSegment.pin, OUTPUT);

  dSegment.setSegments(&gSegment, nullptr, &eSegment, &cSegment);
  pinMode(dSegment.pin, OUTPUT);

  eSegment.setSegments(&gSegment, &dSegment, nullptr, &cSegment);
  pinMode(eSegment.pin, OUTPUT);

  fSegment.setSegments(&aSegment, &gSegment, nullptr, &bSegment);
  pinMode(fSegment.pin, OUTPUT);

  gSegment.setSegments(&aSegment, &dSegment, nullptr, nullptr);
  pinMode(gSegment.pin, OUTPUT);

  dpSegment.setSegments(nullptr, nullptr, &cSegment, nullptr);
  pinMode(dpSegment.pin, OUTPUT);
}

void writeLedStates() {
  digitalWrite(aSegment.pin, aSegment.ledState);
  digitalWrite(bSegment.pin, bSegment.ledState);
  digitalWrite(cSegment.pin, cSegment.ledState);
  digitalWrite(dSegment.pin, dSegment.ledState);
  digitalWrite(eSegment.pin, eSegment.ledState);
  digitalWrite(fSegment.pin, fSegment.ledState);
  digitalWrite(gSegment.pin, gSegment.ledState);
  digitalWrite(dpSegment.pin, dpSegment.ledState);
}

void readJoystickAndMoveCurrentSegment() {
  xValue = analogRead(pinX);
  yValue = analogRead(pinY);

  if (xValue < minThreshold && joyMoved == false) {

    if (currentSegment->leftSegment != nullptr) {
      currentSegment = currentSegment->leftSegment;
    }
    joyMoved = true;
  }
  if (xValue > maxThreshold && joyMoved == false) {
    if (currentSegment->rightSegment != nullptr) {
      currentSegment = currentSegment->rightSegment;
    }
    joyMoved = true;
  }

  if (yValue < minThreshold && joyMoved == false) {

    if (currentSegment->bottomSegment != nullptr) {
      currentSegment = currentSegment->bottomSegment;
    }
    joyMoved = true;
  }
  if (yValue > maxThreshold && joyMoved == false) {
    if (currentSegment->topSegment != nullptr) {
      currentSegment = currentSegment->topSegment;
    }
    joyMoved = true;
  }

  if (xValue >= minThreshold && xValue <= maxThreshold && yValue >= minThreshold && yValue <= maxThreshold) {
    joyMoved = false;
  }
}

void readButton() {
  if (shouldStartCounter) {
    lastDebounceTime = millis();
    shouldStartCounter = false;
  }

  if (millis() - lastDebounceTime > debounceDelay) {
    swState = digitalRead(pinSW);
    if (swState != lastSwState && swState == LOW) {
      currentSegment->ledState = !currentSegment->ledState;
    }
    possiblePress = false;
  }
}

void checkForReset() {
  if (startCounterForReset) {
    lastResetTime = millis();
    startCounterForReset = false;
  }
  swState = digitalRead(pinSW);

  if (swState != lastSwState && swState == LOW) {
    if (millis() - lastResetTime > resetTime) {
      resetSegments();
      possibleReset = false;
    }
  } else {
    possibleReset = false;
  }
}

void resetSegments() {
  aSegment.ledState = LOW;
  bSegment.ledState = LOW;
  cSegment.ledState = LOW;
  dSegment.ledState = LOW;
  eSegment.ledState = LOW;
  fSegment.ledState = LOW;
  gSegment.ledState = LOW;
  dpSegment.ledState = LOW;
}