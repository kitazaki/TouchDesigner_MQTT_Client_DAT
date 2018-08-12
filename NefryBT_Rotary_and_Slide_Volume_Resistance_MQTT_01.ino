#include <Nefry.h>
#include <NefryDisplay.h>
#include <NefrySetting.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <misakiUTF16.h>

void setting(){
  Nefry.disableDisplayStatus();
}

NefrySetting nefrySetting(setting);

void misakiPrint(int x, int y, char * pUTF8) {
  int n=0;
  byte buf[20][8];  //160x8ドットのバナー表示パターン
  while(*pUTF8)
    pUTF8 = getFontData(&buf[n++][0], pUTF8);  // フォントデータの取得

  // 文字の描画
  for (byte i=0; i < 8; i++) {
    for (byte j=0; j < n; j++) {
      for (byte k=0; k<8;k++) {
        if(bitRead(buf[j][i],7-k)) {
          NefryDisplay.setPixel(x + 8*j + k , y + i);
        }
      }
    }
  }
}

// MQTT settings
byte ip[] = {192, 168, 2, 1};  // MQTT IP address
const char *mqtt_topic1 = "/pishield/chan5"; // MQTT Topic (chan5 = rotation)
const char *mqtt_topic2 = "/pishield/chan6"; // MQTT Topic (chan6 = x-axis slider)
const char *mqtt_topic3 = "/pishield/chan7"; // MQTT Topic (chan6 = y-axis slider)
const char *mqtt_client = "NefryBT_MQTT_Client" "_" __DATE__ "_" __TIME__;  // MQTT ClientID

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
}

WiFiClient wclient;
PubSubClient client(ip, 1883, callback, wclient);

const int PIN1 = A2;
const int PIN2 = A3;
const int PIN3 = A7;
int val1 = 0;
int val2 = 0;
int val3 = 0;
char buf[10];

void displayResistanceInfo() {
  int val;
  int f_update = 0;
  val = analogRead(PIN1);
  if (abs(val - val1) > 20) {
    val1 = val;
    // normalize: Y = (X - Xmin) / (Xmax - Xmin) => 0 - 1
    float f_val1 = float(val1) / 4096;
    char s_val1[8];
    dtostrf(f_val1, 2, 5, s_val1);
    Serial.println("Publishing: Val1: " + String(val1) + ", " + s_val1);
    client.publish(mqtt_topic1, s_val1);
    f_update++;
  }
  val = analogRead(PIN2);
  if (abs(val - val2) > 20) {
    val2 = val;
    // normalize: Y = (X - Xmin) / (Xmax - Xmin) => 0 - 1
    float f_val2 = float(val2) / 4096;
    char s_val2[8];
    dtostrf(f_val2, 2, 5, s_val2);
    Serial.println("Publishing: Val2: " + String(val2) + ", " + s_val2);
    client.publish(mqtt_topic2, s_val2);
    f_update++;
  }
  val = analogRead(PIN3);
  if (abs(val - val3) > 20) {
    val3 = val;
    // normalize: Y = (X - Xmin) / (Xmax - Xmin) => 0 - 1
    float f_val3 = float(val3) / 4096;
    char s_val3[8];
    dtostrf(f_val3, 2, 5, s_val3);
    Serial.println("Publishing: Val3: " + String(val3) + ", " + s_val3);
    client.publish(mqtt_topic3, s_val3);
    f_update++;
  }
  if (f_update) {
    NefryDisplay.clear();
    NefryDisplay.setFont(ArialMT_Plain_24);
    misakiPrint(0,0, "ロータリ:");
    misakiPrint(0,20, "スライド1:");
    misakiPrint(0,40, "スライド2:");
    sprintf(buf, "%d", val1);
    NefryDisplay.drawString(50, 0, buf);
    sprintf(buf, "%d", val2);
    NefryDisplay.drawString(50, 20, buf);
    sprintf(buf, "%d", val3);
    NefryDisplay.drawString(50, 40, buf);
    NefryDisplay.display();
    Serial.print("val1: ");
    Serial.print(val1);
    Serial.print(",val2: ");
    Serial.print(val2);
    Serial.print(",val3: ");
    Serial.println(val3);
  }
}

void setup() {
  Serial.begin(115200);
  NefryDisplay.clear();
  NefryDisplay.setFont(ArialMT_Plain_24);
  misakiPrint(0,0, "ロータリ:");
  misakiPrint(0,20, "スライド1:");
  misakiPrint(0,40, "スライド2:");
  NefryDisplay.drawString(50, 0, "0");
  NefryDisplay.drawString(50, 20, "0");
  NefryDisplay.drawString(50, 40, "0");
  NefryDisplay.display();
  pinMode(PIN1, INPUT);
  pinMode(PIN2, INPUT);
  pinMode(PIN3, INPUT);
  Serial.println("displayResistanceInfo Started");
}

void loop() {
  if (!client.connected()) {
    Serial.println("Connecting to MQTT server");
    if (client.connect(mqtt_client)) {
      Serial.println("Connected to MQTT server");
    } else {
      Serial.println("Could not connect to MQTT server");
    }
  }
  displayResistanceInfo();
  if (client.connected()) {
    client.loop();
  }
  delay(100);
}
