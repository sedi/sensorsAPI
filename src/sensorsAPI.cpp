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
  this->taskCreated = 0;
  this->_debug = false;
}

void SensorsAPI::enableDebugging() {
  this->_debug = true;
}
void SensorsAPI::debug(String msg) {
  if (this->_debug) {
    Serial.println(msg);
  }
}

bool SensorsAPI::isRequestFinished() {
  return this->_requestSent;
}

void SensorsAPI::deleteRequestTask() {
  vTaskDelete(this->_t1);
  this->_requestSent = false;
  this->taskCreated = 0;
  this->debug("Deleted send POST task");
}

void SensorsAPI::_sendAsyncPost(void* pvParameter) {
  SensorsAPI* SensorsAPIInstance = (SensorsAPI *) pvParameter;
  TaskHandle_t xHandle = NULL;
  SensorsAPIInstance->debug("starting _sendAsyncPost running on core ");
  SensorsAPIInstance->debug(String(xPortGetCoreID()));
  //SensorsAPIInstance->_requestSent = false;
  while (1) {
    if(WiFi.status()== WL_CONNECTED && !SensorsAPIInstance->_requestSent) {
      HTTPClient http;

      http.begin(SensorsAPIInstance->_apiURL);  	
      http.addHeader("x-api-key", SensorsAPIInstance->_apiKey);

      int startTime = millis();
      SensorsAPIInstance->debug("Starting request...");
      int httpResponseCode = http.POST(SensorsAPIInstance->_req);
      SensorsAPIInstance->debug("Finished in:  ");
      SensorsAPIInstance->debug(String(millis() - startTime));

      SensorsAPIInstance->debug("response code: ");
      SensorsAPIInstance->debug(String(httpResponseCode));
      if(httpResponseCode>0){
        String response = http.getString(); 
        Serial.println(response);           //Print request answer
      }
     SensorsAPIInstance->_requestSent = true;
    } else {
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println(WiFi.status());

        Serial.println("Reconnecting to WiFi...");
        WiFi.disconnect();
        WiFi.reconnect();
        //block thread while wifi is reconencted. Otherwise we will keep calling reconnect.
        while(WiFi.status() != WL_CONNECTED) {
          delay(200);
          Serial.print(".");
        }
        Serial.println("");
        Serial.print("Connected to WiFi network with IP Address: ");
        Serial.println(WiFi.localIP());
      }
    }
    //wait for the kill. Can't kill task here from here because memory will be not freed.
    delay(500);
  }
}

int SensorsAPI::sendReadings(JsonDocument &req) {
  this->debug("\nMain task running on core ");
  this->debug(String(xPortGetCoreID()));

  //delete old task if exists
  //if (this->taskCreated != 0) {
    this->deleteRequestTask();
  //}
  this->debug("Creating New task");
  String requestBody;
  serializeJson(req, requestBody);
  _req = requestBody;

  taskCreated = millis();
  xTaskCreatePinnedToCore(
    this->_sendAsyncPost, /* Task function. */
    "Task1",   /* name of task. */
    30000,     /* Stack size of task */
    (void *)this,      /* parameter of the task */
    1,         /* priority of the task */
    &_t1,    /* Task handle to keep track of created task */
    0
  );        /* pin task to core 0 */
  
  this->debug("Created new task");
  
  return 0;
}

//LEGACY METHOD. First version of method with manual JSON building. Created for tests with single value. Better to use sendReadings instead(). This one is blocking CPU
int SensorsAPI::sendReading(float tempC) {
  if(WiFi.status()== WL_CONNECTED) {
    HTTPClient http;

    http.begin(_apiURL);  	
    http.addHeader("x-api-key", _apiKey);

    String requestBody1, requestBody2, requestBody3;
    requestBody1 = String("{\"temp1\": ");
    requestBody2 = String();
    requestBody3 = String();
    requestBody2 = requestBody1 + tempC;
    requestBody3 = requestBody2 + "}";
    int httpResponseCode = http.POST(requestBody3);

    this->debug("response code: ");
    this->debug(String(httpResponseCode));
    if(httpResponseCode>0){
      String response = http.getString(); 
      this->debug(response);
    }

    return httpResponseCode;

  } else {
    this->debug("WiFi Disconnected");
  }
  return 0;
}
