
// ***** Compilation time features selection
// Comment or uncomment these directives for compilation time configuration 
#define DEBUG                               // Comment to supress debug messages to serial 
#define USE_SECRETS                         // Use Secrets.h as predefined values for WiFi Manager
#define USE_LED                             // Use LED to indicate state
#define USE_SETUP_PIN                       // Use SETUP_PIN for start configuration
#define USE_DHT_SENSOR                      // Use DHT sensor for temperature/humidity measurement
#define USE_BMP280_SENSOR                   // Use BMP280 sensor for temperature/presure measurement
// ***** End of compilation time feature selection

// Set defines for detailed configuration (or nothing and use defaults)

// ***** LED section
#define LED_PIN LED_BUILTIN                 // Pin where LED is connected

// ***** SETUP_PIN section
#define SETUP_PIN D7                        // Pin for reset and new setup - start captive portal with saved values

// ***** DHT sensor section
#define DHT_PIN D6                          // Digital pin connected to the DHT sensor
#define DHT_TYPE DHT11                      // DHT 11 | DHT 22 (AM2302) | DHT 21 (AM2301)
//#define DHT_NO_HEATINDEX                    // Suppress sending heat index
//#define DHT_NO_DEWPOINT                     // Suppress sending dew point

// ***** DHT sensor defaults (overridden by run-time settings)
#define DHT_FIELD_TEMPERATURE "temperature" // DHT temperature field value
#define DHT_FIELD_HUMIDITY "humidity"       // DHT humidity field value
#define DHT_FIELD_HEATINDEX "heatIndex"     // DHT heat index field value
#define DHT_FIELD_DEWPOINT "dewPoint"       // DHT dew point field value

// ***** BMP280 sensor section
#define BMP280_I2C_ADDRESS BMP280_ADDRESS_ALT   // BMP280 I2C ADDRESS
//#define BMP280_NO_TEMPERATURE               // Suppress sending temperature

// ***** BMP280 sensor defaults (overridden by run-time settings)
#define BMP280_FIELD_TEMPERATURE "temperature"  // BMP280 temperature field value
#define BMP280_FIELD_PRESSURE "pressure"        // BMP280 pressure field value

// ***** InfluxDB defaults (overridden by run-time settings)
#define INFLUXDB_MEASUREMENT "temperature"
#define INFLUXDB_LOCATION "Living room"
