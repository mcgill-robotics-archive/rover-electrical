#include "DRV8833.h"
#include <Arduino.h>

DRV8833::DRV8833() {}

void DRV8833::Initialize(int _pin1, int _pin2)
{
  pin1 = _pin1;
  pin2 = _pin2;

  pinMode(_pin1, OUTPUT);
  pinMode(_pin2, OUTPUT);
}

void DRV8833::move(int power)
{
  if (power > 255)
    power = 255;
  if (power < -255)
    power = -255;

  if (power >= 0)
  {
    analogWrite(pin1, power);
    digitalWrite(pin2, LOW);
  }
  else if (power < 0)
  {
    digitalWrite(pin1, LOW);
    analogWrite(pin2, -power);  // Power is negative here so take the abs
  }
}

void DRV8833::stop()
{
  digitalWrite(pin1, LOW);
  digitalWrite(pin2, LOW);
}
