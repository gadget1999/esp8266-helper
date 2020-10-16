// 
// 
// 

#include "WifiConnection.h"

#include "Logger.h"

WifiConnectionClass WifiConnection;

void WiFiEvent(WiFiEvent_t event) {
	DEBUG("[WiFi-event] event: %d", event);

	switch (event) {
	case WIFI_EVENT_STAMODE_GOT_IP:
		INFO("WiFi connected. IP address: ", WiFi.localIP().toString().c_str());
		break;
	case WIFI_EVENT_STAMODE_DISCONNECTED:
		ERROR("WiFi lost connection.");
		break;
	default:
		break;
	}
}

void WifiConnectionClass::SetWifiInfo(const char *SSID, const char *Password, const char *ClientName) {
	_SSID = SSID;
	_Password = Password;
	_ClientName = ClientName;
}

bool WifiConnectionClass::StartAsAP() {
	DEBUG("AP mode starting SSID: %s", _SSID);

  int channel = random(1, 11);
	WiFi.mode(WIFI_AP);
	WiFi.softAP(_SSID, _Password, channel);
	delay(500);
	INFO("WiFi hotspot started. IP: %s (channel: %d)", WiFi.softAPIP().toString().c_str(), channel);
	delay(200);
  return WiFi.isConnected();
}

bool WifiConnectionClass::StartAsStation() {
	DEBUG("Station mode connecting to SSID: %s", _SSID);

	WiFi.mode(WIFI_STA);
	WiFi.setAutoConnect(true);
	WiFi.setAutoReconnect(true);
	//WiFi.onEvent(WiFiEvent);
	WiFi.hostname(_ClientName);

	WiFi.begin(_SSID, _Password);
    long startTime = millis();
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);        
		DEBUG(".");
        if (millis() - startTime > 20000)  {
            ERROR("Setup Wi-Fi timed out.");
            return false;
        }
	}

	INFO("WiFi connected. IP: %s", WiFi.localIP().toString().c_str());
	delay(200);

  return WiFi.isConnected();
}

bool WifiConnectionClass::Setup() {
	delay(300);	

	if (NULL == _SSID) {
		ERROR("Error: SSID not set.");
		return false;
	}

	if (NULL == _ClientName)
		_APMode = true;

	if (_APMode)	{
		return StartAsAP();
	}
	else	{
    return StartAsStation();
	}
}

bool WifiConnectionClass::Loop() {
	if (WiFi.isConnected())
    	return true;

//	if (_APMode)
//		return;

	// not connected, see what needs to be done
	switch (WiFi.status())
	{
	case WL_CONNECT_FAILED:
        delay(10000);
		ERROR("Wrong Wifi configuration...");
		break;
	default:
        delay(10000);
		ERROR("Wifi disconnected (status=%d). Reconnecting...", WiFi.status());
		return Setup();
	}

	return true;
}

bool WifiConnectionClass::IsConnected() {
  return WiFi.isConnected();
}

bool WifiConnectionClass::IsOnline(const char *server) {
  return true;
//  return Ping.ping(server);
}