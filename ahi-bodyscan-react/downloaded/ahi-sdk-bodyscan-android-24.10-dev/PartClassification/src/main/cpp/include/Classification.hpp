//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#ifndef BODYSCAN_CLASSIFICATION_HPP
#define BODYSCAN_CLASSIFICATION_HPP

#include "ahiFactoryClassify.hpp"
#include "AHIAvatarGenClassificationHelper.hpp"
#include "ahiModelsZoo.hpp"

class Classification {
public:
    static ahiClassifyInfo classify(double height,
                                    double weight,
                                    const std::string &gender,
                                    cv::Mat const &frontSilhouette,
                                    cv::Mat const &sideSilhouette,
                                    const std::map<std::string, cv::Point2f> &frontJoints,
                                    const std::map<std::string, cv::Point2f> &sideJoints,
                                    std::string modelScanType,
                                    std::map<std::string, std::pair<char *, std::size_t>> &tfModels,
                                    std::map<std::string, std::pair<char *, std::size_t>> &svrModels,
                                    bool useAverage);

    static ahiClassifyInfo classifyMultiple(double height,
                                       double weight,
                                       const std::string &gender,
                                       const std::vector<cv::Mat>& frontSilhouettes,
                                       const std::vector<cv::Mat>& sideSilhouettes,
                                       const std::vector<std::map<std::string, cv::Point2f>> &frontJoints,
                                       const std::vector<std::map<std::string, cv::Point2f>> &sideJoints,
                                       std::string modelScanType,
                                       std::map<std::string, std::pair<char *, std::size_t>> &tfModels,
                                       std::map<std::string, std::pair<char *, std::size_t>> &svrModels,
                                       bool useAverage);

    static vector<std::string> getTfLiteModelNames();

    static vector<std::string> getSvrModelNames();
};

#endif //BODYSCAN_CLASSIFICATION_HPP
