// WifiConnection.h

#ifndef _WIFICONNECTION_h
#define _WIFICONNECTION_h

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

class WifiConnectionClass
{
 protected:
	 bool _APMode = false;
  bool _Connected = false;

	 const char * _SSID = NULL;
	 const char * _Password = NULL;
	 const char * _ClientName = NULL;

	 bool StartAsAP();
  bool StartAsStation();

 public:
	 void SetWifiInfo(const char *SSID, const char *Password = NULL, const char *ClientName = NULL);

	 bool Setup();
	 bool Loop();

  bool IsConnected();
  bool IsOnline(const char *server);
};

extern WifiConnectionClass WifiConnection;

#endif

