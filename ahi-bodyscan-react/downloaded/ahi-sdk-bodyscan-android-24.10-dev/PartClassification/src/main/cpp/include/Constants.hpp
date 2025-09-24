//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//
#pragma once

#define VALUE_TO_STRING(x) #x
#define VALUE(x) VALUE_TO_STRING(x)
#define VAR_NAME_VALUE(var) #var "="  VALUE(var)

// Default number of frame to run pose inspection on.
#define POSE_FRAME_COUNT    (4)

// Cursor/DB notification min interval (ms).
#define NOTIFY_PERIOD (500)

#define API_PAGE_SIZE   (200)

// Set to 5 max pin tries before native layer blocks attempts.
#define PIN_MAX_TRIES   (5)

#define LOGGING_ENABLED 1

//#define SECURITY_DEBUGDETECT 1

//#define SECURITY_ROOTDETECT 1

//#if (BUILDTYPE > 2)
//    #if (BUILDTYPE < 5) // no logging for NCC builds
//        #define LOGGING_ENABLED 1
//    #endif
//#else
//    #define LOGGING_ENABLED 0
////     Disable (comment) this line to disable security features
////     e.g for debug builds.
////    #define SECURITY_VERIFYAPK 1
////    #define SECURITY_ROOTDETECT 1
////    #define SECURITY_DEBUGDETECT 1
//#endif
//
//#if (BUILDTYPE < 5) // no pinning for NCC builds
////    #define USE_SSL_PINNING 1
////    #define USE_PROXY 1
//#endif
