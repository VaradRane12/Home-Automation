#include <ESP8266WiFi.h>
#include <espnow.h>
#include <DHT.h>

// Pin Definitions
#define LED_PIN 1   // ESP-01 GPIO1
#define DHTPIN 2    // ESP-01 GPIO2 (DHT11 Data pin)
#define DHTTYPE DHT11  

DHT dht(DHTPIN, DHTTYPE);

// Structures for communication
typedef struct sensor_data {
  float temperature;
  float humidity;
} sensor_data;

typedef struct control_message {
  bool requestData;  // True = Request temp/humidity
  bool ledState;     // True = LED ON, False = LED OFF
} control_message;

sensor_data sensorData;
control_message command;

// MAC Address of NodeMCU (Gateway) - Replace with actual MAC
uint8_t gatewayMAC[] = {0x8C, 0xAA, 0xB5, 0xC5, 0x09, 0xF8};

// ESP-NOW Receive Callback (Handles both temp requests and LED control)
void onDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&command, incomingData, sizeof(command));

  // Handle LED control
  if (command.ledState) {
    digitalWrite(LED_PIN, LOW);  // Active-low LED
    Serial.println("LED ON via ESP-NOW");
  } else {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("LED OFF via ESP-NOW");
  }

  // Handle Temperature Request
  if (command.requestData) {
    Serial.println("Received Data Request from NodeMCU");

    float temp = dht.readTemperature();
    float hum = dht.readHumidity();

    if (isnan(temp) || isnan(hum)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    sensorData.temperature = temp;
    sensorData.humidity = hum;

    Serial.print("Sending Temp: "); Serial.println(sensorData.temperature);
    Serial.print("Sending Humidity: "); Serial.println(sensorData.humidity);

    esp_now_send(gatewayMAC, (uint8_t*)&sensorData, sizeof(sensorData));
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);  // LED off by default

  dht.begin();  // Initialize DHT sensor

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW Initialization Failed");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(onDataRecv);
}

void loop() {
  // ESP-01 waits for commands via ESP-NOW
}
