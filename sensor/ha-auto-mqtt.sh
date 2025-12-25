#!/bin/sh
# publish HA MQTT auto-discovery information for the sensors
mosquitto_pub -r -t homeassistant/sensor/hotwater/temp0/config -f payload0.txt
mosquitto_pub -r -t homeassistant/sensor/hotwater/temp1/config -f payload1.txt
