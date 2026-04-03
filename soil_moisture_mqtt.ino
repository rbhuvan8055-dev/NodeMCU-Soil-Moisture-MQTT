// Make sure to downlode all the libraries. Change the wifi password and name(ssid) to your wifi credentials 
//Follow for mor iot/robotics stuff:)

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>

const char* ssid = "YOUR_WIFI_NAME";//Change 
const char* password = "YOUR_WIFI_PASSWORD";//change 

const char* mqtt_server = "broker.mqtt.cool";
const char* mqtt_topic  = "scallion_sensors";

#define SOIL_SENSOR_PIN A0

WiFiClient espClient;
PubSubClient client(espClient);

const long gmtOffset_sec = 19800;
const int daylightOffset_sec = 0;

void setupWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void reconnectMQTT() {
  while (!client.connected()) {
    if (client.connect("NodeMCU_SoilSensor")) {
    } else {
      delay(2000);
    }
  }
}

String getMoistureLevel(int value) {
  if (value < 300) return "dry";
  else if (value < 700) return "moist";
  else return "wet";
}

void setup() {
  Serial.begin(9600);
  setupWiFi();
  client.setServer(mqtt_server, 1883);
  configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org", "time.nist.gov");
}

void loop() {
  if (!client.connected()) reconnectMQTT();
  client.loop();

  int sensorValue = analogRead(SOIL_SENSOR_PIN);
  String moistureLevel = getMoistureLevel(sensorValue);

  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);

  char timeStr[9];
  char dateStr[11];

  strftime(timeStr, sizeof(timeStr), "%H:%M:%S", timeinfo);
  strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", timeinfo);

  StaticJsonDocument<256> doc;

  doc["timestamp"] = timeStr;
  doc["date"] = dateStr;
  doc["type"] = "soil_moisture_sensor";

  JsonObject value = doc.createNestedObject("value");
  value["moisture_level"] = moistureLevel;
  value["sensorvalue"] = sensorValue;

  char payload[256];
  serializeJson(doc, payload);

  client.publish(mqtt_topic, payload);

  delay(5000);
}
