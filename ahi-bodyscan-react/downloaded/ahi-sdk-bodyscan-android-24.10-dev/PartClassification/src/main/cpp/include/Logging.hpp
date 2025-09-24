//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//
#pragma once

#include "Constants.hpp"

#if defined(ANDROID) || defined(__ANDROID__)
#include <android/log.h>
#define LOG_LVL_ERROR   ANDROID_LOG_ERROR
#define LOG_LVL_INFO    ANDROID_LOG_INFO
#else
#define LOG_LVL_ERROR   0
#define LOG_LVL_INFO    1
#endif

#include <stdarg.h>
#include <stdio.h>
#include <iostream>

//#define LOG_TAG         "NDK"

#define LOG_GLOBAL      (1<<0)
#define LOG_API         (1<<1)
#define LOG_JNI         (1<<2)
#define LOG_SCHEMA      (1<<3)
#define LOG_SQLITE      (1<<4)
#define LOG_UTILS       (1<<5)

//#define LOG_LEVEL (LOG_API|LOG_JNI|LOG_SCHEMA|LOG_SQLITE|LOG_UTILS)
#define LOG_LEVEL_ERROR (0xFFFFFFFF)
#define LOG_LEVEL_INFO (0xFFFFFFFF)

#if defined(ANDROID) || defined(__ANDROID__)

typedef void LoggingHandler(char *text);

extern int start_logger(const char *app_name, LoggingHandler* handler);
#endif

#ifndef NDEBUG
    #define LOG_GUARD(cmd)  {cmd;}
    #if defined(ANDROID) || defined(__ANDROID__)
        #define LOG_INFO(module, ...) {if (module&LOG_LEVEL_INFO) __android_log_print(LOG_LVL_INFO, __FILE__, __VA_ARGS__); }
        #define LOG_ERROR(module, ...) {if (module&LOG_LEVEL_ERROR) __android_log_print(LOG_LVL_ERROR, __FILE__, __VA_ARGS__); }
    #else
        #define LOG_INFO(module, ...) {if (module&LOG_LEVEL_INFO) {printf(__VA_ARGS__);printf("\n");} }
        #define LOG_ERROR(module, ...) {if (module&LOG_LEVEL_ERROR) {printf(__VA_ARGS__);printf("\n");} }
    #endif
#else
    #define LOG_GUARD(cmd)
    #define LOG_INFO(module, ...)
    #define LOG_ERROR(module, ...)
#endif
