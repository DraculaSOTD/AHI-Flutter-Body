//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#ifndef BODYSCAN_CONTOURGENERATOR_HPP
#define BODYSCAN_CONTOURGENERATOR_HPP

#include <opencv2/core/types.hpp>
#include "Common.hpp"
#include <map>
#include <AHIBSCereal.hpp>
#include <string>
#include "AvatarGenContour.hpp"
#include "AvatarGenCommon.hpp"

class ContourGenerator {
    static ContourGenerator *getInstance();

public:
    static std::vector<cv::Point> generateIdealContour(
            BodyScanCommon::SexType sex,
            float heightCM, float weightKG,
            int imageHeight, int imageWidth,
            float alignmentZRadians,
            BodyScanCommon::Profile profile,
            std::map<std::string, std::pair<char *, std::size_t>> &cvModelsMale,
            std::map<std::string, std::pair<char *, std::size_t>> &cvModelsFemale);

    static cv::Mat
    generateContourMask(const std::vector<cv::Point> &contour, int imageHeight, int imageWidth);

private:
    static ContourGenerator *inst_; // The one, single instance
};

#endif //BODYSCAN_CONTOURGENERATOR_HPP
