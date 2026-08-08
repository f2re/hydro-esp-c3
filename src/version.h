#pragma once

// Runtime build identity is defined in version.cpp from a generated header.
// Keeping it out of global compiler flags preserves PlatformIO/framework caches
// when only the Git commit changes.
extern const char HYDRO_VERSION[];
extern const char HYDRO_BUILD_SHA[];

#define HYDRO_PRODUCT_NAME "HydroESP-C3"
#define HYDRO_API_VERSION  3
