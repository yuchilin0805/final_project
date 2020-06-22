import serial
import paho.mqtt.client as paho
import time
"""
mqttc = paho.Client()
# Settings for connection
host = "localhost"
topic= "posinfo"
port = 1883

# Callbacks
def on_connect(self, mosq, obj, rc):
    print("Connected rc: " + str(rc))

def on_message(mosq, obj, msg):
    print("[Received] Topic: " + msg.topic + ", Message: " + str(msg.payload) + "\n")

def on_subscribe(mosq, obj, mid, granted_qos):
    print("Subscribed OK")

def on_unsubscribe(mosq, obj, mid, granted_qos):
    print("Unsubscribed OK")

# Set callbacks
mqttc.on_message = on_message
mqttc.on_connect = on_connect
mqttc.on_subscribe = on_subscribe
mqttc.on_unsubscribe = on_unsubscribe

# Connect and subscribe
print("Connecting to " + host + "/" + topic)
mqttc.connect(host, port=1883, keepalive=60)
mqttc.subscribe(topic, 0)
"""

# XBee setting
serdev = '/dev/ttyUSB1'
s = serial.Serial(serdev, 9600)
print("ok")
while(1):
    """x=s.read(6)
    print('x:', x.decode())
    y=s.read(6)
    print("y:",y.decode())"""
    buf=""
    get=s.read(1)
    #print(get.decode())
    if get.decode()=='$':
        while(1):
            get=s.read(1)
            if get.decode()=='#':
                break
            buf+=get.decode()
    print(buf)
    #mesg=x+y
    #mqttc.publish(topic,mesg)


s.close()