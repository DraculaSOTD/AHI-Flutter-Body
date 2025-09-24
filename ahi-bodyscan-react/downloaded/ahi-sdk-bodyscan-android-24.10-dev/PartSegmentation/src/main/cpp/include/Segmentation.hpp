//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#ifndef BODYSCAN_SEGMENTATION_HPP
#define BODYSCAN_SEGMENTATION_HPP

#include <map>

#include <opencv2/core/mat.hpp>

#include "Common.hpp"

class Segmentation {

public:
    cv::Mat segment(const cv::Mat &capture, cv::Mat contourMask, BodyScanCommon::Profile profile,
                    std::map<std::string, cv::Point2f> poseJoints, const char *modelBuffer, std::size_t modelBufferSize);

    std::vector<cv::Mat> segmentAll(
            const std::vector<cv::Mat> &captures,
            std::vector<cv::Mat> contourMasks,
            std::vector<BodyScanCommon::Profile> profiles,
            std::vector<std::map<std::string, cv::Point2f>> poseJoints,
            const char *modelBuffer,
            std::size_t modelBufferSize
    );
};

#endif //BODYSCAN_SEGMENTATION_HPP
