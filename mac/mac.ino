#include <ESP8266WiFi.h>

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);  // Set device to station mode

}

void loop() {
  Serial.print("ESP MAC Address: ");
  Serial.println(WiFi.macAddress());
}
