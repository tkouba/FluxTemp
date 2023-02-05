/*****************************************************************************
 * ESP8266 firmware for sending data to InfluxDB
 *****************************************************************************
 * (c) Tomas Kouba, 2022
 * Licensed under terms of the MIT license
 *****************************************************************************
 * Inspired by ESP-TMEP github.com/ridercz/ESP-TMEP by Michal A. Valasek
 *****************************************************************************/

// Main include
#include "main.h"

void setup() {
  // Get device ID
  sprintf(deviceId, DEVICE_NAME "-%08X", ESP.getChipId());

  // Init serial port
  SERIALBEGIN(9600);
  // Print header
  DPRINTLN();
  DPRINTLN();
  DPRINTLN_F(DEVICE_NAME "/" VERSION);
  DPRINTLN();
  DPRINTFLN("Device ID: %s", deviceId);
  DPRINTLN_F("DEBUG mode is ON");

  // Initialize digital pin PIN_LED as an output.
  #ifdef USE_LED
  pinMode(LED_PIN, OUTPUT);
  // Start setup
  digitalWrite(LED_PIN, LED_ON); 
  #endif

  // Read configuration from LittleFS
  bool configLoaded = loadConfigFile();
  // Configure WiFiManager options
  wm.setDebugOutput(WIFIMANAGER_DEBUG);
  wm.setSaveConfigCallback(saveConfigCallback);
  wm.setAPCallback(configModeCallback);  

  WiFiManagerParameter influxHeader("<h3>InfluxDB parameters</h3>");
  WiFiManagerParameter influxUrlParameter("influx_url", "InfluxDB URL", influxUrl, sizeof(influxUrl));  
  WiFiManagerParameter influxOrgParameter("influx_org", "InfluxDB ORG", influxOrg, sizeof(influxOrg));
  WiFiManagerParameter influxBucketParameter("influx_bucket", "InfluxDB Bucket", influxBucket, sizeof(influxBucket));
  WiFiManagerParameter influxTokenParameter("influx_token", "InfluxDB Token", influxToken, sizeof(influxToken));
  WiFiManagerParameter measurementNameParameter("measurement_name", "Measurement Name", measurementName, sizeof(measurementName));
  WiFiManagerParameter locationParameter("location", "Device location", location, sizeof(location));

  wm.addParameter(&influxHeader);
  wm.addParameter(&influxUrlParameter);
  wm.addParameter(&influxOrgParameter);
  wm.addParameter(&influxBucketParameter);
  wm.addParameter(&influxTokenParameter);
  wm.addParameter(&measurementNameParameter);
  wm.addParameter(&locationParameter);

  #ifdef USE_DHT_SENSOR
  WiFiManagerParameter dhtHeader("<h3>DHT sensor field names</h3>");
  WiFiManagerParameter dhtFieldTemperatureParameter("dht_field_temperature", "Temperature", dhtFieldTemperature, sizeof(dhtFieldTemperature));
  WiFiManagerParameter dhtFieldHumidityParameter("dht_field_humidity", "Humidity", dhtFieldHumidity, sizeof(dhtFieldHumidity));
  wm.addParameter(&dhtHeader);
  wm.addParameter(&dhtFieldTemperatureParameter);
  wm.addParameter(&dhtFieldHumidityParameter);  
  #endif

  #ifdef USE_BMP280_SENSOR
  WiFiManagerParameter bmp280Header("<h3>BMP280 sensor field names</h3>");
  wm.addParameter(&bmp280Header);
  #ifndef BMP280_NO_TEMPERATURE
  WiFiManagerParameter bmp280FieldTemperatureParameter("bmp280_field_temperature", "Temperature", bmp280FieldTemperature, sizeof(bmp280FieldTemperature));
  wm.addParameter(&bmp280FieldTemperatureParameter);
  #endif
  WiFiManagerParameter bmp280FieldPressureParameter("bmp280_field_pressure", "Pressure", bmp280FieldPressure, sizeof(bmp280FieldPressure));
  wm.addParameter(&bmp280FieldPressureParameter);  
  #endif

  // Set setup pin
  #ifdef USE_SETUP_PIN
  pinMode(SETUP_PIN, INPUT_PULLUP);
  bool setupPressed = digitalRead(SETUP_PIN) == LOW;
  if (setupPressed) {
    DPRINTLN_F("Request to start configuration portal...");
    // Starting configuration portal without timeout (manual request for configuration)
    if (!wm.startConfigPortal(deviceId)) {
      DPRINTLN_F("Config portal failed, rebooting.");
      ESP.restart();
    }
  }
  else
  {
    attachInterrupt(SETUP_PIN, interruptRestart, FALLING);
  #endif  
    if (!configLoaded) {
      DPRINTLN_F("Config load failed or config request, starting configuration portal...");
      // Starting configuration portal without timeout (config not loaded, so we have to config)
      if (!wm.startConfigPortal(deviceId)) {
        DPRINTLN_F("Config portal failed, rebooting.");
        ESP.restart();
      }
    } else {
      DPRINTLN_F("Config load successful, connecting to WiFi...");
      // Set configuration portal timeout
      wm.setConfigPortalTimeout(CONFIG_PORTAL_TIMEOUT);
      if (!wm.autoConnect(deviceId)) {
        DPRINTLN_F("Config portal failed, rebooting.");      
        ESP.restart();
      }
    }
  #ifdef USE_SETUP_PIN
  }
  #endif

  // We are connected
  DPRINT_F("Connected to network ");
  DPRINT(WiFi.SSID());
  DPRINT_F(", IP ");
  DPRINTLN(WiFi.localIP());

  // Save configuration if needed
  if (shouldSaveConfig) {
    strncpy(influxUrl, influxUrlParameter.getValue(), sizeof(influxUrl));
    strncpy(influxOrg, influxOrgParameter.getValue(), sizeof(influxOrg));
    strncpy(influxBucket, influxBucketParameter.getValue(), sizeof(influxBucket));
    strncpy(influxToken, influxTokenParameter.getValue(), sizeof(influxToken));
    strncpy(measurementName, measurementNameParameter.getValue(), sizeof(measurementName));
    strncpy(location, locationParameter.getValue(), sizeof(location));
    #ifdef USE_DHT_SENSOR    
    strncpy(dhtFieldTemperature, dhtFieldTemperatureParameter.getValue(), sizeof(dhtFieldTemperature));
    strncpy(dhtFieldHumidity, dhtFieldHumidityParameter.getValue(), sizeof(dhtFieldHumidity));    
    #endif   
    #ifdef USE_BMP280_SENSOR
    #ifndef BMP280_NO_TEMPERATURE
    strncpy(bmp280FieldTemperature, bmp280FieldTemperatureParameter.getValue(), sizeof(bmp280FieldTemperature));
    #endif
    strncpy(bmp280FieldPressure, bmp280FieldPressureParameter.getValue(), sizeof(bmp280FieldPressure));
    #endif
    saveConfigFile();
    shouldSaveConfig = false;
    // Restart after configuration changes
    ESP.restart();
  }
  
  // Configure InfluxDB client
  client.setConnectionParams(influxUrl, influxOrg, influxBucket, influxToken);
  client.setInsecure();  // Ignore invalid certificates, we are not able to validate chain correctly anyway

  // Check server connection
  if (client.validateConnection()) {
    DPRINT_F("Connected to InfluxDB: ");
    DPRINTLN(client.getServerUrl());
  } else {
    DPRINT_F("InfluxDB connection failed: ");
    DPRINTLN(client.getLastErrorMessage());
  }

  #ifdef USE_DHT_SENSOR
  // Initialize DHT sensor device
  dht.begin();
  #endif

  #ifdef USE_BMP280_SENSOR
  if (!bmp280.begin(BMP280_I2C_ADDRESS)) { 
    DPRINTLN_F("Could not find a valid BMP280 sensor, check wiring!");
    fail(FAIL_I2C);
  }
  #endif

  #ifdef USE_LED
  // End setup
  digitalWrite(LED_PIN, LED_OFF); 
  #endif
}

