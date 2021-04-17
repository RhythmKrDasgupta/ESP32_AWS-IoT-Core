#include "secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"


#define AWS_IOT_PUBLISH_TOPIC   "esp32/status"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/ctrl"


// LED CONFIG PIN
#define led1 2
#define led2 15



WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

void connectAWS() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
 
  client.setServer(AWS_IOT_ENDPOINT, 8883);    // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.setCallback(messageHandler);          // Create a message handler

  Serial.print("Connecting to AWS IOT");

  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(100);
  }

  if(!client.connected()){
    Serial.println("AWS IoT Timeout!");
    return;
  }

  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
  Serial.println("AWS IoT Connected!");
}



void publishMessage() {
  StaticJsonDocument<200> doc;
  doc["time"] = millis();  // digitalRead(pin)
  doc["Led_1"] = digitalRead(led1);
  doc["Led_2"] = digitalRead(led2);
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
  Serial.println(jsonBuffer);
}


void messageHandler(char* topic, byte* payload, unsigned int length) {
  Serial.print("Incoming: ");
  Serial.println(topic);
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  const char* message = doc["message"];
  Serial.println(message);
  String ctrl= message;
  // LED CONTROL
  if (ctrl == "on1")  {
    digitalWrite(led1, HIGH);
    Serial.println("LED_1 ON"); 
  }
  else if (ctrl == "off1")  {
    digitalWrite(led1, LOW);
    Serial.println("LED_1 OFF");
  }

  
  if (ctrl == "on2")  {
    digitalWrite(led2, HIGH);
    Serial.println("LED_2 ON");
  }
  else if (ctrl == "off2")  {
    digitalWrite(led2, LOW);
    Serial.println("LED_2 OFF");
  }
}



void setup() {
  pinMode(led1,OUTPUT);
  pinMode(led2,OUTPUT);
  digitalWrite(led1,LOW);
  digitalWrite(led2,LOW);
  Serial.begin(115200);
  connectAWS();    
}

void loop() {
  publishMessage();
  client.loop();
  delay(1000);
}
