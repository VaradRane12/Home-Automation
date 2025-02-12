from flask import Flask, jsonify, request, redirect, url_for
import paho.mqtt.client as mqtt
import threading

app = Flask(__name__)

# Global variables to store the latest sensor data
latest_temp = None
latest_humidity = None

# Global MQTT client variable
mqtt_client = None

# MQTT broker configuration (matches your NodeMCU code)
MQTT_BROKER = "5.196.78.28"  # Your MQTT server (Mosquitto broker)
MQTT_PORT = 1883
TOPIC_TEMP = "testnode1234567890/temp"
TOPIC_HUMIDITY = "testnode1234567890/humidity"
TOPIC_LIGHT = "testnode1234567890/led"

# Callback when the client connects to the MQTT broker
def on_connect(client, userdata, flags, rc):
    print("Connected to MQTT Broker with result code " + str(rc))
    # Subscribe to the topics for temperature and humidity
    client.subscribe(TOPIC_TEMP)
    client.subscribe(TOPIC_HUMIDITY)

# Callback when a PUBLISH message is received from the MQTT broker
def on_message(client, userdata, msg):
    global latest_temp, latest_humidity
    # Decode the payload as a UTF-8 string
    payload = msg.payload.decode("utf-8")
    if msg.topic == TOPIC_TEMP:
        latest_temp = payload
        print(f"Received Temperature: {latest_temp}")
    elif msg.topic == TOPIC_HUMIDITY:
        latest_humidity = payload
        print(f"Received Humidity: {latest_humidity}")

# Function to run the MQTT client loop in a background thread
def mqtt_thread():
    global mqtt_client
    mqtt_client = mqtt.Client()
    mqtt_client.on_connect = on_connect
    mqtt_client.on_message = on_message

    mqtt_client.connect(MQTT_BROKER, MQTT_PORT, 60)
    mqtt_client.loop_forever()

# Start the MQTT client in a separate (daemon) thread
threading.Thread(target=mqtt_thread, daemon=True).start()

# Flask route for a simple HTML dashboard with sensor data and light control buttons
@app.route("/")
def index():
    html = f"""
    <h1>Sensor Data Dashboard</h1>
    <p><strong>Temperature:</strong> {latest_temp if latest_temp is not None else "N/A"}</p>
    <p><strong>Humidity:</strong> {latest_humidity if latest_humidity is not None else "N/A"}</p>
    <h2>Light Control</h2>
    <form action="/publish/light" method="post">
      <button type="submit" name="command" value="ON">Turn Light ON</button>
      <button type="submit" name="command" value="OFF">Turn Light OFF</button>
    </form>
    """
    return html

# Flask route to return sensor data as JSON
@app.route("/data")
def data():
    return jsonify({
        "temperature": latest_temp,
        "humidity": latest_humidity
    })

# Flask route to publish light control commands via MQTT
@app.route("/publish/light", methods=["POST"])
def publish_light():
    global mqtt_client
    command = request.form.get("command", "OFF")
    if mqtt_client is not None:
        mqtt_client.publish(TOPIC_LIGHT, command)
        print(f"Published light command: {command}")
    return redirect(url_for("index"))

if __name__ == "__main__":
    app.run(debug=True, host="0.0.0.0", port=5000)