void loop() {
  if (WiFi.status() != WL_CONNECTED)
  {
    
  }

  BLINK(1); // Blink at every measure  

  bool saveToInflux = false; // Are there any data to save?

  // Define data point with measurement name
  Point pointDevice(measurementName);
  // Set tags
  pointDevice.addTag("device", deviceId);
  pointDevice.addTag("SSID", WiFi.SSID());
  pointDevice.addTag("location", location);

  // Add device data
  pointDevice.addField("rssi", WiFi.RSSI());
  pointDevice.addField("uptime", millis64());

  #ifdef USE_DHT_SENSOR
  DPRINT_F("Reading DHT sensor ... ");
  // Read data from sensor
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  // Read temperature as Celsius (the default)
  float dhtT = dht.readTemperature();
  // Read humidity
  float dhtH = dht.readHumidity();

  // Check if any reads failed and exit early (to try again).
  if (isnan(dhtT) || isnan(dhtH)) {
    DPRINTFLN("Failed to read from DHT%i sensor on pin %i", DHT_TYPE, DHT_PIN);
    BLINK(ERROR_READ);
  }
  else {
    DPRINTLN_F("OK");
    // Add sensor data (only not NaN)
    if (!isnan(dhtT))
      pointDevice.addField(dhtFieldTemperature, dhtT);
    if (!isnan(dhtH))
      pointDevice.addField(dhtFieldHumidity, dhtH);
    // There are some data to write to
    saveToInflux = true; 
    #ifndef DHT_NO_HEATINDEX
    // Compute heat index in Celsius (isFahreheit = false)
    float hic = dht.computeHeatIndex(dhtT, dhtH, false);
    pointDevice.addField(DHT_FIELD_HEATINDEX, hic);
    #endif
    #ifndef DHT_NO_DEWPOINT
    // Compute dew point 
    float dp =  243.04 * (log(dhtH / 100.0) + ((17.625 * dhtT) / (243.04 + dhtT))) / (17.625 - log(dhtH / 100.0) - ((17.625 * dhtT) / (243.04 + dhtT)));
    pointDevice.addField(DHT_FIELD_DEWPOINT, dp);
    #endif
  }  
  #endif

  #ifdef USE_BMP280_SENSOR 
  DPRINT_F("Reading BMP280 sensor ... ");
  // Read pressure
  float bmp280P = bmp280.readPressure();
  #ifndef BMP280_NO_TEMPERATURE
  // Read temperature
  float bmp280T = bmp280.readTemperature();
  // Check if any reads failed and exit early (to try again).
  if (isnan(bmp280P) || isnan(bmp280T)) {
  #else
  if (isnan(bmp280P)) {
  #endif
    DPRINTLN_F("Failed to read from BMP280 sensor");
    BLINK(ERROR_READ);
  }
  else {
    DPRINTLN_F("OK");
    // Add sensor data (only not NaN)
    if (!isnan(bmp280P))
      pointDevice.addField(bmp280FieldPressure, bmp280P);
    #ifndef BMP280_NO_TEMPERATURE
    if (!isnan(bmp280T))
      pointDevice.addField(bmp280FieldTemperature, bmp280T);
    #endif
    // There are some data to write to
    saveToInflux = true; 
  }
  #endif

  // Write data
  if (saveToInflux) {
    DPRINT_F("InfluxDB writing: ");
    DPRINTLN(pointDevice.toLineProtocol()); 
    if (!client.writePoint(pointDevice)) {
      // Cannot write data
      DPRINT_F("InfluxDB write failed: ");
      DPRINTLN(client.getLastErrorMessage());
      BLINK(ERROR_WRITE);
    }
  }
  else {
    DPRINTLN_F("No data to write to InfluxDB.");
  }

  delay(LOOP_INTERVAL);
}

/***** LED blink *****/
#ifdef USE_LED
void blink(int count) {
  for (int i = 0; i < count; i++)
  {
    digitalWrite(LED_PIN, LED_ON); 
    delay(LED_INTERVAL);
    digitalWrite(LED_PIN, LED_OFF); 
    delay(LED_INTERVAL);
  }
}
#endif

// FAIL stop with blinking
void fail(int count) {
  while (true)
  {
    #ifdef USE_LED
    BLINK(count);    
    delay(2*LED_INTERVAL);
    #endif
  }  
}

// Longer than 47 days millis (64 bit)
uint64_t millis64() {
    static uint32_t low32, high32;
    uint32_t new_low32 = millis();
    if (new_low32 < low32) high32++;
    low32 = new_low32;
    return (uint64_t) high32 << 32 | low32;
}

/***** Interrupt *****/
void IRAM_ATTR interruptRestart()
{
  ESP.restart();
}

/**** Configuration file operations *****/

void saveConfigFile() {
  DPRINTLN_F("Saving configuration");
  // Create a JSON document
  StaticJsonDocument<JSON_SIZE> json;
  json[JSON_INFLUXDB_URL] = influxUrl;
  //json[JSON_INFLUXDB_CERT] = influxCert;
  json[JSON_INFLUXDB_ORG] = influxOrg;
  json[JSON_INFLUXDB_BUCKET] = influxBucket;
  json[JSON_INFLUXDB_TOKEN] = influxToken;
  json[JSON_INFLUXDB_MEAS] = measurementName;
  //json[JSON_NTP_SERVER_1] = ntpServer1;
  //json[JSON_NTP_SERVER_2] = ntpServer2;
  //json[JSON_NTP_TZ] = ntpZone;  
  json[JSON_TAG_LOCATION] = location;  
  #ifdef USE_DHT_SENSOR
  json[JSON_DHT_TEMPERATURE] = dhtFieldTemperature;
  json[JSON_DHT_HUMIDITY] = dhtFieldHumidity;
  #endif
  #ifdef USE_BMP280_SENSOR
  #ifndef BMP280_NO_TEMPERATURE
  json[JSON_BMP280_TEMPERATURE] = bmp280FieldTemperature;
  #endif
  json[JSON_BMP280_PRESSURE] = bmp280FieldPressure;
  #endif
  // Open/create JSON file
  DPRINT_F("Opening " JSON_CONFIG_FILE "...");
  File configFile = LittleFS.open(JSON_CONFIG_FILE, "w");
  if (!configFile) {
    DPRINTLN_F("Failed!");
    fail(FAIL_FS);
  }
  DPRINTLN_F("OK");

  // Serialize JSON data to file
  DPRINT_F("Saving configuration...");
  if (serializeJson(json, configFile) == 0) {
    DPRINTLN_F("Failed!");
    fail(FAIL_FS);
  }
  DPRINTLN_F("OK");

  // Close file
  configFile.close();
}

bool loadConfigFile() {
  // Read configuration from FS json
  DPRINT_F("Opening LittleFS...");
  if (!LittleFS.begin()) {
    DPRINTLN_F("Failed!");
    fail(FAIL_FS);
  }
  DPRINTLN_F("OK");

  // Read existing file
  if (!LittleFS.exists(JSON_CONFIG_FILE)) {
    DPRINTLN_F("Configuration file " JSON_CONFIG_FILE " not found.");
    return false;
  }
  DPRINT_F("Opening " JSON_CONFIG_FILE "...");
  File configFile = LittleFS.open(JSON_CONFIG_FILE, "r");
  if (!configFile) {
    DPRINTLN_F("Failed!");
    return false;
  }
  DPRINTLN_F("OK");

  // Parse JSON
  DPRINT_F("Parsing JSON file...");
  StaticJsonDocument<JSON_SIZE> json;
  DeserializationError error = deserializeJson(json, configFile);
  if (error) {
    DPRINTLN_F("Failed!");
    return false;
  }
  strncpy(influxUrl, json[JSON_INFLUXDB_URL], sizeof(influxUrl));
  //strncpy(influxCert, json[JSON_INFLUXDB_CERT]);
  strncpy(influxOrg, json[JSON_INFLUXDB_ORG], sizeof(influxOrg));
  strncpy(influxBucket, json[JSON_INFLUXDB_BUCKET], sizeof(influxBucket));
  strncpy(influxToken, json[JSON_INFLUXDB_TOKEN], sizeof(influxToken));
  strncpy(measurementName, json[JSON_INFLUXDB_MEAS] | INFLUXDB_MEASUREMENT, sizeof(measurementName));
  strncpy(location, json[JSON_TAG_LOCATION] | INFLUXDB_LOCATION, sizeof(location));
  #ifdef USE_DHT_SENSOR
  strncpy(dhtFieldTemperature, json[JSON_DHT_TEMPERATURE] | DHT_FIELD_TEMPERATURE, sizeof(dhtFieldTemperature));
  strncpy(dhtFieldHumidity, json[JSON_DHT_HUMIDITY] | DHT_FIELD_HUMIDITY, sizeof(dhtFieldHumidity));
  #endif
  #ifdef USE_BMP280_SENSOR
  #ifndef BMP280_NO_TEMPERATURE
  strncpy(bmp280FieldTemperature, json[JSON_BMP280_TEMPERATURE] | BMP280_FIELD_TEMPERATURE, sizeof(bmp280FieldTemperature));
  #endif
  strncpy(bmp280FieldPressure, json[JSON_BMP280_PRESSURE] | BMP280_FIELD_PRESSURE, sizeof(bmp280FieldPressure));
  #endif
  //strcpy(ntpServer1, json[JSON_NTP_SERVER_1]);
  //strcpy(ntpServer2, json[JSON_NTP_SERVER_2]);
  //strcpy(ntpZone, json[JSON_NTP_TZ]);
  DPRINTLN_F("OK");
  return true;
}

// **** WiFiManager callbacks 

void saveConfigCallback() {
  shouldSaveConfig = true;
}

void configModeCallback(WiFiManager* myWiFiManager) {
  DPRINTLN_F("Configuration mode:");
  DPRINT_F("  SSID: ");
  DPRINTLN(myWiFiManager->getConfigPortalSSID());
  DPRINT_F("  IP Address: ");
  DPRINTLN(WiFi.softAPIP());
} 
