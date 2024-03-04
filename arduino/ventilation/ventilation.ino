#include "wind.h"

int pin0 = 0;
int pin1 = 0;
int pin2 = 0;
int pin3 = 0;
int door = 0;

float mph = 0.0;

void setup() {
    Serial.begin(115200);
    Serial.println("start");
  
  // put your setup code here, to run once:
  pinMode(13, INPUT);
  pinMode(12, INPUT);
  pinMode(11, INPUT);
  pinMode(10, INPUT);
  pinMode(9, INPUT);

  wind_setup();
}

void loop() {
  // put your main code here, to run repeatedly:
  pin0 = digitalRead(13);
  pin1 = digitalRead(12);
  pin2 = digitalRead(11);
  pin3 = digitalRead(10);
  door = digitalRead(9);

  Serial.print("[DATA]");
  Serial.print("mph:");
  Serial.print(mph);

  Serial.print(", door:");
  Serial.print(door);
  Serial.print(", pos0:");
  Serial.print(pin0);
  Serial.print(", pos1:");
  Serial.print(pin1);
  Serial.print(", pos2:");
  Serial.print(pin2);
  Serial.print(", pos3:");
  Serial.println(pin3);

  mph = wind_read();


  delay(1000);
  
}
