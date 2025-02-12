from flask import Flask, render_template
import paho.mqtt.client as mqtt

app = Flask(__name__)
mqtt_client = mqtt.Client()
mqtt_client.connect("5.196.78.28", 1883, 60)

@app.route("/")
def index():
    return render_template("index.html")

@app.route("/get_temp", methods=["POST"])
def get_temp():
    mqtt_client.publish("home/request", "get_data")
    return "Requested temperature data"

@app.route("/led_on", methods=["POST"])
def led_on():
    mqtt_client.publish("home/led", "led_on")
    return "Turned LED ON"

@app.route("/led_off", methods=["POST"])
def led_off():
    mqtt_client.publish("home/led", "led_off")
    return "Turned LED OFF"

if __name__ == "__main__":
    app.run(debug=True)
