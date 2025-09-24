//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#include <jni.h>
#include "PoseInspection.hpp"
#include "Common.hpp"

extern "C"
JNIEXPORT jobject JNICALL
Java_com_advancedhumanimaging_sdk_bodyscan_partposeinspection_PoseInspectionJNI_getExtremePointsFromBinaryImage(
        JNIEnv *env, jobject thiz, jobject binaryImage) {
    cv::Mat binaryMat = BodyScanCommon::bitmapToMat(env, binaryImage);
    std::map<std::string, int> points = getExtremePointsFromBinaryImage(binaryMat, true);

    jclass hashMapClass = env->FindClass("java/util/HashMap");
    jmethodID hashMapInit = env->GetMethodID(hashMapClass, "<init>", "(I)V");

    jclass integerClass = env->FindClass("java/lang/Integer");
    jmethodID integerInit = env->GetMethodID(integerClass, "<init>", "(I)V");

    jobject hashMapObj = env->NewObject(hashMapClass, hashMapInit, (int) points.size());
    jmethodID hashMapPut = env->GetMethodID(hashMapClass, "put",
                                            "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
    for (const auto &it: points) {
        jobject string1Obj = env->NewStringUTF(it.first.c_str());
        jobject float2Obj = env->NewObject(integerClass, integerInit, it.second);
        env->CallObjectMethod(hashMapObj, hashMapPut, string1Obj, float2Obj);
    }
    return hashMapObj;
}