#include <ESP8266WiFi.h>
#include <PubSubClient.h>

extern "C" {
  #include "user_interface.h"
}

#define WLAN_SSID       ""  // Wi-Fi SSID
#define WLAN_PASS       ""  // Wi-Fi Password

// const char *mqtt_host = "";  // MQTT Host
byte ip[] = {172, 30, 16, 179};  // MQTT IP Address
const char *mqtt_topic = "/pishield/chan6"; // MQTT Topic (chan6 = rotation)
// const char *mqtt_topic = "/pishield/chan7"; // MQTT Topic (chan7 = horizontal move)

// ClientID
const char *mqtt_client = "ESP8266_MQTT_Client";

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
}

WiFiClient wclient;
// PubSubClient client(mqtt_host, 1883, callback, wclient);  // MQTT Host
PubSubClient client(ip, 1883, callback, wclient);  // MQTT IP Address

void setupWiFi() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  pinMode(15, OUTPUT);  // for Wio-Node
  digitalWrite(15, 1);  // for Wio-Node
  Serial.begin(115200);
  setupWiFi();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    setupWiFi();
  }
  if (WiFi.status() == WL_CONNECTED) {
    if (!client.connected()) {
      Serial.println("Connecting to MQTT server");
      if (client.connect(mqtt_client)) {
        Serial.println("Connected to MQTT server");
      } else {
        Serial.println("Could not connect to MQTT server");
      }
    }
    Serial.print("Publishing: ");
    int value = system_adc_read();
    Serial.println("Value: " + String(value));
    // normalize: Y = (X - Xmin) / (Xmax - Xmin) => 0 - 1
    float f = (float(value) - 207) / (334 - 207) - 0.5;
    // cast float => char
    char s[8];
    dtostrf(f, 2, 5, s);
    Serial.println("Value: " + String(value) + ", " + s);
    client.publish(mqtt_topic, s);
    if (client.connected()) {
      client.loop();
    }
  }
  delay(100);
}
