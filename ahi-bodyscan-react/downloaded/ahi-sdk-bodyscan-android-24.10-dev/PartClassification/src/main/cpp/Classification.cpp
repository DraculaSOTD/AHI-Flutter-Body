//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#include "Classification.hpp"

ahiClassifyInfo
Classification::classify(double height,
                         double weight,
                         const std::string &gender,
                         cv::Mat const &frontSilhouette,
                         cv::Mat const &sideSilhouette,
                         const std::map<std::string, cv::Point2f> &front_joints_vector,
                         const std::map<std::string, cv::Point2f> &side_joints_vector,
                         std::string modelScanType,
                         std::map<std::string, std::pair<char *, std::size_t>> &tfModels,
                         std::map<std::string, std::pair<char *, std::size_t>> &svrModels,
                         bool useAverage) {
    return ahiFactoryClassify().getClassifyOutInfo(
            height,
            weight,
            gender,
            frontSilhouette,
            sideSilhouette,
            front_joints_vector,
            side_joints_vector,
            modelScanType,
            tfModels,
            svrModels,
            useAverage
    );
}

ahiClassifyInfo
Classification::classifyMultiple(double height,
                                 double weight,
                                 const std::string &gender,
                                 const std::vector<cv::Mat> &frontSilhouettes,
                                 const std::vector<cv::Mat> &sideSilhouettes,
                                 const std::vector<std::map<std::string, cv::Point2f>> &frontJoints,
                                 const std::vector<std::map<std::string, cv::Point2f>> &sideJoints,
                                 std::string modelScanType,
                                 std::map<std::string, std::pair<char *, std::size_t>> &tfModels,
                                 std::map<std::string, std::pair<char *, std::size_t>> &svrModels,
                                 bool useAverage) {
    return ahiFactoryClassify().getClassifyOutInfoMultipleImages(
            height,
            weight,
            gender,
            frontSilhouettes,
            sideSilhouettes,
            frontJoints,
            sideJoints,
            modelScanType,
            tfModels,
            svrModels,
            useAverage
    );
}

vector<std::string> Classification::getTfLiteModelNames() {
    ahiModelsZoo modelsZoo;
    auto modelsMap = modelsZoo.ahiShapeModelGenderMap;
    modelsMap.insert(modelsZoo.ahiCompositionModelGenderMap.begin(), modelsZoo.ahiCompositionModelGenderMap.end());
    vector<std::string> modelNames;
    for (auto &modelMap: modelsMap) {
        std::string modelName = modelMap.first.second;
        modelNames.push_back(modelName);
    }
    return modelNames;
}

vector<std::string> Classification::getSvrModelNames() {
    ahiModelsZoo modelsZoo;
    return modelsZoo.getSvrModelList("shape_and_composition");
}
