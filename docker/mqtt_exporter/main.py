#!/usr/bin/env python3
import os, sys, time, json, logging, serial
import paho.mqtt.client as mqtt
from prometheus_client import Counter, Gauge, start_http_server

MQTT_ADDRESS = os.getenv("MQTT_ADDRESS")
MQTT_PORT = int(os.getenv("MQTT_PORT", "1883"))

logging.basicConfig(filename='/mqtt_exporter/log/register.log', level=logging.DEBUG)
LOG = logging.getLogger("[mqtt-exporter]")

prom_msg_counter = Counter('number_msgs', 'Number of received messages')
prom_temp_c = Gauge('temp_c', 'Temperature [Celsius Degrees]')
prom_temp_f = Gauge('temp_F', 'Temperature [Fahrenheit Degrees]')
prom_switch = Gauge('switch', 'Switch state (0/1)')

def on_connect(client, userdata, flags, rc, properties=None):
    LOG.info("Connected rc=%s", rc)
    if rc != mqtt.CONNACK_ACCEPTED:
        LOG.error("MQTT error rc=%s", rc)
        return
    client.subscribe("temp_c")
    client.subscribe("temp_F")
    client.subscribe("switch")

def on_message(client, userdata, msg):
    LOG.info("[Topic: %s] %s", msg.topic, msg.payload)
    try:
        val = json.loads(msg.payload)
        val = float(val)
    except Exception:
        return

    prom_msg_counter.inc()
    if msg.topic == "temp_c":
        prom_temp_c.set(val)
    elif msg.topic == "temp_F":
        prom_temp_f.set(val)
    elif msg.topic == "switch":
        prom_switch.set(val)

def main():
    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
    client.on_connect = on_connect
    client.on_message = on_message

    start_http_server(9000)

    while True:
        try:
            client.connect(MQTT_ADDRESS, MQTT_PORT, 60)
            break
        except Exception:
            print("MQTT client could not connect to Mosquitto", flush=True)
            time.sleep(5)

    client.loop_start()

    try:
        ser = serial.Serial("/dev/ttyACM0", 115200, timeout=2)
    except Exception as e:
        LOG.error("Serial open failed: %s", e)
        sys.exit(1)

    while True:
        line = ser.readline()
        if not line:
            continue

        s = line.decode(errors="ignore").strip()
        LOG.debug("Serial: %s", s)

        if "=" not in s:
            continue

        key, val = s.split("=", 1)
        key = key.strip()
        val = val.strip()

        if key not in ("temp_c", "temp_F", "switch"):
            continue

        try:
            fval = float(val)
        except ValueError:
            continue

        # publicar como JSON numérico (para que json.loads funcione)
        client.publish(key, payload=json.dumps(fval), qos=0, retain=False)
        print(f"Published {key}={fval}", flush=True)

if __name__ == "__main__":
    main()
