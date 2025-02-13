#include <ESP8266WiFi.h>
#include <espnow.h>

#define LED_PIN 1  // ESP-01 GPIO1 (Built-in LED)

// Structure to receive LED command
typedef struct led_message {
  bool ledState;  // True = ON, False = OFF
} led_message;

led_message ledCommand;

// ESP-NOW callback for receiving data
void onDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&ledCommand, incomingData, sizeof(ledCommand));

  // Control LED
  if (ledCommand.ledState) {
    digitalWrite(LED_PIN, LOW);  // LED ON (Active-low)
    Serial.println("LED ON via ESP-NOW");
  } else {
    digitalWrite(LED_PIN, HIGH); // LED OFF
    Serial.println("LED OFF via ESP-NOW");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);  // LED OFF by default

  // Set ESP-01 as Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Initialize ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW Initialization Failed");
    return;
  }

  // Set role as a slave (receiver)
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(onDataRecv);
}

void loop() {
  // Wait for LED commands via ESP-NOW
}
