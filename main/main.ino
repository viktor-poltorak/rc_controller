#include <Arduino.h>
#include <Servo.h>
int STEERING_PIN = 9;
int ACCELERATOR_PIN = 8;

int STEERING_MID = 1500;
int ACCELERATOR_MID = 1500;
int acceleratorVal = 0;
int steeringVal = 0;
/*
  INIT commands
  SM:1500 - set STEERING_MID
  AM:1500 - set ACCELERATOR_MID
  state - print current state
*/

Servo wheelServo;
Servo acceleratorServo;

void setup() {
  wheelServo.attach(STEERING_PIN);
  acceleratorServo.attach(ACCELERATOR_PIN);
  Serial.begin(9600);
}

void loop() {
  if (Serial.available() > 0) {
    String command =  Serial.readStringUntil(';');
    command.trim();
    if(command.length() > 0) {
      Serial.print("Command: ");
      Serial.println(command);
      processCommand(command);
    }
  }
}

void processCommand(String command) {
  //wheelServo.writeMicroseconds(angle);
  if(command == "state") {
    printState();
    return;
  }

  if(command == "brake") {
    brake();
    return;
  }

  int pos = command.indexOf(':');
  if (pos != 0)
  {
    String subCommand =  command.substring(0, pos);
    String value = command.substring(pos+1, command.length());
    processSubCommand(subCommand, value);
  }
}

void processSubCommand(String subCommand, String value)
{
  Serial.print("Sub command: ");
  Serial.print(subCommand);
  Serial.print(" = ");
  Serial.println(value);

  if (subCommand == "AM") {
    ACCELERATOR_MID = value.toInt();
  }
  if (subCommand == "SM") {
    STEERING_MID = value.toInt();
  }
  if (subCommand == "A") {
    setAccelerator(value.toInt());
  }
  if (subCommand == "S") {
    setSteering(value.toInt());
  }
}

void brake()
{
  //If car moving forward
  if(acceleratorVal > ACCELERATOR_MID) {
      acceleratorVal = ACCELERATOR_MID - 300;
  }
  //If car moving backward
  if(acceleratorVal < ACCELERATOR_MID) {
      acceleratorVal = ACCELERATOR_MID + 100;
  }

  acceleratorServo.writeMicroseconds(acceleratorVal);
  delayMicroseconds(500);
  //Reset accelerator val
  acceleratorVal = ACCELERATOR_MID;
  acceleratorServo.writeMicroseconds(acceleratorVal);
}

void setAccelerator(int value) {
  acceleratorVal = value;
  acceleratorServo.writeMicroseconds(acceleratorVal);
}

void setSteering(int value) {
  steeringVal = value;
  wheelServo.writeMicroseconds(steeringVal);
}

void printState() {
  Serial.println("Current state:");
  Serial.print("ACCELERATOR_MID: ");
  Serial.println(ACCELERATOR_MID);
  Serial.print("STEERING_MID: ");
  Serial.println(STEERING_MID);
  Serial.print("steering value: ");
  Serial.println(steeringVal);
  Serial.print("accelerator value: ");
  Serial.println(acceleratorVal);
}
