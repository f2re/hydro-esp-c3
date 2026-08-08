#pragma once
#include <Arduino.h>
#include <Preferences.h>

class SecurityManager {
public:
    void begin();
    const String& operatorKey();
    bool verify(const String& candidate);

private:
    Preferences prefs;
    bool initialized = false;
    String key;

    String generateKey();
};

extern SecurityManager securityManager;
