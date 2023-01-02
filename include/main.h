#ifndef MAIN_H_
// Multiple include detection
#define MAIN_H_

/***** Compilation time features selection *****/
// Comment or uncomment these directives for compilation time configuration 
#define DEBUG                               // Comment to supress debug messages to serial 
#define USE_SECRETS                         // Use Secrets.h as predefined values for WiFi Manager
#define USE_LED                             // Use LED to indicate state
#define USE_SETUP_PIN                       // Use SETUP_PIN for start configuration
#define USE_DHT_SENSOR                      // Use DHT sensor for measurement
/***** End of compilation time feature selection *****/

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

#ifdef USE_SECRETS
#include "secrets.h"
#endif

#include "debug.h"

#define VERSION "0.1.0"                     // Version string
#define DEVICE_NAME "ESP-FLUX-TEMP"         // Device name
#define LOOP_INTERVAL 5*60*1000             // Loop delay interval

// ***** LED section
#ifdef USE_LED
#define LED_PIN LED_BUILTIN                 // Pin where LED is connected
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

// ***** SETUP_PIN section
#ifdef USE_SETUP_PIN
#define SETUP_PIN D7                        // Pin for reset and new setup - start captive portal with saved values
// Interrupt routine for restart
void IRAM_ATTR interruptRestart();
#endif

// ***** DHT sensor section
#ifdef USE_DHT_SENSOR
#define DHT_PIN D1                          // Digital pin connected to the DHT sensor
#define DHT_TYPE DHT11                      // DHT 11 | DHT 22 (AM2302) | DHT 21 (AM2301)
DHT dht(DHT_PIN, DHT_TYPE);                 // Initialize DHT sensor.
#endif

// ***** Configuration file section
#define JSON_CONFIG_FILE "/config-v1.json"  // Configuration file name and version
#define JSON_PIN "configPin"                // Configuration PIN
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
#else
#define WIFIMANAGER_DEBUG false             // Supress WiFiManager debug messages
#endif

// Define library static instances
WiFiManager wm;
InfluxDBClient client;

// Define configuration variables
#ifndef INFLUXDB_URL
#define INFLUXDB_URL "https://westeurope-1.azure.cloud2.influxdata.com" // Default value for InfluxDB Cloud
#endif
#ifndef INFLUXDB_BUCKET
#define INFLUXDB_BUCKET "data"              // Default bucket name
#endif
#ifndef INFLUXDB_TOKEN
#define INFLUXDB_TOKEN ""                   // Do not set here, use secrets.h instead
#endif
#ifndef INFLUXDB_ORG
#define INFLUXDB_ORG ""                     // Do not set here, use secrets.h instead
#endif 
char influxUrl[100] = INFLUXDB_URL;         // InfluxDB URL address
//char influxCert[100];
char influxOrg[50] = INFLUXDB_ORG;          // InfluxDB organization id 
char influxBucket[50] = INFLUXDB_BUCKET;    // InfluxDB bucket name
char influxToken[100] = INFLUXDB_TOKEN;     // InfluxDB 
//char ntpServer1[100] = "pool.ntp.org";
//char ntpServer1[100] = "time.nis.gov";
//char ntpZone[100] = TZ_Europe_Prague; // Central Europe timezone (TZ.h), see https://ftp.fau.de/aminet/util/time/tzinfo.txt
char measurementName[50] = "temperature";
char location[50] = "Living room";
char configPin[20];

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