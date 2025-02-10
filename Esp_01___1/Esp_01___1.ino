#include <ESP8266WiFi.h>
#include <espnow.h>
#include <DHT.h>

#define DHTPIN 2          // Use GPIO2 for the DHT sensor
#define DHTTYPE DHT11     // Change to DHT22 if needed
DHT dht(DHTPIN, DHTTYPE);

#define LED_PIN 2       // Use GPIO0 for LED control (ensure proper wiring/pull-up)

uint8_t broadcastAddress[] = {0x8C, 0xAA, 0xB5, 0xC5, 0x09, 0xF8};

typedef struct struct_message {
  float temperature;
  float humidity;
} struct_message;

struct_message myData;

void onSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Send Status: ");
  Serial.println(sendStatus == 0 ? "Success" : "Fail");
}

void setup() {
  Serial.begin(115200);

  // Initialize LED pin
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);  // Assuming LED is active LOW

  // --- Connect temporarily to WiFi to lock the channel ---
  WiFi.mode(WIFI_STA);
  WiFi.begin("Sarkar 1", "asha1981");
  Serial.print("Connecting to WiFi to lock channel");
  while(WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  uint8_t channel = WiFi.channel();
  Serial.print("\nConnected. Channel: ");
  Serial.println(channel);
  // Disconnect from WiFi but remain in STA mode (the channel remains locked)
  WiFi.disconnect();

  // Initialize ESP‑NOW now that the channel is set
  if (esp_now_init() != 0) {
    Serial.println("ESP‑NOW Init Failed");
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
  esp_now_register_send_cb(onSent);

  dht.begin();
}

void loop() {
  // Read sensor data
  myData.temperature = dht.readTemperature();
  myData.humidity = dht.readHumidity();
  
  Serial.print("Sending Data - Temperature: ");
  Serial.print(myData.temperature);
  Serial.print(" °C, Humidity: ");
  Serial.print(myData.humidity);
  Serial.println(" %");
  
  // Send sensor data via ESP‑NOW
  esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
  
  // Blink the LED to indicate sending
  digitalWrite(LED_PIN, LOW);   // Turn LED on (active LOW)
  delay(100);
  digitalWrite(LED_PIN, HIGH);  // Turn LED off
  
  delay(1000);  // Wait 1 second before next reading
}
