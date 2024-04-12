#ifndef SENSORS_API_H
#define SENSORS_API_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>

class SensorsAPI {
  public:
    SensorsAPI(const char* apiURL, const char* apiKey, const char* deviceId, TaskHandle_t task1 );
    int sendReading(float tempC);
    int sendReadings(JsonDocument &req);
    static void _sendAsyncPost(void* pvParameter);
    bool isRequestFinished();
    void deleteRequestTask();
    int taskCreated;
    void enableDebugging();
  private:
    void debug(String msg);
    TaskHandle_t _t1;
    const char* _apiURL;
    const char* _apiKey;
    String _req;
    bool _requestSent;
    bool _debug;
};

#endif
