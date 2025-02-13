#include <ESP8266WiFi.h>
#include <espnow.h>
#include <PubSubClient.h>

// WiFi and MQTT Configuration
const char* ssid = "Sarkar 1";
const char* password = "asha1981";
const char* mqtt_server = "5.196.78.28";

WiFiClient espClient;
PubSubClient client(espClient);

// MAC Address of ESP-01 (Replace with actual MAC)
uint8_t esp01MAC[] = {0xEC, 0xFA, 0xBC, 0x37, 0x31, 0xF0};

// Structures for communication
typedef struct control_message {
  bool requestData;
  bool ledState;
} control_message;

typedef struct sensor_data {
  float temperature;
  float humidity;
} sensor_data;

control_message command;
sensor_data sensorData;

// ESP-NOW Receive Callback (Handles sensor data)
void onDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  Serial.print("here");
  memcpy(&sensorData, incomingData, sizeof(sensorData));
  Serial.print("Received Temp: "); Serial.println(sensorData.temperature);
  Serial.print("Received Humidity: "); Serial.println(sensorData.humidity);

  // Publish sensor data to MQTT
  client.publish("HomeAutomation981237419/temperature", String(sensorData.temperature).c_str());
  client.publish("HomeAutomation981237419/humidity", String(sensorData.humidity).c_str());
}

// MQTT Callback (Handles messages from Flask)
void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  if (message == "get_data") {
    Serial.println("Received MQTT Command: Get Data");
    command.requestData = true;
    esp_now_send(esp01MAC, (uint8_t*)&command, sizeof(command));
  } else if (message == "led_on") {
    Serial.println("Received MQTT Command: LED ON");
    command.requestData = false;
    command.ledState = true;
    esp_now_send(esp01MAC, (uint8_t*)&command, sizeof(command));
  } else if (message == "led_off") {
    Serial.println("Received MQTT Command: LED OFF");
    command.requestData = false;
    command.ledState = false;
    esp_now_send(esp01MAC, (uint8_t*)&command, sizeof(command));
  }
}

void setup() {
  Serial.begin(115200);
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi Connected!");

  // Setup MQTT
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  while (!client.connected()) {
    Serial.print(".");
    client.connect("NodeMCU_Client");
    delay(500);
  }
  Serial.println("MQTT Connected!");
  client.subscribe("HomeAutomation981237419/request");
  client.subscribe("HomeAutomation981237419/led");

  // Setup ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW Initialization Failed");
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_recv_cb(onDataRecv);
}

void loop() {
  client.loop();
}
