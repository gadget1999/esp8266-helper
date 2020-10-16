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

const char*   CONFIG_FILENAME = "/config.json";
const size_t  MAX_FILE_SIZE = 1024;

ConfigurationClass GlobalConfig;

ConfigurationClass::ConfigurationClass()  {
  _pConfig = new Dictionary(MAX_CONFIG_SIZE);
}

ConfigurationClass::~ConfigurationClass() {
  if (NULL != _pConfig)
    delete _pConfig;
}

bool ConfigurationClass::LoadConfig() {
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

  INFO("Config file loaded. (%u items)", _pConfig->count());
  return true;
}

bool ConfigurationClass::AddSchemaField(const char* id, const char* name, int len) {
  if (MAX_CONFIG_SIZE == _SchemaSize) {
    ERROR("Max number of schema field reached.");
    return false;
  }

  if ((NULL == id) || (NULL == name)) {
    ERROR("Invalid schema field.");
    return false;
  }

  _Schema[_SchemaSize].Id = id;
  _Schema[_SchemaSize].Name = name;
  _Schema[_SchemaSize].Len = len;
  _SchemaSize++;
  return true;
}

bool ConfigurationClass::Setup()  {
  if (!LittleFS.begin())  {
    ERROR("Failed to mount file system");
    return false;
  }

  if (!LoadConfig()) 
    return false;

  // check and see if the config is complete
  for (size_t i = 0; i < _SchemaSize; i++) {
    if (_pConfig->search(_Schema[i].Id).isEmpty()) {
      ERROR("Missing config item: %s", _Schema[i].Id);
      return false;
    }
  }

  return true;
}

bool ConfigurationClass::Save() {
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

/*


//needed for library

#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson


//define your default values here, if there are different values in config.json, they are overwritten.
//length should be max size + 1 
char mqtt_server[40];
char mqtt_port[6] = "8080";
char blynk_token[33] = "YOUR_BLYNK_TOKEN";
//default custom static IP
char static_ip[16] = "10.0.1.56";
char static_gw[16] = "10.0.1.1";
char static_sn[16] = "255.255.255.0";

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();

  //clean FS, for testing
  //SPIFFS.format();

  //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          strcpy(mqtt_server, json["mqtt_server"]);
          strcpy(mqtt_port, json["mqtt_port"]);
          strcpy(blynk_token, json["blynk_token"]);

          if(json["ip"]) {
            Serial.println("setting custom ip from config");
            //static_ip = json["ip"];
            strcpy(static_ip, json["ip"]);
            strcpy(static_gw, json["gateway"]);
            strcpy(static_sn, json["subnet"]);
            //strcat(static_ip, json["ip"]);
            //static_gw = json["gateway"];
            //static_sn = json["subnet"];
            Serial.println(static_ip);
            Serial.println("converting ip");
            IPAddress ip = ipFromCharArray(static_ip);
            Serial.println(ip);
          } else {
            Serial.println("no custom ip in config");
          }
        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read
  Serial.println(static_ip);
  Serial.println(blynk_token);
  Serial.println(mqtt_server);


  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 5);
  WiFiManagerParameter custom_blynk_token("blynk", "blynk token", blynk_token, 34);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //set static ip
  IPAddress _ip,_gw,_sn;
  _ip.fromString(static_ip);
  _gw.fromString(static_gw);
  _sn.fromString(static_sn);

  wifiManager.setSTAStaticIPConfig(_ip, _gw, _sn);
  
  //add all your parameters here
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_blynk_token);

  //reset settings - for testing
  //wifiManager.resetSettings();

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
  if (!wifiManager.autoConnect("AutoConnectAP", "password")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

  //read updated parameters
  strcpy(mqtt_server, custom_mqtt_server.getValue());
  strcpy(mqtt_port, custom_mqtt_port.getValue());
  strcpy(blynk_token, custom_blynk_token.getValue());

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
  Serial.println(WiFi.subnetMask());
}

void loop() {
  // put your main code here, to run repeatedly:


}
*/