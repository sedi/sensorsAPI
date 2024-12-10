#ifndef SENSORS_API_H
#define SENSORS_API_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>

class SensorsAPI {
  public:
    SensorsAPI(const char* apiURL, const char* apiKey, const char* deviceId, TaskHandle_t task1 );
    int sendReadings(JsonDocument &req);
    
    int taskCreated;
    void enableDebugging();
    void beginLoop();
    void _httpSend();
  private:
    void debug(String msg);
    TaskHandle_t _t1;
    const char* _apiURL;
    const char* _apiKey;
    String _req;
    bool _requestSent = true;
    bool _debug;
    static void _mainLoop(void* pvParameter);
    bool _availableData = false;
    void _sendAsyncPost();
};

#endif
