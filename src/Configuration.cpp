#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <LittleFS.h>
#include <Dictionary.h>

// for loading 
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h> 

#include "Configuration.h"
#include "WifiConnection.h"
#include "Logger.h"

const char*  CONFIG_FILENAME = "/config.json";
const size_t MAX_FILE_SIZE = 1024;
const size_t MAX_CONFIG_LEN = 32;   // max length of each config entry

ConfigurationClass GlobalConfig;

ConfigurationClass::ConfigurationClass()  {
  _pConfig = new Dictionary(MAX_CONFIG_SIZE);
}

ConfigurationClass::~ConfigurationClass() {
  if (NULL != _pConfig)
    delete _pConfig;
}

bool ConfigurationClass::AddSchemaField(const char* id, const char* name, size_t len) {
  if ((NULL == id) || (NULL == name) || (len > MAX_CONFIG_LEN - 1)) {
    ERROR("Invalid schema field.");
    return false;
  }

  if ((strcmp(id, FIELD_SSID) == 0) ||
      (strcmp(id, FIELD_PASSWORD) == 0) ) {
    ERROR("Cannot add [%s]: it is a built-in field.", id);
    return false;
  }

  if (MAX_CONFIG_SIZE == _SchemaSize) {
    ERROR("Max number of schema field reached.");
    return false;
  }

  _Schema[_SchemaSize].Id = id;
  _Schema[_SchemaSize].Name = name;
  _Schema[_SchemaSize].Len = len;
  _SchemaSize++;
  return true;
}

bool ConfigurationClass::LoadFromFile()  {
  if (_SchemaSize == 0) {
    ERROR("Please call AddSchemaField() to configure schema first.");
    return false;
  }
  
  if (!LittleFS.begin())  {
    ERROR("Failed to mount file system");
    return false;
  }

  File configFile = LittleFS.open(CONFIG_FILENAME, "r");
  if (!configFile) {
    ERROR("Failed to open config file.");
    return false;
  }

  size_t size = configFile.size();
  if (size > MAX_FILE_SIZE) {
    ERROR("Config file size is too large: %u bytes", size);
    return false;
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf.get(), size);

  // store content as Json
  int status = _pConfig->jload(buf.get());
  if (DICTIONARY_OK != status) {
    ERROR("Failed to parse config file: %d", status);
    return false;
  }

  PrintConfig();
  return IsConfigComplete();
}

bool ConfigurationClass::IsConfigComplete() {
  for (size_t i = 0; i < _SchemaSize; i++) {
    if (_pConfig->search(_Schema[i].Id).isEmpty())
      return false;
  }
  return true;
}

void ConfigurationClass::PrintConfig()  {
  // print what we have first
  DEBUG("Current Config setting (%u items):", _pConfig->count());
  for (size_t i = 0; i < _pConfig->count(); i++) {
    DEBUG(" %s=%s", _pConfig->key(i).c_str(), _pConfig->value(i).c_str());
  }

  // check and see if the config is complete
  // check if built-in fields exist
  const char* field = FIELD_SSID;
  if (_pConfig->search(field).isEmpty())
    ERROR(" >>> Missing built-in field: %s", field);
  field = FIELD_PASSWORD;
  if (_pConfig->search(field).isEmpty())
    ERROR(" >>> Missing built-in field: %s", field);
  // check if missing custom fields
  for (size_t i = 0; i < _SchemaSize; i++) {
    field = _Schema[i].Id;
    if (_pConfig->search(field).isEmpty()) {
      ERROR(" >>> Missing custom field: %s", field);
    }
  }
}

bool ConfigurationClass::SaveToFile() {
  if (_pConfig->jsize() == 0) {
    ERROR("Config setting is empty.");
    return false;
  }

  File configFile = LittleFS.open(CONFIG_FILENAME, "w");
  if (!configFile) {
    ERROR("Failed to open config file for writing.");
    return false;
  }

  size_t size = configFile.write(_pConfig->json().c_str());
  if (size == 0) {
    ERROR("Failed to save config file.");
    return false;
  }

  INFO("Config file was saved. (%u bytes)", size);
  return true;
}

String ConfigurationClass::Get(const char* varname)  {
  return _pConfig->search(varname);
}

//////////////////////////////////////////////////////////////
// WifiManager related variables
//////////////////////////////////////////////////////////////

const char* DEFAULT_SSID = "Esp8266Config";
const char* DEFAULT_PASSWORD = "Arduino";
const char* DEFAULT_IP = "10.0.1.56";
const char* DEFAULT_GATEWAY = "10.0.1.1";
const char* DEFAULT_SUBNET = "255.255.255.0";

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
}

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

bool ConfigurationClass::ReConfig() {
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //set static ip
  IPAddress _ip,_gw,_sn;
  _ip.fromString(DEFAULT_IP);
  _gw.fromString(DEFAULT_GATEWAY);
  _sn.fromString(DEFAULT_SUBNET);

  wifiManager.setSTAStaticIPConfig(_ip, _gw, _sn);

  WiFiManagerParameter* fields[MAX_CONFIG_SIZE];
  for (size_t i = 0; i < _SchemaSize; i++) {
    const char* id = _Schema[i].Id;
    const char* name = _Schema[i].Name;
    const char* defVal = Get(_Schema[i].Id).c_str();
    size_t len = _Schema[i].Len;
    // add custom properties
    INFO("Adding parameter(%s, %s, %s, %u)", id, name, defVal, len);
    fields[i] = new WiFiManagerParameter(id, name, defVal, len);
    wifiManager.addParameter(fields[i]);
  }

  //set minimu quality of signal so it ignores AP's under that quality
  //defaults to 8%
  wifiManager.setMinimumSignalQuality();
  
  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  //wifiManager.setTimeout(120);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  Serial.println("Start config portal.");
  if (!wifiManager.startConfigPortal(DEFAULT_SSID, DEFAULT_PASSWORD)) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

  //read updated parameters
  _pConfig->destroy();
  // process SSID and Password separately as they are built-in WifiManager fields
  _pConfig->insert(FIELD_SSID, wifiManager.getConfigPortalSSID().c_str());
  // process custom fields
  for (size_t i = 0; i < _SchemaSize; i++) {
    INFO("Got field: %s=%s", fields[i]->getID(), fields[i]->getValue());
    _pConfig->insert(fields[i]->getID(), fields[i]->getValue());
    delete fields[i];
  }
  PrintConfig();

  return true;  
}

/*

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["mqtt_server"] = mqtt_server;
    json["mqtt_port"] = mqtt_port;
    json["blynk_token"] = blynk_token;

    json["ip"] = WiFi.localIP().toString();
    json["gateway"] = WiFi.gatewayIP().toString();
    json["subnet"] = WiFi.subnetMask().toString();

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.prettyPrintTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }

  Serial.println("local ip");
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.gatewayIP());


//needed for library

#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson


//define your default values here, if there are different values in config.json, they are overwritten.
//length should be max size + 1 
char mqtt_server[40];
char mqtt_port[6] = "8080";
char blynk_token[33] = "YOUR_BLYNK_TOKEN";

*/