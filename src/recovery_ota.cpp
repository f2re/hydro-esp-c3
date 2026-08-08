#include "recovery_ota.h"

#include <ArduinoOTA.h>

#include "config.h"
#include "event_log.h"
#include "ota_manager.h"

namespace {
constexpr uint16_t RECOVERY_OTA_PORT = 3232;

const char* otaErrorText(ota_error_t error) {
    switch (error) {
        case OTA_AUTH_ERROR: return "auth";
        case OTA_BEGIN_ERROR: return "begin";
        case OTA_CONNECT_ERROR: return "connect";
        case OTA_RECEIVE_ERROR: return "receive";
        case OTA_END_ERROR: return "end";
        default: return "unknown";
    }
}
}

void RecoveryOTA::begin(RelayController* r, WiFiManager* w, OledDisplay* o) {
    if (ready) return;

    relay = r;
    wifi = w;
    oled = o;

    ArduinoOTA.setPort(RECOVERY_OTA_PORT);
    ArduinoOTA.setHostname(MDNS_HOST);
    // The application already owns mDNS for hydro.local. The recovery channel
    // only needs the numeric IP, so do not let ArduinoOTA restart mDNS.
    ArduinoOTA.setMdnsEnabled(false);

    ArduinoOTA.onStart([this]() {
        if (relay) relay->off(PumpStopReason::Ota);
        eventLog.record(EventType::OtaStarted);
        otaManager.begin();
        if (oled) oled->drawOTA(0);
        Serial.println("[RECOVERY OTA] Update started on port 3232");
    });

    ArduinoOTA.onProgress([this](unsigned int progress, unsigned int total) {
        otaManager.setProgress(progress, total);
        const uint8_t percent = total
            ? static_cast<uint8_t>((static_cast<uint64_t>(progress) * 100U) / total)
            : 0;
        if (oled) oled->drawOTA(percent);
    });

    ArduinoOTA.onEnd([this]() {
        otaManager.end(true);
        if (oled) oled->drawOTAComplete();
        Serial.println("[RECOVERY OTA] Update complete; rebooting");
    });

    ArduinoOTA.onError([this](ota_error_t error) {
        otaManager.end(false);
        if (oled) oled->drawOTAError();
        Serial.printf("[RECOVERY OTA] ERROR %u (%s)\n",
                      static_cast<unsigned>(error), otaErrorText(error));
    });

    ArduinoOTA.begin();
    ready = true;
    Serial.printf("[RECOVERY OTA] Ready at %s:%u (independent of Web UI)\n",
                  wifi ? wifi->localIP().c_str() : "0.0.0.0",
                  RECOVERY_OTA_PORT);
}

void RecoveryOTA::handle() {
    if (ready) ArduinoOTA.handle();
}

RecoveryOTA recoveryOTA;
