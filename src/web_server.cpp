#include "web_server.h"
#include "web_ui.h"
#include <Update.h>
#include <AsyncJson.h>

void WebServerManager::begin(RelayController* r, Scheduler* s, NTPManager* n, WiFiManager* w) {
    relay = r;
    scheduler = s;
    ntp = n;
    wifi = w;
    setupRoutes();
    server.begin();
    Serial.println("[HTTP] Web server started on port 80");
}
void WebServerManager::setupRoutes() {
    Serial.println("[HTTP] Registering routes...");

    server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
        Serial.printf("[HTTP] GET / from %s\n", request->client()->remoteIP().toString().c_str());
        request->send(200, "text/html", WEB_UI_HTML);
    });

    server.on("/ping", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "pong");
    });

    server.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest *request) {
        Serial.printf("[HTTP] GET /api/status from %s\n", request->client()->remoteIP().toString().c_str());
        JsonDocument doc;

        doc["time"] = ntp->getTimeString();
        doc["date"] = ntp->getDateString();
        doc["uptime"] = millis() / 1000;
        doc["relay"] = relay->isOn();
        doc["rssi"] = WiFi.RSSI();
        doc["ssid"] = WiFi.SSID();
        doc["next"] = scheduler->getNextWateringString();
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    server.on("/api/relay/on", HTTP_POST, [this](AsyncWebServerRequest *request) {
        uint16_t duration = 60; // По умолчанию 60 сек
        if (request->hasParam("duration")) {
            duration = request->getParam("duration")->value().toInt();
        }
        relay->runFor(duration);
        request->send(200, "application/json", "{\"status\":\"ok\"}");
    });

    server.on("/api/relay/off", HTTP_POST, [this](AsyncWebServerRequest *request) {
        relay->off();
        request->send(200, "application/json", "{\"status\":\"ok\"}");
    });

    server.on("/api/schedule", HTTP_GET, [this](AsyncWebServerRequest *request) {
        Config cfg;
        configStorage.load(cfg);
        JsonDocument doc;
        JsonArray arr = doc.to<JsonArray>();
        for (int i = 0; i < cfg.schedule_count; i++) {
            JsonObject obj = arr.add<JsonObject>();
            obj["h"] = cfg.schedule[i].hour;
            obj["m"] = cfg.schedule[i].minute;
            obj["d"] = cfg.schedule[i].duration_sec;
        }
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    server.on("/api/schedule/reset", HTTP_POST, [this](AsyncWebServerRequest *request) {
        Config cfg;
        // Загружаем дефолт (в configStorage.load зашито заполнение дефолтом если файла нет)
        // Но чтобы сбросить принудительно, нам нужно обнулить текущий и загрузить из констант
        cfg.schedule_count = 0;
        for (int i = 0; i < SCHEDULE_COUNT; i++) {
            cfg.schedule[i] = WATERING_SCHEDULE[i];
            cfg.schedule_count++;
        }
        configStorage.save(cfg);
        scheduler->updateConfig(cfg.schedule, cfg.schedule_count);
        request->send(200, "application/json", "{\"status\":\"ok\"}");
    });

    AsyncCallbackJsonWebHandler* scheduleHandler = new AsyncCallbackJsonWebHandler("/api/schedule", [this](AsyncWebServerRequest *request, JsonVariant &json) {
        JsonArray arr = json.as<JsonArray>();
        Config cfg;
        configStorage.load(cfg);
        cfg.schedule_count = 0;
        for (JsonObject obj : arr) {
            if (cfg.schedule_count >= 32) break; // Увеличили лимит до 32
            cfg.schedule[cfg.schedule_count].hour = obj["h"];
            cfg.schedule[cfg.schedule_count].minute = obj["m"];
            cfg.schedule[cfg.schedule_count].duration_sec = obj["d"];
            cfg.schedule_count++;
        }
        configStorage.save(cfg);
        scheduler->updateConfig(cfg.schedule, cfg.schedule_count);
        request->send(200, "application/json", "{\"status\":\"ok\"}");
    });
    server.addHandler(scheduleHandler);

    server.on("/api/reboot", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200, "application/json", "{\"status\":\"ok\"}");
        delay(500);
        ESP.restart();
    });

    server.on("/api/config", HTTP_GET, [this](AsyncWebServerRequest *request) {
        Config cfg;
        configStorage.load(cfg);
        JsonDocument doc;
        doc["ssid"] = cfg.wifi_ssid;
        doc["pass"] = cfg.wifi_pass;
        doc["tz"] = cfg.timezone_offset;
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    AsyncCallbackJsonWebHandler* configHandler = new AsyncCallbackJsonWebHandler("/api/config", [this](AsyncWebServerRequest *request, JsonVariant &json) {
        JsonObject obj = json.as<JsonObject>();
        Config cfg;
        configStorage.load(cfg);
        cfg.wifi_ssid = obj["ssid"].as<String>();
        cfg.wifi_pass = obj["pass"].as<String>();
        cfg.timezone_offset = obj["tz"].as<int>();
        configStorage.save(cfg);
        request->send(200, "application/json", "{\"status\":\"ok\"}");
        delay(500);
        ESP.restart();
    });
    server.addHandler(configHandler);

    // OTA Upload
    server.on("/ota/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
        delay(500);
        ESP.restart();
    }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
        if (!index) {
            otaManager.begin();
            Serial.printf("Update Start: %s\n", filename.c_str());
            if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
                Update.printError(Serial);
            }
        }
        if (!Update.hasError()) {
            if (Update.write(data, len) != len) {
                Update.printError(Serial);
            }
        }
        if (final) {
            if (Update.end(true)) {
                otaManager.end(true);
                Serial.printf("Update Success: %uB\n", index + len);
            } else {
                Update.printError(Serial);
                otaManager.end(false);
            }
        }
        otaManager.setProgress(index + len, request->contentLength());
    });

    // Captive Portal
    server.onNotFound([this](AsyncWebServerRequest *request) {
        Serial.printf("[HTTP] 404/Captive: %s %s from %s\n", request->methodToString(), request->url().c_str(), request->client()->remoteIP().toString().c_str());
        if (wifi->isAPMode()) {
            request->send(200, "text/html", WEB_UI_HTML);
        } else {
            request->send(404);
        }
    });
}
