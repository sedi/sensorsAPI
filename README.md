SensorsAPI Library

Simple library for esp32 boards which calls HTTP POST to send sensors measurements to cloud API.

It is using CORE_0 for sending requests, so main core becomes onblocked. There is required special code in main loop() which kills task and frees memory after query is made.







TODO:
* Implement non blocking requests
* Stop using freeRTOS tasks to second core will not be occupied.