#include "sensorsAPI.h"


SensorsAPI::SensorsAPI(const char* apiURL, const char* apiKey, const char* deviceId, TaskHandle_t task1) {
  //copy deviceId to API url. Stayed with char arrays
  int bufferSize = strlen(apiURL) + strlen(deviceId) + 1;
  char* concatString = new char[ bufferSize ];
  strcpy( concatString, apiURL );
  strcat( concatString, deviceId );

  this->_apiURL = concatString;
  this->_apiKey = apiKey;
  this->_t1 = task1;
  this->_debug = false;
}

void SensorsAPI::enableDebugging() {
  this->_debug = true;
}
void SensorsAPI::debug(String msg) {
  if (this->_debug) {
    String coreID = String(xPortGetCoreID());
    Serial.print("[CORE ");
    Serial.print(coreID);
    Serial.print("] ");
    Serial.println(msg);
  }
}

void SensorsAPI::beginLoop() {
  xTaskCreatePinnedToCore(
    this->_mainLoop, /* Task function. */
    "Task1",   /* name of task. */
    10000,     /* Stack size of task */
    (void *)this,      /* parameter of the task */
    1,         /* priority of the task */
    &_t1,    /* Task handle to keep track of created task */
    0      /* pin task to core 0 */
  );        
}

void SensorsAPI::_mainLoop(void* pvParameter) {
  SensorsAPI* SensorsAPIInstance = (SensorsAPI *) pvParameter;
  SensorsAPIInstance->debug("\nNew API task running on core ");
  SensorsAPIInstance->debug(String(xPortGetCoreID()));

  while (1) {
    if (SensorsAPIInstance->_availableData && SensorsAPIInstance->_requestSent) {
      SensorsAPIInstance->debug("Sending Data...");
      SensorsAPIInstance->debug(SensorsAPIInstance->_req);
      SensorsAPIInstance->_requestSent = false;
      SensorsAPIInstance->_sendAsyncPost();
      SensorsAPIInstance->debug("Finished sending data");
    } else {
      //no data to send
      //SensorsAPIInstance->debug("No data to send...");
    }
    delay(100);
  }
}

void SensorsAPI::_httpSend() {
  HTTPClient http;

  http.begin(this->_apiURL);  	
  http.addHeader("x-api-key", this->_apiKey);

  int startTime = millis();
  this->debug("Starting request...");
  int httpResponseCode = http.POST(this->_req);
  this->debug("Finished in:  ");
  this->debug(String(millis() - startTime));

  this->debug("response code: ");
  this->debug(String(httpResponseCode));
  if(httpResponseCode>0){
    String response = http.getString(); 
    Serial.println(response);           //Print request answer
  }
}

void SensorsAPI::_sendAsyncPost() {
  if(WiFi.status()== WL_CONNECTED) {
    this->_httpSend();
    this->_availableData = false;
    this->_requestSent = true;

  } else {
    if (WiFi.status() != WL_CONNECTED) {
      this->debug("Reconnecting to WiFi...");
      WiFi.disconnect();
      WiFi.reconnect();
      //block thread while wifi is reconencted. Otherwise we will keep calling reconnect.
      while(WiFi.status() != WL_CONNECTED) {
        delay(200);
        this->debug(".");
      }
      this->debug("");
      this->debug("Reconnected to WiFi network with IP Address: ");
      this->debug(WiFi.localIP().toString());

      this->_httpSend();
      this->_availableData = false;
      this->_requestSent = true;
    }
  }
}

int SensorsAPI::sendReadings(JsonDocument &req) {
  if (this->_requestSent) {
    serializeJson(req, _req);
    this->_availableData = true;
  } else {
    this->debug("Data not sent. Request not finished.");
  }
  return 0;
}
