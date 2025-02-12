#include <ESP8266WiFi.h>
#include <espnow.h>

// Pin Definitions
#define LED_PIN 1  // ESP-01 GPIO2
#define DHTPIN 2  
#define DHTTYPE DHT11

typedef struct struct_message {
  float temperature;
  float humidity;
} struct_message;

typedef struct led_message {
  bool ledState;  // LED control (true = ON, false = OFF)
} led_message;

struct_message sensorData;
led_message ledCommand;

// MAC Address of NodeMCU (Gateway) - Replace with actual MAC
uint8_t gatewayMAC[] = {0x8C, 0xAA, 0xB5, 0xC5, 0x09, 0xF8};

// Callback function for ESP-NOW data reception
void onDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&ledCommand, incomingData, sizeof(ledCommand));
  if (ledCommand.ledState) {
    digitalWrite(LED_PIN, LOW);  // Active-low LED
    Serial.println("LED ON via ESP-NOW");
  } else {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("LED OFF via ESP-NOW");
  }
}

void setup() {
  Serial.begin(115200);

  // Initialize LED Pin
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);  // LED off by default

  // Set ESP-01 as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Initialize ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW Initialization Failed");
    return;
  }

  // Set role as slave
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  
  // Register receive callback
  esp_now_register_recv_cb(onDataRecv);
}

void loop() {
  // Sensor data simulation (Replace with actual sensor readings)
  sensorData.temperature = random(20, 30);
  sensorData.humidity = random(40, 60);

  // Send sensor data to NodeMCU
  esp_now_send(gatewayMAC, (uint8_t*)&sensorData, sizeof(sensorData));

  Serial.print("Sent Temp: "); Serial.println(sensorData.temperature);
  Serial.print("Sent Humidity: "); Serial.println(sensorData.humidity);

  delay(5000); // Send data every 5 seconds
}
