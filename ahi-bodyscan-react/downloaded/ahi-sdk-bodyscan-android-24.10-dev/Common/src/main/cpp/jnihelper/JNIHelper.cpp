//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#include <jni.h>
#include <string>
#include <Common.hpp>
#include "JNIHelper.hpp"

BodyScanCommon::Profile JNIHelper::getNativeProfile(JNIEnv *env, jobject profile) {
//     profile
    jclass jProfileClass = env->FindClass("com/advancedhumanimaging/sdk/bodyscan/common/Profile");
    jmethodID jProfileGetName = env->GetMethodID(jProfileClass, "name", "()Ljava/lang/String;");
    if (jProfileGetName == NULL) {
        return static_cast<BodyScanCommon::Profile>(NULL);
    }
    jstring jProfileType = (jstring) (env->CallObjectMethod(profile, jProfileGetName));
    const char *profileTypeString = env->GetStringUTFChars(jProfileType, 0);
    auto result = (strcmp(profileTypeString, "front") == 0) ? BodyScanCommon::Profile::front
                                                            : BodyScanCommon::Profile::side;
    env->ReleaseStringUTFChars(jProfileType, profileTypeString);
    return result;
}

BodyScanCommon::SexType JNIHelper::getNativeSexType(JNIEnv *env, jobject sex) {
    jclass jSexTypeClass = env->FindClass("com/advancedhumanimaging/sdk/bodyscan/common/SexType");
    jmethodID jSexTypeGetName = env->GetMethodID(jSexTypeClass, "name", "()Ljava/lang/String;");
    if (jSexTypeGetName == NULL) {
        return static_cast<BodyScanCommon::SexType>(NULL);
    }
    jstring jSexType = (jstring) (env->CallObjectMethod(sex, jSexTypeGetName));
    const char *sexTypeString = env->GetStringUTFChars(jSexType, 0);
    auto result = (strcmp(sexTypeString, "male") == 0) ? BodyScanCommon::SexType::male
                                                       : BodyScanCommon::SexType::female;
    env->ReleaseStringUTFChars(jSexType, sexTypeString);
    return result;
}

std::map<std::string, cv::Point2f> JNIHelper::getNativeJoints(JNIEnv *env, jobject joints) {

    jclass jPointFClass = env->FindClass("android/graphics/PointF");
    jfieldID jPointFGetX = env->GetFieldID(jPointFClass, "x", "F");
    jfieldID jPointFGetY = env->GetFieldID(jPointFClass, "y", "F");


    auto nativeJoints = std::map<std::string, cv::Point2f>();

    jclass jMapClass = env->FindClass("java/util/Map");
    jmethodID jMapGetKeySet = env->GetMethodID(jMapClass, "keySet", "()Ljava/util/Set;");
    jmethodID jMapGetValue = env->GetMethodID(jMapClass, "get",
                                              "(Ljava/lang/Object;)Ljava/lang/Object;");
    jobject jKeySet = env->CallObjectMethod(joints, jMapGetKeySet);

    jclass jSetClass = env->FindClass("java/util/Set");
    jmethodID jSetToArray = env->GetMethodID(jSetClass, "toArray", "()[Ljava/lang/Object;");
    jobjectArray jKeyArray = static_cast<jobjectArray>(env->CallObjectMethod(jKeySet, jSetToArray));
    auto keysLength = env->GetArrayLength(jKeyArray);
    for (int index = 0; index < keysLength; ++index) {
        jstring jKey = static_cast<jstring>(env->GetObjectArrayElement(jKeyArray, index));
        const char *nativeKey = env->GetStringUTFChars(jKey, NULL);

        auto jValue = env->CallObjectMethod(joints, jMapGetValue, jKey);//PointF
        float x = env->GetFloatField(jValue, jPointFGetX);
        float y = env->GetFloatField(jValue, jPointFGetY);
        auto nativeValue = cv::Point2f(x, y);

        // insert key, value pair into nativeJoints
        auto nativePair = std::pair<std::string, cv::Point2f>(nativeKey, nativeValue);
        nativeJoints.insert(nativePair);

        // Delete local references
        env->ReleaseStringUTFChars(jKey, nativeKey);
        env->DeleteLocalRef(jKey);
        env->DeleteLocalRef(jValue);
    }
    // Delete local references
    env->DeleteLocalRef(jKeySet);
    env->DeleteLocalRef(jKeyArray);

    // return result
    return nativeJoints;
}

jobjectArray JNIHelper::getJContour(JNIEnv *env, std::vector<cv::Point2f> nativeContour) {
    jclass jPointFClass = env->FindClass("android/graphics/PointF");
    jobjectArray jResult = env->NewObjectArray(nativeContour.size(), jPointFClass, NULL);
    jmethodID jPointInit = env->GetMethodID(jPointFClass, "<init>", "(FF)V");
    for (int index = 0; index < nativeContour.size(); ++index) {
        cv::Point2f point = nativeContour[index];
        jobject jPointF = env->NewObject(jPointFClass, jPointInit, point.x, point.y);
        env->SetObjectArrayElement(jResult, index, jPointF);
    }
    return jResult;
}

