#include "SmartDHT.h"
#include "Logger.h"

SmartDHT::SmartDHT(uint8_t pin, uint8_t type)	{
	_DHTSensor = new DHT(pin, type);
}

SmartDHT::~SmartDHT()	{
	if (_DHTSensor != NULL)
		delete _DHTSensor;
}

void SmartDHT::SetTopicTemperature(const char *topic) {
	_TopicTemperature = topic;
}

void SmartDHT::SetTopicHumidity(const char *topic) {
	_TopicHumidity = topic;
}

bool SmartDHT::CheckBound(float newValue, float prevValue, float maxDiff) {
	return !isnan(newValue) &&
		(newValue < prevValue - maxDiff || newValue > prevValue + maxDiff);
}

bool SmartDHT::Setup() {
	if ((NULL == _TopicTemperature) ||
		(NULL == _TopicHumidity)
		)
		return false;

	_DHTSensor->begin();
	return true;
}

void SmartDHT::Loop() {
}

#define SENSOR_OK                 0x0
#define SENSOR_TEMPERATURE_FAILED 0x1
#define SENSOR_HUMIDITY_FAILED    0x2

bool SmartDHT::ReportData(float delta) {
	int rc = SENSOR_OK;

	float newTemp = _DHTSensor->readTemperature(true);
	if (isnan(newTemp))
		rc |= SENSOR_TEMPERATURE_FAILED;
	DEBUG("Temperature reading: %d", (int)newTemp);

	float newHum = _DHTSensor->readHumidity();
	if (isnan(newHum))
		rc |= SENSOR_HUMIDITY_FAILED;
	DEBUG("Humidity reading: %d", (int)newHum);

	//  float newTemp = 77;
	//  float newHum = 33;

	if (CheckBound(newTemp, _lastTemperature, delta)) {
		_lastTemperature = newTemp;
		INFO("New temperature: %d", (int)newTemp);
		MQTTClient.Publish(_TopicTemperature, String(newTemp).c_str(), true);
	}

	if (CheckBound(newHum, _lastHudimity, delta)) {
		_lastHudimity = newHum;
		INFO("New humidity: %d", (int)newHum);
		MQTTClient.Publish(_TopicHumidity, String(newHum).c_str(), true);
	}

//	String status = String((rc * 10) + (actionCount % 10));
	//_MQTTClient->Publish(TOPIC_STATUS, status.c_str());

	return (rc != SENSOR_OK);
}
