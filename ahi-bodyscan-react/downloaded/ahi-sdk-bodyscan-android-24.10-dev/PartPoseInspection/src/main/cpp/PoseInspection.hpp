//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#ifndef BODYSCAN_POSEINSPECTION_HPP
#define BODYSCAN_POSEINSPECTION_HPP

#include <map>
#include <string>
#include <opencv2/imgproc.hpp>

std::map<std::string, int> getExtremePointsFromBinaryImage(cv::Mat binaryImage, bool isTempPose);

#endif //BODYSCAN_POSEINSPECTION_HPP
