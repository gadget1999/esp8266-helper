#ifndef _RGBLIGHT_h
#define _RGBLIGHT_h

#if defined(ARDUINO) && ARDUINO >= 100
 #include <Arduino.h>
#else
 #include "WProgram.h"
#endif

typedef struct ColorCode {
  byte Red;
  byte Green;
  byte Blue;
} ColorCode;

ColorCode COLOR_CODE_BLACK = { 0, 0, 0 };
ColorCode COLOR_CODE_RED = { 255, 0, 0 };
ColorCode COLOR_CODE_ORANGE = { 255, 140, 0 };
ColorCode COLOR_CODE_YELLOW = { 255, 255, 0 };
ColorCode COLOR_CODE_GREEN = { 0, 255, 0 };
ColorCode COLOR_CODE_AQUA = { 0, 255, 255 };
ColorCode COLOR_CODE_BLUE = { 0, 0, 255 };
ColorCode COLOR_CODE_PURPLE = { 255, 0, 255 };

class RGBLight
{
  protected:
    int _pinRed = 8;
    int _pinGreen = 6;
    int _pinBlue = 7;

    ColorCode _lastColor = { 0, 0, 0 };

  public:
    RGBLight(int pinRed, int pinGreen, int pinBlue)  {
      _pinRed = pinRed;
      _pinGreen = pinGreen;
      _pinBlue = pinBlue;
    }

    void Setup()  {
      pinMode(_pinRed, OUTPUT);
      pinMode(_pinGreen, OUTPUT);
      pinMode(_pinBlue, OUTPUT);
    }

    void Demo1()  {	// rain bow blink
      INFO("Start to show demo1.");
      ShowColorBlink(COLOR_CODE_RED);
      ShowColorBlink(COLOR_CODE_ORANGE);
      ShowColorBlink(COLOR_CODE_YELLOW);
      ShowColorBlink(COLOR_CODE_GREEN);
      ShowColorBlink(COLOR_CODE_AQUA);
      ShowColorBlink(COLOR_CODE_BLUE);
      ShowColorBlink(COLOR_CODE_PURPLE);
    }

    void Demo2()  {	// random blink
      INFO("Start to show demo2.");
      ColorCode color;
      color.Red = random(255);
      color.Green = random(255);
      color.Red = random(255);
      ShowColorBlink(color);
    }

    void ShowColor(ColorCode color) {
      analogWrite(_pinRed, color.Red);
      analogWrite(_pinGreen, color.Green);
      analogWrite(_pinBlue, color.Blue);
      _lastColor = color;
    }

    void ShowColorBlink(ColorCode color, int step = 500)  {
      ShowColor(COLOR_CODE_BLACK);
      delay(step);
      ShowColor(color);
      delay(step);
    }

    void ShowColorTransition(ColorCode color) {
      const int transionSteps = 50;
      const int transionTime = 2000;

      float stepRed = (float)(color.Red - _lastColor.Red) / transionSteps;
      float stepGreen = (float)(color.Green - _lastColor.Green) / transionSteps;
      float stepBlue = (float)(color.Blue - _lastColor.Blue) / transionSteps;
      int stepLength = transionTime / transionSteps;

      for (int i = 1; i <= transionSteps; i++) {
        ColorCode tmpColor;
        tmpColor.Red = (int)(_lastColor.Red + stepRed * i);
        tmpColor.Green = (int)(_lastColor.Green + stepGreen * i);
        tmpColor.Blue = (int)(_lastColor.Blue + stepBlue * i);

        ShowColor(tmpColor);
        delay(stepLength);
      }
    }

    #define MAX_COLOR_SIZE  16
    ColorCode GetColorCode(const char *color) {
      char buffer[MAX_COLOR_SIZE] = { 0 };
      size_t len = strlen(color);
      if (len >= MAX_COLOR_SIZE)
        len = MAX_COLOR_SIZE - 1;
      strncpy(buffer, color, len);

      char delimiters[] = "-:, ";
      char* value;
      ColorCode ret = {0, 0, 0};

      value = strtok(buffer, delimiters);
      if (value != NULL)  {
        //DEBUG("red: %s", value);
        ret.Red = (byte)atoi(value);
      }

      value = strtok(NULL, delimiters);
      if (value != NULL)  {
        //DEBUG("green: %s", value);
        ret.Green = (byte)atoi(value);
      }

      value = strtok(NULL, delimiters);
      if (value != NULL)  {
        //DEBUG("blue: %s", value);
        ret.Blue = (byte)atoi(value);
      }

      DEBUG("Color code: (%u-%u-%u)", ret.Red, ret.Green, ret.Blue);
      return ret;
    }
};

#endif

