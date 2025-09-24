//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#ifndef ahiFactoryClassify_H_
#define ahiFactoryClassify_H_

#include "Types.hpp"
#include "Mutex.hpp"
#include "Thread.hpp"
#include "queue.h"
#include "priority_queue.h"
#include "AssetManager.hpp"
#include "ahiModelsZoo.hpp"
#include "ahiFactoryTensor.hpp"
#include "AHIAvatarGenClassificationHelper.hpp"
#include "log2022.h"

typedef struct {
    std::string classErrMsg;
    std::map<std::string, float> classificationResultsCurrent;
    std::map<std::string, float> classificationResultsAll;

    std::string currentClassResultsAsJson;
    std::string allClassResultsAsJson;

} ahiClassifyInfo;


class ahiFactoryClassify {
public:

    const std::string AHI_RAW_CHEST = "cm_raw_chest";
    const std::string AHI_RAW_WAIST = "cm_raw_waist";
    const std::string AHI_RAW_HIPS = "cm_raw_hips";
    const std::string AHI_RAW_THIGH = "cm_raw_thigh";
    const std::string AHI_RAW_INSEAM = "cm_raw_inseam";
    const std::string AHI_RAW_WEIGHTPRED = "kg_raw_weightPredict";
    const std::string AHI_RAW_BODYFAT = "percent_raw_bodyFat";
    const std::string AHI_GEN_FITNESS = "ml_gen_fitness";
    const std::string AHI_GEN_FFM = "ml_gen_ffm";
    const std::string AHI_GEN_GYNOID = "ml_gen_gynoid";
    const std::string AHI_GEN_ANDROID = "ml_gen_android";
    const std::string AHI_GEN_VAT = "ml_gen_vat";

    std::string modelFileName;
    ahiModelsZoo modelsZoo;
    uint8_t mKeydata[32];
    uint8_t mNoncedata[12];

    ahiFactoryClassify() = default;

    void initClassify();

    bool isClassifyInit;

    void getFactorTensorInstant();

    ahiFactoryTensor classifyFT;

    std::string to_lowerStr(std::string str);

    std::map<std::string, std::unique_ptr<tflite::Interpreter>> loadAllTfModels(std::map<std::string, std::pair<char *, std::size_t>> &tfModels);

    bool prepareTensorFlowClassifyModel(std::unique_ptr<tflite::Interpreter> loadedTfModel, std::string &classModelFileName);

    bool loadTensorFlowClassifyModelFromBufferOrFile(char *buffer, std::size_t buffer_size, std::string &classModelFileName);

    ahi_avatar_gen::classification_helper classification_helper;

    bool ahiSVRClassification(double height, double weight, std::string gender,
                              cv::Mat const frontSilhouette,
                              cv::Mat const sideSilhouette,
                              std::map<std::string, cv::Point2f> frontJoints,
                              std::map<std::string, cv::Point2f> side_Joints,
                              std::string modelScanType,
                              std::vector<double> &sil_features_for_DL,
                              std::vector<double> &svr_class_results,
                              std::map<std::string, std::pair<char *, std::size_t>> &svrModels,
                              std::vector<std::pair<std::string, std::vector<float>>> &classResultsRawPairs);

    bool ahiDLClassification(double height,
                             double weight,
                             const std::string &gender,
                             cv::Mat const &frontSilhouette,
                             cv::Mat const &sideSilhouette,
                             std::vector<double> imageFeatureVector,
                             const std::string &modelScanType,
                             std::map<std::string, std::unique_ptr<tflite::Interpreter>> &loadedTfModels,
                             std::vector<std::pair<std::string, std::vector<float>>> &classResultsRawPairs);

    ahiClassifyInfo getClassifyOutInfo(double height,
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

    ahiClassifyInfo getClassifyOutInfoMultipleImages(double height,
                                                     double weight,
                                                     const std::string &gender,
                                                     std::vector<cv::Mat> frontSilhouettes,
                                                     std::vector<cv::Mat> sideSilhouettes,
                                                     std::vector<std::map<std::string, cv::Point2f>> frontJoints,
                                                     std::vector<std::map<std::string, cv::Point2f>> sideJoints,
                                                     std::string modelScanType,
                                                     std::map<std::string, std::pair<char *, std::size_t>> &tfModels,
                                                     std::map<std::string, std::pair<char *, std::size_t>> &svrModels,
                                                     bool useAverage);

    std::string transformClassificationResultsToJson(std::map<std::string, float> resultsMap);
};

#endif