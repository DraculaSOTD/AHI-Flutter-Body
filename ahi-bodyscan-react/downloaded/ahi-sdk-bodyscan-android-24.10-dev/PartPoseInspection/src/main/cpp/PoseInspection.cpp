//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#include "PoseInspection.hpp"

std::map<std::string, int> getExtremePointsFromBinaryImage(cv::Mat binaryImage, bool isTempPose) {
    std::vector<cv::Point> contourPoints;
    std::vector<cv::Vec4i> hierarchy;
    std::vector<std::vector<cv::Point> > contours;
    if (binaryImage.channels() == 4) {
        cv::cvtColor(binaryImage, binaryImage, cv::COLOR_BGRA2GRAY);
    }
    cv::findContours(binaryImage.clone(), contours, hierarchy, cv::RETR_TREE,
                     cv::CHAIN_APPROX_SIMPLE);
    for (size_t i = 0; i < contours.size(); i++) {
        std::vector<cv::Point> contour = contours[i];
        for (size_t j = 0; j < contour.size(); j++) {
            contourPoints.push_back(contour[j]);
        }
    }
    int largeValue = 2 * MAX(binaryImage.size().height, binaryImage.size().width);
    int xLeft = 0, yLeft = 0, yRight = 0, xTop = 0, xBottom = 0, yBottom = 0;
    int xBottomLeft = 0;
    int xCenter = 0;
    int yCenter = 0;
    // Set the large values
    int xRight = largeValue;
    int yTop = largeValue;
    int xBottomRight = largeValue;
    for (int indexPoint = 0; indexPoint < contourPoints.size(); indexPoint++) {
        xCenter = xCenter + contourPoints[indexPoint].x;
        yCenter = yCenter + contourPoints[indexPoint].y;
        xLeft = std::max(xLeft, contourPoints[indexPoint].x);
        if (xLeft == contourPoints[indexPoint].x) {
            yLeft = contourPoints[indexPoint].y;
        }
        xRight = std::min(xRight, contourPoints[indexPoint].x);
        if (xRight == contourPoints[indexPoint].x) {
            yRight = contourPoints[indexPoint].y;
        }
        yTop = std::min(yTop, contourPoints[indexPoint].y);
        if (yTop == contourPoints[indexPoint].y) {
            xTop = contourPoints[indexPoint].x;
        }
        yBottom = std::max(yBottom, contourPoints[indexPoint].y);
        if (yBottom == contourPoints[indexPoint].y) {
            xBottom = contourPoints[indexPoint].x;
        }
        if (contourPoints[indexPoint].y > (0.8 * 1280)) {
            xBottomRight = std::min(xBottomRight, contourPoints[indexPoint].x);
            xBottomLeft = std::max(xBottomLeft, contourPoints[indexPoint].x);
        }
    }
    xCenter = xCenter / (contourPoints.size() + 1e-6);
    yCenter = yCenter / (contourPoints.size() + 1e-6);

    std::map<std::string, int> outputLocations;
    outputLocations.insert({"xLeft", xLeft});
    outputLocations.insert({"yLeft", yLeft});
    outputLocations.insert({"xRight", xRight});
    outputLocations.insert({"yRight", yRight});
    outputLocations.insert({"xCenter", xCenter});
    outputLocations.insert({"yCenter", yCenter});
    outputLocations.insert({"xBottom", xBottom});
    outputLocations.insert({"yBottom", yBottom});
    outputLocations.insert({"xTop", xTop});
    outputLocations.insert({"yTop", yTop});
    outputLocations.insert({"xBottomLeft", xBottomLeft});
    outputLocations.insert({"xBottomRight", xBottomRight});

    // contour binary image
    if (isTempPose) {
        // getting contour head center
        std::vector<cv::Point2f> pts;
        float topDelta = 0.1 * (yBottom - yTop); // roughly how much half of the head in pixels
        for (int indexPoint = 0; indexPoint < contourPoints.size(); indexPoint++) {
            // don't think this can go with the above as we need y_top and y_bottom from the above loop
            if (contourPoints[indexPoint].y > (yTop - 1) &&
                contourPoints[indexPoint].y < (yTop + topDelta)) {
                pts.push_back(contourPoints[indexPoint]);
            }
        }
        if (pts.size() >= 5) {
            cv::RotatedRect head_ellipse = cv::fitEllipseDirect(pts);
            cv::Rect contFaceROI;
            contFaceROI.width = 0.9 * head_ellipse.size.width;
            contFaceROI.height = 0.9 * head_ellipse.size.height;
            // this is what needs to be compared against actual face center
            cv::Point NoseTip = cv::Point(head_ellipse.center.x, int(yTop + topDelta));
            contFaceROI.x = head_ellipse.center.x - contFaceROI.width / 2. + 2;
            contFaceROI.y = head_ellipse.center.y - contFaceROI.height / 2.;;
            // below thrd need to be tested as it is not final
            int FaceDistThrdForInspection = abs(
                    head_ellipse.center.y + contFaceROI.height / 2. - NoseTip.y);
            cv::circle(binaryImage, NoseTip, 20, 0, -1);
            cv::circle(binaryImage, head_ellipse.center, 10, 0, -1);
            cv::rectangle(binaryImage, contFaceROI, 0);
            outputLocations.insert({"xContourFaceCenter", NoseTip.x});
            outputLocations.insert({"yContourFaceCenter", NoseTip.y});
            outputLocations.insert({"ContourFaceWidth", contFaceROI.width});
            outputLocations.insert({"ContourFaceHeight", contFaceROI.height});
            outputLocations.insert({"FaceDistThrdForInspection", FaceDistThrdForInspection});
        } else {
            outputLocations.clear();
        }
    }
    return outputLocations;
}
