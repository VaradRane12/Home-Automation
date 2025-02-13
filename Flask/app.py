from flask import Flask, render_template
import paho.mqtt.client as mqtt

app = Flask(__name__)
mqtt_client = mqtt.Client()
mqtt_client.connect("5.196.78.28", 1883, 60)

# Store last received data
temperature = None
humidity = None

# MQTT Callback Function
def on_message(client, userdata, message):
    global temperature, humidity
    topic = message.topic
    payload = message.payload.decode()
    if topic == "HomeAutomation981237419/temperature":
        temperature = payload
    elif topic == "HomeAutomation981237419/humidity":
        humidity = payload

mqtt_client.on_message = on_message
mqtt_client.subscribe("HomeAutomation981237419/temperature")
mqtt_client.subscribe("HomeAutomation981237419/humidity")
mqtt_client.loop_start()

@app.route("/")
def index():
    return render_template("index.html", temperature=temperature, humidity=humidity)

@app.route("/get_temp", methods=["POST"])
def get_temp():
    mqtt_client.publish("HomeAutomation981237419/request", "get_data")
    return "Requested temperature data"

@app.route("/led_on", methods=["POST"])
def led_on():
    mqtt_client.publish("HomeAutomation981237419/led", "led_on")
    return "Turned LED ON"

@app.route("/led_off", methods=["POST"])
def led_off():
    mqtt_client.publish("HomeAutomation981237419/led", "led_off")
    return "Turned LED OFF"

if __name__ == "__main__":
    app.run(debug=True)
