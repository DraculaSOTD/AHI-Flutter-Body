//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#include "ahiFactoryInspection.hpp"

#include <iostream>

#include <opencv2/imgproc/types_c.h>

#include "CameraConstants.hpp"
#include "Logging.hpp"

// this function is for generating the corner points of the convex hull
ahiExtremas
ahiFactoryInspection::getExtremePointsFromBinaryImage(cv::Mat const &binaryPose, bool isTempPose) {
    // get the contour from the templatePose
    std::vector<cv::Point> contourPoints;
    std::vector<cv::Vec4i> hierarchy;
    std::vector<std::vector<cv::Point>> contours;
    if (binaryPose.channels() > 1) {
        cv::cvtColor(binaryPose, binaryPose, cv::COLOR_BGRA2GRAY);
    }
    cv::findContours(binaryPose.clone(), contours, hierarchy, CV_RETR_TREE,
                     cv::CHAIN_APPROX_SIMPLE);
    for (auto &contour:contours) {
        for (auto &contourPoint:contour) {
            contourPoints.push_back(contourPoint);
        }
    }
    int largeValue = 2 * cv::max(binaryPose.rows, binaryPose.cols);
    int xLeft = 0, yLeft = 0, yRight = 0, xTop = 0, xBottom = 0, yBottom = 0;
    int xBottomLeft = 0, yBottomLeft = 0;
    int xCenter = 0;
    int yCenter = 0;
    // Set the large values
    int xRight = largeValue;
    int yTop = largeValue;
    int xBottomRight = largeValue, yBottomRight = 0;
    for (auto &contourPoint:contourPoints) {
        xCenter = xCenter + contourPoint.x;
        yCenter = yCenter + contourPoint.y;
        if ((contourPoint.y > 400) && (contourPoint.y < 800)) {
            if (contourPoint.x > CAMERA_WIDTH / 2) // 720/2
            {
                xLeft = std::max(xLeft, contourPoint.x);
                if (xLeft == contourPoint.x) {
                    yLeft = contourPoint.y;
                }
            } else {
                xRight = std::min(xRight, contourPoint.x);
                if ((xRight == contourPoint.x) & xRight) {
                    yRight = contourPoint.y;
                }
            }
        }
        yTop = std::min(yTop, contourPoint.y);
        if (yTop == contourPoint.y) {
            xTop = contourPoint.x;
        }
        yBottom = std::max(yBottom, contourPoint.y);
        if (yBottom == contourPoint.y) {
            xBottom = contourPoint.x;
        }
        if (contourPoint.y > (int) (0.8 * CAMERA_HEIGHT)) {
            xBottomRight = std::min(xBottomRight, contourPoint.x);
            if (xBottomRight == contourPoint.x) {
                yBottomRight = contourPoint.y;
            }
            xBottomLeft = std::max(xBottomLeft, contourPoint.x);

            if (xBottomLeft == contourPoint.x) {
                yBottomLeft = contourPoint.y;
            }
        }
    }
    xCenter = (int) (xCenter / (contourPoints.size() + 1e-6));
    yCenter = (int) (yCenter / (contourPoints.size() + 1e-6));
    yBottomLeft = 0.5 * (yBottomLeft + yBottom);
    yBottomRight = 0.5 * (yBottomRight + yBottom);
    if (xBottomLeft < 340) {
        xBottomLeft = 10000;
        yBottomLeft = 10000;
    }
    if (xBottomRight > CAMERA_WIDTH / 2) {
        xBottomRight = -10000;
        yBottomRight = 10000;
    }
    ahiExtremas outputLocations;
    // check that the face is in the correct location on the y axis (not too high or too low)
    if (isTempPose) {
        // getting contour head center
        std::vector<cv::Point2f> pts;
        float topDelta = 0.1 * (yBottom - yTop); // roughly how much half of the head in pixels
        // don't think this can go with the above as we need y_top and y_bottom from the above loop
        for (int indexPoint = 0; indexPoint < contourPoints.size(); indexPoint++) {
            if (contourPoints[indexPoint].y > (yTop - 1) &&
                contourPoints[indexPoint].y < (yTop + topDelta)) {
                pts.push_back(contourPoints[indexPoint]);
            }
        }
        cv::RotatedRect head_ellipse = cv::fitEllipseDirect(pts);
        cv::Rect contFaceROI;
        contFaceROI.width = 0.9 * head_ellipse.size.width;
        contFaceROI.height = 0.9 * head_ellipse.size.height;
        // this is what needs to be compared against actual face center
        cv::Point NoseTip = cv::Point(head_ellipse.center.x, int(yTop + topDelta));
        contFaceROI.x = head_ellipse.center.x - contFaceROI.width / 2. + 2;
        contFaceROI.y = head_ellipse.center.y - contFaceROI.height / 2.;
        // below thrd need to be tested as it is not final
        int FaceDistThrdForInspection = abs(
                head_ellipse.center.y + contFaceROI.height / 2. - NoseTip.y);
        cv::circle(binaryPose, NoseTip, 20, 0, -1);
        cv::circle(binaryPose, head_ellipse.center, 10, 0, -1);
        cv::rectangle(binaryPose, contFaceROI, 0);
        outputLocations.xLeft = xLeft;
        outputLocations.xRight = xRight;
        outputLocations.yLeft = yLeft;
        outputLocations.yRight = yRight;
        outputLocations.xCenter = xCenter;
        outputLocations.yCenter = yCenter;
        outputLocations.xBottom = xBottom;
        outputLocations.yBottom = yBottom;
        outputLocations.xTop = xTop;
        outputLocations.yTop = yTop;
        outputLocations.xBottomLeft = xBottomLeft;
        outputLocations.yBottomLeft = yBottomLeft;
        outputLocations.xBottomRight = xBottomRight;
        outputLocations.yBottomRight = yBottomRight;
        outputLocations.xContourFaceCenter = NoseTip.x;
        outputLocations.yContourFaceCenter = NoseTip.y;
        outputLocations.ContourFaceWidth = contFaceROI.width;
        outputLocations.ContourFaceHeight = contFaceROI.height;
        outputLocations.FaceDistThrdForInspection = FaceDistThrdForInspection;
    } else {
        outputLocations.xLeft = xLeft;
        outputLocations.xRight = xRight;
        outputLocations.yLeft = yLeft;
        outputLocations.yRight = yRight;
        outputLocations.xCenter = xCenter;
        outputLocations.yCenter = yCenter;
        outputLocations.xBottom = xBottom;
        outputLocations.yBottom = yBottom;
        outputLocations.xTop = xTop;
        outputLocations.yTop = yTop;
        outputLocations.xBottomLeft = xBottomLeft;
        outputLocations.yBottomLeft = yBottomLeft;
        outputLocations.xBottomRight = xBottomRight;
        outputLocations.yBottomRight = yBottomRight;
    }
    return outputLocations;
}

