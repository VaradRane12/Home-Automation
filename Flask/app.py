from flask import Flask, render_template, request
import paho.mqtt.client as mqtt

app = Flask(__name__)

# MQTT Broker Configuration
MQTT_BROKER = "5.196.78.28"
MQTT_PORT = 1883
MQTT_TOPICS = ["home/temperature", "home/humidity", "home/door_status"]

# Global Variables for Storing Data
sensor_data = {
    "temperature": "N/A",
    "humidity": "N/A",
    "door_status": "Unknown"
}

# MQTT Callback when a message is received
def on_message(client, userdata, msg):
    topic = msg.topic
    payload = msg.payload.decode("utf-8")

    if topic == "home/temperature":
        sensor_data["temperature"] = payload
    elif topic == "home/humidity":
        sensor_data["humidity"] = payload
    elif topic == "home/door_status":
        sensor_data["door_status"] = payload

    print(f"Received: {topic} → {payload}")

# MQTT Setup
client = mqtt.Client()
client.on_message = on_message
client.connect(MQTT_BROKER, MQTT_PORT, 60)

# Subscribe to all topics
for topic in MQTT_TOPICS:
    client.subscribe(topic)

client.loop_start()  # Start the MQTT loop in the background

@app.route("/")
def index():
    return render_template("index.html", sensor_data=sensor_data)

@app.route("/led/<state>")
def control_led(state):
    if state == "on":
        client.publish("home/led", "ON")
    else:
        client.publish("home/led", "OFF")
    return "OK"

if __name__ == "__main__":
    app.run(debug=True)
