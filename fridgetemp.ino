/***********************************
 * IoT Fridge Sensor Project
 ***********************************/
#include <Arduino.h>
#include "esp_timer.h"
#include <WiFiMulti.h>
#include <base64.h>
#include "DHTesp.h" //used for dht22 sensor
#include "general_config.h"
#include "Esp32MQTTClient.h"

//setup defines and statics for project
DHTesp dht;
static bool hasIoTHub = false;
//GENERAL DEFINES
const int dhtPin = 22;               /* pin that the dht sensor is connected to */
#define uS_TO_S_FACTOR 1000000    /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  60         /* Time ESP32 will go to sleep (in seconds) */
#define SUBTRACT_TO_CALIBRATE 64  /* Amount to subtract from temperature to calibrate it. */
#define MAX_BOOT_CYCLE 100        /* maximum number of boots allowed before system is restarted for durability */
RTC_DATA_ATTR int bootCount = 0;  /* boot counter */



// Helpers for this board

#ifdef __cplusplus
extern "C" {
#endif

uint8_t temprature_sens_read();
//uint8_t g_phyFuns;

#ifdef __cplusplus
}
#endif


// getDefaultSensor - harvests temperature information and builds response JSON string.
String getDefaultSensor() {



  int64_t time_since_boot = esp_timer_get_time();
  char upstr[256];
  sprintf(upstr, "%lld", time_since_boot);
  String upTime = String(upstr);
  
  
  //do work here for internal temp sensor
  String normTemp = String(getTempNormalized());
  delay(10);

  // Reading temperature and humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
  TempAndHumidity lastValues = dht.getTempAndHumidity();

  // Reading temperature and humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
  String normDHTTemp = String(dht.toFahrenheit(lastValues.temperature));
  String normDHTHumidity = String(lastValues.humidity);
  
  String payload = String("{\"Uptime\":\"") +  upTime + "," +
                   String("\"InternalTempSensor\":\"") +  normTemp + "," +
                   String("\"ExternalTempSensor\":\"") +  normDHTTemp + "," +
                   String("\"ExternalHumiditySensor\":\"") +  normDHTHumidity + "," +
                   String("\"Scale:\"") + "F" + "," +
                   String("\",\"Sig:\"") + WiFi.RSSI() +
                   String("\"}");


  return  payload;
}

// returns a float with the normalized temperature.
float getTempNormalized(){
  //this requires us to normalize with a calibration value.
  return (temprature_sens_read() - SUBTRACT_TO_CALIBRATE);
}

// sendTelemetryData - sends data to Azure IOT using MQTT client.
void sendTelemetryData(String data) {

  Serial.println("start sending events.");
  if (hasIoTHub)
  {
    char buff[128];

    // replace the following line with your data sent to Azure IoTHub
    data.toCharArray(buff, 128);
    
    if (Esp32MQTTClient_SendEvent(buff))
    {
      Serial.println("Sending data succeed");
    }
    else
    {
      Serial.println("Failure...");
    }
    delay(5000);
  }

}

//sendTelemetry - wraps the call to send data to azure iot and method to get sensor data.
void sendTelemetry() {
  sendTelemetryData(getDefaultSensor());
}

// setup - main setup method to initialize esp32
void setup() {

 // Initialize temperature sensor
dht.setup(dhtPin, DHTesp::DHT22);

  //open serial
  //TODO: do I need this in clean deploy?
  Serial.begin(115200);
  //Increment boot number and print it every reboot, later we will check this and reboot after so much uptime.
  ++bootCount;
  
  //check if boot count exceeds max cycle and then issue a restart.
  if(bootCount > MAX_BOOT_CYCLE){
    Serial.println("Restarting device due to max boot cycle.");
    delay(100);
    esp_restart();
  }


  Serial.println("Boot number: " + String(bootCount));
  Serial.println("Starting connecting WiFi.");
  delay(10);
  
  //start wifi, connect and delay until status is connected.
  WiFi.begin(wifissid, wifipassword);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  delay(10);
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  delay(10);
  Serial.println(WiFi.localIP());
  delay(10);

  //connect the esp client.
  if (!Esp32MQTTClient_Init((const uint8_t*)mqttConnectionString))
  {
    hasIoTHub = false;
    Serial.println("Initializing IoT hub failed.");
    return;
  }
  hasIoTHub = true;

  /*
  First we configure the wake up source
  */
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");
  delay(100);
  
  // gather and send telemetry to mqtt.
  sendTelemetry();
  delay(100);

  //go to sleep 
  Serial.println("Going to sleep now");
  delay(100);
  esp_deep_sleep_start();
}

//loop - no work performed in loop for this project.
void loop(){}
