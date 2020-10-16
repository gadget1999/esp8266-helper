
#include "MQTTClient.hpp"

#define BUFFER_SIZE   256

MQTTClientClass MQTTClient;

void MQTTCallback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  String cmdValue = (char *)payload;

  // use sub-topic as the command name
  String cmdName = String(topic);
  int posDelimiter = cmdName.indexOf('/');
  if ((posDelimiter > 0) && (posDelimiter < (int)length - 1)) {
    cmdName = cmdName.substring(posDelimiter);
  }

  MQTTClient.DispatchCommand(cmdName.c_str(), cmdValue.c_str());
}

