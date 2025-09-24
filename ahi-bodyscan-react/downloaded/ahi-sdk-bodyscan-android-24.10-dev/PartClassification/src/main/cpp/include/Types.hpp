//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//
#pragma once

#if defined(ANDROID) || defined(__ANDROID__)
#include <jni.h>
#include <android/log.h>
#endif

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>
#include <memory>
#include <set>
#include <map>
#include <chrono>
#include <unordered_map>
#include <algorithm>
#include <locale>
#include "json/reader.h"
#include "json/writer.h"
#include "json/value.h"
#include "ctype.h"
#include "Logging.hpp"
#include "Constants.hpp"

using namespace std;
using namespace Json;


typedef uint64_t ulong;


ulong clock_ms();

typedef std::chrono::duration<long,std::milli> ms_duration;
typedef std::chrono::time_point<std::chrono::system_clock,ms_duration> tp_clock;
typedef std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp_ms;

// Number of seconds before token expiry
// that the token should be refreshed.
// (currently set to 10 mins).
#define TOKEN_EXPIRY_RENEW_SEC  600

#ifndef PT_DENY_ATTACH
#define PT_DENY_ATTACH  31
#endif

//#define MAPTYPE hash_map
#define MAPTYPE unordered_map

#define DLL_EXPORTED __attribute__((__visibility__("default")))
#define DLL_HIDDEN __attribute__((__visibility__("hidden")))
