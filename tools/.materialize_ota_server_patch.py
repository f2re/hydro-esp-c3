#!/usr/bin/env python3
"""One-shot materializer for the OTA server patch; removed before merge."""

from pathlib import Path

PATH = Path(__file__).resolve().parents[1] / "src" / "web_server.cpp"
text = PATH.read_text(encoding="utf-8")


def replace_once(old: str, new: str, label: str) -> None:
    global text
    count = text.count(old)
    if count != 1:
        raise SystemExit(f"{label}: expected exactly one match, found {count}")
    text = text.replace(old, new, 1)


replace_once(
'''        request->send(200, "application/json", "{\\"status\\":\\"ok\\"}");
        delay(350);
        ESP.restart();
    });

    server.on("/api/config", HTTP_GET,''',
'''        request->send(200, "application/json", "{\\"status\\":\\"ok\\",\\"rebooting\\":true}");
        otaManager.scheduleRestart(1200);
    });

    server.on("/api/config", HTTP_GET,''',
"reboot route",
)

replace_once(
'''            relay->off(PumpStopReason::Reboot);
            request->send(200, "application/json", "{\\"status\\":\\"ok\\",\\"rebooting\\":true}");
            delay(350);
            ESP.restart();
        }
    );''',
'''            relay->off(PumpStopReason::Reboot);
            request->send(200, "application/json", "{\\"status\\":\\"ok\\",\\"rebooting\\":true}");
            otaManager.scheduleRestart(1200);
        }
    );''',
"config reboot",
)

replace_once(
'''    server.on("/ota/upload", HTTP_POST, [this](AsyncWebServerRequest *request) {
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
            Serial.printf("[OTA] Start: %s\\n", filename.c_str());
            if (!Update.begin(UPDATE_SIZE_UNKNOWN)) Update.printError(Serial);
        }
        if (!Update.hasError() && Update.write(data, len) != len) {
            Update.printError(Serial);
        }
        if (final) {
            if (Update.end(true)) {
                otaManager.end(true);
                if (oled) oled->drawOTA(100);
                Serial.printf("[OTA] Success: %u bytes\\n", static_cast<unsigned>(index + len));
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
    });''',
'''    server.on("/ota/upload", HTTP_POST, [this](AsyncWebServerRequest *request) {
        const bool failed = Update.hasError();
        AsyncWebServerResponse* response = request->beginResponse(
            failed ? 500 : 200,
            "text/plain",
            failed ? "FAIL" : "OK"
        );
        response->addHeader("Cache-Control", "no-store");
        response->addHeader("Connection", "close");
        request->send(response);

        if (failed) {
            if (oled) oled->drawOTAError();
            Serial.println("[OTA] Upload failed; controller will stay online");
            return;
        }

        // Reboot from a one-shot esp_timer, not from the AsyncWebServer request
        // callback. This leaves enough time for the final HTTP 200/OK to reach
        // the client before the TCP stack disappears.
        if (oled) oled->drawOTAComplete();
        Serial.println("[OTA] HTTP acknowledgement queued; reboot in 1500 ms");
        otaManager.scheduleRestart(1500);
    }, [this](AsyncWebServerRequest *request, String filename, size_t index,
              uint8_t *data, size_t len, bool final) {
        if (!index) {
            eventLog.record(EventType::OtaStarted);
            relay->off(PumpStopReason::Ota);
            otaManager.begin();
            if (oled) oled->drawOTA(0);
            Serial.printf("[OTA] Start: %s\\n", filename.c_str());
            if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
                Update.printError(Serial);
                otaManager.end(false);
                if (oled) oled->drawOTAError();
            }
        }

        if (!Update.hasError() && Update.write(data, len) != len) {
            Update.printError(Serial);
        }

        if (!final) {
            otaManager.setProgress(index + len, request->contentLength());
            if (oled && request->contentLength() > 0) {
                uint8_t progress = static_cast<uint8_t>(
                    ((static_cast<uint64_t>(index + len) * 100U) /
                     request->contentLength()));
                // Multipart overhead makes the uploaded firmware slightly
                // smaller than the HTTP request body. Reserve 100% for a
                // successful Update.end(true), never show a false completion.
                if (progress > 99) progress = 99;
                oled->drawOTA(progress);
            }
            return;
        }

        if (!Update.hasError() && Update.end(true)) {
            otaManager.end(true);
            if (oled) oled->drawOTAComplete();
            Serial.printf("[OTA] Success: %u bytes\\n", static_cast<unsigned>(index + len));
        } else {
            if (!Update.hasError()) Update.printError(Serial);
            otaManager.end(false);
            if (oled) oled->drawOTAError();
        }
    });''',
"OTA route",
)

PATH.write_text(text, encoding="utf-8")
print("materialized reliable OTA HTTP acknowledgement patch")
