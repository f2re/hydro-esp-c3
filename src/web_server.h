#pragma once
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "relay_controller.h"
#include "scheduler.h"
#include "ntp_manager.h"
#include "wifi_manager.h"
#include "config_storage.h"
#include "ota_manager.h"
#include "oled_display.h"
#include "web_assets.h"

class WebServerManager {
public:
    WebServerManager() : server(80) {
        setupIdentityRoutes();
    }
    void begin(RelayController* relay, Scheduler* scheduler, NTPManager* ntp, WiFiManager* wifi, OledDisplay* oled);
    
private:
    AsyncWebServer server;
    RelayController* relay;
    Scheduler* scheduler;
    NTPManager* ntp;
    WiFiManager* wifi;
    OledDisplay* oled;

    static void sendCachedFlashAsset(AsyncWebServerRequest* request,
                                     const char* contentType,
                                     const uint8_t* data,
                                     size_t length) {
        AsyncWebServerResponse* response = request->beginResponse(
            200, contentType, data, length
        );
        response->addHeader("Cache-Control", "public, max-age=604800");
        request->send(response);
    }

    void setupIdentityRoutes() {
        server.on("/favicon.svg", HTTP_GET, [](AsyncWebServerRequest* request) {
            sendCachedFlashAsset(
                request,
                "image/svg+xml",
                reinterpret_cast<const uint8_t*>(HYDRO_FAVICON_SVG),
                sizeof(HYDRO_FAVICON_SVG) - 1
            );
        });

        // Browsers still probe /favicon.ico automatically even when no explicit
        // <link rel="icon"> is present. Keep a real compact ICO for that path.
        server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest* request) {
            sendCachedFlashAsset(
                request,
                "image/x-icon",
                HYDRO_FAVICON_ICO,
                sizeof(HYDRO_FAVICON_ICO)
            );
        });
    }

    void setupRoutes();
};
