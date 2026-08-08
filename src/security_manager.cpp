#include "security_manager.h"
#include <esp_system.h>

namespace {
constexpr char KEY_ALPHABET[] = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
constexpr size_t KEY_LENGTH = 10;
}

void SecurityManager::begin() {
    if (initialized) return;
    prefs.begin("hydrosec", false);
    key = prefs.getString("operator_key", "");
    if (key.length() != KEY_LENGTH) {
        key = generateKey();
        prefs.putString("operator_key", key);
    }
    initialized = true;
}

String SecurityManager::generateKey() {
    String result;
    result.reserve(KEY_LENGTH);
    const size_t alphabetSize = sizeof(KEY_ALPHABET) - 1;
    for (size_t i = 0; i < KEY_LENGTH; ++i) {
        result += KEY_ALPHABET[esp_random() % alphabetSize];
    }
    return result;
}

const String& SecurityManager::operatorKey() {
    begin();
    return key;
}

bool SecurityManager::verify(const String& candidate) {
    begin();
    if (candidate.length() != key.length()) return false;
    uint8_t diff = 0;
    for (size_t i = 0; i < key.length(); ++i) {
        diff |= static_cast<uint8_t>(candidate[i] ^ key[i]);
    }
    return diff == 0;
}

SecurityManager securityManager;
