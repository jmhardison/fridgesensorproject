# Fridge Sensor Project

This project is for an ESP32 Fridge Sensor IoT device.
It currently is designed to harvest both an internal temperature reading, as well as external temperature and humidity through a DHT22 sensor.

This data is then structured into a JSON object and sent to an MQTT server.

```
{
    "Uptime":"x",
    "InternalTempSensor":"x",
    "ExternalTempSensor":"x",
    "ExternalHumiditySensor":"x",
    "Sig":"x"
}
```