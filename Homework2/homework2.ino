/*
Task: Use a separate potentiometer for controlling each color of the RGB LED: Red, Green, and Blue.
This control must leverage digital electronics.
Specifically, you need to read the potentiometer’s value with Arduino and then write a mapped value to the LED pins.

https://github.com/danitns/IntroductionToRobotics
*/

const int redLedPin = 9;
const int greenLedPin = 10;
const int blueLedPin = 11;

const int redLedIntensityPin = A2;
const int greenLedIntensityPin = A1;
const int blueLedIntensityPin = A0;

long redLedIntensityValueRaw = 0;
long greenLedIntensityValueRaw = 0;
long blueLedIntensityValueRaw = 0;

long redLedIntensityValueMapped = 0;
long greenLedIntensityValueMapped = 0;
long blueLedIntensityValueMapped = 0;

const long minPotValue = 0;
const long maxPotValue = 1023;
const long minAnalogWriteValue = 0;
const long maxAnalogWriteValue = 255;

void setup() {
  pinMode(redLedPin, OUTPUT);
  pinMode(blueLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);
}

void loop() {
  // read the values from the potentiometers
  redLedIntensityValueRaw = analogRead(redLedIntensityPin);
  greenLedIntensityValueRaw = analogRead(greenLedIntensityPin);
  blueLedIntensityValueRaw = analogRead(blueLedIntensityPin);

  // map the values from 0 - 1023 to 0 - 255
  redLedIntensityValueMapped = mapPotValueToAnalogValue(redLedIntensityValueRaw);
  greenLedIntensityValueMapped = mapPotValueToAnalogValue(greenLedIntensityValueRaw);
  blueLedIntensityValueMapped = mapPotValueToAnalogValue(blueLedIntensityValueRaw);

  analogWrite(redLedPin, redLedIntensityValueMapped);
  analogWrite(greenLedPin, greenLedIntensityValueMapped);
  analogWrite(blueLedPin, blueLedIntensityValueMapped);
}


long mapPotValueToAnalogValue(long rawValue) {
  return map(rawValue, minPotValue, maxPotValue, minAnalogWriteValue, maxAnalogWriteValue);
}
