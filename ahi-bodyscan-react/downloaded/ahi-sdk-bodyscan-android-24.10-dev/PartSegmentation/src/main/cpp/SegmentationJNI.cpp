//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

/** Segmentation */
#include <jni.h>

#include "jnihelper/JNIHelper.hpp"
#include "Segmentation.hpp"

extern "C"
JNIEXPORT jobject JNICALL
Java_com_advancedhumanimaging_sdk_bodyscan_partsegmentation_jni_SegmentationJNI_segment(JNIEnv *env,
                                                                                        jobject thiz,
                                                                                        jobject capture,
                                                                                        jobject contour_mask,
                                                                                        jobject profile,
                                                                                        jobject pose_joints,
                                                                                        jbyteArray buffer,
                                                                                        jint buffer_size) {
    try {
        cv::Mat captureMat = BodyScanCommon::bitmapToMat(env, capture);
        cv::cvtColor(captureMat, captureMat, cv::COLOR_RGBA2RGB);
        cv::Mat contourMat = BodyScanCommon::bitmapToMat(env, contour_mask);
        cv::cvtColor(contourMat, contourMat, cv::COLOR_RGBA2GRAY);
        auto nativeProfile = JNIHelper::getNativeProfile(env, profile);
        auto nativeJoints = JNIHelper::getNativeJoints(env, pose_joints);
        jboolean isCopy;
        jbyte *nativeBuffer = env->GetByteArrayElements(buffer, &isCopy);

        // Call C++ method
        auto result = Segmentation().segment(captureMat, contourMat, nativeProfile, nativeJoints,
                                             reinterpret_cast<const char *>(nativeBuffer),
                                             buffer_size);

        // Create Java Bitmap
        jclass bmpCfgCls = env->FindClass("android/graphics/Bitmap$Config");
        jmethodID bmpClsValueOfMid = env->GetStaticMethodID(bmpCfgCls, "valueOf",
                                                            "(Ljava/lang/String;)Landroid/graphics/Bitmap$Config;");
        jobject jBmpCfg = env->CallStaticObjectMethod(bmpCfgCls, bmpClsValueOfMid,
                                                      env->NewStringUTF("ARGB_8888"));
        jclass bmpCls = env->FindClass("android/graphics/Bitmap");
        jmethodID createBitmapMid = env->GetStaticMethodID(bmpCls, "createBitmap",
                                                           "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
        jobject jBmpObj = env->CallStaticObjectMethod(bmpCls, createBitmapMid, 720, 1280, jBmpCfg);

        // copy Mat to Bitmap
        BodyScanCommon::matToBitmap(env, result, jBmpObj, false);
        // Free memory
        return jBmpObj;
    } catch (std::exception &e) {
        return nullptr;
    }
}

std::vector<BodyScanCommon::Profile> javaProfileArrayToCpp(JNIEnv *env, jobjectArray profiles) {
    auto profilesSize = env->GetArrayLength(profiles);
    std::vector<BodyScanCommon::Profile> nativeProfiles;
    for (int index = 0; index < profilesSize; ++index) {
        jobject jProfile = env->GetObjectArrayElement(profiles, index);
        auto profile = JNIHelper::getNativeProfile(env, jProfile);
        nativeProfiles.push_back(profile);
    }
    return nativeProfiles;
}

extern "C"
JNIEXPORT jobjectArray JNICALL
Java_com_advancedhumanimaging_sdk_bodyscan_partsegmentation_jni_SegmentationJNI_segmentAll(
        JNIEnv *env,
        jobject thiz,
        jobjectArray captures,
        jobjectArray contour_masks,
        jobjectArray profiles,
        jobjectArray pose_joints,
        jbyteArray buffer,
        jint buffer_size
) {
    try {
        auto captureMats = JNIHelper::javaBitmapArrayToCpp(env, captures);
        for (auto &captureMat: captureMats) {
            cv::cvtColor(captureMat, captureMat, cv::COLOR_RGBA2RGB);
        }
        auto contourMats = JNIHelper::javaBitmapArrayToCpp(env, contour_masks);
        for (auto &contourMat: contourMats) {
            cv::cvtColor(contourMat, contourMat, cv::COLOR_RGBA2GRAY);
        }
        auto nativeProfiles = javaProfileArrayToCpp(env, profiles);
        auto nativeJoints = JNIHelper::javaJointsArrayToCpp(env, pose_joints);
        jboolean isCopy;
        jbyte *nativeBuffer = env->GetByteArrayElements(buffer, &isCopy);

        // Call C++ method
        auto silhouettes = Segmentation().segmentAll(captureMats, contourMats, nativeProfiles, nativeJoints,
                                                     reinterpret_cast<const char *>(nativeBuffer), buffer_size);

        jclass jBitmapClass = env->FindClass("android/graphics/Bitmap");
        jobjectArray jSilhouettes = env->NewObjectArray(silhouettes.size(), jBitmapClass, nullptr);
        for (int index = 0; index < silhouettes.size(); ++index) {
            jobject jBitmap = BodyScanCommon::createBitmap(env, 720, 1280);
            BodyScanCommon::matToBitmap(env, silhouettes[index], jBitmap, false);
            env->SetObjectArrayElement(jSilhouettes, index, jBitmap);
        }
        return jSilhouettes;
    } catch (std::exception &e) {
        return nullptr;
    }
}