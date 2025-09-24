//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#include <jni.h>
#include <string>
#include <AHIAvatarGenInversion.hpp>
#include <jnihelper/JNIHelper.hpp>
#include "AvatarGenCommon.hpp"

extern "C"
JNIEXPORT jstring JNICALL
Java_com_advancedhumanimaging_sdk_bodyscan_partinversion_InversionJNI_invert(
        JNIEnv *env,
        jobject thiz,
        jobject sex,
        jdouble height_cm,
        jdouble weight_kg,
        jdouble chest_cm,
        jdouble waist_cm,
        jdouble hip_cm,
        jdouble inseam_cm,
        jdouble fitness,
        jobject cv_models_male,
        jobject cv_models_female) {
    BodyScanCommon::SexType nativeSex = JNIHelper().getNativeSexType(env, sex);
    auto cvModelsMapMale = JNIHelper::javaModelsMapToCpp(env, cv_models_male);
    auto cvModelsMapFemale = JNIHelper::javaModelsMapToCpp(env, cv_models_female);
    avatar_gen::inversion invert;
    std::string errorString;
    std::vector<std::string> mesh = invert.invert(nativeSex, (float) height_cm, (float) weight_kg,
                                                  (float) chest_cm, (float) waist_cm,
                                                  (float) hip_cm, (float) inseam_cm,
                                                  (float) fitness, errorString,
                                                  cvModelsMapMale, cvModelsMapFemale, ' ');
    std::ostringstream result;
    for (const auto &a : mesh) {
        result << a << '\n';
    }
    return env->NewStringUTF(result.str().c_str());
}