//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#ifndef ahiFactoryInspection_H_
#define ahiFactoryInspection_H_

#include <json/value.h>
#include <opencv2/core/mat.hpp>

#include "Common.hpp"

// typedef for joints centroid, the entire pose and inspection
typedef struct {
    cv::Point CentroidHeadTop, CentroidNeck, CentroidRightShoulder;
    cv::Point CentroidLeftShoulder, CentroidRightElbow, CentroidLeftElbow;
    cv::Point CentroidRightHand, CentroidLeftHand, CentroidRightHip, CentroidLeftHip;
    cv::Point CentroidRightKnee, CentroidLeftKnee, CentroidRightAnkle, CentroidLeftAnkle;

    float CentroidHeadTopConfidence, CentroidNeckConfidence, CentroidRightShoulderConfidence;
    float CentroidLeftShoulderConfidence, CentroidRightElbowConfidence, CentroidLeftElbowConfidence;
    float CentroidRightHandConfidence, CentroidLeftHandConfidence, CentroidRightHipConfidence, CentroidLeftHipConfidence;
    float CentroidRightKneeConfidence, CentroidLeftKneeConfidence, CentroidRightAnkleConfidence, CentroidLeftAnkleConfidence;

    float FaceConfidence;
    cv::Rect Face;
    bool threeOutOfFiveRule = false;
    bool RA = false;
    bool LA = false;
    bool UB = false;
    bool LB = false;
    bool GE = false;
    bool DP = true;
    bool BG = true;
    bool LL = false;
    bool RL = false;
    bool FaceInExpectedContour = false;
    bool CameraIsPotentiallyHigh = false;
    bool CameraIsPotentiallyLow = false;
    int countFrontInspectionResults = 0;
    int countSideInspectionResults = 0;
    int numOfDetectedFaces = 0;
    std::string view = "";
    std::string gender = "";

    bool headFound = false;
    bool leftHandFound = false;
    bool rightHandFound = false;
    bool leftLegFound = false;
    bool rightLegFound = false;
    bool headInGreenZone = false;
    bool anklesInGreenZone = false;
    int yTopUp = 0;
    int yTopLow = 1280;
    int yBotUp = 0;
    int yBotLow = 1280;
    std::string ErrorMsg = "";
    std::string poseUsed = "";

    inline Json::Value vectorToJson() {
        Json::Value childJson;
        childJson["FaceH"] = Face.height;
        childJson["FaceW"] = Face.width;
        childJson["FaceX"] = Face.x;
        childJson["FaceY"] = Face.y;
        childJson["HeadTopX"] = CentroidHeadTop.x;
        childJson["HeadTopY"] = CentroidHeadTop.y;
        childJson["NeckX"] = CentroidNeck.x;
        childJson["NeckY"] = CentroidNeck.y;
        childJson["RightShoulderX"] = CentroidRightShoulder.x;
        childJson["RightShoulderY"] = CentroidRightShoulder.y;
        childJson["LeftShoulderX"] = CentroidLeftShoulder.x;
        childJson["LeftShoulderY"] = CentroidLeftShoulder.y;
        childJson["RightElbowX"] = CentroidRightElbow.x;
        childJson["RightElbowY"] = CentroidRightElbow.y;
        childJson["LeftElbowX"] = CentroidLeftElbow.x;
        childJson["LeftElbowY"] = CentroidLeftElbow.y;
        childJson["RightHandX"] = CentroidRightHand.x;
        childJson["RightHandY"] = CentroidRightHand.y;
        childJson["LeftHandX"] = CentroidLeftHand.x;
        childJson["LeftHandY"] = CentroidLeftHand.y;
        childJson["RightHipX"] = CentroidRightHip.x;
        childJson["RightHipY"] = CentroidRightHip.y;
        childJson["LeftHipX"] = CentroidLeftHip.x;
        childJson["LeftHipY"] = CentroidLeftHip.y;
        childJson["RightKneeX"] = CentroidRightKnee.x;
        childJson["RightKneeY"] = CentroidRightKnee.y;
        childJson["LeftKneeX"] = CentroidLeftKnee.x;
        childJson["LeftKneeY"] = CentroidLeftKnee.y;
        childJson["RightAnkleX"] = CentroidRightAnkle.x;
        childJson["RightAnkleY"] = CentroidRightAnkle.y;
        childJson["LeftAnkleX"] = CentroidLeftAnkle.x;
        childJson["LeftAnkleY"] = CentroidLeftAnkle.y;
        childJson["RightAnkleX"] = CentroidRightAnkle.x;
        childJson["RightAnkleY"] = CentroidRightAnkle.y;

        childJson["FaceConfidence"] = FaceConfidence;
        childJson["HeadTopConfidence"] = CentroidHeadTopConfidence;
        childJson["NeckConfidence"] = CentroidNeckConfidence;
        childJson["RightShoulderConfidence"] = CentroidRightShoulderConfidence;
        childJson["LeftShoulderConfidence"] = CentroidLeftShoulderConfidence;
        childJson["RightElbowConfidence"] = CentroidRightElbowConfidence;
        childJson["LeftElbowConfidence"] = CentroidLeftElbowConfidence;
        childJson["RightHandConfidence"] = CentroidRightHandConfidence;
        childJson["LeftHandConfidence"] = CentroidLeftHandConfidence;
        childJson["RightHipConfidence"] = CentroidRightHipConfidence;
        childJson["LeftHipConfidence"] = CentroidLeftHipConfidence;
        childJson["RightKneeConfidence"] = CentroidRightKneeConfidence;
        childJson["LeftKneeConfidence"] = CentroidLeftKneeConfidence;
        childJson["RightAnkleConfidence"] = CentroidRightAnkleConfidence;
        childJson["LeftAnkleConfidence"] = CentroidLeftAnkleConfidence;
        childJson["RightAnkleConfidence"] = CentroidRightAnkleConfidence;

        childJson["GE"] = GE;
        childJson["LA"] = LA;
        childJson["RA"] = RA;
        childJson["LL"] = LL;
        childJson["RL"] = RL;
        childJson["BG"] = true;
        childJson["DP"] = true;
        childJson["UB"] = UB;
        childJson["LB"] = LB; //
        childJson["FaceInExpectedContour"] = FaceInExpectedContour;
        childJson["CameraIsPotentiallyHigh"] = CameraIsPotentiallyHigh;
        childJson["CameraIsPotentiallyLow"] = CameraIsPotentiallyLow;
        childJson["numOfDetectedFaces"] = numOfDetectedFaces;
        childJson["view"] = view;
        childJson["gender"] = gender;
        childJson["headFound"] = headFound;
        childJson["leftHandFound"] = leftHandFound;
        childJson["rightHandFound"] = rightHandFound;
        childJson["leftLegFound"] = leftLegFound;
        childJson["rightLegFound"] = rightLegFound;

        childJson["headInGreenZone"] = headInGreenZone;
        childJson["anklesInGreenZone"] = anklesInGreenZone;

        childJson["yTopUp"] = yTopUp;
        childJson["yTopLow"] = yTopLow;
        childJson["yBotUp"] = yBotUp;
        childJson["yBotLow"] = yBotLow;
        childJson["ErrorMsg"] = ErrorMsg;
        childJson["poseUsed"] = poseUsed;
        return childJson;
    }

    inline std::vector<cv::Point> tranformToCvJoints() {
        std::vector<cv::Point> Joints;
        cv::Point joint_;
        for (int i = 0; i < 14; i++) {
            if (i == 0) {
                joint_ = CentroidHeadTop;
            }
            if (i == 1) {
                joint_ = CentroidNeck;
            }
            if (i == 2) {
                joint_ = CentroidRightShoulder;
            }
            if (i == 3) {
                joint_ = CentroidRightElbow;
            }
            if (i == 4) {
                joint_ = CentroidRightHand;
            }
            if (i == 5) {
                joint_ = CentroidLeftShoulder;
            }
            if (i == 6) {
                joint_ = CentroidLeftElbow;
            }
            if (i == 7) {
                joint_ = CentroidLeftHand;
            }
            if (i == 8) {
                joint_ = CentroidRightHip;
            }
            if (i == 9) {
                joint_ = CentroidRightKnee;
            }
            if (i == 10) {
                joint_ = CentroidRightAnkle;
            }
            if (i == 11) {
                joint_ = CentroidLeftHip;
            }
            if (i == 12) {
                joint_ = CentroidLeftKnee;
            }
            if (i == 13) {
                joint_ = CentroidLeftAnkle;
            }
            Joints.push_back(joint_);
        }
        return Joints;
    }

} ahiPoseInfo;

