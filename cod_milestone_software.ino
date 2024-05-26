#include <GP2Y0A02YK0F.h>
#include <SPI.h>
#include <SD.h>
#include <string.h>

GP2Y0A02YK0F irSensor;

const int chipSelect = 10;
char currentFile[16];
File file;

const int buttonPinStart = 2;
const int buttonPinReset = 3;

const int motorRotateStepPin = 9;
const int motorRotateDirPin = 8;
const int motorVerticalStepPin = 7;
const int motorVerticalDirPin = 6;

int sdCardInserted = 0;
int scanning = 0;
int reseting = 0;

int verticalTurns = 0;
int maxHeight = 10; // 10cm
int steps = 200; // steps for a full rotation
int verticalSteps = 10 * 200 * 2;
// 200 steps for a full rotation, the rod will advance 0.5mm
// so, to go up 1 cm, we will need 10mm * 200 steps / 0.5mm

void setup() {
  // initialize IR sensor
  irSensor.begin(A0);

  // initialize SD card, find file to write to
  sdCardInserted = SD.begin();
  if (!sdCardInserted) {
    return;
  }

  char fileName[16];
  int fileNo = 0;

  while (1) {
    memset(fileName, 0, 16);
    sprintf(fileName, "Scan%d", fileNo);
    if (!SD.exists(fileName)) {
      memset(currentFile, 0, 16);
      sprintf(currentFile, "%s", fileName);
      break;
    }
    fileNo++;
  }

  // initialize buttons
  pinMode(buttonPinStart, INPUT);
  pinMode(buttonPinReset, INPUT);

  //initialize motors
  pinMode(motorRotateStepPin, OUTPUT);
  pinMode(motorRotateDirPin, OUTPUT);
  pinMode(motorVerticalStepPin, OUTPUT);
  pinMode(motorVerticalDirPin, OUTPUT);
  verticalTurns = 0;
}

void loop() {
  int startButtonState = digitalRead(buttonPinStart);
  int resetButtonState = digitalRead(buttonPinReset);
  
  if (!sdCardInserted) {
    setup();
  }

  if (startButtonState == LOW && scanning == 0 && reseting == 0) {
    scanning = 1;
    setup();
    
    digitalWrite(motorVerticalDirPin, LOW);
    for (int i = 0; i < maxHeight; i++) {
      for (int j = 0; j < rotateSteps; j++) {
        rotateMotor(motorRotateStepPin, 1, 0);
        delay(200);
        double distance;
        distance = irSensor.getDistanceMilimeter();
        writeToSD(distance);
      }

      rotateMotor(motorVerticalStepPin, verticalSteps, 1);
      delay(1000);
      writeToSD(9999);
    }
    resetPosition();
  }

  if (resetButtonState == LOW && scanning == 1 && reseting == 0) {
    resetPosition();
    SD.remove(currentFile);
  }

}

void rotateMotor(int pinNo, int steps, int vertical) {
  for (int i=0; i<steps; i++) {
    digitalWrite(pinNo, LOW);
    delay(1);
    digitalWrite(pinNo, HIGH);
    delay(1);

    if (vertical) {
      verticalTurns++;
    }
  }
}

void writeToSD(double distance) {
  file = SD.open(currentFile, FILE_WRITE);

  if (file) {
    file.print(senseDistance);
    file.println();
    
    file.close();
  }
}

void resetPosition() {
  reseting = 1;
  digitalWrite(motorVerticalDirPin, HIGH);
  rotateMotor(motorVerticalStepPin, verticalTurns, 0);
  reseting = 0;
}
