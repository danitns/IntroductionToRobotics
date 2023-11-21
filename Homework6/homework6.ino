#include <EEPROM.h>

// EEPROM stored values
byte ultrasonicAlertThreshold = 4;
const int ultrasonicAlertThresholdAddress = 0;
int ldrAlertThreshold = 250;
const int ldrAlertThresholdAddress = sizeof(ultrasonicAlertThreshold) + ultrasonicAlertThresholdAddress;
unsigned int sensorSamplingInterval = 1000;
const int sensorSamplingIntervalAddress = sizeof(ldrAlertThreshold) + ldrAlertThresholdAddress;

// RGB LED variables
const int redLedPin = 11;
const int greenLedPin = 10;
const int blueLedPin = 9;
byte redLedIntensity = 0;
byte redLedIntensityFromUserInput = 0;
byte greenLedIntensity = 0;
byte greenLedIntensityFromUserInput = 0;
byte blueLedIntensity = 0;
byte blueLedIntensityFromUserInput = 0;
bool autoLed = true;

// photocell sensor variables
const int photocellPin = A0;

// ultrasonic sensor variables
const int trigPin = 6;
const int echoPin = 5;

// Variables for logged data
int ultrasonicSensorData[10];
int ldrSensorData[10];
byte index;
bool displayDataOnSerial = false;
long lastRead = 0;

enum menuType {MAIN_MENU,
 SENSORS_SETTINGS, SENSORS_SAMPLING_INTERVAL, ULTRASONIC_ALERT_THRESHOLD, LDR_ALERT_THRESHOLD,
 LOG_RESET, 
 SYS_STATUS, CURRENT_SENSOR_READING,  
 LED_CONTROL, MANUAL_COLOR_CONTROL, RED_SETTINGS, GREEN_SETTINGS, BLUE_SETTINGS};

enum menuType currentMenu = MAIN_MENU;

String mainMenu[4] = {"1. Sensor Settings", "2. Reset Logger Data", "3. System Status", "4. RGB Led Control"};
String submenu1[4] = {"1.1 Sensors Sampling Interval", "1.2 Ultrasonic Alert Threshold", "1.3 LDR Alert Threshold", "1.4 Back"};
String submenu2[2] = {"2.1 Yes", "2.2 No"};
String submenu3[4] = {"3.1 Current Sensor Readings", "3.2 Current Sensor Settings", "3.3 Display Logged Data", "3.4 Back"};
String submenu4[3] = {"4.1 Manual Color Control", "4.2 LED: Toggle Automatic ON/OFF", "4.3 Back"};   

void setup() {
  // Initialize serial communication at 9600 bits per second
  Serial.begin(9600);

  // uncomment following line to initialize EEPROM values when you are running the program for the first time
  // initEEPROMStoredValues();

  initPins();
  
  // Display the initial menu interface to the Serial Monitor upon startup
  printMenu(mainMenu, 4);
}

void loop() {
  // Get values from EEPROM
  getValuesFromEEPROM();

  // read values from sensors and log them 
  if(millis() - lastRead > sensorSamplingInterval) {
    updateSensors();
    lastRead = millis();
  }

  // Continuously checks for incoming serial data
  if (Serial.available()) {
    // Reads an integer value from the serial buffer (user's menu choice)
    int choice = Serial.parseInt();
    // Calls function to print a message based on the user's choice
    executeChosenAction(choice);

    displayMenu();
  }
}

void initPins() {
  // led
  pinMode(redLedPin, OUTPUT);
  pinMode(blueLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);

  // photocell
  pinMode(photocellPin, INPUT);

  // ultrasonic
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void initEEPROMStoredValues() { 
  EEPROM.put(ultrasonicAlertThresholdAddress, ultrasonicAlertThreshold);
  EEPROM.put(ldrAlertThresholdAddress, ldrAlertThreshold);
  EEPROM.put(sensorSamplingIntervalAddress, sensorSamplingInterval);
}

void getValuesFromEEPROM() {
  EEPROM.get(ultrasonicAlertThresholdAddress, ultrasonicAlertThreshold);
  EEPROM.get(ldrAlertThresholdAddress, ldrAlertThreshold);
  EEPROM.get(sensorSamplingIntervalAddress, sensorSamplingInterval);
}

void updateSensors() {
  int valueFromUltrasonicSensor = 0;
  int valueFromPhotocellSensor = 0;

  // read values from sensors and add them to the arrays
  valueFromUltrasonicSensor = readValueFromUltrasonicSensor();
  ultrasonicSensorData[index] = valueFromUltrasonicSensor;

  valueFromPhotocellSensor = readValueFromPhotocellSensor();
  ldrSensorData[index] = valueFromPhotocellSensor;

  index++;
  if(index > 9) {
    index = 0;
  }

  // display data if we are in the CURRENT_SENSOR_READING menu
  if(displayDataOnSerial) {
    displayData(valueFromUltrasonicSensor, valueFromPhotocellSensor);
  }

  // update led color after every sensor read
  updateLedColor(valueFromUltrasonicSensor, valueFromPhotocellSensor);
}

int readValueFromUltrasonicSensor() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  // Sound wave reflects from the obstacle, so to calculate the distance we consider half of the distance traveled.
  int distance = duration*0.034/2;
  
  return distance;
}

