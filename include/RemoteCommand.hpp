#ifndef RemoteCommand_h
#define RemoteCommand_h

//#include <FirebaseArduino.h>    // should not have included this, but its implementation has embedded JSON and not compatible with built-in
#include <ArduinoJson.h>
#include <functional>

#include "WifiConnection.h"

#define MAX_EVENT_HANDLERS		  5
#define REMOTE_COMMAND_HANDLER  std::function<void(JsonObject)>

typedef struct {
  const char * Name = NULL;
  REMOTE_COMMAND_HANDLER Handler;
} RemoteCommandHandler;

typedef struct {
  const char * WifiSSID = NULL;
  const char * WifiPassword = NULL;
  const char * WifiClientId = NULL;

  const char * ServerAddress = NULL;
  int ServerPort = 0;
  const char * ServerUserName = NULL;
  const char * ServerPassword = NULL;

  const char * ServiceEndPoint = NULL;
  const char * ServiceClientId = NULL;
} RemoteCommandConfig;

class RemoteCommand {
protected:
  RemoteCommandHandler _commands[MAX_EVENT_HANDLERS];
  RemoteCommandConfig * _config = NULL;

public:
  virtual bool Setup(RemoteCommandConfig * config) {
    _config = config;

    if (WifiConnection.IsConnected())
      return true;

    // setup WiFi if not connected yet
    DEBUG("Connecting to Wifi...");
    WifiConnection.SetWifiInfo(config->WifiSSID, config->WifiPassword, config->WifiPassword);
    if (!WifiConnection.Setup())
      return false;

    // ping server in station mode
    DEBUG("Pinging [%s]...", config->ServerAddress);
    if ((NULL != config->WifiClientId) && !WifiConnection.IsOnline(config->ServerAddress)) {
      ERROR("Ping to [%s] failed.", config->ServerAddress);
      return false;
    }

    return true;
  }

  bool RegisterCommand(const char *cmdName, REMOTE_COMMAND_HANDLER cmdHandler) {
    DEBUG("Register command: %s", cmdName);

    for (int i = 0; i < MAX_EVENT_HANDLERS; i++) {
      if (_commands[i].Name != NULL) {
        if (strcasecmp(_commands[i].Name, cmdName) != 0)
          continue;

        ERROR("Command handler [%s] already registered.", cmdName);
        return false;
      }

      // not found, add it and return
      _commands[i].Name = cmdName;
      _commands[i].Handler = cmdHandler;
      return true;
    }

    ERROR("Failed to register event handler [%s]: registry is full.", cmdName);
    return false;
  }

  bool DispatchCommand(const char *cmdName, const char *cmdArgs) {
    String cmdIn = String(cmdArgs);
    if (cmdIn[0] == '\"') cmdIn.remove(0, 1);
    if (cmdIn[cmdIn.length() - 1] == '\"') cmdIn.remove(cmdIn.length() - 1);

    StaticJsonDocument<256> jsonBuffer;
    deserializeJson(jsonBuffer, cmdIn);
    JsonObject params = jsonBuffer.as<JsonObject>();
    if (!params.isNull()) {
      ERROR("Failed to parse command arguments.");
      return false;
    }

    String name;
    if ((NULL == cmdName) || (strlen(cmdName) == 0)) {
      const char * tmpStr = (const char*)(params["CMD"]);
      if (NULL == tmpStr)   {
        ERROR("No command name specified.");
        return false;
      }
      name = String(tmpStr);
    }
    else {
      name = String(cmdName);
    }
    if (name.length() == 0) {
      ERROR("Invalid command name.");
      return false;
    }

    for (int i = 0; i < MAX_EVENT_HANDLERS; i++) {
      if (_commands[i].Name == NULL)
        break;	// end of registry, break out

      if (strcasecmp(_commands[i].Name, name.c_str()) != 0)
        continue;

      // found function
      DEBUG("Calling command handler: %s(%s)", name.c_str(), cmdIn.c_str());
      _commands[i].Handler(params);
      return true;
    }

    // handler not found
    ERROR("Cannot find command handler for [%s].", name.c_str());
    return false;
  }
};

#endif