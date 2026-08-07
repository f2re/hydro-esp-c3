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
        request->send(200, "text/html; charset=utf-8", WEB_UI_HTML);
    });

    server.on("/ping", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "pong");
    });

    server.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest *request) {
        JsonDocument doc;
        doc["time"] = ntp->getTimeString();
        doc["date"] = ntp->getDateString();
        doc["uptime"] = millis() / 1000;
        doc["relay"] = relay->isOn();
        doc["relay_remaining"] = relay->remainingSec();
        doc["relay_progress"] = relay->progress();
        doc["rssi"] = WiFi.RSSI();
        doc["ssid"] = WiFi.SSID();
        doc["ip"] = wifi->localIP();
        doc["ap_mode"] = wifi->isAPMode();
        doc["next"] = scheduler->getNextWateringString();

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    server.on("/api/relay/on", HTTP_POST, [this](AsyncWebServerRequest *request) {
        long duration = 60;
        if (request->hasParam("duration")) {
            duration = request->getParam("duration")->value().toInt();
        }
        if (duration < 1) duration = 60;
        if (duration > 3600) duration = 3600;

        relay->runFor((uint16_t)duration);
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
        for (uint8_t i = 0; i < cfg.schedule_count; i++) {
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
        configStorage.load(cfg);
        cfg.schedule_count = 0;
        for (uint8_t i = 0; i < SCHEDULE_COUNT && i < 48; i++) {
            cfg.schedule[cfg.schedule_count++] = WATERING_SCHEDULE[i];
        }
        configStorage.save(cfg);
        scheduler->updateConfig(cfg.schedule, cfg.schedule_count);
        request->send(200, "application/json", "{\"status\":\"ok\"}");
    });

    AsyncCallbackJsonWebHandler* scheduleHandler = new AsyncCallbackJsonWebHandler(
        "/api/schedule",
        [this](AsyncWebServerRequest *request, JsonVariant &json) {
            if (!json.is<JsonArray>()) {
                request->send(400, "application/json", "{\"error\":\"schedule_must_be_array\"}");
                return;
            }

            JsonArray arr = json.as<JsonArray>();
            if (arr.size() > 48) {
                request->send(400, "application/json", "{\"error\":\"too_many_slots\",\"max\":48}");
                return;
            }

            WateringSlot parsed[48];
            uint8_t count = 0;

            for (JsonObject obj : arr) {
                int h = obj["h"].as<int>();
                int m = obj["m"].as<int>();
                int d = obj["d"].as<int>();

                if (h < 0 || h > 23 || m < 0 || m > 59 || d < 1 || d > 3600) {
                    request->send(400, "application/json", "{\"error\":\"invalid_slot\"}");
                    return;
                }

                for (uint8_t i = 0; i < count; i++) {
                    if (parsed[i].hour == h && parsed[i].minute == m) {
                        request->send(400, "application/json", "{\"error\":\"duplicate_time\"}");
                        return;
                    }
                }

                parsed[count].hour = (uint8_t)h;
                parsed[count].minute = (uint8_t)m;
                parsed[count].duration_sec = (uint16_t)d;
                count++;
            }

            Config cfg;
            configStorage.load(cfg);
            cfg.schedule_count = count;
            for (uint8_t i = 0; i < count; i++) {
                cfg.schedule[i] = parsed[i];
            }

            configStorage.save(cfg);
            scheduler->updateConfig(cfg.schedule, cfg.schedule_count);
            request->send(200, "application/json", "{\"status\":\"ok\"}");
        }
    );
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
        doc["has_pass"] = cfg.wifi_pass.length() > 0;
        doc["tz"] = cfg.timezone_offset;

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    AsyncCallbackJsonWebHandler* configHandler = new AsyncCallbackJsonWebHandler(
        "/api/config",
        [this](AsyncWebServerRequest *request, JsonVariant &json) {
            if (!json.is<JsonObject>()) {
                request->send(400, "application/json", "{\"error\":\"config_must_be_object\"}");
                return;
            }

            JsonObject obj = json.as<JsonObject>();
            String ssid = obj["ssid"].as<String>();
            String pass = obj["pass"].as<String>();
            int tz = obj["tz"].as<int>();

            ssid.trim();
            if (ssid.length() < 1 || ssid.length() > 32) {
                request->send(400, "application/json", "{\"error\":\"invalid_ssid\"}");
                return;
            }
            if (pass.length() > 63) {
                request->send(400, "application/json", "{\"error\":\"invalid_password\"}");
                return;
            }
            if (tz < -12 || tz > 14) {
                request->send(400, "application/json", "{\"error\":\"invalid_timezone\"}");
                return;
            }

            Config cfg;
            configStorage.load(cfg);
            cfg.wifi_ssid = ssid;
            if (pass.length() > 0) {
                cfg.wifi_pass = pass;
            }
            cfg.timezone_offset = tz;
            configStorage.save(cfg);

            request->send(200, "application/json", "{\"status\":\"ok\",\"rebooting\":true}");
            delay(500);
            ESP.restart();
        }
    );
    server.addHandler(configHandler);

    // OTA upload. NOTE: transport is local HTTP; authentication/signature validation
    // is intentionally left as a separate hardening task to avoid locking existing users out.
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

    server.onNotFound([this](AsyncWebServerRequest *request) {
        Serial.printf("[HTTP] 404/Captive: %s %s from %s\n",
            request->methodToString(), request->url().c_str(), request->client()->remoteIP().toString().c_str());
        if (wifi->isAPMode()) {
            request->send(200, "text/html; charset=utf-8", WEB_UI_HTML);
        } else {
            request->send(404);
        }
    });
}