void ahiFactoryInspection::updateResult(ahiJsonPose &poseResult, std::string &result) {
    std::stringstream ss;
    ss << "{";
    ss << "\"Face\":" << poseResult.Face << ",";
    ss << "\"GE\":" << (poseResult.GE ? 1 : 0) << ",";
    ss << "\"LA\":" << (poseResult.LA ? 1 : 0) << ",";
    ss << "\"RA\":" << (poseResult.RA ? 1 : 0) << ",";
    ss << "\"LL\":" << (poseResult.LL ? 1 : 0) << ",";
    ss << "\"RL\":" << (poseResult.RL ? 1 : 0) << ",";
    ss << "\"BG\":" << (poseResult.BG ? 1 : 0) << ",";
    ss << "\"DP\":" << (poseResult.DP ? 1 : 0) << ",";
    ss << "\"UB\":" << (poseResult.UB ? 1 : 0) << ",";
    ss << "\"LB\":" << (poseResult.LB ? 1 : 0) << ",";
    ss << "\"FaceInExpectedContour\":" << (poseResult.FaceInExpectedContour ? 1 : 0) << ",";
    ss << "\"CameraIsPotentiallyHigh\":" << (poseResult.CameraIsPotentiallyHigh ? 1 : 0) << ",";
    ss << "\"CameraIsPotentiallyLow\":" << (poseResult.CameraIsPotentiallyLow ? 1 : 0);
    ss << "}";
    result = ss.str();
}

