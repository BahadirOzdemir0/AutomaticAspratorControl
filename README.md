# Smart Kitchen Extractor Fan Controller (ESP8266 + Arduino IoT Cloud)

This project implements an intelligent extractor fan control system using an **ESP8266** microcontroller, **DHT11** for temperature and humidity sensing, and **MQ135** for air quality monitoring. It determines the optimal fan speed (3 levels) based on environmental data using **fuzzy logic**, and supports **manual override and lamp control** through **Arduino IoT Cloud**.

## Features

* 🚨 **Air Quality Detection**: Monitors VOC levels using the MQ135 gas sensor.
* 🌡️ **Environmental Sensing**: Reads temperature and humidity data via DHT11.
* 🤖 **Fuzzy Logic Control**: Determines fan speed intelligently based on recent environmental conditions.
* 📲 **IoT Cloud Connectivity**: Provides remote control and monitoring via Arduino IoT Cloud.
* 💡 **Lamp Control**: Allows toggling of a lamp output from the cloud.
* 🛑 **Safety Alarms**: Alerts for gas anomalies and high temperatures.
* 🔄 **Watchdog Timer**: Ensures system stability with periodic watchdog resets.

## Hardware Requirements

* ESP8266 (NodeMCU v3)
* DHT11 Temperature and Humidity Sensor
* MQ135 Gas Sensor
* 3 Output Channels (Fan speed levels via relay/triac control)
* Lamp Output
* Appropriate GPIO pin wiring

## File Structure

* `main.ino`: Main Arduino sketch implementing sensor reading, fuzzy control, and IoT handling.
* `arduino_secrets.h`: Contains your Wi-Fi SSID and password (not included in repo).
* `thingProperties.h`: Auto-generated file for Arduino IoT Cloud variable bindings.

## How It Works

1. Reads temperature, humidity, and gas concentration periodically.
2. Stores recent 20 measurements to calculate average values.
3. Applies fuzzy logic rules to decide the appropriate fan speed.
4. In auto mode, updates fan speed every minute.
5. In manual mode, allows user to control fan speed and lamp state from Arduino Cloud.

## Example Output

```
T: 28.00 C | H: 45.00 % | PPM: 175.22
Avg T: 27.5 | Avg H: 46.2 | Avg P: 170.1 → Fan Speed: 2
```

## Notes

* Watchdog timer is enabled for safety (15s timeout).
* You must define cloud variables (`aspspeed`, `asplamp`, `otomatikMod`, etc.) in the Arduino IoT Cloud UI.
* **Make sure to install required libraries: **************************\`\`**************************, and others**
