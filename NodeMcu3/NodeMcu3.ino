#include <ESP8266WiFi.h>
#include <espnow.h>
#include <PubSubClient.h>
#include <DHT.h>

// **WiFi & MQTT Configuration**
const char* mqtt_server = "5.196.78.28";
const char* ssid = "Sarkar 1";
const char* password = "asha1981";

WiFiClient espClient;
PubSubClient client(espClient);

// **DHT Sensor Configuration**
#define DHTPIN D2  // GPIO4 on NodeMCU
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// **ESP-01 MAC Address (Replace with actual MAC)**
uint8_t esp01MAC[] = {0xEC, 0xFA, 0xBC, 0x37, 0x31, 0xF0};

// **Structure for Sending LED Command**
typedef struct {
  bool ledState;
} led_command;

led_command ledData;

// **ESP-NOW Send Callback**
void onDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("ESP-NOW Send Status: ");
  Serial.println(sendStatus == 0 ? "Success" : "Failed");
}

// **MQTT Callback Function**
void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("📩 Received MQTT message: ");
  Serial.println(message);

  if (String(topic) == "home/led") {
    if (message == "ON") {
      ledData.ledState = true;
      Serial.println("💡 Sending LED ON command to ESP-01");
    } else if (message == "OFF") {
      ledData.ledState = false;
      Serial.println("💡 Sending LED OFF command to ESP-01");
    }

    esp_now_send(esp01MAC, (uint8_t*)&ledData, sizeof(ledData));
  }
}

void setup() {
  Serial.begin(115200);
  
  dht.begin();  // Start DHT sensor

  // **Connect to WiFi**
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("✅ Connected!");

  // **Setup MQTT**
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  reconnectMQTT();

  // **Setup ESP-NOW**
  if (esp_now_init() != 0) {
    Serial.println("❌ ESP-NOW Initialization Failed");
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(onDataSent);
  esp_now_add_peer(esp01MAC, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
}

void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();  // Handle MQTT messages

  // **Read & Publish DHT Data Every 10 Sec**
  static unsigned long lastSent = 0;
  if (millis() - lastSent > 10000) {
    lastSent = millis();

    float temp = dht.readTemperature();
    float hum = dht.readHumidity();

    if (!isnan(temp) && !isnan(hum)) {
      char tempStr[8], humStr[8];
      dtostrf(temp, 6, 2, tempStr);
      dtostrf(hum, 6, 2, humStr);

      client.publish("home/temperature", tempStr);
      client.publish("home/humidity", humStr);

      Serial.print("📡 Sent Temp: "); Serial.println(tempStr);
      Serial.print("📡 Sent Humidity: "); Serial.println(humStr);
    } else {
      Serial.println("❌ Failed to read from DHT sensor!");
    }
  }
}

// **Reconnect MQTT**
void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("🔄 Connecting to MQTT...");
    if (client.connect("NodeMCU_Client")) {
      Serial.println("✅ Connected!");
      client.subscribe("home/led");
    } else {
      Serial.print("❌ Failed, rc=");
      Serial.print(client.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}
