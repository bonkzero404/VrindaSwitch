#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <Wire.h>
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <PubSubClient.h>
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson

#define relayPin D1

WiFiClient espClient;
PubSubClient client(espClient);

int chipID = ESP.getChipId();
String deviceID = String(chipID, DEC);
int val = 0;
//define your default values here, if there are different values in config.json, they are overwritten.
char mqtt_server[40];
char mqtt_port[6] = "1883";
char mqtt_username[40];
char mqtt_password[40];

//flag for saving data
bool shouldSaveConfig = false;

String local_ip;
String local_ssid;

//WiFiManager
//Local intialization. Once its business is done, there is no need to keep it around
WiFiManager wifiManager;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if((char)payload[0] == 'o' && (char)payload[1] == 'n') {
    digitalWrite(relayPin,HIGH);
  } //on
  else if((char)payload[0] == 'o' && (char)payload[1] == 'f' && (char)payload[2] == 'f') {
    digitalWrite(relayPin,LOW);
  } //off
  else if((char)payload[0] == 'i' && (char)payload[1] == 'n' && (char)payload[2] == 'f' && (char)payload[3] == 'o') {
    if (!client.connected()) {
      reconnect();
    }

    val = digitalRead(relayPin);
    String relayStat = "off";

    if (val == 0) {
      relayStat = "off";
    } else {
      relayStat = "on";
    }

    String infoMessage = "{\"ip\":\"" + local_ip + "\",\"ssid\":\"" + local_ssid + "\",\"id\":\"" + deviceID + "\",\"user\":\"" + mqtt_username + "\",\"stat\":\"" + (String)relayStat + "\"}";
    client.publish(("ESP:INFO:" + (String)deviceID).c_str(), infoMessage.c_str());
  } //info
  else if((char)payload[0] == 'r' && (char)payload[1] == 'e' && (char)payload[2] == 's' && (char)payload[3] == 'e' && (char)payload[4] == 't') {
    SPIFFS.format();
    WiFiManager wifiManager;
    wifiManager.resetSettings();
    delay(5000);

    ESP.reset();
  } //reset

}

void reconnect() {
  int nFail = 1;
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(deviceID.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");

      // In Topic
      String inCommand = "ESP:COMMAND:" + deviceID;

      // JSON Message
      val = digitalRead(relayPin);
      String relayStat = "off";

      if (val == 0) {
        relayStat = "off";
      } else {
        relayStat = "on";
      }

      String infoMessage = "{\"ip\":\"" + local_ip + "\",\"ssid\":\"" + local_ssid + "\",\"id\":\"" + deviceID + "\",\"user\":\"" + mqtt_username + "\",\"stat\":\"" + (String)relayStat + "\"}";

      // Once connected, publish an announcement...
      client.publish(("ESP:SERVER:" + deviceID).c_str(), infoMessage.c_str());

      // ... and resubscribe
      client.subscribe(inCommand.c_str());

    } else {
      // Not authorize
      if (client.state() == 5) {
        if (nFail > 3) {
          wifiManager.resetSettings();
          delay(5000);
          ESP.reset();
        } else {
          Serial.print("failed, rc=");
          Serial.print(client.state());
          Serial.println(" try again in 5 seconds");
          delay(5000);
          nFail = nFail + 1;
        }

      }
      /* ********** Need Help for better way ****
      // Connection refused
      else if (client.state() == -2) {
        if (nFail > 20) {
          wifiManager.resetSettings();
          delay(5000);
          ESP.reset();
        } else {
          Serial.print("failed, rc=");
          Serial.print(client.state());
          Serial.println(" try again in 5 seconds");
          delay(5000);
          nFail = nFail + 1;
        }

      }
      */
      else {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
        delay(5000);
      }
    }
  }
}

String IP2Str(IPAddress address) {
 return String(address[0]) + "." +
        String(address[1]) + "." +
        String(address[2]) + "." +
        String(address[3]);
}

void setup() {
  pinMode(relayPin,OUTPUT);

  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();

  //clean FS, for testing
  // SPIFFS.format();

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
          strcpy(mqtt_username, json["mqtt_username"]);
          strcpy(mqtt_password, json["mqtt_password"]);

        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read



  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_mqtt_server("server", "MQTT Server", mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_port("port", "MQTT Port", mqtt_port, 5);
  WiFiManagerParameter custom_mqtt_username("username", "MQTT Username", mqtt_username, 40);
  WiFiManagerParameter custom_mqtt_password("password", "MQTT Password", mqtt_password, 40);

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //add all your parameters here
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_username);
  wifiManager.addParameter(&custom_mqtt_password);

  //reset settings - for testing
  wifiManager.resetSettings();

  //set minimu quality of signal so it ignores AP's under that quality
  //defaults to 8%
  //wifiManager.setMinimumSignalQuality();

  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  //wifiManager.setTimeout(120);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("VrindaSwitch ESP8266", "password")) {
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
  strcpy(mqtt_username, custom_mqtt_username.getValue());
  strcpy(mqtt_password, custom_mqtt_password.getValue());

  randomSeed(micros());

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["mqtt_server"] = mqtt_server;
    json["mqtt_port"] = mqtt_port;
    json["mqtt_username"] = mqtt_username;
    json["mqtt_password"] = mqtt_password;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }

  Serial.println("local ip");
  Serial.println(WiFi.localIP());

  local_ip = IP2Str(WiFi.localIP());
  local_ssid = String(WiFi.SSID());

  client.setServer(mqtt_server, atoi(mqtt_port));
  client.setCallback(callback);

}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
