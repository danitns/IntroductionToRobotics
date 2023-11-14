// Define connections to the shift register
const int latchPin = 11;  // Connects to STCP (latch pin) on the shift register
const int clockPin = 10;  // Connects to SHCP (clock pin) on the shift register
const int dataPin = 12;   // Connects to DS (data pin) on the shift register

// Define connections to the digit control pins for a 4-digit display
const int segD1 = 4;
const int segD2 = 5;
const int segD3 = 6;
const int segD4 = 7;

// Store the digits in an array for easy access
int displayDigits[] = { segD1, segD2, segD3, segD4 };
const int displayCount = 4;  // Number of digits in the display

// Define the number of unique encodings (0-9, A-F for hexadecimal)
const int encodingsNumber = 16;
// Define byte encodings for the hexadecimal characters 0-F
byte byteEncodings[encodingsNumber] = {
  //A B C D E F G DP
  B11111100,  // 0
  B01100000,  // 1
  B11011010,  // 2
  B11110010,  // 3
  B01100110,  // 4
  B10110110,  // 5
  B10111110,  // 6
  B11100000,  // 7
  B11111110,  // 8
  B11110110,  // 9
  B11101110,  // A
  B00111110,  // b
  B10011100,  // C
  B01111010,  // d
  B10011110,  // E
  B10001110   // F
};

// Variables for buttons
const int startButtonPin = 3;
const int resetButtonPin = 8;
const int lapButtonPin = 2;

volatile bool startButtonPressed = false;
volatile bool lapButtonPressed = false;

volatile unsigned long lastInterruptTimeStart = 0;
volatile unsigned long lastInterruptTimeLap = 0;

byte resetButtonState = LOW;
byte readingReset = HIGH;
byte lastReadingReset = HIGH;
unsigned int lastDebounceTimeForReset = 0;

const unsigned long debounceDelay = 200;

// Variables for controlling the display update timing

bool inStopwatchMode = true;
bool isStopwatchPaused = true;
unsigned long lastIncrement = 0;
unsigned long delayCount = 100;     // Delay between updates (milliseconds)
unsigned long stopwatchNumber = 0;  // The number being displayed
unsigned long displayNumber = 0;

const int memorySize = 4;
unsigned long lapMemory[memorySize];
unsigned int lapMemoryIndexForAdd = 0;
unsigned int lapMemoryIndexForDisplay = 0;

void setup() {
  // Initialize the pins connected to the shift register as outputs
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  // Initialize buttons
  initButtons();

  // Initialize digit control pins and set them to LOW (off)
  for (int i = 0; i < displayCount; i++) {
    pinMode(displayDigits[i], OUTPUT);
    digitalWrite(displayDigits[i], LOW);
  }
  // Begin serial communication for debugging purposes
  Serial.begin(9600);
}

void loop() {
  // the stopwatch has two states: stopwatch mode and view lap times mode
  if (inStopwatchMode) {
    if (!isStopwatchPaused) {
      incrementNumber();
    }
    displayNumber = stopwatchNumber;
  } else {
    displayNumber = lapMemory[lapMemoryIndexForDisplay];
  }

  // start button feature
  if (startButtonPressed) {
    if (inStopwatchMode) {
      isStopwatchPaused = !isStopwatchPaused;
    } else {
      inStopwatchMode = true;
      isStopwatchPaused = true;
    }

    startButtonPressed = false;
  }

  // reset button feature
  readingReset = digitalRead(resetButtonPin);
  if (readingReset != lastReadingReset) {
    lastDebounceTimeForReset = millis();
  }

  if ((millis() - lastDebounceTimeForReset) > debounceDelay) {
    if (readingReset != resetButtonState) {
      resetButtonState = readingReset;

      if (resetButtonState == LOW) {
        if (inStopwatchMode) {
          if (isStopwatchPaused) {
            stopwatchNumber = 0;
          }
        } else {
          for (int i = 0; i < memorySize; i++) {
            lapMemory[i] = 0;
          }
        }
      }
    }
  }
  lastReadingReset = readingReset;

  // lap button feature
  if (lapButtonPressed) {
    if (inStopwatchMode) {
      if (isStopwatchPaused) {
        inStopwatchMode = false;
      } else {
        Serial.println(stopwatchNumber);
        lapMemory[lapMemoryIndexForAdd] = stopwatchNumber;
        lapMemoryIndexForAdd++;
        if (lapMemoryIndexForAdd == memorySize) {
          lapMemoryIndexForAdd = 0;
        }
      }
    } else {
      lapMemoryIndexForDisplay++;
      if (lapMemoryIndexForDisplay == memorySize) {
        lapMemoryIndexForDisplay = 0;
      }
    }
    lapButtonPressed = false;
  }

  // display number
  writeNumber(displayNumber);
}

