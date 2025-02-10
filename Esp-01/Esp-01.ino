#include <ESP8266WiFi.h>
#include <espnow.h>
#include <DHT.h>

#define DHTPIN 2  // GPIO2 (DHT11 Data pin)
#define DHTTYPE DHT11  // Change to DHT22 if using DHT22

DHT dht(DHTPIN, DHTTYPE);

uint8_t broadcastAddress[] = {0x8C, 0xAA, 0xB5, 0xC5, 0x09, 0xF8};  // Replace with NodeMCU MAC address

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
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW Init Failed");
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
  esp_now_register_send_cb(onSent);
  
  dht.begin();
}

void loop() {
  myData.temperature = dht.readTemperature();
  myData.humidity = dht.readHumidity();
  
  Serial.print("Humidity: ");
  Serial.print(myData.humidity);
  Serial.print("%  Temperature: ");
  Serial.print(myData.temperature);
  Serial.println("°C");

  esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
  
  delay(1000);  // Send data every 5 seconds
}
