//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#ifndef BODYSCAN_JNIHELPER_HPP
#define BODYSCAN_JNIHELPER_HPP

#include <map>
#include <opencv2/core/types.hpp>
#include <opencv2/core/types_c.h>
#include "Common.hpp"

class JNIHelper {
public:
    static BodyScanCommon::Profile getNativeProfile(JNIEnv *env, jobject profile);

    static BodyScanCommon::SexType getNativeSexType(JNIEnv *env, jobject sex);

    static std::map<std::string, cv::Point2f> getNativeJoints(JNIEnv *env, jobject joints);

    static CvSize getCvSize(JNIEnv *env, jobject size);

    static std::vector<cv::Point2f> getNativeContour(JNIEnv *env, jobjectArray contour);

    static jobjectArray getJContour(JNIEnv *env, std::vector<cv::Point2f> nativeContour);

    static std::map<std::string, std::pair<char *, std::size_t>> javaModelsMapToCpp(JNIEnv *env, jobject hashMap);

    static std::vector<std::map<std::string, cv::Point2f>> javaJointsArrayToCpp(JNIEnv *env, jobjectArray joints);

    static std::vector<cv::Mat> javaBitmapArrayToCpp(JNIEnv *env, jobjectArray silhouettes);
};


#endif //BODYSCAN_JNIHELPER_HPP
