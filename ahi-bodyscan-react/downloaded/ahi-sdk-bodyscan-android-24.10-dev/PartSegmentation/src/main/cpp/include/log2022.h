//
// Created by YongGyu Lee on 2021/11/27.
//

#ifndef LOG_H_
#define LOG_H_

#define ANDROID_LOG_TAG ""

namespace ahi {

#define LOG_ERROR_HERE                                            \
    __android_log_print(ANDROID_LOG_ERROR,                        \
                        ANDROID_LOG_TAG,                          \
                        "Error at %s, %s, line %d",               \
                        __FILE__, __PRETTY_FUNCTION__, __LINE__)

} // namespace ahi

#endif // LOG_H_