// typedef for silhouette extremas
typedef struct {
    int xLeft, yLeft, xRight, yRight, xCenter, yCenter;
    int xBottom, yBottom, xTop, yTop, xBottomLeft, xBottomRight, yBottomLeft, yBottomRight;
    int xContourFaceCenter, yContourFaceCenter, ContourFaceWidth, ContourFaceHeight, FaceDistThrdForInspection;
} ahiExtremas;

// typedef for the front Pose
typedef struct {
    float HeadTopOverlapRatio, LeftHandOverlapRatio, RightHandOverlapRatio, LeftLegOverlapRatio, RightLegOverlapRatio;
    cv::Rect HeadTopFromML, HeadTopFromTL, FaceDetectedFromML;
    cv::Rect LeftHandFromML, RightHandFromML, LeftLegFromML, RightLegFromML;
    cv::Rect LeftHandFromTL, RightHandFromTL, LeftLegFromTL, RightLegFromTL;
    bool HeadInContour = false, LeftHandInContour = false, RightHandInContour = false, LeftLegInContour = false, RightLegInContour = false;
    bool GE = false;
    bool LA = false;
    bool RA = false;
    bool LL = false;
    bool RL = false;
    bool BG = true;
    bool DP = true;
    bool UB = false;
    bool LB = false; //
    bool FaceInExpectedContour = false;
    bool CameraIsPotentiallyHigh = false;
    bool CameraIsPotentiallyLow = false;
    int Face = 0;
    std::string view = "";
} ahiFrontPose;

