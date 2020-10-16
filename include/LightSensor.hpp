#ifndef _LIGHTSENSOR_h
#define _LIGHTSENSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
 #include <Arduino.h>
#else
 #include "WProgram.h"
#endif

class LightSensor
{
  protected:
    byte _pinLight;

  public:
    LightSensor(byte pin) {
      _pinLight = pin;
    }
    
    void Setup()  {
      pinMode(_pinLight, INPUT);
    }

    long GetLightLevel() {
      return analogRead(_pinLight);
    }
};

#endif

