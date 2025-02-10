#include <ESP8266WiFi.h>
#include <espnow.h>
#include <PubSubClient.h>

// WiFi and MQTT Server details
const char* ssid = "Sarkar 1";
const char* password = "asha1981";  
const char* mqtt_server = "5.196.78.28";  // Example: test.mosquitto.org

WiFiClient espClient;
PubSubClient client(espClient);

// Structure to hold received data
typedef struct struct_message {
  float temperature;
  float humidity;
} struct_message;

struct_message incomingData;

// Callback function when data is received via ESP-NOW
void onDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
    struct_message myData;
    memcpy(&myData, incomingData, sizeof(myData));

    Serial.print("Publishing - Temp: "); Serial.println(myData.temperature);
    Serial.print("Publishing - Humidity: "); Serial.println(myData.humidity);

    // Convert float to char array
    char tempStr[10], humStr[10];
    dtostrf(myData.temperature, 6, 2, tempStr);
    dtostrf(myData.humidity, 6, 2, humStr);

    Serial.print("MQTT Temp String: "); Serial.println(tempStr);
    Serial.print("MQTT Hum String: "); Serial.println(humStr);

    // Ensure topics are correct and messages are sent
    if (client.publish("testnode1234567890/temp", tempStr)) {
        Serial.println("Temperature sent to MQTT successfully!");
    } else {
        Serial.println("Failed to send Temperature!");
    }

    delay(100);  // Small delay to avoid message loss

    if (client.publish("testnode1234567890/humidity", humStr)) {
        Serial.println("Humidity sent to MQTT successfully!");
    } else {
        Serial.println("Failed to send Humidity!");
    }
}


// Connect to WiFi
void setup_wifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected, IP address: " + WiFi.localIP().toString());
}

// Reconnect to MQTT server if connection is lost
void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("NodeMCU_Client")) {
      Serial.println("Connected!");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" Retry in 5 seconds...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  
  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW Init Failed");
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
}