bool ahiFactoryInspection::checkForInspection(ahiJsonPose result, std::string const &viewStr) {
    bool resultInspection = false;
    // re-attempt process for single image
    if (viewStr == "front") {
        resultInspection = ((result.LA) && (result.RA) && (result.LL) && (result.RL));
    }
    if (viewStr == "side") {
        resultInspection = ((result.UB) && (result.LB));
    }
    return resultInspection;
}

cv::Mat ahiFactoryInspection::getBinaryImageWithRect(cv::Size imageSize, cv::Rect rect) {
    int width = int(imageSize.width);
    int height = int(imageSize.height);
    cv::Mat outputImage = cv::Mat::zeros(height, width, CV_8U);
    cv::rectangle(outputImage, rect, cv::Scalar(255, 0, 0), -1);
    return outputImage;
}

bool ahiFactoryInspection::isValid(const cv::Point &point) {
    return (point.x > 0 && point.y > 0);
}

bool ahiFactoryInspection::haveExtremities(const ahiPoseInfo &poseInfo,
                                           BodyScanCommon::Profile viewType) {
    if (!isValid(poseInfo.CentroidHeadTop)) {
        return false;
    }
    if (!isValid(poseInfo.CentroidNeck)) {
        return false;
    }
    if (!isValid(poseInfo.CentroidLeftAnkle)) {
        return false;
    }
    if (!isValid(poseInfo.CentroidRightAnkle)) {
        return false;
    }
    if (BodyScanCommon::Profile::side == viewType) {
        return true;
    }
    if (!isValid(poseInfo.CentroidLeftHand)) {
        return false;
    }
    if (!isValid(poseInfo.CentroidRightHand)) {
        return false;
    }
    return true;
}

bool ahiFactoryInspection::isRectContainsPoint(cv::Rect rect, cv::Point P) {
    try {
        return ((P.x > rect.x) && (P.y > rect.y) && (P.x < rect.x + rect.width) &&
                (P.y < rect.y + rect.height));
    }
    catch (cv::Exception &e) {
        return false;
    }
}

