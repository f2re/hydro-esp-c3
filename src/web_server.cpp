#include "web_server.h"
#include "web_ui_v2.h"
#include "version.h"
#include "event_log.h"
#include <Update.h>
#include <AsyncJson.h>
#include <esp_system.h>
#include <math.h>
#include <time.h>

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

void sendJson(AsyncWebServerRequest *request, JsonDocument &doc, int status = 200) {
    String response;
    serializeJson(doc, response);
    request->send(status, "application/json", response);
}

void eventTimestamp(uint32_t localEpoch, char* buffer, size_t size) {
    if (!localEpoch || size == 0) {
        if (size) buffer[0] = '\0';
        return;
    }
    const time_t raw = static_cast<time_t>(localEpoch);
    struct tm value {};
    gmtime_r(&raw, &value);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", &value);
}

void addHydraulicFields(JsonDocument &doc, const Config &cfg) {
    doc["pump_flow_lpm"] = static_cast<float>(cfg.pump_flow_ml_min) / 1000.0f;
    doc["delivery_efficiency_pct"] = cfg.delivery_efficiency_pct;
    doc["hydraulics_calibrated"] = cfg.pump_flow_ml_min > 0;
    doc["calibration_protocol_version"] = cfg.calibration_protocol_version;
    doc["calibration_sample_count"] = cfg.calibration_sample_count;
    doc["calibration_cv_pct"] = static_cast<float>(cfg.calibration_cv_x100) / 100.0f;
    doc["calibration_epoch"] = cfg.calibration_local_epoch;
    if (cfg.calibration_local_epoch) {
        char timestamp[20];
        eventTimestamp(cfg.calibration_local_epoch, timestamp, sizeof(timestamp));
        doc["calibrated_at"] = timestamp;
    }
}
}

