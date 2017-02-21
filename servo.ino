#include <Servo.h>
int WHEEL_PIN = 9;
int angle = 1500; // переменная для хранения позиции сервы
Servo wheelServo;

void setup() {
  wheelServo.attach(WHEEL_PIN);
  Serial.begin(9600);
}

void loop() {
  if (Serial.available() > 0) {
    int input = Serial.parseInt();
    if (input > 0) {
      angle = input;
      Serial.println(angle);
      wheelServo.writeMicroseconds(angle);
    }
  }

 // wheelServo.writeMicroseconds(angle);
//  delayMicroseconds(20);
//  wheelServo.writeMicroseconds(angle + 30);  
}

