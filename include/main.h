#ifndef MAIN_H_
// Multiple include detection
#define MAIN_H_

// Version and device name first
#define VERSION "0.1.1"                     // Version string
#define DEVICE_NAME "ESP-FLUX-TEMP"         // Device name

// Include compile time configuration
#include "config.h"

// Basic include
#include <Arduino.h>
// Wifi support
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
// File system for configuration store
#include <LittleFS.h>
#include <ArduinoJson.h>
// InfluxDB
#include <InfluxDbClient.h>
//#include <TZ.h> // Time zone constants https://github.com/esp8266/Arduino/blob/master/cores/esp8266/TZ.h
// Adafruit DHT sensor library
#ifdef USE_DHT_SENSOR
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#endif
#ifdef USE_BMP280_SENSOR
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#endif

#ifdef USE_SECRETS
#include "secrets.h"
#endif

#include "debug.h"

#ifndef LOOP_INTERVAL
#define LOOP_INTERVAL 5*60*1000             // Loop delay interval (default value is 5 min)
#endif

// ***** LED section
#ifdef USE_LED
#ifndef LED_PIN
#define LED_PIN LED_BUILTIN                 // Pin where LED is connected
#endif
#define LED_INTERVAL 150                    // LED blink interval
#define LED_ON LOW                          // Turns the LED *on*, D1 Mini: LOW, Arduino: HIGH
#define LED_OFF HIGH                        // Turns the LED *off*, D1 Mini: HIGH, Arduino: LOW
// LED blink
void blink(int count);                      
#define BLINK(int) blink(int)               // LED blink
#else
#define BLINK(int)                          // DO NOTHING - LED blink
#endif

// ***** Errors and fails - for LED
#define ERROR_READ 2                        // Error reading sensor
#define ERROR_WRITE 3                       // Error write data to InfluxDB
#define FAIL_FS 5                           // LittleFS fail
#define FAIL_I2C 7                          // Fail I2C initialisation

// ***** SETUP_PIN section
#ifdef USE_SETUP_PIN
#ifndef SETUP_PIN
#define SETUP_PIN D7                        // Pin for reset and new setup - start captive portal with saved values
#endif
// Interrupt routine for restart
void IRAM_ATTR interruptRestart();
#endif

// ***** DHT sensor section
#ifdef USE_DHT_SENSOR
#ifndef DHT_PIN
#define DHT_PIN D6                          // Digital pin connected to the DHT sensor
#endif
#ifndef DHT_TYPE
#define DHT_TYPE DHT11                      // DHT 11 | DHT 22 (AM2302) | DHT 21 (AM2301)
#endif
#ifndef DHT_FIELD_TEMPERATURE
#define DHT_FIELD_TEMPERATURE "temperature" // DHT temperature field value
#endif
#ifndef DHT_FIELD_HUMIDITY
#define DHT_FIELD_HUMIDITY "humidity"       // DHT humidity field value
#endif
#ifndef DHT_FIELD_HEATINDEX
#define DHT_FIELD_HEATINDEX "heatIndex"     // DHT heat index field value
#endif
#ifndef DHT_FIELD_DEWPOINT
#define DHT_FIELD_DEWPOINT "dewPoint"       // DHT dew point field value
#endif
DHT dht(DHT_PIN, DHT_TYPE);                 // Initialize DHT sensor
char dhtFieldTemperature[15] = DHT_FIELD_TEMPERATURE;
char dhtFieldHumidity[15] = DHT_FIELD_HUMIDITY;
#define JSON_DHT_TEMPERATURE "dhtTemperature"
#define JSON_DHT_HUMIDITY "dhtHumidity"
#endif

// ***** BMP280 sensor section
#ifdef USE_BMP280_SENSOR
#ifndef BMP280_I2C_ADDRESS
#define BMP280_I2C_ADDRESS BMP280_ADDRESS_ALT   // BMP280 I2C ADDRESS
#endif
#ifndef BMP280_FIELD_TEMPERATURE
#define BMP280_FIELD_TEMPERATURE "temperature"  // BMP280 temperature field value
#endif
#ifndef BMP280_FIELD_PRESSURE
#define BMP280_FIELD_PRESSURE "pressure"        // BMP280 pressure field value
#endif
Adafruit_BMP280 bmp280;                     // I2C connection for sensor
#ifndef BMP280_NO_TEMPERATURE
char bmp280FieldTemperature[15] = BMP280_FIELD_TEMPERATURE;
#define JSON_BMP280_TEMPERATURE "bmp280Temperature"
#endif
char bmp280FieldPressure[15] = BMP280_FIELD_PRESSURE;
#define JSON_BMP280_PRESSURE "bmp280Pressure"
#endif