// This function inspect the given pose to the template pose
bool ahiFactoryInspection::inspectFrontPoseWithDetectedPosePlusContour(cv::Mat const &tPose,
                                                                       cv::Rect const faceRect,
                                                                       ahiPoseInfo &frontPoseInfo,
                                                                       int yTopUp, int yTopLow,
                                                                       int yBotUp, int yBotLow) {
    try {
        ahiFrontPose finalResult{};
        ahiExtremas pointsTemplatePose = getExtremePointsFromBinaryImage(tPose, true);
        // pointsTemplatePose.ContourFaceWidth and pointsTemplatePose.ContourFaceHeight are available but not implemented here yet
        int yContourFaceCenter = pointsTemplatePose.yContourFaceCenter;
        int FaceDistThrdForInspection = pointsTemplatePose.FaceDistThrdForInspection;
        int yDetectedFaceCenter = faceRect.y + faceRect.height / 2;
        bool isFaceInAcceptableLocation;
        // x co-ordinate does not affect the height of the persons face, so don't factor it into our thresholding
        float heightDifference = abs(yContourFaceCenter - yDetectedFaceCenter);
        isFaceInAcceptableLocation = bool(heightDifference < FaceDistThrdForInspection);
        finalResult.FaceInExpectedContour = false;
        finalResult.CameraIsPotentiallyHigh = false;
        finalResult.CameraIsPotentiallyLow = false;
        if (isFaceInAcceptableLocation) {
            finalResult.FaceInExpectedContour = true;
        } else {
            LOG_GUARD(std::cout << "Face is not in expected location ..\n")
            if (yDetectedFaceCenter > yContourFaceCenter) {
                // detected face is appearing below the expected location
                // Should check ankle here too in the future
                finalResult.CameraIsPotentiallyHigh = true;
                LOG_GUARD(std::cout << "Camera is potentially too high ..\n")
            } else {
                // detected face is appearing above the expected location
                // Camera is lower than expected
                finalResult.CameraIsPotentiallyLow = true;
                LOG_GUARD(std::cout << "Camera is potentially too low ..\n")
            }
        }
        int widthBox = faceRect.width * 1.2; // just init, it must be contour dep
        int heightBox = faceRect.height * 1.2; // just init, it must be contour dep
        int originx, originy;
        int lengthTemplateBox = 0, widthTemplateBox = 0;
        double tHandBoxConstH = 0.75 * (0.5 * 0.6); // RND
        double tHandBoxConstW = 0.75 * (0.5 * 0.6); // RND
        double tLegBoxConstH = 0.8 * (0.183); // RND
        double tLegBoxConstW = 2.0 * 0.123; // RND
        cv::Rect binaryImageBoxML, binaryImageBoxTL;
        cv::Point posePoint;
        cv::Mat matPoseInspecROI = tPose.clone();
        for (int countJoints = 0; countJoints < 5; countJoints++) {
            switch (countJoints) {
                case 0: // head
                {
                    int ContourFaceWidth =
                            2 * (pointsTemplatePose.yContourFaceCenter - pointsTemplatePose.yTop);
                    if (abs(pointsTemplatePose.xTop - tPose.cols / 2) > 5) {
                        pointsTemplatePose.xTop = tPose.cols / 2;
                    }
                    widthBox = ContourFaceWidth;
                    heightBox = ContourFaceWidth;
                    originx = MAX(0, pointsTemplatePose.xTop) - 0.5 * ContourFaceWidth;
                    int yPoint = pointsTemplatePose.yTop;
                    posePoint = frontPoseInfo.CentroidHeadTop;
                    originy = MAX(0, pointsTemplatePose.yTop - 20);
                    binaryImageBoxTL = cv::Rect(originx, originy, widthBox, heightBox);
                    finalResult.FaceInExpectedContour = isRectContainsPoint(binaryImageBoxTL,
                                                                            posePoint);
                    break;
                }
                case 1: // left arm
                {
                    // iOS
                    // LeftHand
//                resultPart = @"LA";
//                NSInteger lengthTemplateBox = MFZ_INSPECT_tHandBoxConstH_front * ABS([[pointsTemplatePose objectForKey:@"yTop"] integerValue] - [[pointsTemplatePose objectForKey:@"yBottom"] integerValue]);
//                NSInteger widthTemplateBox = MFZ_INSPECT_tHandBoxConstW_front * ABS([[pointsTemplatePose objectForKey:@"xLeft"] integerValue] - [[pointsTemplatePose objectForKey:@"xRight"] integerValue]);
//                NSInteger originx = MAX(0, [[pointsTemplatePose objectForKey:@"xLeft"] integerValue] - widthTemplateBox);
//                NSInteger originy = MAX(0, [[pointsTemplatePose objectForKey:@"yRight"] intValue] - (lengthTemplateBox * 0.7));
//                binaryImageBoxTL = CGRectMake(originx, originy, widthTemplateBox, lengthTemplateBox);
//                posePoint = [poseDictionary[@"CentroidLeftHand"] CGPointValue];
                    // left arm
                    lengthTemplateBox = std::abs(
                            pointsTemplatePose.yTop - pointsTemplatePose.yBottom);
                    widthTemplateBox = std::abs(
                            pointsTemplatePose.xLeft - pointsTemplatePose.xRight);
                    lengthTemplateBox = (int) (tHandBoxConstH * lengthTemplateBox);
                    widthTemplateBox = (int) (tHandBoxConstW * widthTemplateBox);
                    originx = (int) std::max(0.0, (double) pointsTemplatePose.xLeft) -
                              widthTemplateBox;
                    originy = (int) std::max(0.0, (double) pointsTemplatePose.yRight) -
                              widthTemplateBox;
                    binaryImageBoxTL = cv::Rect(originx, originy, widthTemplateBox,
                                                lengthTemplateBox);
                    posePoint = frontPoseInfo.CentroidLeftHand;
                    finalResult.LA = isRectContainsPoint(binaryImageBoxTL, posePoint);
                    break;
                }
                case 2: // right arm
                {
                    //iOS
                    // RightHand
//                resultPart = @"RA";
//                NSInteger lengthTemplateBox = MFZ_INSPECT_tHandBoxConstH_front * ABS([[pointsTemplatePose objectForKey:@"yTop"] integerValue] - [[pointsTemplatePose objectForKey:@"yBottom"] integerValue]);
//                NSInteger widthTemplateBox = MFZ_INSPECT_tHandBoxConstW_front * ABS([[pointsTemplatePose objectForKey:@"xLeft"] integerValue] - [[pointsTemplatePose objectForKey:@"xRight"] integerValue]);
//                NSInteger originx = MAX(0, [[pointsTemplatePose objectForKey:@"xRight"] integerValue]);
//                NSInteger originy = MAX(0, [[pointsTemplatePose objectForKey:@"yRight"] intValue] - (lengthTemplateBox * 0.7));
//                binaryImageBoxTL = CGRectMake(originx, originy, widthTemplateBox, lengthTemplateBox);
//                posePoint = [poseDictionary[@"CentroidRightHand"] CGPointValue];
                    // right arm
                    lengthTemplateBox = std::abs(
                            pointsTemplatePose.yTop - pointsTemplatePose.yBottom);
                    widthTemplateBox = std::abs(
                            pointsTemplatePose.xLeft - pointsTemplatePose.xRight);
                    lengthTemplateBox = (int) (tHandBoxConstW * lengthTemplateBox);
                    widthTemplateBox = (int) (tHandBoxConstH * widthTemplateBox);
                    originx = (int) std::max(0.0, (double) pointsTemplatePose.xRight);
                    originy = (int) std::max(0.0, (double) pointsTemplatePose.yRight) -
                              widthTemplateBox;
                    binaryImageBoxTL = cv::Rect(originx, originy, widthTemplateBox,
                                                lengthTemplateBox);
                    posePoint = frontPoseInfo.CentroidRightHand;
                    finalResult.RA = isRectContainsPoint(binaryImageBoxTL, posePoint);
                    break;
                }
                case 3: // left leg
                {
                    //iOS
//                resultPart = @"LL";
//                NSInteger lengthTemplateBox = MFZ_INSPECT_tLegBoxConstH_front * ABS([[pointsTemplatePose objectForKey:@"yTop"] integerValue] - [[pointsTemplatePose objectForKey:@"yBottom"] integerValue]);
//                NSInteger widthTemplateBox = MFZ_INSPECT_tLegBoxConstW_front * ABS([[pointsTemplatePose objectForKey:@"xLeft"] integerValue] - [[pointsTemplatePose objectForKey:@"xRight"] integerValue]);
//                NSInteger originx = MAX(0, [[pointsTemplatePose objectForKey:@"xBottomLeft"] integerValue] - widthTemplateBox);
//                NSInteger originy = MAX(0, [[pointsTemplatePose objectForKey:@"yBottom"] integerValue] - (1.05 * lengthTemplateBox));
//                originy = originy - (originy / 20);
//                if (!CGRectEqualToRect(CGRectZero, idealZoneAnkles)) {
//                    originy = [[NSNumber numberWithFloat:idealZoneAnkles.origin.y] integerValue];;
//                    lengthTemplateBox = idealZoneAnkles.size.height;
//                }
//                binaryImageBoxTL = CGRectMake(originx, originy, widthTemplateBox, lengthTemplateBox);
//                posePoint = [poseDictionary[@"CentroidLeftAnkle"] CGPointValue];
                    lengthTemplateBox = std::abs(
                            pointsTemplatePose.yTop - pointsTemplatePose.yBottom);
                    widthTemplateBox = std::abs(
                            pointsTemplatePose.xLeft - pointsTemplatePose.xRight);
                    lengthTemplateBox = (int) (tLegBoxConstH * lengthTemplateBox);
                    widthTemplateBox = (int) (tLegBoxConstW * widthTemplateBox);
                    originx = (int) (std::max(0, pointsTemplatePose.xBottomLeft) -
                                     widthTemplateBox);
                    originy = (int) (std::max(0, pointsTemplatePose.yBottom) -
                                     1.05 * lengthTemplateBox);
                    binaryImageBoxTL = cv::Rect(originx, originy, widthTemplateBox,
                                                lengthTemplateBox);
                    posePoint = frontPoseInfo.CentroidLeftAnkle;
                    finalResult.LL = isRectContainsPoint(binaryImageBoxTL, posePoint);
                    break;
                }
                case 4: // right leg
                {
                    lengthTemplateBox = std::abs(
                            pointsTemplatePose.yTop - pointsTemplatePose.yBottom);
                    widthTemplateBox = std::abs(
                            pointsTemplatePose.xLeft - pointsTemplatePose.xRight);
                    lengthTemplateBox = (int) (tLegBoxConstH * lengthTemplateBox);
                    widthTemplateBox = (int) (tLegBoxConstW * widthTemplateBox);
                    originx = (int) std::max(0, pointsTemplatePose.xBottomRight);
                    originy = (int) (std::max(0, pointsTemplatePose.yBottom) -
                                     1.05 * lengthTemplateBox);
                    binaryImageBoxTL = cv::Rect(originx, originy, widthTemplateBox,
                                                lengthTemplateBox);
                    posePoint = frontPoseInfo.CentroidRightAnkle;
                    finalResult.RL = isRectContainsPoint(binaryImageBoxTL, posePoint);
                    break;
                }
                default:
                    continue;
            }
            cv::Scalar COLOR(255 / (countJoints + 2), 255 / (countJoints + 2),
                             255 / (countJoints + 2)); // just RND color
            cv::rectangle(matPoseInspecROI, binaryImageBoxTL, COLOR, 2);
            cv::circle(matPoseInspecROI, posePoint, 25, COLOR, -1);
        }
        frontPoseInfo.FaceInExpectedContour =
                frontPoseInfo.headFound && finalResult.FaceInExpectedContour;
        frontPoseInfo.LA = frontPoseInfo.leftHandFound && finalResult.LA;
        frontPoseInfo.RA = frontPoseInfo.rightHandFound && finalResult.RA;
        frontPoseInfo.LL = frontPoseInfo.leftLegFound && finalResult.LL;
        frontPoseInfo.RL = frontPoseInfo.rightLegFound && finalResult.RL;
        frontPoseInfo.CameraIsPotentiallyHigh = finalResult.CameraIsPotentiallyHigh;
        frontPoseInfo.CameraIsPotentiallyLow = finalResult.CameraIsPotentiallyLow;
        // we can use these for front pose as well
        frontPoseInfo.UB = frontPoseInfo.LA && frontPoseInfo.RA;
        frontPoseInfo.LB = frontPoseInfo.LL && frontPoseInfo.RL;
        int countResults = int(frontPoseInfo.FaceInExpectedContour) + int(frontPoseInfo.LA) +
                           int(frontPoseInfo.RA) + int(frontPoseInfo.LL) + int(frontPoseInfo.RL);
        frontPoseInfo.countFrontInspectionResults = countResults;
        frontPoseInfo.threeOutOfFiveRule = bool((countResults >= 2) && (countResults <= 5));
        return true;
    }
    catch (cv::Exception &CVer) {
        frontPoseInfo.GE = true;
        frontPoseInfo.ErrorMsg = "Exception error in front inspection" + std::string(CVer.what());
        LOG_GUARD(std::cout << CVer.what() << "in front pose inspection ..\n")
        return false;
    }
}

