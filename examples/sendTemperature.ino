#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <sensorsAPI.h>
#include <ArduinoJson.h>

#include <WiFi.h>

const char* ssid     = "******";
const char* password = "******";

const char* apiName = "https://**************";
const char* apiKey = "**********************";
//indetify your device on GUI:
const char* deviceID = "esp32_Przemek";

TaskHandle_t Task1;
SensorsAPI api(apiName, apiKey, deviceID, &Task1);


//temperature sensor stuff:
// GPIO where the DS18B20 is connected to
const int oneWireBus = 0;
const int conversionTime = 4000;
unsigned long lastTime = 0;

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Number of temperature devices found
int numberOfDevices;

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(115200);

  // Start the DS18B20 sensor
  sensors.begin();

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  if (millis() - lastTime > conversionTime) {
    //We need to delete task inside main thread because this the only way to release memory.
    if (api.taskCreated != 0 && api.isRequestFinished()) {
      api.deleteRequestTask();
    }
    sensors.requestTemperatures();
    float temperatureC = sensors.getTempCByIndex(0);
    float temperatureF = sensors.getTempFByIndex(0);
    Serial.print(temperatureC);
    Serial.println("ºC");
    Serial.print(temperatureF);
    Serial.println("ºF");

    JsonDocument doc;
    doc["tempC"] = temperatureC;
    doc["tempF"] = temperatureF;
    serializeJson(doc, Serial);

    //this command will spawn new thread on core0 and send temperatures.
    api.sendReadings(doc);
    
    lastTime = millis();
  }
  delay(50);
}


