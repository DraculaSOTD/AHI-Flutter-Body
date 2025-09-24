//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#include "Segmentation.hpp"

#include "ahiCommon.hpp"
#include "ahiFactoryInspection.hpp"
#include "ahiFactorySegment.hpp"
#include "Common.hpp"

cv::Mat
Segmentation::segment(const cv::Mat &capture, cv::Mat contourMask, BodyScanCommon::Profile profile,
                      std::map<std::string, cv::Point2f> poseJoints, const char *modelBuffer,
                      std::size_t modelBufferSize) {
    std::string profileString = profile == BodyScanCommon::Profile::front ? "front" : "side";
    ahiPoseInfo poseInfoPredictions;
    poseInfoPredictions.CentroidHeadTop = poseJoints.at("CentroidHeadTop");
    poseInfoPredictions.CentroidNeck = poseJoints.at("CentroidNeck");
    poseInfoPredictions.CentroidRightAnkle = poseJoints.at("CentroidRightAnkle");
    poseInfoPredictions.CentroidLeftAnkle = poseJoints.at("CentroidLeftAnkle");
    poseInfoPredictions.CentroidRightKnee = poseJoints.at("CentroidRightKnee");
    poseInfoPredictions.CentroidLeftKnee = poseJoints.at("CentroidLeftKnee");
    poseInfoPredictions.CentroidRightHip = poseJoints.at("CentroidRightHip");
    poseInfoPredictions.CentroidLeftHip = poseJoints.at("CentroidLeftHip");
    poseInfoPredictions.CentroidRightHand = poseJoints.at("CentroidRightHand");
    poseInfoPredictions.CentroidLeftHand = poseJoints.at("CentroidLeftHand");
    poseInfoPredictions.CentroidRightElbow = poseJoints.at("CentroidRightElbow");
    poseInfoPredictions.CentroidLeftElbow = poseJoints.at("CentroidLeftElbow");
    poseInfoPredictions.CentroidRightShoulder = poseJoints.at("CentroidRightShoulder");
    poseInfoPredictions.CentroidLeftShoulder = poseJoints.at("CentroidLeftShoulder");
    ahiSegmentInfo segInfo = ahiSegmentInfo();
    ahiCommon common = ahiCommon();
    // load the tflite model first
    common.loadTensorFlowModelFromBuffer(modelBuffer, modelBufferSize,
                                         "segnet.tflite"); // could also use "segmentnet.tflite"

    // load the image
    common.feedInputBufferImageToCpp(capture.data, capture, "segnet.tflite");
    common.segment(capture, contourMask, poseInfoPredictions, profileString, segInfo);
    return segInfo.segmentMask;
}


std::vector<cv::Mat> Segmentation::segmentAll(
        const std::vector<cv::Mat> &captures,
        std::vector<cv::Mat> contourMasks,
        std::vector<BodyScanCommon::Profile> profiles,
        std::vector<std::map<std::string, cv::Point2f>> poseJoints,
        const char *modelBuffer,
        std::size_t modelBufferSize
) {
    ahiCommon common = ahiCommon();
    // load the tflite model first
    common.loadTensorFlowModelFromBuffer(modelBuffer, modelBufferSize, "segnet.tflite"); // could also use "segmentnet.tflite"
    std::vector<cv::Mat> silhouettes;
    for (int index = 0; index < captures.size(); ++index) {
        std::string profile = profiles[index] == BodyScanCommon::Profile::front ? "front" : "side";
        auto joints = poseJoints[index];
        auto contourMask = contourMasks[index];
        auto capture = captures[index];
        ahiPoseInfo poseInfoPredictions;
        poseInfoPredictions.CentroidHeadTop = joints.at("CentroidHeadTop");
        poseInfoPredictions.CentroidNeck = joints.at("CentroidNeck");
        poseInfoPredictions.CentroidRightAnkle = joints.at("CentroidRightAnkle");
        poseInfoPredictions.CentroidLeftAnkle = joints.at("CentroidLeftAnkle");
        poseInfoPredictions.CentroidRightKnee = joints.at("CentroidRightKnee");
        poseInfoPredictions.CentroidLeftKnee = joints.at("CentroidLeftKnee");
        poseInfoPredictions.CentroidRightHip = joints.at("CentroidRightHip");
        poseInfoPredictions.CentroidLeftHip = joints.at("CentroidLeftHip");
        poseInfoPredictions.CentroidRightHand = joints.at("CentroidRightHand");
        poseInfoPredictions.CentroidLeftHand = joints.at("CentroidLeftHand");
        poseInfoPredictions.CentroidRightElbow = joints.at("CentroidRightElbow");
        poseInfoPredictions.CentroidLeftElbow = joints.at("CentroidLeftElbow");
        poseInfoPredictions.CentroidRightShoulder = joints.at("CentroidRightShoulder");
        poseInfoPredictions.CentroidLeftShoulder = joints.at("CentroidLeftShoulder");
        ahiSegmentInfo segInfo = ahiSegmentInfo();
        // load the image
        common.feedInputBufferImageToCpp(capture.data, capture, "segnet.tflite");
        common.segment(capture, contourMask, poseInfoPredictions, profile, segInfo);
        silhouettes.push_back(segInfo.segmentMask);
    }
    return silhouettes;
}