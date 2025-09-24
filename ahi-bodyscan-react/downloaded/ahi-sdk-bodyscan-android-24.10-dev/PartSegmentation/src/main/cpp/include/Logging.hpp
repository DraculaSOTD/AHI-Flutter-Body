//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#pragma once

#if defined(ANDROID) || defined(__ANDROID__)
    #include <android/log.h>
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
