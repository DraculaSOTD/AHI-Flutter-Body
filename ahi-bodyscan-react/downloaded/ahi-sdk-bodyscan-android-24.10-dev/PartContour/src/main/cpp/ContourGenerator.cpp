//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#include "ContourGenerator.hpp"

ContourGenerator *ContourGenerator::inst_ = nullptr;

ContourGenerator *ContourGenerator::getInstance() {
    if (inst_ == nullptr) {
        inst_ = new ContourGenerator();
    }
    return (inst_);
}

std::vector<cv::Point>
ContourGenerator::generateIdealContour(BodyScanCommon::SexType sex,
                                       float heightCM,
                                       float weightKG,
                                       int imageHeight,
                                       int imageWidth,
                                       float alignmentZRadians,
                                       BodyScanCommon::Profile profile,
                                       std::map<std::string, std::pair<char *, std::size_t>> &cvModelsMale,
                                       std::map<std::string, std::pair<char *, std::size_t>> &cvModelsFemale) {
    cv::Mat contourMat;
    std::string error;
    float thetaInDegrees = alignmentZRadians * (180.0 / M_PI);
    avatar_gen::common::getInstance(sex, cvModelsMale, cvModelsFemale);
    return avatar_gen::contour::predict(imageHeight, imageWidth, heightCM, weightKG, sex, profile, contourMat, thetaInDegrees, error, 255, 0, -1, -1,
                                        3);
}

cv::Mat
ContourGenerator::generateContourMask(const std::vector<cv::Point> &contour, int imageHeight,
                                      int imageWidth) {
    cv::Mat contMask = cv::Mat::zeros(imageHeight, imageWidth, CV_8U);
    try {
        std::vector<std::vector<cv::Point> > _contours;
        _contours.push_back(contour);
        cv::drawContours(contMask, _contours, -1, cv::Scalar(255), cv::FILLED);
    }
    catch (cv::Exception &CVer) {}
    return contMask;
}
