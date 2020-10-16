
#ifndef Logger_h
#define Logger_h

#include <Arduino.h>
#include <stdarg.h>

#ifndef LOGGING_LEVEL_NONE
#define LOGGING_LEVEL_NONE 0
#endif

#ifndef LOGGING_LEVEL_ERROR
#define LOGGING_LEVEL_ERROR 1
#endif

#ifndef LOGGING_LEVEL_INFO
#define LOGGING_LEVEL_INFO 2
#endif

#ifndef LOGGING_LEVEL_DEBUG
#define LOGGING_LEVEL_DEBUG 3
#endif

class LoggerClass {
  private:
    int	_loggingLevel = LOGGING_LEVEL_INFO;
    const char * _MQTTLogTopic = NULL;

  public:
    void SetLoggingLevel(int logLevel);
    void SetMQTTLogTopic(const char * logTopic);

	void Log(int loggingLevel, const char *fmt, ...);
};

// singleton design due to no threading and limited memory
extern LoggerClass Logger;

#define ERROR(fmt, ...)		Logger.Log(LOGGING_LEVEL_ERROR, fmt, ##__VA_ARGS__)
#define INFO(fmt, ...)		Logger.Log(LOGGING_LEVEL_INFO, fmt, ##__VA_ARGS__)
#define DEBUG(fmt, ...)		Logger.Log(LOGGING_LEVEL_DEBUG, fmt, ##__VA_ARGS__)

#endif