CvSize JNIHelper::getCvSize(JNIEnv *env, jobject size) {
    jclass jSizeClass = env->FindClass(
            "android/util/Size");
    jmethodID jSizeGetWidth = env->GetMethodID(jSizeClass, "getWidth", "()I");
    jmethodID jSizeGetHeight = env->GetMethodID(jSizeClass, "getHeight", "()I");
    if (jSizeGetWidth == NULL || jSizeGetHeight == NULL) {
        return CvSize();
    }
    int width = env->CallIntMethod(size, jSizeGetWidth);
    int height = env->CallIntMethod(size, jSizeGetHeight);
    return cvSize(width, height);
}

std::vector<cv::Point2f> JNIHelper::getNativeContour(JNIEnv *env, jobjectArray contour) {
    jclass jPointFClass = env->FindClass("android/graphics/PointF");
    jfieldID jPointFGetX = env->GetFieldID(jPointFClass, "x", "F");
    jfieldID jPointFGetY = env->GetFieldID(jPointFClass, "y", "F");

    auto contourSize = env->GetArrayLength(contour);
    std::vector<cv::Point2f> nativeContour(contourSize);
    for (int index = 0; index < contourSize; ++index) {
        jobject jPointF = env->GetObjectArrayElement(contour, index);
        float x = env->GetFloatField(jPointF, jPointFGetX);
        float y = env->GetFloatField(jPointF, jPointFGetY);
        auto point = cv::Point2f(x, y);
        auto itr = nativeContour.end();
        nativeContour.insert(itr, point);
    }
    return nativeContour;
}

std::map<std::string, std::pair<char *, std::size_t>> JNIHelper::javaModelsMapToCpp(JNIEnv *env, jobject hashMap) {
    std::map<std::string, std::pair<char *, std::size_t>> modelsMap;
    try {
        // Get the Map's entry Set.
        jclass mapClass = env->FindClass("java/util/Map");
        jmethodID entrySet =
                env->GetMethodID(mapClass, "entrySet", "()Ljava/util/Set;");
        jobject set = env->CallObjectMethod(hashMap, entrySet);
        // Obtain an iterator over the Set
        jclass setClass = env->FindClass("java/util/Set");
        jmethodID iterator =
                env->GetMethodID(setClass, "iterator", "()Ljava/util/Iterator;");
        jobject iter = env->CallObjectMethod(set, iterator);
        // Get the Iterator method IDs
        jclass iteratorClass = env->FindClass("java/util/Iterator");
        jmethodID hasNext = env->GetMethodID(iteratorClass, "hasNext", "()Z");
        jmethodID next =
                env->GetMethodID(iteratorClass, "next", "()Ljava/lang/Object;");
        // Get the Entry class method IDs
        jclass entryClass = env->FindClass("java/util/Map$Entry");
        jmethodID getKey =
                env->GetMethodID(entryClass, "getKey", "()Ljava/lang/Object;");
        jmethodID getValue =
                env->GetMethodID(entryClass, "getValue", "()Ljava/lang/Object;");
        jclass pairClass = env->FindClass("kotlin/Pair");
        jfieldID first = env->GetFieldID(pairClass, "first", "Ljava/lang/Object;");
        jfieldID second = env->GetFieldID(pairClass, "second", "Ljava/lang/Object;");
        // Iterate over the entry Set
        while (env->CallBooleanMethod(iter, hasNext)) {
            jobject entry = env->CallObjectMethod(iter, next);
            auto key = (jstring) env->CallObjectMethod(entry, getKey);
            std::string keyStr = env->GetStringUTFChars(key, nullptr);
            auto value = env->CallObjectMethod(entry, getValue);
            auto jBuffer = env->GetObjectField(value, first);
            auto jBufferSize = env->GetObjectField(value, second);
            auto *buffer = reinterpret_cast< char *>(env->GetByteArrayElements(reinterpret_cast<jbyteArray>(jBuffer), nullptr));
            auto bufferSize = env->CallIntMethod(jBufferSize, env->GetMethodID(env->FindClass("java/lang/Integer"), "intValue", "()I"));;
            std::pair<char *, std::size_t> valuePair = std::make_pair(buffer, bufferSize);
            modelsMap.insert(std::make_pair(keyStr, valuePair));
            env->DeleteLocalRef(entry);
            env->DeleteLocalRef(key);
            env->DeleteLocalRef(value);
        }
    } catch (std::exception &e) {}
    return modelsMap;
}

std::vector<cv::Mat> JNIHelper::javaBitmapArrayToCpp(JNIEnv *env, jobjectArray silhouettes) {
    auto silhouettesSize = env->GetArrayLength(silhouettes);
    std::vector<cv::Mat> nativeSilhouettes;
    for (int index = 0; index < silhouettesSize; ++index) {
        jobject jSilhouette = env->GetObjectArrayElement(silhouettes, index);
        auto silhouette = BodyScanCommon::bitmapToMat(env, jSilhouette);
        nativeSilhouettes.push_back(silhouette);
    }
    return nativeSilhouettes;
}

std::vector<std::map<std::string, cv::Point2f>> JNIHelper::javaJointsArrayToCpp(JNIEnv *env, jobjectArray joints) {
    auto jointsSize = env->GetArrayLength(joints);
    std::vector<std::map<std::string, cv::Point2f>> nativeJoints;
    for (int index = 0; index < jointsSize; ++index) {
        jobject jJoint = env->GetObjectArrayElement(joints, index);
        auto joint = JNIHelper::getNativeJoints(env, jJoint);
        nativeJoints.push_back(joint);
    }
    return nativeJoints;
}


