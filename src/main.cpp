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

  if (!configLoaded) {
    // Generate random config PIN
    itoa(random(1000000, 99999999), configPin, DEC);    
  }

  // Configure WiFiManager options
  wm.setDebugOutput(WIFIMANAGER_DEBUG);
  wm.setSaveConfigCallback(saveConfigCallback);
  wm.setAPCallback(configModeCallback);  

  WiFiManagerParameter influxUrlParameter("influx_url", "InfluxDB URL", influxUrl, sizeof(influxUrl));  
  WiFiManagerParameter influxOrgParameter("influx_org", "InfluxDB ORG", influxOrg, sizeof(influxOrg));
  WiFiManagerParameter influxBucketParameter("influx_bucket", "InfluxDB Bucket", influxBucket, sizeof(influxBucket));
  WiFiManagerParameter influxTokenParameter("influx_token", "InfluxDB Token", influxToken, sizeof(influxToken));
  WiFiManagerParameter measurementNameParameter("measurement_name", "Measurement Name", measurementName, sizeof(measurementName));
  WiFiManagerParameter locationParameter("location", "Device location", location, sizeof(location));
  WiFiManagerParameter configPinParameter("config_pin", "Configuration PIN", configPin, sizeof(configPin));

  wm.addParameter(&influxUrlParameter);
  wm.addParameter(&influxOrgParameter);
  wm.addParameter(&influxBucketParameter);
  wm.addParameter(&influxTokenParameter);
  wm.addParameter(&measurementNameParameter);
  wm.addParameter(&locationParameter);
  wm.addParameter(&configPinParameter);

  // Set setup pin
  #ifdef USE_SETUP_PIN
  pinMode(SETUP_PIN, INPUT_PULLUP);
  bool setupPressed = digitalRead(SETUP_PIN) == LOW;
  if (setupPressed) {
    DPRINTLN_F("Request to start configuration portal...");
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
      if (!wm.startConfigPortal(deviceId)) {
        DPRINTLN_F("Config portal failed, rebooting.");
        ESP.restart();
      }
    } else {
      DPRINTLN_F("Config load successful, connecting to WiFi...");
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
  DPRINT_F("Configuration PIN: ");
  DPRINTLN(configPin);

  // Save configuration if needed
  if (shouldSaveConfig) {
    strncpy(influxUrl, influxUrlParameter.getValue(), sizeof(influxUrl));
    strncpy(influxOrg, influxOrgParameter.getValue(), sizeof(influxOrg));
    strncpy(influxBucket, influxBucketParameter.getValue(), sizeof(influxBucket));
    strncpy(influxToken, influxTokenParameter.getValue(), sizeof(influxToken));
    strncpy(measurementName, measurementNameParameter.getValue(), sizeof(measurementName));
    strncpy(location, locationParameter.getValue(), sizeof(location));    
    strncpy(configPin, configPinParameter.getValue(), sizeof(configPin));
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

  #ifdef USE_LED
  // End setup
  digitalWrite(LED_PIN, LED_OFF); 
  #endif
}

void loop() {
  BLINK(1); // Blink at every measure

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
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  // Add sensor data (only not NaN)
  if (!isnan(h))
    pointDevice.addField("humidity", h);
  if (!isnan(t))
    pointDevice.addField("temperature", t);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    DPRINTFLN("Failed to read from DHT%i sensor on pin %i", DHT_TYPE, DHT_PIN);
    BLINK(ERROR_READ);
  }
  else {
    DPRINTLN_F("OK");
    // Compute heat index in Celsius (isFahreheit = false)
    float hic = dht.computeHeatIndex(t, h, false);
    pointDevice.addField("heatIndex", hic);
    // Compute dew point 
    float dp =  243.04 * (log(h / 100.0) + ((17.625 * t) / (243.04 + t))) / (17.625 - log(h / 100.0) - ((17.625 * t) / (243.04 + t)));
    pointDevice.addField("dewPoint", dp);
  }  
  #endif

  // Write data
  DPRINT_F("InfluxDB writing: ");
  DPRINTLN(pointDevice.toLineProtocol()); 
  if (!client.writePoint(pointDevice)) {
    // Cannot write data
    DPRINT_F("InfluxDB write failed: ");
    DPRINTLN(client.getLastErrorMessage());
    BLINK(ERROR_WRITE);
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
  // Create a JSON document
  StaticJsonDocument<1024> json;
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
  json["configPin"] = configPin;

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
  StaticJsonDocument<2048> json;
  DeserializationError error = deserializeJson(json, configFile);
  if (error) {
    DPRINTLN_F("Failed!");
    return false;
  }
  strncpy(influxUrl, json[JSON_INFLUXDB_URL], sizeof(influxUrl));
  //strcpy(influxCert, json[JSON_INFLUXDB_CERT]);
  strncpy(influxOrg, json[JSON_INFLUXDB_ORG], sizeof(influxOrg));
  strncpy(influxBucket, json[JSON_INFLUXDB_BUCKET], sizeof(influxBucket));
  strncpy(influxToken, json[JSON_INFLUXDB_TOKEN], sizeof(influxToken));
  strncpy(measurementName, json[JSON_INFLUXDB_MEAS], sizeof(measurementName));
  //strcpy(ntpServer1, json[JSON_NTP_SERVER_1]);
  //strcpy(ntpServer2, json[JSON_NTP_SERVER_2]);
  //strcpy(ntpZone, json[JSON_NTP_TZ]);
  strncpy(location, json[JSON_TAG_LOCATION], sizeof(location));
  strncpy(configPin, json[JSON_PIN], sizeof(configPin));
  DPRINTLN_F("OK");
  return true;
}

void deleteConfigFile() {
  DPRINT_F("Opening LittleFS...");
  if (!LittleFS.begin()) {
    DPRINTLN_F("Failed!");
    fail(FAIL_FS);
  }
  DPRINTLN_F("OK");
  if (LittleFS.exists(JSON_CONFIG_FILE)) {
    DPRINT_F("Deleting " JSON_CONFIG_FILE "...");
    if (!LittleFS.remove(JSON_CONFIG_FILE)) {
      DPRINTLN_F("Failed!");
      fail(FAIL_FS);
    }
    DPRINTLN_F("OK");
  }
  else {
    DPRINTLN_F("Configuration file " JSON_CONFIG_FILE " does not exist");
  }
}

// **** WiFiManager callbacks 

void saveConfigCallback() {
  shouldSaveConfig = true;
}

void configModeCallback(WiFiManager* myWiFiManager) {
  DPRINTLN("Configuration mode:");
  DPRINT("  SSID: ");
  DPRINTLN(myWiFiManager->getConfigPortalSSID());
  DPRINT("  IP Address: ");
  DPRINTLN(WiFi.softAPIP());
} 
