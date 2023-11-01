// MyFloor works like a linked list
struct MyFloor {
  int floorNumber;
  int buttonPin;
  int ledPin;
  MyFloor* nextFloor;
  MyFloor* previousFloor;

  MyFloor(int _floorNumber = 0, int _buttonPin = 0, int _ledPin = 0, MyFloor* _nextFloor = nullptr, MyFloor* _previousFloor = nullptr)
    : floorNumber(_floorNumber), buttonPin(_buttonPin), ledPin(_ledPin), nextFloor(_nextFloor), previousFloor(_previousFloor) {
  }
};

MyFloor floor1(1, 13, 12);
MyFloor floor2(2, 11, 10, nullptr, &floor1);
MyFloor floor3(3, 9, 8, nullptr, &floor2);

const int elevatorLed = 7;

const long elevatorLedBlinkTime = 200;
unsigned long previousMilisForElevatorLedBlink = 0;
byte elevatorLedState = LOW;

const long closingDoorsTime = 2000;
unsigned long previousMillisForClosingDoorsTime = 0;
const long elevatorMovingTime = 2000;
unsigned long previousMillisForElevatorMovingTime = 0;

const long openingDoorsTime = 500;
unsigned long previousMillisForOpeningDoorsTime = 0;

const int buzzerPin = 6;
const int baseBuzzerTone = 1000;
const int buzzerToneVariationForClosingDoors = 100;
const int buzzerToneForOpeningDoors = 2000;
int buzzerTone = 0;


MyFloor currentFloor;
MyFloor desiredFloor;
byte areDoorsClosing = false;
byte isElevatorMoving = false;
byte areDoorsOpening = false;


void setup() {
  floor1.nextFloor = &floor2;
  floor2.nextFloor = &floor3;

  currentFloor = floor1;
  desiredFloor = currentFloor;

  pinMode(floor1.buttonPin, INPUT_PULLUP);
  pinMode(floor1.ledPin, OUTPUT);

  pinMode(floor2.buttonPin, INPUT_PULLUP);
  pinMode(floor2.ledPin, OUTPUT);

  pinMode(floor3.buttonPin, INPUT_PULLUP);
  pinMode(floor3.ledPin, OUTPUT);

  pinMode(elevatorLed, OUTPUT);
}

void loop() {
  unsigned long currentMillis = millis();
  
  // turn on only the current floor led
  digitalWrite(elevatorLed, LOW);
  digitalWrite(floor1.ledPin, LOW);
  digitalWrite(floor2.ledPin, LOW);
  digitalWrite(floor3.ledPin, LOW);
  digitalWrite(currentFloor.ledPin, HIGH);


  if (desiredFloor.floorNumber != currentFloor.floorNumber) {
    // elevator has two states when this condition is true:
    // the doors are closing or the elevator is moving
    if (areDoorsClosing) {
      if (currentMillis - previousMillisForClosingDoorsTime >= closingDoorsTime) {
        areDoorsClosing = false;
        isElevatorMoving = true;
        previousMillisForElevatorMovingTime = currentMillis;
      }

      if (currentMillis - previousMilisForElevatorLedBlink >= elevatorLedBlinkTime) {
        buzzerTone = baseBuzzerTone + (buzzerToneVariationForClosingDoors * elevatorLedState);
        tone(buzzerPin, buzzerTone);
        elevatorLedState = !elevatorLedState;
        previousMilisForElevatorLedBlink = currentMillis;
      }
    }

    if (isElevatorMoving) {
      if (currentMillis - previousMillisForElevatorMovingTime >= elevatorMovingTime) {
        if (currentFloor.floorNumber < desiredFloor.floorNumber) {
          currentFloor = *currentFloor.nextFloor;
        } else {
          currentFloor = *currentFloor.previousFloor;
        }
        previousMillisForElevatorMovingTime = currentMillis;

        // check if the elevator is at the desired floor
        if (currentFloor.floorNumber == desiredFloor.floorNumber) {
          areDoorsOpening = true;
          previousMillisForOpeningDoorsTime = currentMillis;
        }
      }
      elevatorLedState = HIGH;
    }

    digitalWrite(elevatorLed, elevatorLedState);
  } else {
    // two more states when elevator is at desired floor
    // the doors are opening or the elevator is stationary and we can read user input
    isElevatorMoving = false;
    if (areDoorsOpening) {
      tone(buzzerPin, buzzerToneForOpeningDoors);
      if (currentMillis - previousMillisForOpeningDoorsTime >= openingDoorsTime) {
        areDoorsOpening = false;
      }
    } else {
      noTone(buzzerPin);
      // the debounce is created by reading the user input(pressed button) only when the elevator is on the desired floor and the doors are not opening
      desiredFloor = readButtons(currentMillis);
    }
  }
}

MyFloor readButtons(unsigned long currentMillis) {
  MyFloor value = currentFloor;
  if (!isElevatorMoving) {
    value = (!digitalRead(floor1.buttonPin) && floor1.buttonPin != currentFloor.buttonPin) ? 
                floor1 : ((!digitalRead(floor2.buttonPin) && floor2.buttonPin != currentFloor.buttonPin) ? 
                            floor2 : ((!digitalRead(floor3.buttonPin) && floor3.buttonPin != currentFloor.buttonPin) ? 
                                          floor3 : currentFloor));
    if (value.buttonPin != currentFloor.buttonPin) {
      areDoorsClosing = true;
      previousMillisForClosingDoorsTime = currentMillis;
      desiredFloor = value;
    }
  }
  return value;
}