// typedef for the side pose
typedef struct {
    float HeadTopOverlapRatio, HandOverlapRatio, LegOverlapRatio;
    cv::Rect HeadTopFromML, HeadTopFromTL, FaceDetectedFromML;
    cv::Rect HandFromML, HandFromTL, LegFromML, LegFromTL;
    bool HeadInContour = false, HandInContour = false, LegInContour = false;
    bool GE = false;
    bool LA = false;
    bool RA = false;
    bool LL = false;
    bool RL = false;
    bool BG = true;
    bool DP = true;
    bool UB = false;
    bool LB = false;
    bool FaceInExpectedContour = false;
    bool CameraIsPotentiallyHigh = false;
    bool CameraIsPotentiallyLow = false;
    int Face = 0;
    std::string view = "";
} ahiSidePose;

// json object structure for pose results
typedef struct {
    bool GE = false;
    bool LA = false;
    bool RA = false;
    bool LL = false;
    bool RL = false;
    bool BG = true;
    bool DP = true;
    bool UB = false;
    bool LB = false;
    bool FaceInExpectedContour = false;
    bool CameraIsPotentiallyHigh = false;
    bool CameraIsPotentiallyLow = false;
    int Face = 0;
    std::string view = "";
} ahiJsonPose;

class ahiFactoryInspection {
public:
    bool checkForInspection(ahiJsonPose result, std::string const &viewStr);

    cv::Mat getBinaryImageWithRect(cv::Size imageSize, cv::Rect rect);

    bool isValid(const cv::Point &point);

    bool haveExtremities(const ahiPoseInfo &poseInfo, BodyScanCommon::Profile viewType);

    bool isRectContainsPoint(cv::Rect rect, cv::Point P);

    ahiExtremas getExtremePointsFromBinaryImage(cv::Mat const &binaryPose, bool isTempPose);

    bool
    inspectFrontPoseWithDetectedPosePlusContour(cv::Mat const &tPose, cv::Rect const faceRect,
                                                ahiPoseInfo &frontPoseInfo, int yTopUp, int yTopLow,
                                                int yBotUp, int yBotLow);

    bool
    inspectSidePoseWithDetectedPosePlusContour(cv::Mat const &tPose, cv::Rect const faceRect,
                                               ahiPoseInfo &sidePoseInfo, int yTopUp, int yTopLow,
                                               int yBotUp, int yBotLow);

    bool
    inspectWithDetectedPosePlusContour(cv::Mat const &tPose, cv::Rect const faceRect,
                                       ahiPoseInfo &sidePoseInfo, int yTopUp, int yTopLow,
                                       int yBotUp, int yBotLow);

    void updateResult(ahiJsonPose &poseResult, std::string &result);

    ahiFactoryInspection() = default;
};

#endif