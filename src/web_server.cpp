#include "web_server.h"
#include "web_ui.h"
#include <Update.h>
#include <AsyncJson.h>

void WebServerManager::begin(RelayController* r, Scheduler* s, NTPManager* n, WiFiManager* w, OledDisplay* o) {
    relay = r;
    scheduler = s;
    ntp = n;
    wifi = w;
    oled = o;
    setupRoutes();
    server.begin();
    Serial.println("[HTTP] Web server started on port 80");
}
void WebServerManager::setupRoutes() {
    Serial.println("[HTTP] Registering routes...");

    server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
        Serial.printf("[HTTP] GET / from %s\n", request->client()->remoteIP().toString().c_str());
        AsyncResponseStream *response = request->beginResponseStream("text/html");
        response->print("<!DOCTYPE html><html lang=\"ru\"><head><meta charset=\"UTF-8\">");
        response->print("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no\">");
        response->print("<title>HydroESP-C3</title>");
        response->print(WEB_UI_CSS);
        response->print("</head>");
        response->print(WEB_UI_BODY);
        response->print(WEB_UI_JS);
        request->send(response);
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
        
        Config cfg;
        configStorage.load(cfg);
        float sunrise, sunset;
        ntp->getSunriseSunset(cfg.latitude, cfg.longitude, sunrise, sunset);
        
        char buf[10];
        snprintf(buf, 10, "%02d:%02d", (int)sunrise, (int)((sunrise - (int)sunrise) * 60));
        doc["sunrise"] = String(buf);
        snprintf(buf, 10, "%02d:%02d", (int)sunset, (int)((sunset - (int)sunset) * 60));
        doc["sunset"] = String(buf);
        doc["last_sol"] = cfg.last_solution_change;
        doc["now_epoch"] = ntp->getEpochTime();
        
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
        doc["lat"] = cfg.latitude;
        doc["lon"] = cfg.longitude;
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
        cfg.latitude = obj["lat"].as<float>();
        cfg.longitude = obj["lon"].as<float>();
        configStorage.save(cfg);
        request->send(200, "application/json", "{\"status\":\"ok\"}");
    });
    server.addHandler(configHandler);

    server.on("/api/solution/reset", HTTP_POST, [this](AsyncWebServerRequest *request) {
        Config cfg;
        configStorage.load(cfg);
        cfg.last_solution_change = ntp->getEpochTime();
        configStorage.save(cfg);
        request->send(200, "application/json", "{\"status\":\"ok\"}");
    });

    // OTA Upload
    server.on("/ota/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
        delay(500);
        ESP.restart();
    }, [this](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
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
                oled->drawOTA(100);
                Serial.printf("Update Success: %uB\n", index + len);
            } else {
                Update.printError(Serial);
                otaManager.end(false);
            }
        }
        int progress = 0;
        if (request->contentLength() > 0) {
            progress = (index + len) * 100 / request->contentLength();
        }
        otaManager.setProgress(index + len, request->contentLength());
        oled->drawOTA(progress);
    });

    // Captive Portal
    server.onNotFound([this](AsyncWebServerRequest *request) {
        Serial.printf("[HTTP] 404/Captive: %s %s from %s\n", request->methodToString(), request->url().c_str(), request->client()->remoteIP().toString().c_str());
        if (wifi->isAPMode()) {
            AsyncResponseStream *response = request->beginResponseStream("text/html");
            response->print("<!DOCTYPE html><html lang=\"ru\"><head><meta charset=\"UTF-8\">");
            response->print("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no\">");
            response->print("<title>HydroESP-C3</title>");
            response->print(WEB_UI_CSS);
            response->print("</head>");
            response->print(WEB_UI_BODY);
            response->print(WEB_UI_JS);
            request->send(response);
        } else {
            request->send(404);
        }
    });
}
