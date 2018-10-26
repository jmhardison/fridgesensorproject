/***********************************
 * IoT Fridge Sensor Project
 ***********************************/

//WIFI SSID and Password
const char* wifissid     = "ssid here";
const char* wifipassword = "password here";

//MQTT connection string
static const char* mqttConnectionString = "connection string here, form of HostName=x;DeviceId=x;SharedAccessKey=";

//GENERAL DEFINES
const int dhtPin = 22;               /* pin that the dht sensor is connected to */
#define uS_TO_S_FACTOR 1000000    /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  60         /* Time ESP32 will go to sleep (in seconds) */
#define SUBTRACT_TO_CALIBRATE 64  /* Amount to subtract from temperature to calibrate it. */
#define MAX_BOOT_CYCLE 100        /* maximum number of boots allowed before system is restarted for durability */
RTC_DATA_ATTR int bootCount = 0;  /* boot counter */
