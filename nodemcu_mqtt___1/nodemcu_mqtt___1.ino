#include <ESP8266WiFi.h>
#include <espnow.h>
#include <PubSubClient.h>

// WiFi and MQTT Server details
const char* ssid = "Sarkar 1";
const char* password = "asha1981";  
const char* mqtt_server = "5.196.78.28";  // Your MQTT broker

WiFiClient espClient;
PubSubClient client(espClient);

// Structure to hold sensor data
typedef struct struct_message {
  float temperature;
  float humidity;
} struct_message;

// Global variable to store the latest sensor data
volatile struct_message sensorData;
volatile bool newDataAvailable = false;  // Flag indicating new data received

// MQTT topics for publishing
#define TOPIC_TEMP      "testnode1234567890/temp"
#define TOPIC_HUMIDITY  "testnode1234567890/humidity"
#define TOPIC_LIGHT     "testnode1234567890/light"

// MQTT callback for control commands (if needed)
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Process control commands (for example, to control a light)
  // Not implemented here since our focus is on publishing sensor data
}

// ESP‑NOW callback to receive sensor data from ESP‑01
void onDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  struct_message myData;
  memcpy(&myData, incomingData, sizeof(myData));
  
  // Copy the received data into the volatile global variable using memcpy
  memcpy((void *)&sensorData, &myData, sizeof(myData));
  newDataAvailable = true;
  
  // Debug output (this is safe to call here)
  Serial.print("Received via ESP‑NOW - Temperature: ");
  Serial.println(myData.temperature);
  Serial.print("Received via ESP‑NOW - Humidity: ");
  Serial.println(myData.humidity);
}

// Connect to WiFi
void setup_wifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected, IP address: " + WiFi.localIP().toString());
}

// Reconnect to MQTT broker if disconnected
void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    String clientId = "NodeMCU_Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("Connected to MQTT!");
      client.subscribe(TOPIC_LIGHT);  // Subscribe if you want to process control messages
    } else {
      Serial.print("MQTT connect failed, rc=");
      Serial.print(client.state());
      Serial.println(" - retrying in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(mqttCallback);
  
  // Initialize ESP‑NOW for receiving data from ESP‑01
  if (esp_now_init() != 0) {
    Serial.println("ESP‑NOW Init Failed");
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(onDataRecv);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  // Check if new sensor data is available
  if (newDataAvailable) {
    // Convert sensor values to char arrays for MQTT publishing
    char tempStr[10], humStr[10];
    dtostrf(sensorData.temperature, 6, 2, tempStr);
    dtostrf(sensorData.humidity, 6, 2, humStr);
    
    // Publish temperature data
    if (client.publish(TOPIC_TEMP, tempStr)) {
      Serial.println("Temperature published to MQTT successfully!");
    } else {
      Serial.println("Failed to publish Temperature to MQTT!");
    }
    
    // Small delay between publishes
    delay(100);
    
    // Publish humidity data
    if (client.publish(TOPIC_HUMIDITY, humStr)) {
      Serial.println("Humidity published to MQTT successfully!");
    } else {
      Serial.println("Failed to publish Humidity to MQTT!");
    }
    
    // Clear the flag so we don't publish the same data again
    newDataAvailable = false;
  }
}
