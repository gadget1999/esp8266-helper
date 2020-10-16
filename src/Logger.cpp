#ifdef WiFi_h
	#include "MQTTClient.h"
#endif

#include "Logger.h"

#define BUFFER_SIZE   256

LoggerClass Logger;

//void sendUdpSyslog(String msgtosend); // forward definition

void LoggerClass::SetLoggingLevel(int logLevel)  {
  this->_loggingLevel = logLevel;
}

void LoggerClass::SetMQTTLogTopic(const char * logTopic)  {
  this->_MQTTLogTopic = logTopic;
}

void LoggerClass::Log(int loggingLevel, const char *format, ...) {
	if (loggingLevel > this->_loggingLevel)
		return;

	char buf[BUFFER_SIZE] = { 0 };
    sprintf(buf, "%ld: ", millis());
    Serial.print(buf);
    
	va_list args;
	va_start(args, format);
	vsnprintf(buf, BUFFER_SIZE - 1, format, args);
	va_end(args);
	Serial.println(buf);

#ifdef WiFi_h
	if (NULL != _MQTTLogTopic)	{
		MQTTClient.Publish(_MQTTLogTopic, buf);
	}
#endif
}