// ***** Configuration file section
#define JSON_SIZE 2048
#define JSON_CONFIG_FILE "/config-v1.json"  // Configuration file name and version
#define JSON_INFLUXDB_URL "influxUrl"       // InfluxDB URL
//#define JSON_INFLUXDB_CERT "influxCert"     // InfluxDB server certificate
#define JSON_INFLUXDB_ORG "influxOrg"       // InfluxDB ORG
#define JSON_INFLUXDB_BUCKET "influxBucket" // InfluxDB Bucket
#define JSON_INFLUXDB_TOKEN "influxToken"   // InfluxDB Token
#define JSON_INFLUXDB_MEAS "measurementName"// InfluxDB Measurement name
//#define JSON_NTP_SERVER_1 "ntp1"            // NTP Server 1
//#define JSON_NTP_SERVER_2 "ntp2"            // NTP Server 2
//#define JSON_NTP_TZ "ntpTz"                 // Timezone for NTP
#define JSON_TAG_LOCATION "location"        // Tag location

#ifdef DEBUG
#define WIFIMANAGER_DEBUG true              // Show WiFiManager debug messages
//#define DHT_DEBUG                           // Uncomment to enable printing out DHT debug messages.
#else
#define WIFIMANAGER_DEBUG false             // Supress WiFiManager debug messages
#endif

// Define library static instances
WiFiManager wm;
InfluxDBClient client;

// InfluxDB configuration default values, set this values in secrets.h for
#ifndef INFLUXDB_URL
#define INFLUXDB_URL "https://westeurope-1.azure.cloud2.influxdata.com" // Default value for InfluxDB Cloud, use secrets.h for own default value
#endif
#ifndef INFLUXDB_BUCKET
#define INFLUXDB_BUCKET "data"              // Default bucket name, use secrets.h for own default value
#endif
#ifndef INFLUXDB_TOKEN
#define INFLUXDB_TOKEN ""                   // Do not set here, use secrets.h instead
#endif
#ifndef INFLUXDB_ORG
#define INFLUXDB_ORG ""                     // Do not set here, use secrets.h instead
#endif
#ifndef INFLUXDB_MEASUREMENT
#define INFLUXDB_MEASUREMENT "temperature"  // InfluxDB 
#endif
#ifndef INFLUXDB_LOCATION
#define INFLUXDB_LOCATION "Living room"
#endif

// Define InfluxDB configuration variables
char influxUrl[100] = INFLUXDB_URL;         // InfluxDB URL address
//char influxCert[100];
char influxOrg[50] = INFLUXDB_ORG;          // InfluxDB organization id 
char influxBucket[50] = INFLUXDB_BUCKET;    // InfluxDB bucket name
char influxToken[100] = INFLUXDB_TOKEN;     // InfluxDB 
char measurementName[20] = INFLUXDB_MEASUREMENT; // InfluxDB measuremen name
char location[20] = INFLUXDB_LOCATION;      // InfluxDB TAG location
//char ntpServer1[100] = "pool.ntp.org";
//char ntpServer1[100] = "time.nis.gov";
//char ntpZone[100] = TZ_Europe_Prague; // Central Europe timezone (TZ.h), see https://ftp.fau.de/aminet/util/time/tzinfo.txt

// Define internal variables
char deviceId[25];                          // Device identifier (config WiFi name), DEVICE_NAME and chip ID
bool shouldSaveConfig = false;              // Request to save configuration, set by WiFi manager callback

/***** Global function headers *****/
// Longer than 47 days millis (64 bit)
uint64_t millis64();                       
// FAIL stop with LED blinking
void fail(int count);                       
// Configration file operations
void saveConfigFile();
// Load configuration file
bool loadConfigFile();
// Delete configuration file
void deleteConfigFile();
// WiFiManager callbacks
void saveConfigCallback();
void configModeCallback(WiFiManager* myWiFiManager);

#endif