void WebServerManager::begin(RelayController* r, Scheduler* s, NTPManager* n,
                             WiFiManager* w, OledDisplay* o) {
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
        Config cfg;
        configStorage.load(cfg);

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
        doc["pump_source"] = EventLog::sourceCode(relay->source());
        doc["ssid"] = wifi->isAPMode() ? AP_SSID : WiFi.SSID();
        if (!wifi->isAPMode() && WiFi.status() == WL_CONNECTED) {
            doc["rssi"] = WiFi.RSSI();
        }
        doc["ip"] = wifi->localIP();
        doc["ap_mode"] = wifi->isAPMode();
        doc["next"] = scheduler->getNextWateringString();
        doc["schedule_count"] = scheduler->count();
        doc["automation_enabled"] = scheduler->isEnabled();
        float sunrise = 6.0f;
        float sunset = 18.0f;
        ntp->getSunriseSunset(cfg.latitude, cfg.longitude, sunrise, sunset);
        doc["sunrise"] = sunrise;
        doc["sunset"] = sunset;
        doc["last_solution_change"] = cfg.last_solution_change;
        doc["now_epoch"] = ntp->getLocalEpoch();
        addHydraulicFields(doc, cfg);
        doc["event_count"] = eventLog.count();
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
        doc["event_log_capacity"] = EVENT_LOG_CAPACITY;
        doc["event_log_session_only"] = true;
        sendJson(request, doc);
    });

    server.on("/api/events", HTTP_GET, [](AsyncWebServerRequest *request) {
        JsonDocument doc;
        doc["session_only"] = true;
        JsonArray arr = doc["events"].to<JsonArray>();
        for (uint8_t i = 0; i < eventLog.count(); ++i) {
            OperationEvent event;
            if (!eventLog.getNewest(i, event)) continue;
            JsonObject item = arr.add<JsonObject>();
            item["sequence"] = event.sequence;
            item["uptime"] = event.uptime_sec;
            item["type"] = EventLog::typeCode(event.type);
            item["source"] = EventLog::sourceCode(event.source);
            item["reason"] = EventLog::stopReasonCode(event.reason);
            item["value"] = event.value;
            if (event.local_epoch) {
                char timestamp[20];
                eventTimestamp(event.local_epoch, timestamp, sizeof(timestamp));
                item["timestamp"] = timestamp;
            }
        }
        sendJson(request, doc);
    });

    server.on("/api/events/clear", HTTP_POST, [](AsyncWebServerRequest *request) {
        eventLog.clear();
        request->send(200, "application/json", "{\"status\":\"ok\"}");
    });

    server.on("/api/relay/on", HTTP_POST, [this](AsyncWebServerRequest *request) {
        long duration = DEFAULT_MANUAL_SECONDS;
        if (request->hasParam("duration")) {
            duration = request->getParam("duration")->value().toInt();
        }
        if (duration < 1) duration = DEFAULT_MANUAL_SECONDS;
        if (duration > MAX_WATERING_SECONDS) duration = MAX_WATERING_SECONDS;

        relay->runFor(static_cast<uint16_t>(duration), PumpSource::WebManual);
        request->send(200, "application/json", "{\"status\":\"ok\"}");
    });

    server.on("/api/relay/off", HTTP_POST, [this](AsyncWebServerRequest *request) {
        relay->off(PumpStopReason::Manual);
        request->send(200, "application/json", "{\"status\":\"ok\"}");
    });

    server.on("/api/calibration/start", HTTP_POST, [this](AsyncWebServerRequest *request) {
        if (scheduler->isEnabled()) {
            request->send(409, "application/json", "{\"error\":\"automation_must_be_paused\"}");
            return;
        }
        if (relay->isOn()) {
            request->send(409, "application/json", "{\"error\":\"pump_busy\"}");
            return;
        }

        long duration = 30;
        if (request->hasParam("duration")) {
            duration = request->getParam("duration")->value().toInt();
        }
        if (duration < 5 || duration > 120) {
            request->send(400, "application/json", "{\"error\":\"calibration_duration_5_120\"}");
            return;
        }

        relay->runFor(static_cast<uint16_t>(duration), PumpSource::Calibration);
        request->send(200, "application/json", "{\"status\":\"ok\"}");
    });

    server.on("/api/automation", HTTP_GET, [this](AsyncWebServerRequest *request) {
        JsonDocument doc;
        doc["enabled"] = scheduler->isEnabled();
        sendJson(request, doc);
    });

    AsyncCallbackJsonWebHandler* automationHandler = new AsyncCallbackJsonWebHandler(
        "/api/automation",
        [this](AsyncWebServerRequest *request, JsonVariant &json) {
            if (!json.is<JsonObject>()) {
                request->send(400, "application/json", "{\"error\":\"automation_must_be_object\"}");
                return;
            }
            JsonObject obj = json.as<JsonObject>();
            if (!obj["enabled"].is<bool>()) {
                request->send(400, "application/json", "{\"error\":\"enabled_must_be_boolean\"}");
                return;
            }

            const bool enabled = obj["enabled"].as<bool>();
            const bool changed = enabled != scheduler->isEnabled();

            if (!enabled && relay->isOn() && relay->source() == PumpSource::Schedule) {
                relay->off(PumpStopReason::AutomationPaused);
            }
            scheduler->setEnabled(enabled);

            Config cfg;
            configStorage.load(cfg);
            cfg.automation_enabled = enabled;
            configStorage.save(cfg);

            if (changed) {
                eventLog.record(enabled ? EventType::AutomationEnabled : EventType::AutomationPaused);
            }

            JsonDocument doc;
            doc["status"] = "ok";
            doc["enabled"] = enabled;
            sendJson(request, doc);
        }
    );
    server.addHandler(automationHandler);

    server.on("/api/hydraulics", HTTP_GET, [](AsyncWebServerRequest *request) {
        Config cfg;
        configStorage.load(cfg);
        JsonDocument doc;
        doc["flow_lpm"] = static_cast<float>(cfg.pump_flow_ml_min) / 1000.0f;
        doc["efficiency_pct"] = cfg.delivery_efficiency_pct;
        doc["calibrated"] = cfg.pump_flow_ml_min > 0;
        doc["protocol_version"] = cfg.calibration_protocol_version;
        doc["sample_count"] = cfg.calibration_sample_count;
        doc["cv_pct"] = static_cast<float>(cfg.calibration_cv_x100) / 100.0f;
        doc["calibration_epoch"] = cfg.calibration_local_epoch;
        if (cfg.calibration_local_epoch) {
            char timestamp[20];
            eventTimestamp(cfg.calibration_local_epoch, timestamp, sizeof(timestamp));
            doc["calibrated_at"] = timestamp;
        }
        sendJson(request, doc);
    });

    AsyncCallbackJsonWebHandler* hydraulicsHandler = new AsyncCallbackJsonWebHandler(
        "/api/hydraulics",
        [this](AsyncWebServerRequest *request, JsonVariant &json) {
            if (!json.is<JsonObject>()) {
                request->send(400, "application/json", "{\"error\":\"hydraulics_must_be_object\"}");
                return;
            }
            JsonObject obj = json.as<JsonObject>();
            const float flow = obj["flow_lpm"] | 0.0f;
            const int efficiency = obj["efficiency_pct"] | 85;

            if (!isfinite(flow) || !((flow == 0.0f) || (flow >= 0.05f && flow <= 100.0f))) {
                request->send(400, "application/json", "{\"error\":\"flow_lpm_must_be_0_or_0_05_100\"}");
                return;
            }
            if (efficiency < 10 || efficiency > 100) {
                request->send(400, "application/json", "{\"error\":\"efficiency_pct_10_100\"}");
                return;
            }

            const bool hasSamples = !obj["sample_count"].isNull();
            int sampleCount = 0;
            float cvPct = 0.0f;
            int protocolVersion = HYDRO_CALIBRATION_PROTOCOL_VERSION;
            if (hasSamples) {
                sampleCount = obj["sample_count"].as<int>();
                cvPct = obj["cv_pct"] | 0.0f;
                if (!obj["protocol_version"].isNull()) {
                    protocolVersion = obj["protocol_version"].as<int>();
                }
                if (sampleCount < 0 || sampleCount > 9 ||
                    !isfinite(cvPct) || cvPct < 0.0f || cvPct > 500.0f ||
                    protocolVersion < 0 || protocolVersion > HYDRO_CALIBRATION_PROTOCOL_VERSION) {
                    request->send(400, "application/json", "{\"error\":\"invalid_calibration_quality\"}");
                    return;
                }
            }

            Config cfg;
            configStorage.load(cfg);
            cfg.pump_flow_ml_min = static_cast<uint32_t>(lroundf(flow * 1000.0f));
            cfg.delivery_efficiency_pct = static_cast<uint8_t>(efficiency);

            if (flow == 0.0f) {
                cfg.calibration_protocol_version = 0;
                cfg.calibration_sample_count = 0;
                cfg.calibration_cv_x100 = 0;
                cfg.calibration_local_epoch = 0;
            } else if (hasSamples) {
                cfg.calibration_protocol_version = static_cast<uint8_t>(protocolVersion);
                cfg.calibration_sample_count = static_cast<uint8_t>(sampleCount);
                cfg.calibration_cv_x100 = static_cast<uint16_t>(lroundf(cvPct * 100.0f));
                const uint32_t suppliedEpoch = obj["calibration_epoch"] | 0UL;
                cfg.calibration_local_epoch = suppliedEpoch
                    ? suppliedEpoch
                    : (ntp->isSynced() ? ntp->getLocalEpoch() : 0);
            }

            configStorage.save(cfg);
            eventLog.record(EventType::HydraulicsSaved, PumpSource::None,
                            PumpStopReason::None,
                            static_cast<int32_t>(cfg.pump_flow_ml_min));

            JsonDocument doc;
            doc["status"] = "ok";
            doc["flow_lpm"] = static_cast<float>(cfg.pump_flow_ml_min) / 1000.0f;
            doc["efficiency_pct"] = cfg.delivery_efficiency_pct;
            doc["calibrated"] = cfg.pump_flow_ml_min > 0;
            doc["protocol_version"] = cfg.calibration_protocol_version;
            doc["sample_count"] = cfg.calibration_sample_count;
            doc["cv_pct"] = static_cast<float>(cfg.calibration_cv_x100) / 100.0f;
            doc["calibration_epoch"] = cfg.calibration_local_epoch;
            if (cfg.calibration_local_epoch) {
                char timestamp[20];
                eventTimestamp(cfg.calibration_local_epoch, timestamp, sizeof(timestamp));
                doc["calibrated_at"] = timestamp;
            }
            sendJson(request, doc);
        }
    );
    server.addHandler(hydraulicsHandler);

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
        eventLog.record(EventType::ScheduleChanged, PumpSource::None,
                        PumpStopReason::None, cfg.schedule_count);
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
            eventLog.record(EventType::ScheduleChanged, PumpSource::None,
                            PumpStopReason::None, count);
            request->send(200, "application/json", "{\"status\":\"ok\"}");
        }
    );
    server.addHandler(scheduleHandler);

    server.on("/api/reboot", HTTP_POST, [this](AsyncWebServerRequest *request) {
        eventLog.record(EventType::RebootRequested);
        relay->off(PumpStopReason::Reboot);
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
        doc["lat"] = cfg.latitude;
        doc["lon"] = cfg.longitude;
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
            Config cfg;
            configStorage.load(cfg);
            String ssid = obj["ssid"].as<String>();
            const String pass = obj["pass"].as<String>();
            const int tz = obj["tz"].as<int>();
            const float latitude = obj["lat"].isNull()
                ? cfg.latitude
                : obj["lat"].as<float>();
            const float longitude = obj["lon"].isNull()
                ? cfg.longitude
                : obj["lon"].as<float>();
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
            if (latitude < -90.0f || latitude > 90.0f ||
                longitude < -180.0f || longitude > 180.0f) {
                request->send(400, "application/json", "{\"error\":\"invalid_coordinates\"}");
                return;
            }

            cfg.wifi_ssid = ssid;
            if (pass.length() > 0) cfg.wifi_pass = pass;
            cfg.timezone_offset = tz;
            cfg.latitude = latitude;
            cfg.longitude = longitude;
            configStorage.save(cfg);
            eventLog.record(EventType::ConfigChanged);

            relay->off(PumpStopReason::Reboot);
            request->send(200, "application/json", "{\"status\":\"ok\",\"rebooting\":true}");
            delay(350);
            ESP.restart();
        }
    );
    server.addHandler(configHandler);

    server.on("/api/solution/reset", HTTP_POST, [this](AsyncWebServerRequest *request) {
        if (!ntp->isSynced()) {
            request->send(409, "application/json", "{\"error\":\"time_not_synced\"}");
            return;
        }
        Config cfg;
        configStorage.load(cfg);
        cfg.last_solution_change = ntp->getLocalEpoch();
        configStorage.save(cfg);
        request->send(200, "application/json", "{\"status\":\"ok\"}");
    });

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
            eventLog.record(EventType::OtaStarted);
            relay->off(PumpStopReason::Ota);
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
                if (oled) oled->drawOTA(100);
                Serial.printf("[OTA] Success: %u bytes\n", static_cast<unsigned>(index + len));
            } else {
                Update.printError(Serial);
                otaManager.end(false);
            }
        }
        otaManager.setProgress(index + len, request->contentLength());
        if (oled && request->contentLength() > 0) {
            const uint8_t progress = static_cast<uint8_t>(
                ((index + len) * 100U) / request->contentLength());
            oled->drawOTA(progress);
        }
    });

    server.onNotFound([this](AsyncWebServerRequest *request) {
        if (wifi->isAPMode()) {
            request->send(200, "text/html; charset=utf-8", WEB_UI_HTML);
        } else {
            request->send(404);
        }
    });
}