bool ahiFactoryInspection::inspectSidePoseWithDetectedPosePlusContour(cv::Mat const &tPose,
                                                                      cv::Rect const faceRect,
                                                                      ahiPoseInfo &sidePoseInfo,
                                                                      int yTopUp, int yTopLow,
                                                                      int yBotUp, int yBotLow) {
    try {
        ahiSidePose finalResult{};
        ahiExtremas pointsTemplatePose = getExtremePointsFromBinaryImage(tPose, true);
        cv::Rect headML, handML, legML;
        cv::Rect headTL, handTL, legTL;
        int lengthTemplateBox, widthTemplateBox;
        auto tHandBoxConstH = (float) (0.5 * 0.2); // RND
        auto tHandBoxConstW = (float) 0.4; // RND
        auto tLegBoxConstH = (float) (0.35 * 0.5); // RND
        auto tLegBoxConstW = (float) 0.8; // RND
        int originx, originy;
        cv::Point posePoint;
        cv::Mat matPoseInspecROI = tPose.clone();
        cv::Rect binaryImageBoxML, binaryImageBoxTL;
        int ContourFaceWidth =
                2.2 * (pointsTemplatePose.yContourFaceCenter - pointsTemplatePose.yTop);
        for (int cnt = 0; cnt < 5; cnt++) {
            switch (cnt) {
                case 0: {
                    originx = (int) (std::max(0.0,
                                              pointsTemplatePose.xTop - 0.5 * ContourFaceWidth));
                    originy = (int) (std::max(0.0, (double) pointsTemplatePose.yTop) - 20);
                    binaryImageBoxTL = cv::Rect(originx, originy, ContourFaceWidth,
                                                ContourFaceWidth);
                    posePoint = sidePoseInfo.CentroidHeadTop;
                    finalResult.FaceInExpectedContour = isRectContainsPoint(binaryImageBoxTL,
                                                                            posePoint);
                    break;
                }
                case 1: // LA
                {
                    lengthTemplateBox = std::abs(
                            pointsTemplatePose.yTop - pointsTemplatePose.yBottom);
                    widthTemplateBox = std::abs(
                            pointsTemplatePose.xLeft - pointsTemplatePose.xRight);
                    lengthTemplateBox = ContourFaceWidth / 2;
                    widthTemplateBox = ContourFaceWidth / 2;
                    originx = (int) std::max(0.0, (CAMERA_WIDTH / 2.0 - widthTemplateBox));
                    originy = (int) std::max(0.0, (CAMERA_HEIGHT / 2.0 - lengthTemplateBox));
                    binaryImageBoxTL = cv::Rect(originx, originy, widthTemplateBox,
                                                lengthTemplateBox);
                    posePoint = sidePoseInfo.CentroidLeftHand;
                    finalResult.LA = isRectContainsPoint(binaryImageBoxTL, posePoint);
                    break;
                }
                case 2: // RA
                {
                    lengthTemplateBox = std::abs(
                            pointsTemplatePose.yTop - pointsTemplatePose.yBottom);
                    widthTemplateBox = std::abs(
                            pointsTemplatePose.xLeft - pointsTemplatePose.xRight);
                    lengthTemplateBox = ContourFaceWidth / 2;
                    widthTemplateBox = ContourFaceWidth / 2;
                    originx = (int) std::max(0.0, (CAMERA_WIDTH / 2.0 - widthTemplateBox));
                    originy = (int) std::max(0.0, (CAMERA_HEIGHT / 2.0 - lengthTemplateBox));

                    binaryImageBoxTL = cv::Rect(originx, originy, widthTemplateBox,
                                                lengthTemplateBox);
                    posePoint = sidePoseInfo.CentroidRightHand;
                    finalResult.LA = isRectContainsPoint(binaryImageBoxTL, posePoint);
                    break;
                }
                case 3:// LL
                {
                    lengthTemplateBox = std::abs(
                            pointsTemplatePose.yTop - pointsTemplatePose.yBottom);
                    widthTemplateBox = std::abs(
                            pointsTemplatePose.xLeft - pointsTemplatePose.xRight);
                    lengthTemplateBox = (int) (tLegBoxConstH * lengthTemplateBox);
                    widthTemplateBox = (int) (tLegBoxConstW * widthTemplateBox);
                    originx = (int) std::max(0.0, (double) pointsTemplatePose.xBottomRight);
                    originy = (int) std::max(0.0, (double) pointsTemplatePose.yBottom -
                                                  lengthTemplateBox);
                    binaryImageBoxTL = cv::Rect(originx, originy, widthTemplateBox,
                                                lengthTemplateBox);
                    posePoint = sidePoseInfo.CentroidLeftAnkle;
                    finalResult.LL = isRectContainsPoint(binaryImageBoxTL, posePoint);
                    break;
                }
                case 4:// RL
                {
                    lengthTemplateBox = std::abs(
                            pointsTemplatePose.yTop - pointsTemplatePose.yBottom);
                    widthTemplateBox = std::abs(
                            pointsTemplatePose.xLeft - pointsTemplatePose.xRight);
                    lengthTemplateBox = (int) (tLegBoxConstH * lengthTemplateBox);
                    widthTemplateBox = (int) (tLegBoxConstW * widthTemplateBox);
                    originx = (int) std::max(0.0, (double) pointsTemplatePose.xBottomRight);
                    originy = (int) std::max(0.0, (double) pointsTemplatePose.yBottom -
                                                  lengthTemplateBox);
                    binaryImageBoxTL = cv::Rect(originx, originy, widthTemplateBox,
                                                lengthTemplateBox);
                    posePoint = sidePoseInfo.CentroidRightAnkle;
                    finalResult.RL = isRectContainsPoint(binaryImageBoxTL, posePoint);
                    break;
                }
                default:
                    continue;
            }
            cv::Scalar COLOR(255 / (cnt + 2), 255 / (cnt + 2), 255 / (cnt + 2)); // just RND color
            cv::rectangle(matPoseInspecROI, binaryImageBoxTL, COLOR, 2);
            cv::circle(matPoseInspecROI, posePoint, 25, COLOR, -1);
        }
        finalResult.UB = finalResult.RA || finalResult.LA;
        finalResult.LB = finalResult.RL || finalResult.LL;
        sidePoseInfo.FaceInExpectedContour =
                sidePoseInfo.headFound && finalResult.FaceInExpectedContour;
        sidePoseInfo.LA = sidePoseInfo.leftHandFound && finalResult.LA;
        sidePoseInfo.RA = sidePoseInfo.rightHandFound && finalResult.RA;
        sidePoseInfo.LL = sidePoseInfo.leftLegFound && finalResult.LL;
        sidePoseInfo.RL = sidePoseInfo.rightLegFound && finalResult.RL;
        sidePoseInfo.CameraIsPotentiallyHigh = finalResult.CameraIsPotentiallyHigh;
        sidePoseInfo.CameraIsPotentiallyLow = finalResult.CameraIsPotentiallyLow;
        // we can use these for front pose as well
        sidePoseInfo.UB = sidePoseInfo.LA || sidePoseInfo.RA;
        sidePoseInfo.LB = sidePoseInfo.LL || sidePoseInfo.RL;
        int countResults =
                int(sidePoseInfo.FaceInExpectedContour) + int(sidePoseInfo.UB) +
                int(sidePoseInfo.LB);
        sidePoseInfo.threeOutOfFiveRule = bool((countResults >= 2) && (countResults <= 5));
        return true;
    }
    catch (cv::Exception &CVer) {
        sidePoseInfo.GE = true;
        sidePoseInfo.ErrorMsg = "Exception error in side inspection" + std::string(CVer.what());
        LOG_GUARD(std::cout << CVer.what() << "in side pose inspection ..\n")
        return false;
    }
}

bool
ahiFactoryInspection::inspectWithDetectedPosePlusContour(cv::Mat const &tPose,
                                                         cv::Rect const faceRect,
                                                         ahiPoseInfo &poseInfo, int yTopUp,
                                                         int yTopLow,
                                                         int yBotUp, int yBotLow) {
    bool poseSucess = false;
    if (faceRect.empty()) {
        poseInfo.numOfDetectedFaces = 0;
        poseInfo.ErrorMsg = "No face was detected";
        poseSucess = false;
    }
    if (poseInfo.view == "front") {
        poseSucess = inspectFrontPoseWithDetectedPosePlusContour(tPose, faceRect, poseInfo, yTopUp,
                                                                 yTopLow, yBotUp, yBotLow);
    } else {
        poseSucess = inspectSidePoseWithDetectedPosePlusContour(tPose, faceRect, poseInfo, yTopUp,
                                                                yTopLow, yBotUp, yBotLow);
    }
    return poseSucess;
}