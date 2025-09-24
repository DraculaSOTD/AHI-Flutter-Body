//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#include <jnihelper/JNIHelper.hpp>
#include "ContourGenerator.hpp"

extern "C"
JNIEXPORT jobjectArray JNICALL
Java_com_advancedhumanimaging_sdk_bodyscan_partcontour_ContourGeneratorJNI_generateIdealContour(
        JNIEnv *env,
        jobject thiz,
        jobject sex,
        jfloat height_cm,
        jfloat weight_kg,
        jobject image_size,
        jfloat alignment_zradians,
        jobject profile,
        jobject cv_models_male,
        jobject cv_models_female) {
    BodyScanCommon::SexType sexType = JNIHelper::getNativeSexType(env, sex);
    BodyScanCommon::Profile profileType = JNIHelper::getNativeProfile(env, profile);
    auto cvModelsMapMale = JNIHelper::javaModelsMapToCpp(env, cv_models_male);
    auto cvModelsMapFemale = JNIHelper::javaModelsMapToCpp(env, cv_models_female);
    // image_size
    jclass jSizeClass = env->FindClass(
            "android/util/Size");
    jmethodID jSizeGetWidth = env->GetMethodID(jSizeClass, "getWidth", "()I");
    jmethodID jSizeGetHeight = env->GetMethodID(jSizeClass, "getHeight", "()I");
    if (jSizeGetWidth == nullptr || jSizeGetHeight == nullptr) {
        return nullptr;
    }
    int imageWidth = env->CallIntMethod(image_size, jSizeGetWidth);
    int imageHeight = env->CallIntMethod(image_size, jSizeGetHeight);

    auto result = ContourGenerator::generateIdealContour(sexType, height_cm, weight_kg, imageHeight,
                                                         imageWidth, alignment_zradians,
                                                         profileType, cvModelsMapMale, cvModelsMapFemale);

    jclass jPointFClass = env->FindClass("android/graphics/PointF");
    jobjectArray jResult = env->NewObjectArray(result.size(), jPointFClass, nullptr);
    jmethodID jPointInit = env->GetMethodID(jPointFClass, "<init>", "(FF)V");
    for (int index = 0; index < result.size(); ++index) {
        cv::Point2f point = result[index];
        jobject jPointF = env->NewObject(jPointFClass, jPointInit, point.x, point.y);
        env->SetObjectArrayElement(jResult, index, jPointF);
    }
    return jResult;
}
extern "C"
JNIEXPORT jobject JNICALL
Java_com_advancedhumanimaging_sdk_bodyscan_partcontour_ContourGeneratorJNI_generateContourMask(
        JNIEnv *env, jobject thiz, jobjectArray contour, jobject image_size) {
    try {
        jclass jPointFClass = env->FindClass("android/graphics/PointF");
        jfieldID jPointFGetX = env->GetFieldID(jPointFClass, "x", "F");
        jfieldID jPointFGetY = env->GetFieldID(jPointFClass, "y", "F");
        auto contourSize = env->GetArrayLength(contour);
        std::vector<cv::Point> nativeContour;
        for (int index = 0; index < contourSize; ++index) {
            jobject jPointF = env->GetObjectArrayElement(contour, index);
            int x = (int) env->GetFloatField(jPointF, jPointFGetX);
            int y = (int) env->GetFloatField(jPointF, jPointFGetY);
            auto point = cv::Point(x, y);
            nativeContour.push_back(point);
        }

        // image_size
        jclass jSizeClass = env->FindClass(
                "android/util/Size");
        jmethodID jSizeGetWidth = env->GetMethodID(jSizeClass, "getWidth", "()I");
        jmethodID jSizeGetHeight = env->GetMethodID(jSizeClass, "getHeight", "()I");
        if (jSizeGetWidth == nullptr || jSizeGetHeight == nullptr) {
            return nullptr;
        }
        int imageWidth = env->CallIntMethod(image_size, jSizeGetWidth);
        int imageHeight = env->CallIntMethod(image_size, jSizeGetHeight);

        auto result = ContourGenerator::generateContourMask(nativeContour, imageHeight, imageWidth);
        jobject contourBitmap = BodyScanCommon::createBitmap(env, imageWidth, imageHeight);
        BodyScanCommon::matToBitmap(env, result, contourBitmap, true);
        return contourBitmap;
    } catch (std::exception e) {
        return nullptr;
    }
}