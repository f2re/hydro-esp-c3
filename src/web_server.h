#pragma once
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "relay_controller.h"
#include "scheduler.h"
#include "ntp_manager.h"
#include "wifi_manager.h"
#include "config_storage.h"
#include "ota_manager.h"

class WebServerManager {
public:
    WebServerManager() : server(80) {}
    void begin(RelayController* relay, Scheduler* scheduler, NTPManager* ntp, WiFiManager* wifi);
    
private:
    AsyncWebServer server;
    RelayController* relay;
    Scheduler* scheduler;
    NTPManager* ntp;
    WiFiManager* wifi;

    void setupRoutes();
};