int readValueFromPhotocellSensor() {
  int photocellValue = analogRead(photocellPin);
  return photocellValue;
}

void displayData(int ultrasonicSensorValue, int photocellSensorValue) {
  if(displayDataOnSerial) {
    Serial.print("Distance: ");
    Serial.print(ultrasonicSensorValue);
    Serial.println();
    Serial.print("Photocell value: ");
    Serial.print(photocellSensorValue);
    Serial.println();
  }
}

void updateLedColor(int ultrasonicSensorValue, int photocellSensorValue) {
  if(autoLed) {
    // display red when values are less than min thresholds or display a red-orange-green color
    if(ultrasonicSensorValue < ultrasonicAlertThreshold || photocellSensorValue < ldrAlertThreshold) {
      redLedIntensity = 255;
      greenLedIntensity = 0;
    } else {
      byte redIntensityFromUltrasonic = map(ultrasonicSensorValue, 2400, 2, 0, 255);
      byte redIntensityFromPhotocell = map(photocellSensorValue, 750, 200, 0, 255);

      byte greenIntensityFromUltrasonic = map(ultrasonicSensorValue, 2, 2400, 0, 255);
      byte greenIntensityFromPhotocell = map(photocellSensorValue, 200, 750, 0, 255);

      // calculate the mean of the two intensities
      redLedIntensity = (redIntensityFromUltrasonic + redIntensityFromPhotocell) / 2;
      greenLedIntensity = (greenIntensityFromUltrasonic + greenIntensityFromPhotocell) / 2;
      blueLedIntensity = 0;
    }
    analogWrite(redLedPin, redLedIntensity);
    analogWrite(greenLedPin, greenLedIntensity);
    analogWrite(blueLedPin, blueLedIntensity);
  } else {
    analogWrite(redLedPin, redLedIntensityFromUserInput);
    analogWrite(greenLedPin, greenLedIntensityFromUserInput);
    analogWrite(blueLedPin, blueLedIntensityFromUserInput);
  }
}


// Function to display a menu of options to the user
void printMenu(String options[], int numberOfElements) {
  for(int i = 0; i < numberOfElements; i++) {
    Serial.println(options[i]);
  }
}

void executeChosenAction(int option) {
  switch (currentMenu) {
    case MAIN_MENU:
      changePrintedMenu(option);
      break;
    case SENSORS_SETTINGS:
      changeSettings(option);
      break;
    case SENSORS_SAMPLING_INTERVAL:
      changeSamplingInterval(option);
      break;
    case ULTRASONIC_ALERT_THRESHOLD:
      changeUltrasonicAlertThreshold(option);
      break;
    case LDR_ALERT_THRESHOLD:
      changeLdrAlertThreshold(option);
      break;
    case LOG_RESET:
      resetData(option);
      break;
    case SYS_STATUS:
      displaySystemStatus(option);
      break;
    case CURRENT_SENSOR_READING:
      displayCurrentReadings(option);
      break;
    case LED_CONTROL:
      changeLedSettings(option);
      break;
    case RED_SETTINGS:
      redLedIntensityFromUserInput = (option >= 0 && option < 256) ? option : 0;
      currentMenu = GREEN_SETTINGS;
      break;
    case GREEN_SETTINGS:
      greenLedIntensityFromUserInput = (option >= 0 && option < 256) ? option : 0;
      currentMenu = BLUE_SETTINGS;
      break;
    case BLUE_SETTINGS:
      blueLedIntensityFromUserInput = (option >= 0 && option < 256) ? option : 0;
      currentMenu = LED_CONTROL;
      break;
    default:
      Serial.println("Error in executing action");
      break;
  }
}

void changePrintedMenu(int option) {
  switch (option) {
    case 1:
      currentMenu = SENSORS_SETTINGS;
      break;
    case 2:
      currentMenu = LOG_RESET;
      break;
    case 3:
      currentMenu = SYS_STATUS;
      break;
    case 4:
      currentMenu = LED_CONTROL;
      break;
    default:
      currentMenu = MAIN_MENU;
      break;
  }
}

void changeSettings(int option){
  switch (option) {
    case 1:
      currentMenu = SENSORS_SAMPLING_INTERVAL;
      break;
    case 2:
      currentMenu = ULTRASONIC_ALERT_THRESHOLD;
      break;
    case 3:
      currentMenu = LDR_ALERT_THRESHOLD;
      break;
    case 4:
      currentMenu = MAIN_MENU;
      break;
    default:
      Serial.println("INVALID OPTION");
      currentMenu = SENSORS_SETTINGS;
      break;
  }
}

