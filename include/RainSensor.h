// RainSensor.h

#ifndef _RAINSENSOR_h
#define _RAINSENSOR_h

class RainSensor
{
 protected:
	 int _pinDigital = -1;
	 int _pinAnalog = -1;

 public:
	 RainSensor(int pinDigital, int pinAnalog = -1);
	 
	 void Setup();

	 bool IsWaterDetected();
};

#endif

