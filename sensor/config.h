#ifndef CONFIG_H

#define CONFIG_H

// wifi settings
#define SSID1 "your-wifi-ssid"
#define PWD1 "your-wifi-password"

// NTP server
#define NTP_SERVER "monolith.onasticksoftware.net"

// MQTT broker
#define MQTT_BROKER NTP_SERVER

// GPIO connected to one-wire bus for temperature sensors
#define ONE_WIRE_BUS 14

// GPIO used for status LED
#define STATUS_LED 15

// GPIO used for ADC to the battery
#define BATTERY_ADC 3

#endif
