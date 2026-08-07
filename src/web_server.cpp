#include "web_server.h"
#include "web_ui_v2.h"
#include "version.h"
#include <Update.h>
#include <AsyncJson.h>
#include <esp_system.h>

namespace {
const char* resetReasonText(esp_reset_reason_t reason) {
    switch (reason) {
        case ESP_RST_POWERON: return "питание включено";
        case ESP_RST_EXT: return "внешний сброс";
        case ESP_RST_SW: return "программный сброс";
        case ESP_RST_PANIC: return "panic / исключение";
        case ESP_RST_INT_WDT: return "watchdog прерывания";
        case ESP_RST_TASK_WDT: return "watchdog задачи";
        case ESP_RST_WDT: return "другой watchdog";
        case ESP_RST_DEEPSLEEP: return "выход из deep sleep";
        case ESP_RST_BROWNOUT: return "просадка питания";
        case ESP_RST_SDIO: return "SDIO reset";
        default: return "неизвестно";
    }
}

void sendJson(AsyncWebServerRequest *request, JsonDocument &doc) {
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}
}

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
        request->send(200, "text/html; charset=utf-8", WEB_UI_HTML);
    });

    server.on("/manifest.webmanifest", HTTP_GET, [](AsyncWebServerRequest *request) {
        static const char manifest[] PROGMEM = R"json({
  "name":"HydroESP-C3",
  "short_name":"HydroESP",
  "start_url":"/",
  "scope":"/",
  "display":"standalone",
  "background_color":"#0a0f13",
  "theme_color":"#0a0f13",
  "description":"Локальное управление гидропонной установкой"
})json";
        request->send(200, "application/manifest+json; charset=utf-8", manifest);
    });

    server.on("/ping", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "pong");
    });

    server.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest *request) {
        JsonDocument doc;
        doc["api_version"] = HYDRO_API_VERSION;
        doc["version"] = HYDRO_VERSION;
        doc["build"] = HYDRO_BUILD_SHA;
        doc["time"] = ntp->getTimeString();
        doc["date"] = ntp->getDateString();
        doc["time_synced"] = ntp->isSynced();
        doc["uptime"] = millis() / 1000;
        doc["relay"] = relay->isOn();
        doc["relay_remaining"] = relay->remainingSec();
        doc["relay_progress"] = relay->progress();
        doc["ssid"] = wifi->isAPMode() ? AP_SSID : WiFi.SSID();
        if (!wifi->isAPMode() && WiFi.status() == WL_CONNECTED) {
            doc["rssi"] = WiFi.RSSI();
        }
        doc["ip"] = wifi->localIP();
        doc["ap_mode"] = wifi->isAPMode();
        doc["next"] = scheduler->getNextWateringString();
        doc["schedule_count"] = scheduler->count();
        sendJson(request, doc);
    });

    server.on("/api/diagnostics", HTTP_GET, [](AsyncWebServerRequest *request) {
        const esp_reset_reason_t reason = esp_reset_reason();
        JsonDocument doc;
        doc["api_version"] = HYDRO_API_VERSION;
        doc["version"] = HYDRO_VERSION;
        doc["build"] = HYDRO_BUILD_SHA;
        doc["free_heap"] = ESP.getFreeHeap();
        doc["min_free_heap"] = ESP.getMinFreeHeap();
        doc["flash_size"] = ESP.getFlashChipSize();
        doc["sketch_size"] = ESP.getSketchSize();
        doc["free_sketch_space"] = ESP.getFreeSketchSpace();
        doc["reset_reason"] = static_cast<int>(reason);
        doc["reset_reason_text"] = resetReasonText(reason);
        sendJson(request, doc);
    });

    server.on("/api/relay/on", HTTP_POST, [this](AsyncWebServerRequest *request) {
        long duration = DEFAULT_MANUAL_SECONDS;
        if (request->hasParam("duration")) {
            duration = request->getParam("duration")->value().toInt();
        }
        if (duration < 1) duration = DEFAULT_MANUAL_SECONDS;
        if (duration > MAX_WATERING_SECONDS) duration = MAX_WATERING_SECONDS;

        relay->runFor(static_cast<uint16_t>(duration));
        request->send(200, "application/json", "{\"status\":\"ok\"}");
    });

    server.on("/api/relay/off", HTTP_POST, [this](AsyncWebServerRequest *request) {
        relay->off();
        request->send(200, "application/json", "{\"status\":\"ok\"}");
    });

    server.on("/api/schedule", HTTP_GET, [](AsyncWebServerRequest *request) {
        Config cfg;
        configStorage.load(cfg);
        JsonDocument doc;
        JsonArray arr = doc.to<JsonArray>();
        for (uint8_t i = 0; i < cfg.schedule_count; ++i) {
            JsonObject obj = arr.add<JsonObject>();
            obj["h"] = cfg.schedule[i].hour;
            obj["m"] = cfg.schedule[i].minute;
            obj["d"] = cfg.schedule[i].duration_sec;
        }
        sendJson(request, doc);
    });

    server.on("/api/schedule/reset", HTTP_POST, [this](AsyncWebServerRequest *request) {
        Config cfg;
        configStorage.load(cfg);
        cfg.schedule_count = 0;
        for (uint8_t i = 0; i < SCHEDULE_COUNT && i < MAX_SCHEDULE_SLOTS; ++i) {
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
            if (arr.size() > MAX_SCHEDULE_SLOTS) {
                request->send(400, "application/json", "{\"error\":\"too_many_slots\"}");
                return;
            }

            WateringSlot parsed[MAX_SCHEDULE_SLOTS]{};
            uint8_t count = 0;
            for (JsonObject obj : arr) {
                const int h = obj["h"].as<int>();
                const int m = obj["m"].as<int>();
                const int d = obj["d"].as<int>();
                if (h < 0 || h > 23 || m < 0 || m > 59 || d < 1 || d > MAX_WATERING_SECONDS) {
                    request->send(400, "application/json", "{\"error\":\"invalid_slot\"}");
                    return;
                }
                for (uint8_t i = 0; i < count; ++i) {
                    if (parsed[i].hour == h && parsed[i].minute == m) {
                        request->send(400, "application/json", "{\"error\":\"duplicate_time\"}");
                        return;
                    }
                }
                parsed[count++] = {
                    static_cast<uint8_t>(h),
                    static_cast<uint8_t>(m),
                    static_cast<uint16_t>(d)
                };
            }

            Config cfg;
            configStorage.load(cfg);
            cfg.schedule_count = count;
            for (uint8_t i = 0; i < count; ++i) cfg.schedule[i] = parsed[i];
            configStorage.save(cfg);
            scheduler->updateConfig(cfg.schedule, cfg.schedule_count);
            request->send(200, "application/json", "{\"status\":\"ok\"}");
        }
    );
    server.addHandler(scheduleHandler);

    server.on("/api/reboot", HTTP_POST, [this](AsyncWebServerRequest *request) {
        relay->off();
        request->send(200, "application/json", "{\"status\":\"ok\"}");
        delay(350);
        ESP.restart();
    });

    server.on("/api/config", HTTP_GET, [](AsyncWebServerRequest *request) {
        Config cfg;
        configStorage.load(cfg);
        JsonDocument doc;
        doc["ssid"] = cfg.wifi_ssid;
        doc["has_pass"] = cfg.wifi_pass.length() > 0;
        doc["tz"] = cfg.timezone_offset;
        sendJson(request, doc);
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
            const String pass = obj["pass"].as<String>();
            const int tz = obj["tz"].as<int>();
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
            if (pass.length() > 0) cfg.wifi_pass = pass;
            cfg.timezone_offset = tz;
            configStorage.save(cfg);

            relay->off();
            request->send(200, "application/json", "{\"status\":\"ok\",\"rebooting\":true}");
            delay(350);
            ESP.restart();
        }
    );
    server.addHandler(configHandler);

    // Local OTA transport. Authentication + signed-image verification is a
    // separate security milestone; do not expose this HTTP service to WAN.
    server.on("/ota/upload", HTTP_POST, [this](AsyncWebServerRequest *request) {
        const bool failed = Update.hasError();
        request->send(failed ? 500 : 200, "text/plain", failed ? "FAIL" : "OK");
        if (!failed) {
            delay(450);
            ESP.restart();
        }
    }, [this](AsyncWebServerRequest *request, String filename, size_t index,
              uint8_t *data, size_t len, bool final) {
        if (!index) {
            relay->off();
            otaManager.begin();
            Serial.printf("[OTA] Start: %s\n", filename.c_str());
            if (!Update.begin(UPDATE_SIZE_UNKNOWN)) Update.printError(Serial);
        }
        if (!Update.hasError() && Update.write(data, len) != len) {
            Update.printError(Serial);
        }
        if (final) {
            if (Update.end(true)) {
                otaManager.end(true);
                Serial.printf("[OTA] Success: %u bytes\n", static_cast<unsigned>(index + len));
            } else {
                Update.printError(Serial);
                otaManager.end(false);
            }
        }
        otaManager.setProgress(index + len, request->contentLength());
    });

    server.onNotFound([this](AsyncWebServerRequest *request) {
        if (wifi->isAPMode()) {
            request->send(200, "text/html; charset=utf-8", WEB_UI_HTML);
        } else {
            request->send(404);
        }
    });
}
