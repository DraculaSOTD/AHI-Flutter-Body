//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#ifndef BODYSCAN_COMMON_HPP
#define BODYSCAN_COMMON_HPP

#define LOGE(LOG_TAG, ...) ((void)__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))
#define LOGD(LOG_TAG, ...) ((void)__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))

#include <jni.h>
#include <opencv2/core/mat.hpp>
#include <android/bitmap.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <android/log.h>

namespace BodyScanCommon {
    const unsigned int N_INV_RCALF = 343;
    const unsigned int N_INV_RTHIGH = 302;
    const unsigned int N_INV_R_UARM = 304;
    const float MIN_H = 50.00;
    const float MAX_H = 255.00;
    const float MIN_W = 16.00;
    const float MAX_W = 300.00;
    const unsigned int N_VERTS = 1002;
    const unsigned int N_VERTS_3 = 3006;
    const unsigned int N_VERTS_INV = 10777;
    const unsigned int N_VERTS_INV_3 = 32331;
    const unsigned int N_FACES = 2000;
    const unsigned int N_FACES_3 = 6000;
    const unsigned int N_FACES_INV = 21550;
    const unsigned int N_FACES_INV_3 = 64650;
    const unsigned int N_MU = 7;
    const unsigned int N_BONES = 17;
    const unsigned int N_SKELS = 18;
    const unsigned int N_L_RINGS_VEC = 75427;

    /**
     * Represents the profile of the user (e.g. how they are standing and facing the camera).
     * Either as front or side profile orientation of their body.
     */
    typedef enum Profile {
        /** Represents front user profile orientation. */
        front,
        /** Represents side user profile orientation. */
        side
    } Profile;

    /**
     * Represents sex type. Either male or female.
     */
    typedef enum {
        /** Male type. */
        male,
        /** Female type. */
        female,
    } SexType;

    void throwJavaException(JNIEnv *env, const char *msg);

    void matToBitmap(JNIEnv *env, const cv::Mat &src, jobject bitmap, jboolean needPremultiplyAlpha);

    jobject createBitmap(JNIEnv *env, int width, int height);

    void matToBitmap(JNIEnv *env, const cv::Mat &src, jobject bitmap, jboolean needPremultiplyAlpha);

    cv::Mat bitmapToMat(JNIEnv *env, jobject bitmap);
}

#endif //BODYSCAN_COMMON_HPP
