#include <ESP8266WiFi.h>
#include <espnow.h>
#include <PubSubClient.h>

// WiFi and MQTT Server details
const char* ssid = "Sarkar 1";
const char* password = "asha1981";
const char* mqtt_server = "5.196.78.28";  

WiFiClient espClient;
PubSubClient client(espClient);

// Structure for sensor data
typedef struct struct_message {
  float temperature;
  float humidity;
} struct_message;

// Structure for LED control
typedef struct led_message {
  bool ledState;
} led_message;

struct_message sensorData;
led_message ledCommand;

// ESP-01 MAC Address - Replace with actual MAC EC:FA:BC:37:31:F0
uint8_t esp01MAC[] = {0xEC, 0xFA, 0xBC, 0x37, 0x31, 0xF0};

// MQTT topics
#define TOPIC_TEMP      "testnode1234567890/temp"
#define TOPIC_HUMIDITY  "testnode1234567890/humidity"
#define TOPIC_LED       "testnode1234567890/led"

// MQTT callback function
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Received MQTT message on topic: ");
  Serial.println(topic);

  if (strcmp(topic, TOPIC_LED) == 0) {  
    payload[length] = '\0';  // Ensure string termination
    String message = String((char*)payload);

    if (message.equalsIgnoreCase("on")) {
      ledCommand.ledState = true;
    } else if (message.equalsIgnoreCase("off")) {
      ledCommand.ledState = false;
    } else {
      Serial.println("Invalid LED command received!");
      return;
    }

    // Send LED command to ESP-01 via ESP-NOW
    esp_now_send(esp01MAC, (uint8_t*)&ledCommand, sizeof(ledCommand));

    Serial.print("LED command sent via ESP-NOW: ");
    Serial.println(ledCommand.ledState ? "ON" : "OFF");
  }
}

// ESP‑NOW callback to receive sensor data from ESP-01
void onDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&sensorData, incomingData, sizeof(sensorData));

  Serial.print("Received via ESP‑NOW - Temp: ");
  Serial.println(sensorData.temperature);
  Serial.print("Received via ESP‑NOW - Humidity: ");
  Serial.println(sensorData.humidity);

  // Publish sensor data to MQTT
  char tempStr[10], humStr[10];
  dtostrf(sensorData.temperature, 6, 2, tempStr);
  dtostrf(sensorData.humidity, 6, 2, humStr);

  client.publish(TOPIC_TEMP, tempStr);
  client.publish(TOPIC_HUMIDITY, humStr);
}

// WiFi setup
void setup_wifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
}

// MQTT Reconnection
void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    String clientId = "NodeMCU_Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("Connected to MQTT!");
      client.subscribe(TOPIC_LED);  // Subscribe to LED control topic
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

  // Initialize ESP‑NOW
  if (esp_now_init() != 0) {
    Serial.println("ESP‑NOW Init Failed");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_add_peer(esp01MAC, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
  esp_now_register_recv_cb(onDataRecv);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