void initButtons() {
  pinMode(startButtonPin, INPUT_PULLUP);
  pinMode(resetButtonPin, INPUT_PULLUP);
  pinMode(lapButtonPin, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(startButtonPin), handleInterruptForStartButton, FALLING);
  attachInterrupt(digitalPinToInterrupt(lapButtonPin), handleInterruptForLapButton, FALLING);
}

void handleInterruptForStartButton() {
  static unsigned long interruptTimeForStartButton = 0;  // Timestamp for the current interrupt, retains its value between ISR calls
  interruptTimeForStartButton = micros();                // Capture the current time in microseconds
  // Debounce logic: If interrupts come in faster succession than the debounce delay, they're ignored
  if (interruptTimeForStartButton - lastInterruptTimeStart > debounceDelay * 1000) {  // Convert debounceDelay to microseconds for comparison
    startButtonPressed = true;
  }
  // Update the last interrupt timestamp
  lastInterruptTimeStart = interruptTimeForStartButton;
}

void handleInterruptForLapButton() {
  static unsigned long interruptTimeForLapButton = 0;
  interruptTimeForLapButton = micros();

  if (interruptTimeForLapButton - lastInterruptTimeLap > debounceDelay * 1000) {

    lapButtonPressed = true;
  }

  lastInterruptTimeLap = interruptTimeForLapButton;
}

void incrementNumber() {
  if (millis() - lastIncrement > delayCount) {
    stopwatchNumber++;

    // convert 60 seconds to minutes
    if ((stopwatchNumber / 100) % 10 == 6) {
      stopwatchNumber = stopwatchNumber / 1000 * 1000 + 1000;
    }

    stopwatchNumber %= 10000;  // Wrap around after 9999
    lastIncrement = millis();
  }
}

void writeReg(int digit) {
  // Prepare to shift data by setting the latch pin low
  digitalWrite(latchPin, LOW);
  // Shift out the byte representing the current digit to the shift register
  shiftOut(dataPin, clockPin, MSBFIRST, digit);
  // Latch the data onto the output pins by setting the latch pin high
  digitalWrite(latchPin, HIGH);
}


void activateDisplay(int displayNumber) {
  // Turn off all digit control pins to avoid ghosting
  for (int i = 0; i < displayCount; i++) {
    digitalWrite(displayDigits[i], HIGH);
  }
  // Turn on the current digit control pin
  digitalWrite(displayDigits[displayNumber], LOW);
}

void writeNumber(int number) {
  int currentNumber = number;
  int lastDigit = 0;

  // display each digit of the number or 0 if the number of digits < number of displays
  for (int displayDigit = 3; displayDigit >= 0; displayDigit--) {
    lastDigit = currentNumber % 10;
    activateDisplay(displayDigit);

    // turn on dp for correct format (Minute.SecondSecond.TenthOfSec)
    byte encodingOfDigit = byteEncodings[lastDigit];
    if (displayDigit == 0 || displayDigit == 2) {
      encodingOfDigit = encodingOfDigit | B00000001;
    }
    writeReg(encodingOfDigit);
    currentNumber /= 10;

    writeReg(B00000000);  // Clear the register to avoid ghosting
  }
}