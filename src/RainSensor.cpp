// 
// 
// 

#include <Arduino.h>
#include "RainSensor.h"

RainSensor::RainSensor(int pinDigital, int pinAnalog) {
	_pinDigital = pinDigital;
	_pinAnalog = pinAnalog;
}

void RainSensor::Setup()	{
	pinMode(_pinDigital, INPUT);
}

bool RainSensor::IsWaterDetected() {
	int val = digitalRead(_pinDigital);
	return (val == 0);
}

