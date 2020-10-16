#include <Arduino.h>

#include <Logger.h>
#include <WifiConnection.h>
#include <RemoteCommand.hpp>
#include <MQTTClient.hpp>
#include <SmartDHT.h>
#include <RGBLight.hpp>
#include <RF.hpp>
#include <LightSensor.hpp>
#include <Configuration.h>

#define DEFAULT_OPTION_LEN  20

void setup() {
  Serial.begin(115200);
  Serial.println("Setup ...");

delay(6000);

  Logger.SetLoggingLevel(LOGGING_LEVEL_DEBUG);

  GlobalConfig.AddSchemaField("SSID", "SSID", DEFAULT_OPTION_LEN);
  GlobalConfig.AddSchemaField("Password", "Wifi Password", DEFAULT_OPTION_LEN);
  GlobalConfig.AddSchemaField("MQTT_Server", "MQTT Server Address", DEFAULT_OPTION_LEN);
  GlobalConfig.AddSchemaField("MQTT_User", "MQTT User", DEFAULT_OPTION_LEN);
  GlobalConfig.AddSchemaField("MQTT_Password", "MQTT Password", DEFAULT_OPTION_LEN);
  GlobalConfig.Setup();

  String test;
  test = GlobalConfig.Get("SSID");
  INFO("Config[SSID]=%s", test.c_str());

  test = GlobalConfig.Get("ESE");
  INFO("Config[ESE]=%s", test.c_str());

}

void loop() {}