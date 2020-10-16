#ifndef SmartDHT_h
#define SmartDHT_h

#include <DHT.h>
#include "MQTTClient.hpp"

class SmartDHT
{
private:
	DHT * _DHTSensor = NULL;
	float _lastTemperature = FP_NAN;
	float _lastHudimity = FP_NAN;

	const char * _TopicTemperature = NULL;
	const char * _TopicHumidity = NULL;

	bool CheckBound(float newValue, float prevValue, float maxDiff);

public:
	SmartDHT(uint8_t pin, uint8_t type);
	~SmartDHT();

	void SetTopicTemperature(const char *topic);
	void SetTopicHumidity(const char *topic);

	bool Setup();
	void Loop();

	bool ReportData(float delta);
};

#endif