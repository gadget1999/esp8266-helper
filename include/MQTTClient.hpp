#ifndef MQTTClient_h
#define MQTTClient_h

#include <PubSubClient.h>

#include "Logger.h"
#include "RemoteCommand.hpp"

void MQTTCallback(char* topic, byte* payload, unsigned int length);
class MQTTClientClass : public RemoteCommand {
  protected:
    PubSubClient _transport;
    WiFiClient _wifiClient;

    bool Connect() {
      if (_transport.connected())
        return true;
      
      // Attempt to connect
      DEBUG("Connecting to MQTT server...");
      if (_transport.connect(_config->ServiceClientId, _config->ServerUserName, _config->ServerPassword)) {
        DEBUG("MQTT connected.");
        // resubscribe topic
        if (_config->ServiceEndPoint != NULL) {
          INFO("Subscribing MQTT topic: %s", _config->ServiceEndPoint);
          _transport.subscribe(_config->ServiceEndPoint);
        }

        return true;
      }
      else
      {
        return false;
      }
    }

  public:
    bool Setup(RemoteCommandConfig * config) {
      _transport.setClient(_wifiClient);

      if (!RemoteCommand::Setup(config))
        return false;

      _transport.setServer(config->ServerAddress, config->ServerPort);
      _transport.setCallback(MQTTCallback);

      return Connect();
    }

    // maxRetries: 0 means forever
    void Loop(int maxRetries = 3) {
      int nRetries = 0;
      while (!_transport.loop() || !_transport.connected()) {
        nRetries++;
        INFO("Retry MQTT connection (#%d). Current state: %d", nRetries, _transport.state());
        if (!Connect()) {
          ERROR("Failed to connect, code=%d", _transport.state());
          if ((maxRetries > 0) && (nRetries >= maxRetries)) {
            INFO("Give up re-connect after %d retries.", nRetries);
            break;
          }
          // Wait 500ms before retrying
          delay(500);
        }
      }

      // loop to process queue
      delay(100);      
    }

    bool Publish(const char* topic, const char* payload, bool retained = false) {
      return _transport.publish(topic, payload, retained);
    }
};

// singleton design due to no threading and limited memory
extern MQTTClientClass MQTTClient;

#endif
