#include "version.h"

#if __has_include("generated_build_info.h")
#include "generated_build_info.h"
#endif

#ifndef HYDRO_GENERATED_VERSION
#define HYDRO_GENERATED_VERSION "dev"
#endif

#ifndef HYDRO_GENERATED_BUILD_SHA
#define HYDRO_GENERATED_BUILD_SHA "unknown"
#endif

const char HYDRO_VERSION[] = HYDRO_GENERATED_VERSION;
const char HYDRO_BUILD_SHA[] = HYDRO_GENERATED_BUILD_SHA;
