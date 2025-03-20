#include "Otto.h"
#include <ps5Controller.h>

Otto myOtto; // This is Otto!

#define LeftLeg 17
#define LeftFoot 18
#define RightLeg 4
#define RightFoot 16
#define BT_LED 2
#define PS5ControllerId "7c:66:ef:12:fa:6e"

void setup()
{
  Serial.begin(115200);

  pinMode(BT_LED, OUTPUT);
  ps5.begin(PS5ControllerId);
  myOtto.init(LeftLeg, RightLeg, LeftFoot, RightFoot, false, 34); // Set the servo pins and Buzzer pin
  myOtto.home();
  delay(1000);
}

void loop()
{
  while (ps5.isConnected() == false)
  {
    digitalWrite(BT_LED, LOW);
    Serial.println("PS5 controller not found");
    delay(300);
  }

  digitalWrite(BT_LED, HIGH);
  Serial.println("Ready");

  while (ps5.isConnected() == true)
  {
    if (ps5.Up())
    {
      Serial.println("Forward");
      myOtto.walk(2, 1000, 1);
      delay(1700);
    }
    else if (ps5.Down())
    {
      Serial.println("Backward");
      myOtto.walk(2, 1000, -1);
      delay(1700);
    }
    else if (ps5.Left())
    {
      Serial.println("Left Turn");
      myOtto.turn(2, 1000, 1); // 3 steps turning LEFT
      delay(1700);
    }
    else if (ps5.Right())
    {
      Serial.println("Right Turn");
      myOtto.turn(2, 1000, -1);
      delay(1700);
    }
    else if (ps5.Circle())
    {
      Serial.println("Home");
      myOtto.setRestState(false);
      myOtto.home();
      delay(1000);
    }
  }
}