void changeSamplingInterval(int value) {
  if(value < 1 || value > 10) {
    Serial.println("Incorrect value");
  } else {
    EEPROM.put(sensorSamplingIntervalAddress, value * 1000);
    Serial.print("New sampling value is ");
    Serial.print(value);
    Serial.println();
    currentMenu = SENSORS_SETTINGS;
  }
}

void changeUltrasonicAlertThreshold(int value) {
  if(value < 1 || value > 50) {
    Serial.println("Incorrect value");
  } else {
    EEPROM.put(ultrasonicAlertThresholdAddress, value);
    Serial.print("New sampling value is ");
    Serial.print(value);
    Serial.println();
    currentMenu = SENSORS_SETTINGS;
  }
}

void changeLdrAlertThreshold(int value) {
  if(value < 100 || value > 300) {
    Serial.println("Incorrect value");
  } else {
    EEPROM.put(ldrAlertThresholdAddress, value);
    Serial.print("New sampling value is ");
    Serial.print(value);
    Serial.println();
    currentMenu = SENSORS_SETTINGS;
  }
}

void resetData(int value) {
  if(value == 1) {
    deleteDataFromArrays();
    Serial.println("Reset completed");
  }
  currentMenu = MAIN_MENU;
}

void deleteDataFromArrays() {
  for(int i = 0; i < 10; i++) {
    ldrSensorData[i] = 0;
    ultrasonicSensorData[i] = 0;
  }
}

void displaySystemStatus(int option) {
  switch(option) {
    case 1:
      currentMenu = CURRENT_SENSOR_READING;
      break;
    case 2:
      displayCurrentSensorSettings();
      break;
    case 3:
      displayLoggedData();
      break;
    case 4:
      currentMenu = MAIN_MENU;\
      break;
    default:
      Serial.println("Invalid option");
      break;
  }
} 

void displayCurrentReadings(int option) {
  if(option) {
    displayDataOnSerial = false;
    currentMenu = SYS_STATUS;
  }
}

void displayCurrentSensorSettings() {
  Serial.println("Current settings: ");
  Serial.print(F("Min threshold for ultrasonic sesnsor: "));
  Serial.print(ultrasonicAlertThreshold);
  Serial.println();
  Serial.print(F("Min threshold for LDR sensor: "));
  Serial.print(ldrAlertThreshold);
  Serial.println();
  Serial.print(F("Sampling rate: "));
  Serial.print(sensorSamplingInterval);
  Serial.println();
}

void displayLoggedData() {
  Serial.println("Logged data: ");
  Serial.println("Values from ultrasonic sensor: ");
  displayDataFromArray(ultrasonicSensorData, 10);
  Serial.println("Values from LDR sensor: ");
  displayDataFromArray(ldrSensorData, 10);
}

void displayDataFromArray(int array[], byte size) {
  for(byte i = 0; i < size; i++) {
    Serial.print(array[i]);
    Serial.print(" ");
  }
  Serial.println();
}

void changeLedSettings(int option) {
  switch(option) {
    case 1:
      currentMenu = RED_SETTINGS;
      break;
    case 2:
      autoLed = !autoLed;
      if(autoLed) {
        Serial.println("Auto LED is ON");
        break;
      }
      Serial.println("Auto LED is OFF");
      break;
    case 3:
      currentMenu = MAIN_MENU;
      break;
    default:
      Serial.println("Invalid option.");
      break;
  }
}

// Function to print different messages based on the user's selection
void displayMenu() {
  switch (currentMenu) {
    case MAIN_MENU:
      printMenu(mainMenu, 4);
      break;
    case SENSORS_SETTINGS:
      printMenu(submenu1, 4);
      break;
    case SENSORS_SAMPLING_INTERVAL:
      Serial.println(F("Please enter a value between 1 and 10 (seconds) for sampling interval"));
      break;
    case ULTRASONIC_ALERT_THRESHOLD:
      Serial.println(F("Please enter a value between 1 and 50 for the ultrasonic sensor"));
      break;
    case LDR_ALERT_THRESHOLD:
      Serial.println(F("Please enter a value between 100 and 300 for the ldr sensor"));
      break;
    case LOG_RESET:
      printMenu(submenu2, 2);
      break;
    case SYS_STATUS:
      printMenu(submenu3, 4);
      break;
    case CURRENT_SENSOR_READING:
      Serial.println(F("Press any number to exit. Values from sensors:"));
      displayDataOnSerial = true;
      break;
    case LED_CONTROL:
      printMenu(submenu4, 3);
      break;
    case RED_SETTINGS:
      Serial.println(F("Insert a value between 0 and 255 for red color intensity."));
      break;
    case GREEN_SETTINGS:
      Serial.println(F("Insert a value between 0 and 255 for green color intensity."));
      break;
    case BLUE_SETTINGS:
      Serial.println(F("Insert a value between 0 and 255 for blue color intensity."));
      break;
    default:
      printMenu(mainMenu, 4);
      break;
  }
}