//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#ifndef AHI_COMMON_H_
#define AHI_COMMON_H_

#include <string>

#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>

#include "ahiFactoryFace.hpp"
#include "ahiFactoryPose.hpp"
#include "ahiFactorySegment.hpp"

typedef struct
{
    std::vector<cv::Point2i> originalContourPoints;
    cv::Mat originalContourMat;
    cv::Mat scaledContourMat;
    std::vector<cv::Point> scaledContourPoints;
    int yTopUp=0;
    int yTopLow=0;
    int yBotUp=0;
    int yBotLow=0;
    std::string view="";
} contourInfo;

typedef struct {
    std::string poseInspectionResults;
    cv::Mat frontSegMask;
    cv::Mat sideSegMask;
    std::string errorMsg;
} ahiCommonResult;

class ahiCommon {
public:
    ahiCommon() = default;
    bool loadTensorFlowModelFromBuffer(const char* buffer, std::size_t buffer_size, std::string modelNameStr);
    bool feedInputBufferImageToCpp(const void* data, cv::Mat mat, std::string toMLProcessName);
    bool feedMlKitFaceToCpp(int faceX, int faceY, int faceHeight, int faceWidth, int numOfDetectedFaces);
    bool feedMlKitPoseToCpp(std::vector<float> mlkitToResults);
    bool feedMlKitSegmentToCpp(cv::Mat mlkitResultsToCpp);
    contourInfo genContour(int image_Height, int image_Width, double heightCM, double weightKG,std::string genderStr, std::string viewStr, float theta, bool calcGreenZones);
    std::vector<cv::Point> calcScaledContourPoints(std::vector<cv::Point> originalContourPoints, float headTopY, float ankleY, cv::Mat& scaledContourMat);
    bool detectFace(cv::Mat image, ahiFaceInfo& faceInfo);
    bool detectPose(cv::Mat image, std::string genderStr, std::string viewStr, ahiPoseInfo &poseInfoPredictions);
    bool inspect(ahiPoseInfo poseInfoPredictions, cv::Mat contour, int yTopUp, int yTopLow, int yBotUp, int yBotLow, bool doFullInspection);
    bool segment(cv::Mat image, cv::Mat contourMask, ahiPoseInfo poseInfoPredictions, std::string viewStr, ahiSegmentInfo& segInfo);
    std::string transformDetectedResultsToJson(ahiPoseInfo &poseInfoPredictions);
    contourInfo frontContourInfo;
    contourInfo sideContourInfo;
    cv::Mat origImageMat;
    int originalImageHeight;
    int originalImageWidth;
    int originalImageNumOfChannels;
    bool isPaddedForResize;
    std::string currModelFileName ="";
    // mlkit face results
    std::string faceDetectionTypeName;
    std::string poseDetectionTypeName;
    std::string segmentTypeName;
    mlKitFaceInfo mlkitFaceData;
    std::vector<float> mlkitPoseData;
    cv::Mat mlkitSegmentData;
private:
    int num_thread_ = 2;
};

#endif // AHI_COMMON_H_
