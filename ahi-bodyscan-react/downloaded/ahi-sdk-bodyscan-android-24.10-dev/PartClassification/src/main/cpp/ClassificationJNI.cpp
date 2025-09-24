//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#include <jni.h>
#include "Common.hpp"
#include "jnihelper/JNIHelper.hpp"
#include "Classification.hpp"

jobject cppResultsMapToJava(JNIEnv *env, const std::map<std::string, float> &results) {
    jclass hashMapClass = env->FindClass("java/util/HashMap");
    jmethodID hashMapInit = env->GetMethodID(hashMapClass, "<init>", "(I)V");
    jclass floatClass = env->FindClass("java/lang/Float");
    jmethodID floatInit = env->GetMethodID(floatClass, "<init>", "(F)V");
    jobject hashMapObj = env->NewObject(hashMapClass, hashMapInit, (int) results.size());
    jmethodID hashMapPut = env->GetMethodID(hashMapClass, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
    for (const auto &it: results) {
        jobject stringObj = env->NewStringUTF(it.first.c_str());
        jobject floatObj = env->NewObject(floatClass, floatInit, it.second);
        env->CallObjectMethod(hashMapObj, hashMapPut, stringObj, floatObj);
    }
    return hashMapObj;
}

extern "C"
JNIEXPORT jobject
Java_com_advancedhumanimaging_sdk_bodyscan_partclassification_ClassificationJNI_classify(
        JNIEnv *env,
        jobject thiz,
        jdouble height,
        jdouble weight,
        jobject sex,
        jobject frontSilhouette,
        jobject sideSilhouette,
        jobject frontJoints,
        jobject sideJoints,
        jobject tfModels,
        jobject svrModels,
        jboolean useAverage
) {
    try {
        auto nativeFrontMap = JNIHelper::getNativeJoints(env, frontJoints);
        auto nativeSideMap = JNIHelper::getNativeJoints(env, sideJoints);
        auto tfModelsMap = JNIHelper::javaModelsMapToCpp(env, tfModels);
        auto svrModelsMap = JNIHelper::javaModelsMapToCpp(env, svrModels);
        BodyScanCommon::SexType sexType = JNIHelper::getNativeSexType(env, sex);
        std::string sexStr = sexType == BodyScanCommon::male ? "M" : "F";
        auto result = Classification::classify(
                height,
                weight,
                sexStr,
                BodyScanCommon::bitmapToMat(env, frontSilhouette),
                BodyScanCommon::bitmapToMat(env, sideSilhouette),
                nativeFrontMap,
                nativeSideMap,
                "shape_and_comp",
                tfModelsMap,
                svrModelsMap,
                useAverage
        );
        return cppResultsMapToJava(env, result.classificationResultsCurrent);
    } catch (std::exception &e) {
        return nullptr;
    }
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_advancedhumanimaging_sdk_bodyscan_partclassification_ClassificationJNI_classifyMultiple(JNIEnv *env,
                                                                                                 jobject thiz,
                                                                                                 jdouble height,
                                                                                                 jdouble weight,
                                                                                                 jobject sex,
                                                                                                 jobjectArray frontSilhouettes,
                                                                                                 jobjectArray sideSilhouettes,
                                                                                                 jobjectArray frontJoints,
                                                                                                 jobjectArray sideJoints,
                                                                                                 jobject tfModels,
                                                                                                 jobject svrModels,
                                                                                                 jboolean useAverage) {
    auto nativeFrontJoints = JNIHelper::javaJointsArrayToCpp(env, frontJoints);
    auto nativeSideJoints = JNIHelper::javaJointsArrayToCpp(env, sideJoints);
    auto nativeFrontSilhouettes = JNIHelper::javaBitmapArrayToCpp(env, frontSilhouettes);
    auto nativeSideSilhouettes = JNIHelper::javaBitmapArrayToCpp(env, sideSilhouettes);
    auto tfModelsMap = JNIHelper::javaModelsMapToCpp(env, tfModels);
    auto svrModelsMap = JNIHelper::javaModelsMapToCpp(env, svrModels);
    BodyScanCommon::SexType sexType = JNIHelper::getNativeSexType(env, sex);
    std::string sexStr = sexType == BodyScanCommon::male ? "M" : "F";
    auto result = Classification::classifyMultiple(
            height,
            weight,
            sexStr,
            nativeFrontSilhouettes,
            nativeSideSilhouettes,
            nativeFrontJoints,
            nativeSideJoints,
            "shape_and_comp",
            tfModelsMap,
            svrModelsMap,
            useAverage
    );
    return cppResultsMapToJava(env, result.classificationResultsCurrent);
}

extern "C"
JNIEXPORT jobjectArray JNICALL
Java_com_advancedhumanimaging_sdk_bodyscan_partclassification_ClassificationJNI_getTfLiteModelNames(JNIEnv *env, jobject thiz) {
    auto modelNames = Classification::getTfLiteModelNames();
    jclass jStringClass = env->FindClass("java/lang/String");
    jobjectArray jModelNames = env->NewObjectArray((jsize) modelNames.size(), jStringClass, nullptr);
    for (int index = 0; index < modelNames.size(); ++index) {
        jobject modelName = env->NewStringUTF(modelNames[index].c_str());
        env->SetObjectArrayElement(jModelNames, index, modelName);
    }
    return jModelNames;
}

extern "C"
JNIEXPORT jobjectArray JNICALL
Java_com_advancedhumanimaging_sdk_bodyscan_partclassification_ClassificationJNI_getSvrModelNames(JNIEnv *env, jobject thiz) {
    auto modelNames = Classification::getSvrModelNames();
    jclass jStringClass = env->FindClass("java/lang/String");
    jobjectArray jModelNames = env->NewObjectArray((jsize) modelNames.size(), jStringClass, nullptr);
    for (int index = 0; index < modelNames.size(); ++index) {
        jobject modelName = env->NewStringUTF(modelNames[index].c_str());
        env->SetObjectArrayElement(jModelNames, index, modelName);
    }
    return jModelNames;
}
