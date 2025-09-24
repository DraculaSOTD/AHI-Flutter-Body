//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#include "ahiFactoryClassify.hpp"

std::string ahiFactoryClassify::to_lowerStr(std::string str) {
    std::for_each(str.begin(), str.end(), [](char &c) {
        c = ::tolower(c);
    });

    return str;
}

void ahiFactoryClassify::getFactorTensorInstant() {
    ahiFactoryTensor classifyFT;
    isClassifyInit = true;

}

void ahiFactoryClassify::initClassify() {
    getFactorTensorInstant();
}

std::map<std::string, std::unique_ptr<tflite::Interpreter>>
ahiFactoryClassify::loadAllTfModels(std::map<std::string, std::pair<char *, std::size_t>> &tfModels) {
    std::map<std::string, std::unique_ptr<tflite::Interpreter>> loadedModels;
    for (auto &model: tfModels) {
        char *buffer = model.second.first;
        std::size_t bufferSize = model.second.second;
        auto loadedModel = tflite::FlatBufferModel::BuildFromBuffer(buffer, bufferSize);
        if (loadedModel != nullptr) {
            auto interpreter = classifyFT.buildOptimalInterpreter(std::move(loadedModel));
            loadedModels[model.first] = std::move(interpreter);
        }
    }
    return loadedModels;
}

bool
ahiFactoryClassify::prepareTensorFlowClassifyModel(
        std::unique_ptr<tflite::Interpreter> loadedTfModel,
        std::string &classModelFileName
) {
    if (!isClassifyInit) {
        initClassify();
    }
    modelFileName = classModelFileName;
    classifyFT.modelFileName = classModelFileName;

    if (loadedTfModel != nullptr) {
        classifyFT.mInterpreter = std::move(loadedTfModel);
        classifyFT.GetModelInpOutNames();
    } else {
        return false;
    }
    return classifyFT.mInterpreter != nullptr;
}

bool
ahiFactoryClassify::loadTensorFlowClassifyModelFromBufferOrFile(char *buffer, std::size_t buffer_size, std::string &classModelFileName) {
    if (!isClassifyInit) {
        initClassify();
    }
    modelFileName = classModelFileName;
    classifyFT.modelFileName = classModelFileName;

    if (buffer_size > 10)  // model as buffer from Java or elsewhere
    {
        classifyFT.mModel = tflite::FlatBufferModel::BuildFromBuffer(buffer, buffer_size);//,error_reporter);
//        classifyFT.loadModel(buffer, buffer_size);

        bool saveSucess = classifyFT.saveModelBufferOnDevice(modelFileName, buffer, buffer_size);

    } else if (classModelFileName.size() > 0 && buffer_size < 10) // full model name path is supplied
    {
        // here I can use the list of pose tflite models and load them. We need the full path of the model here
        classifyFT.mModel = tflite::FlatBufferModel::BuildFromFile(classModelFileName.c_str());//, error_reporter); // VerifyAndBuildFromFile

        //if above doesn't pass then we can try to download it here, save to cache/data folder and update the class modelFilename (full path)

    } else if (classModelFileName.size() < 0 && buffer_size < 10) // no buffer or file name
    {
        return false;
    }

    if (classifyFT.mModel != nullptr) {
        classifyFT.buildOptimalInterpreter();
        classifyFT.GetModelInpOutNames();
    } else {
        //unable to load classModelFileName
        ;
    }
    return classifyFT.mModel != nullptr;
}

void refineExtraMeasImageBased(std::map<std::string, float> extraMeas, std::string dataSite, std::map<std::string, float> &allExtraMeasurementsDict) {
    bool isMKH = dataSite.find("mkh") != std::string::npos;
    bool isIBV = dataSite.find("ibv") != std::string::npos;
    if (isMKH) {
        float upperArmLength_mkh_imgBased =
                extraMeas["upperArmLength_mkh_imgBased"] * 0.8333 + 7.9379;;
        allExtraMeasurementsDict["upperArmLength_mkh_imgBased"] = upperArmLength_mkh_imgBased;

        float upperArmCirc_mkh_imgBased = extraMeas["upperArmCirc_mkh_imgBased"] * 0.9775 + 3.5838;;
        allExtraMeasurementsDict["upperArmCirc_mkh_imgBased"] = upperArmCirc_mkh_imgBased;

        float calfCirc_mkh_imgBased = extraMeas["calfCirc_mkh_imgBased"] * 0.8869 + 0.9154;;
        allExtraMeasurementsDict["calfCirc_mkh_imgBased"] = calfCirc_mkh_imgBased;

        float upperLegLength_mkh_imgBased =
                extraMeas["upperLegLength_mkh_imgBased"] * 0.9017 + 11.5164;;
        allExtraMeasurementsDict["upperLegLength_mkh_imgBased"] = upperLegLength_mkh_imgBased;

        float lowerLegLength_mkh_imgBased =
                extraMeas["lowerLegLength_mkh_imgBased"] * 0.6133 + 9.2206;
        allExtraMeasurementsDict["lowerLegLength_mkh_imgBased"] = lowerLegLength_mkh_imgBased;
    }
    if (isIBV) {
        float hipHeightButtock_ibv_imgBased = extraMeas["hipHeightButtock_ibv_imgBased"];
        float kneeHeight_ibv_imgBased = extraMeas["kneeHeight_ibv_imgBased"];
        float upperLegLength_ibv_imgBased =
                0.7618 * (hipHeightButtock_ibv_imgBased - kneeHeight_ibv_imgBased) + 13.5536;
        allExtraMeasurementsDict["upperLegLength_ibv_imgBased"] = upperLegLength_ibv_imgBased;

        float seatHeight_ibv_imgBased = extraMeas["seatHeight_ibv_imgBased"];
        float upperLegLength_ibv_imgBased_ =
                0.8315 * (seatHeight_ibv_imgBased - kneeHeight_ibv_imgBased) + 14.1937;
        allExtraMeasurementsDict["upperLegLength_ibv_imgBased_"] = upperLegLength_ibv_imgBased_;

        float ankleHeight_ibv_imgBased = extraMeas["ankleHeight_ibv_imgBased"];
        float lowerLegLength_ibv_imgBased =
                1.0432 * (kneeHeight_ibv_imgBased - ankleHeight_ibv_imgBased) + 2.2673;
        allExtraMeasurementsDict["lowerLegLength_ibv_imgBased"] = lowerLegLength_ibv_imgBased;
    }
}

void refineExtraMeasFeatBased(std::map<std::string, float> extraMeas, std::string dataSite, std::map<std::string, float> &allExtraMeasurementsDict) {
    bool isMKH = dataSite.find("mkh") != std::string::npos || dataSite.find("mh") != std::string::npos;
    bool isIBV = dataSite.find("ibv") != std::string::npos;
    if (isMKH) {
        float upperLegLength_M1_mhFeat = extraMeas["upperLegLength_M1_mhFeat"] * 1.2956 + 0.3349;
        allExtraMeasurementsDict["upperLegLength_M1_mhFeat"] = upperLegLength_M1_mhFeat;

        float lowerLegLength_M1_mhFeat = extraMeas["lowerLegLength_M1_mhFeat"] * 0.7842 + 1.7951;;
        allExtraMeasurementsDict["lowerLegLength_M1_mhFeat"] = lowerLegLength_M1_mhFeat;

        float lowerArmLength_M1_mhFeat = extraMeas["lowerArmLength_M1_mhFeat"] * 0.7842 + 1.7951;;
        allExtraMeasurementsDict["lowerArmLength_M1_mhFeat"] = lowerArmLength_M1_mhFeat;

        float upperArmLength_M1_mhFeat = extraMeas["upperArmLength_M1_mhFeat"] * 1.0366 + 2.7727;;
        allExtraMeasurementsDict["upperArmLength_M1_mhFeat"] = upperArmLength_M1_mhFeat;

        float calfCirc_M1_mhFeat = extraMeas["calfCirc_M1_mhFeat"] * 0.8728 + 1.0915;;
        allExtraMeasurementsDict["calfCirc_M1_mhFeat"] = calfCirc_M1_mhFeat;
    }
    if (isIBV) {
        float hipHeightButtock_M1_ibvFeat = extraMeas["hipHeightButtock_M1_ibvFeat"];
        float kneeHeight_M1_ibvFeat = extraMeas["kneeHeight_M1_ibvFeat"];
        float upperLegLength_M1_ibvFeat =
                0.9147 * (hipHeightButtock_M1_ibvFeat - kneeHeight_M1_ibvFeat) + 7.3845;
        allExtraMeasurementsDict["upperLegLength_M1_ibvFeat"] = upperLegLength_M1_ibvFeat;

        float seatHeight_M1_ibvFeat = extraMeas["seatHeight_M1_ibvFeat"];
        float upperLegLength_M1_ibvFeat_ =
                0.7786 * (seatHeight_M1_ibvFeat - kneeHeight_M1_ibvFeat) + 16.0385;
        allExtraMeasurementsDict["upperLegLength_M1_ibvFeat_"] = upperLegLength_M1_ibvFeat_;

        float hipHeightButtock_M2_ibvFeat = extraMeas["hipHeightButtock_M2_ibvFeat"];
        float kneeHeight_M2_ibvFeat = extraMeas["kneeHeight_M2_ibvFeat"];
        float upperLegLength_M2_ibvFeat =
                0.9439 * (hipHeightButtock_M2_ibvFeat - kneeHeight_M2_ibvFeat) + 6.2720;
        allExtraMeasurementsDict["upperLegLength_M2_ibvFeat"] = upperLegLength_M2_ibvFeat;

        float seatHeight_M2_ibvFeat = extraMeas["seatHeight_M2_ibvFeat"];
        float upperLegLength_M2_ibvFeat_ =
                0.8080 * (seatHeight_M2_ibvFeat - kneeHeight_M2_ibvFeat) + 14.9179;
        allExtraMeasurementsDict["upperLegLength_M2_ibvFeat_"] = upperLegLength_M2_ibvFeat_;

        float ankleHeight_M1_ibvFeat = extraMeas["ankleHeight_M1_ibvFeat"];
        float lowerLegLength_M1_ibvFeat =
                1.0432 * (kneeHeight_M1_ibvFeat - ankleHeight_M1_ibvFeat) + 2.2673;
        allExtraMeasurementsDict["lowerLegLength_M1_ibvFeat"] = lowerLegLength_M1_ibvFeat;

        float ankleHeight_M2_ibvFeat = extraMeas["ankleHeight_M2_ibvFeat"];
        float lowerLegLength_M2_ibvFeat =
                1.0755 * (kneeHeight_M2_ibvFeat - ankleHeight_M2_ibvFeat) + 0.8338;
        allExtraMeasurementsDict["lowerLegLength_M2_ibvFeat"] = lowerLegLength_M2_ibvFeat;
    }
}

void handleFeatureBasedExtraMeas(std::vector<float> tf_result,
                                 std::string outNodeName,
                                 std::string extraModelSite,
                                 std::map<std::string, float> &allMeasurementsDict) {
    extraMeasFeatBasedMeasStruct extraMeasFeatBased;
    std::map<std::string, float> extraMeas;

    int lMeas = tf_result.size();
    //default
    std::string site = "";
    // default and just initialization
    auto siteNames = extraMeasFeatBased.namesUWAmkh;
    // default and just initialization
    auto extraScales = extraMeasFeatBased.scalesUWAmkh;

    //uwa
    if (lMeas < 6) {
        site = "_" + extraModelSite + "_uwa_mh";
        siteNames = extraMeasFeatBased.namesUWAmkh;
        extraScales = extraMeasFeatBased.scalesUWAmkh;
    }
    //uwa
    if (lMeas > 6 && lMeas < 10) {
        site = "_" + extraModelSite + "_uwa_ibv";
        siteNames = extraMeasFeatBased.namesUWAibv;
        extraScales = extraMeasFeatBased.scalesUWAibv;
    } else if (lMeas > 10 && lMeas < 30) {
        site = "_" + extraModelSite + "_mh";
        siteNames = extraMeasFeatBased.namesMH;
        extraScales = extraMeasFeatBased.scalesMH;
    } else if (lMeas > 30) {
        site = "_" + extraModelSite + "_ibv";
        siteNames = extraMeasFeatBased.namesIBV;
        extraScales = extraMeasFeatBased.scalesIBV;
    }

    for (int n = 0; n < lMeas; n++) {
        extraMeas[siteNames[n] + site + "Feat"] = tf_result[n] * extraScales[n];
        // copy all extraMeas
        allMeasurementsDict[siteNames[n] + site + "Feat"] = tf_result[n] * extraScales[n];
    }

    // refine those with biases and offsets
    refineExtraMeasFeatBased(extraMeas, site, allMeasurementsDict);
}

std::vector<double> gen_randnorm_vector(double mu, double sigma, int vect_size) {
    cv::RNG rng(cv::getCPUTickCount()); //random seed
    std::vector<double> rnd_norm_vec(vect_size);
    for (int n = 0; n < vect_size; n++) {
        rnd_norm_vec[n] = rng.gaussian(sigma) + mu;
    }
    return rnd_norm_vec;
}

void handleImageBasedExtraMeas(std::vector<float> tf_result,
                               std::string outNodeName,
                               std::map<std::string, float> &allMeasurementsDict) {
    extraMeasImageBasedMeasStruct extraMeasImageBased;
    std::map<std::string, float> extraMeas;

    int lMeas = tf_result.size();
    std::string site = ""; //default
    auto siteNames = extraMeasImageBased.namesUWA; // just init
    if (lMeas < 6) //uwa outNodeName == Identity_?
    {
        site = "_uwa";
        siteNames = extraMeasImageBased.namesUWA;
    } else if (lMeas > 10 && lMeas < 30) {
        site = "_mkh";
        siteNames = extraMeasImageBased.namesMKH;
    } else if (lMeas > 30) {
        site = "_ibv";
        siteNames = extraMeasImageBased.namesIBV;
    }

    for (int n = 0; n < lMeas; n++) {
        extraMeas[siteNames[n] + site + "_imgBased"] = tf_result[n];
        allMeasurementsDict[siteNames[n] + site + "_imgBased"] = tf_result[n];
    }

    refineExtraMeasImageBased(extraMeas, site, allMeasurementsDict);
}

std::vector<float> getMeasVect(std::map<std::string, float> &allMeasurementsDict,
                               std::vector<std::string> currExtraMeasNames,
                               std::map<std::string, float> &allRemainingMeasurementsDict) {
    std::vector<float> measVect;
    for (int i = 0; i < currExtraMeasNames.size(); i++) {
        if (allMeasurementsDict[currExtraMeasNames[i]]) {
            allRemainingMeasurementsDict.erase(currExtraMeasNames[i]);
            measVect.push_back(allMeasurementsDict[currExtraMeasNames[i]]);
        }
    }
    return measVect;
}

std::string replaceStr(std::string str, std::string substr1, std::string substr2) {
    for (size_t index = str.find(substr1, 0); index != std::string::npos && substr1.length(); index = str.find(substr1, index + substr2.length()))
        str.replace(index, substr1.length(), substr2);
    return str;
}

std::string eraseMeasSite(std::string str) {
    str = replaceStr(str, "_M1_ibvFeat", "");
    str = replaceStr(str, "_M1_mhFeat", "");
    str = replaceStr(str, "_M2_ibvFeat", "");
    str = replaceStr(str, "_ibv_imgBased", "");
    str = replaceStr(str, "_mkh_imgBased", "");
    return str;
}

bool ahiFactoryClassify::ahiSVRClassification(double height, double weight, std::string gender,
                                              cv::Mat const frontSilhouette,
                                              cv::Mat const sideSilhouette,
                                              std::map<std::string, cv::Point2f> front_joints_vector,
                                              std::map<std::string, cv::Point2f> side_joints_vector,
                                              std::string modelScanType,
                                              std::vector<double> &sil_features_for_DL,
                                              std::vector<double> &svr_class_results,
                                              std::map<std::string, std::pair<char *, std::size_t>> &svrModels,
                                              std::vector<std::pair<std::string, std::vector<float>>> &classResultsRawPairs) {
// Prep the classification helper
//Amar: This is how we call in 2022, returning name value as  pairs from the cpp. Same as dictionary and value in objc
    sil_features_for_DL.clear();
    svr_class_results = classification_helper.classify(height, weight, gender, frontSilhouette, sideSilhouette, front_joints_vector,
                                                       side_joints_vector, sil_features_for_DL, svrModels, classResultsRawPairs);
    if (svr_class_results.empty()) {
        return false;
    }
    return true;
}

// iterate over DL model
bool ahiFactoryClassify::ahiDLClassification(double height,
                                             double weight,
                                             const std::string &gender,
                                             cv::Mat const &frontSilhouette,
                                             cv::Mat const &sideSilhouette,
                                             std::vector<double> imageFeatureVector,
                                             const std::string &modelScanType,
                                             std::map<std::string, std::unique_ptr<tflite::Interpreter>> &loadedTfModels,
                                             std::vector<std::pair<std::string, std::vector<float>>> &classResultsRawPairs) {

    if (!isClassifyInit) {
        initClassify();
    }

    std::vector<float> ChestDL, WaistDL, HipsDL, InseamDL, ThighDL, FatFreeMassDL, FatDL, GynoidFatDL, AndroidFatDL, VFATDL;

    std::vector<float> PredHeightGivenWeightGivenFeatDL, PredWeightGivenHeightGivenFeatDL, PredHeightGivenFeatDL, PredWeightGivenFeatDL;

    std::map<std::string, float> allMeasurementsDict;
    std::string extraModelSite;
    bool isFemale = to_lowerStr(gender).find("f") != std::string::npos;

    auto modelGenderMaps = modelsZoo.ahiShapeModelGenderMap; // default

    bool isShape = to_lowerStr(modelScanType).find("shape") != std::string::npos;

    bool isComposition = to_lowerStr(modelScanType).find("comp") != std::string::npos;

    if (!isShape && isComposition) {
        modelGenderMaps = modelsZoo.ahiCompositionModelGenderMap;
    }

    if (isShape && isComposition) {
        modelGenderMaps.insert(modelsZoo.ahiCompositionModelGenderMap.begin(), modelsZoo.ahiCompositionModelGenderMap.end());
    }


    for (auto iter = modelGenderMaps.begin(); iter != modelGenderMaps.end(); iter++) {
        //get ready for the current model
        classifyFT.mInterpreter = nullptr;
        classifyFT.mInputs.clear();

        std::string currModelFileName = iter->first.second;;
        int classModelId = iter->first.first;
        classifyFT.modelFileName = currModelFileName;

        ahiModelGender currModelGender = iter->second;
        if ((currModelGender == ahiModelGender::Male && isFemale) || (currModelGender == ahiModelGender::Female && !isFemale)) {
            continue;
        }

        if (classifyFT.mInterpreter == nullptr) // not loaded from elsewhere, e.g. java/kotlin. Here we need either the model buffer or its full path
        {
            // Move loaded tf model from map
            bool loadModelSuccess = prepareTensorFlowClassifyModel(std::move(loadedTfModels[currModelFileName]), (string &) "");
            // unable to load currModelFileName
            if (!loadModelSuccess)
                continue;
        }

        std::vector<std::string> InputNames = classifyFT.mInputNames;
        std::vector<std::string> outputNames = classifyFT.mOutputNames;

        // we need to automate the below to iterate through multiple inputs, but we dont need this here maybe future
        int pWidth = classifyFT.getInputDim(0, 1);
        int pHeight = classifyFT.getInputDim(0, 2);
        cv::Size targetSize(pWidth, pHeight);

        switch (classModelId) {
            default:
            case ModelClassV1:
            case ModelClassV2male:
            case ModelClassV2female:
            case ModelClassV2p5:
            case ModelClassV3male:
            case ModelClassV3female: {
                isShape = true;
                isComposition = false;
                cv::Mat image_featuresMat = cv::Mat(126, 1, CV_64F, imageFeatureVector.data());
                image_featuresMat.convertTo(image_featuresMat, CV_32F);
                classifyFT.addInput(InputNames[0], ahiTensorInput(image_featuresMat,
                                                                  false)); //InputNames[0] is "silhouettes" (actually the features)
            }
                break;
            case ModelClassV3p1: {
                isShape = true;
                isComposition = false;
                std::vector<double> additional_data(2);
                additional_data[0] = (height / 255.0f - 0.5149143288800009) / 0.1858516016514072;
                additional_data[1] = (weight / 255.0f - 0.5149143288800009) / 0.1858516016514072;
                cv::Mat addInputMat = cv::Mat(2, 1, CV_64F, additional_data.data());
                addInputMat.convertTo(addInputMat, CV_32F);
                classifyFT.addInput("additional_data", ahiTensorInput(addInputMat, false));
                cv::Size target_size = cv::Size(256, 256);
                int top_, bottom_, left_, right_;
                cv::Mat prep_front = classifyFT.preprocess_image_std_or_robust(frontSilhouette,
                                                                               target_size, top_,
                                                                               bottom_, left_, right_, true, 2.0,
                                                                               2.0);
                cv::Mat prep_side = classifyFT.preprocess_image_std_or_robust(sideSilhouette, target_size,
                                                                              top_, bottom_, left_, right_, true,
                                                                              1.5, 1.5);

                if (prep_front.rows != prep_side.rows || prep_front.cols != prep_side.cols) {
                    return false;
                }
                std::vector<cv::Mat> front_side_mats(2);
                front_side_mats[0] = prep_front;
                front_side_mats[1] = prep_side;
                cv::Mat front_side_mats_merged;
                cv::merge(front_side_mats, front_side_mats_merged);
                front_side_mats_merged.convertTo(front_side_mats_merged, CV_32F);
                classifyFT.addInput("silhouettes", ahiTensorInput(front_side_mats_merged / 255.0f, true, false, target_size,
                                                                  {255.0f}, {0.}));
            }
                break;

            case ModelClassTBFIM1: {
                isComposition = true;
                isShape = false;
                std::vector<double> additional_data(2);
                additional_data[0] = height;
                additional_data[1] = weight;
                cv::Mat addInputMat = cv::Mat(2, 1, CV_64F, additional_data.data());//cv::Mat(2, 1, CV_32F,additional_data.data());
                addInputMat.convertTo(addInputMat, CV_32F);
                classifyFT.addInput("additional_data", ahiTensorInput(addInputMat));
                cv::Size target_size = cv::Size(256, 256);
                int top_, bottom_, left_, right_;
                cv::Mat prep_front = classifyFT.preprocess_image_std_or_robust(frontSilhouette,
                                                                               target_size, top_,
                                                                               bottom_, left_, right_, true, 2.0,
                                                                               2.0);
                cv::Mat prep_side = classifyFT.preprocess_image_std_or_robust(sideSilhouette, target_size,
                                                                              top_, bottom_, left_, right_, true,
                                                                              1.5, 1.5);
                if (prep_front.rows != prep_side.rows || prep_front.cols != prep_side.cols) {
                    return false;
                }
                std::vector<cv::Mat> front_side_mats(2);
                front_side_mats[0] = prep_front;
                front_side_mats[1] = prep_side;
                cv::Mat front_side_mats_merged;
                cv::merge(front_side_mats, front_side_mats_merged);
                front_side_mats_merged.convertTo(front_side_mats_merged, CV_32F);
                classifyFT.addInput("silhouettes",
                                    ahiTensorInput(front_side_mats_merged, true, false, target_size,
                                                   {1.0f}, {0.}));
            }
                break;

                // note this uses the same data as the above TBFIM1
            case ModelClassTBFIM2: {
                isComposition = true;
                isShape = false;
                std::vector<double> additional_data(2);
                additional_data[0] = height;
                additional_data[1] = weight;
                cv::Mat addInputMat = cv::Mat(2, 1, CV_64F, additional_data.data());
                addInputMat.convertTo(addInputMat, CV_32F);
                classifyFT.addInput("additional_data", ahiTensorInput(addInputMat, false));
                cv::Size target_size = cv::Size(256, 256);
                int top_, bottom_, left_, right_;
                cv::Mat prep_front = classifyFT.preprocess_image_std_or_robust(frontSilhouette,
                                                                               target_size, top_,
                                                                               bottom_, left_, right_, true, 2.0,
                                                                               2.0);
                cv::Mat prep_side = classifyFT.preprocess_image_std_or_robust(sideSilhouette, target_size,
                                                                              top_, bottom_, left_, right_, true,
                                                                              1.5, 1.5);

                if (prep_front.rows != prep_side.rows || prep_front.cols != prep_side.cols) {
                    return false;
                }
                std::vector<cv::Mat> front_side_mats(2);
                front_side_mats[0] = prep_front;
                front_side_mats[1] = prep_side;
                cv::Mat front_side_mats_merged;
                cv::merge(front_side_mats, front_side_mats_merged);
                front_side_mats_merged.convertTo(front_side_mats_merged, CV_32F);
                classifyFT.addInput("silhouettes",
                                    ahiTensorInput(front_side_mats_merged, true, false, target_size,
                                                   {1.0f}, {0.}));
            }
                break;

            case ModelClassExmeasImageBased: {
                isShape = true;
                isComposition = false;
                cv::Mat inputImageFeat;
                cv::Mat inputHWGFeat;
                cv::Mat inputNormalDist;

                bool isPrepOkay = classifyFT.prepareInputsForImageBasedExtraMeas(frontSilhouette, sideSilhouette, height, weight, gender,
                                                                                 inputImageFeat,
                                                                                 inputHWGFeat, inputNormalDist);
                cv::Size target_size = cv::Size(256, 256);
                classifyFT.addInput("input_1",
                                    ahiTensorInput(inputImageFeat, true, false, target_size,
                                                   {1.0f}, {0.}));
                classifyFT.addInput("input_2", ahiTensorInput(inputHWGFeat, false));
                classifyFT.addInput("input_3", ahiTensorInput(inputNormalDist, false));
            }
                break;

            case ModelClassExmeasFeatBasedM23: {
                extraModelSite = "M1";
                isShape = true;
                isComposition = false;
                std::vector<double> extarMeasFeat23(127, 0);
                for (int n = 0; n < 126; n++) // the 126 can be dynamic length
                {
                    extarMeasFeat23[n + 1] = imageFeatureVector[n];
                }
                extarMeasFeat23[0] = 1.0; // default male
                extarMeasFeat23[1] = extarMeasFeat23[1] / 200.0;
                extarMeasFeat23[2] = extarMeasFeat23[2] / 184.0;
                if (isFemale) {
                    extarMeasFeat23[0] = 0;
                }
                cv::Mat image_featuresMat = cv::Mat(127, 1, CV_64F, extarMeasFeat23.data());
                image_featuresMat.convertTo(image_featuresMat, CV_32F);
                classifyFT.addInput("image_features", ahiTensorInput(image_featuresMat)); //

                std::vector<double> randNormMH = gen_randnorm_vector(0.0, 1.0, 64);
                cv::Mat input_randnormMat = cv::Mat(64, 1, CV_64F, randNormMH.data());
                input_randnormMat.convertTo(input_randnormMat, CV_32F);
                classifyFT.addInput("input_randnorm", ahiTensorInput(input_randnormMat));
            }
                break;

            case ModelClassExmeasFeatBasedM60A:
            case ModelClassExmeasFeatBasedM60B: {
                isShape = true;
                isComposition = false;
                std::vector<double> extarMeasFeat60Plus(127, 0);
                for (int n = 0; n < 126; n++) // the 126 can be dynamic length
                {
                    extarMeasFeat60Plus[n + 1] = imageFeatureVector[n];
                }
                extarMeasFeat60Plus[0] = 1.0; // default male
                extarMeasFeat60Plus[1] = extarMeasFeat60Plus[1] / 210.0;
                extarMeasFeat60Plus[2] = extarMeasFeat60Plus[2] / 170.0;
                if (isFemale) {
                    extarMeasFeat60Plus[0] = 0;
                }
                cv::Mat image_featuresMat = cv::Mat(127, 1, CV_64F, extarMeasFeat60Plus.data());
                image_featuresMat.convertTo(image_featuresMat, CV_32F);
                classifyFT.addInput("image_features", ahiTensorInput(image_featuresMat)); //

                std::vector<double> randNormMH = gen_randnorm_vector(0.0, 1.0, 96);
                cv::Mat input_randnormMat = cv::Mat(96, 1, CV_64F, randNormMH.data());
                input_randnormMat.convertTo(input_randnormMat, CV_32F);
                classifyFT.addInput("input_randnorm", ahiTensorInput(input_randnormMat));

                if (classModelId == ModelClassExmeasFeatBasedM60A) {
                    extraModelSite = "M1";
                }
                if (classModelId == ModelClassExmeasFeatBasedM60B) {
                    extraModelSite = "M2";
                }
            }
                break;

            case ModelClassHeighWeightFeatBased: {
                isComposition = true;
                isShape = false;
                std::vector<double> image_features(127, 0);
                for (int n = 0; n < 126; n++) // the 126 can be dynamic length
                {
                    image_features[n + 1] = imageFeatureVector[n];
                }
                image_features[0] = 1.0; // default male
                if (isFemale) {
                    image_features[0] = 0.;
                }

                cv::Mat image_featuresMat = cv::Mat(127, 1, CV_64F, image_features.data());
                image_featuresMat.convertTo(image_featuresMat, CV_32F);
                classifyFT.addInput("image_features", ahiTensorInput(image_featuresMat)); //
            }
                break;
        }
        //Invoking (inference) starts here
        ahiTensorOutputMap classOutputs;
        bool invokeSucessCurrent = classifyFT.invokeMIMO(classifyFT.mInputs, classOutputs);
        //handle classOutputs
        if (invokeSucessCurrent) {
            // use named measurments as I have done it for iOS via Pairs, them
            int numOfOutputForTheCurrModel = (int) classOutputs.size();
            for (auto iter = classOutputs.begin(); iter != classOutputs.end(); iter++) {

                std::string currModelOutNodeName = iter->first;
                cv::Mat currOutResult = iter->second._mat;

                bool isExMeasFeatBased = currModelOutNodeName.find("decoder/eval/measfc") != string::npos;
                bool isExMeasImgBased = currModelOutNodeName.find("Identity") != string::npos;

                bool isHeightWeightPred = currModelOutNodeName.find("pred_height_weight/BiasAdd") != string::npos;
                bool isHeightPred = currModelOutNodeName.find("pred_height/BiasAdd") != string::npos;
                bool isWeightPred = currModelOutNodeName.find("pred_weight/BiasAdd") != string::npos;

                bool isAnyHW = isHeightWeightPred || isHeightPred || isWeightPred;
                std::vector<float> tf_result;
                tf_result.assign(currOutResult.begin<float>(), currOutResult.end<float>());

                //Chest, Waist, Hips etc.
                // Extra meas or handling of MIMO here
                size_t count = tf_result.size();
                if (count > 80) // we don't calc beyon extra measurment size of about 70
                {
                    continue;
                }

                bool bValid = true;
                for (size_t i = 0; i < count; i++) {
                    if (isnan(tf_result[i])) {
                        bValid = false;
                        break;
                    }

                    if (isinf(tf_result[i])) {
                        bValid = false;
                        break;
                    }

                    if (tf_result[i] < 0) { // unless one of our predictions is expected to be  < 0
                        bValid = false;
                        break;
                    }
                }

                if (bValid) {

                    if (isShape) {
                        // before adding extra measurement. We can change this further in the future
                        if (numOfOutputForTheCurrModel == 1) {

                            if (count == 3) //chest waist and hips
                            {
                                ChestDL.push_back(tf_result[0]);
                                WaistDL.push_back(tf_result[1]);
                                HipsDL.push_back(tf_result[2]);
                            }
                        }

                        // new models e.g. extra measurements models
                        if (isExMeasFeatBased) {
                            handleFeatureBasedExtraMeas(tf_result, currModelOutNodeName, extraModelSite, allMeasurementsDict);
                        } else if (isExMeasImgBased) {
                            handleImageBasedExtraMeas(tf_result, currModelOutNodeName, allMeasurementsDict);
                        }

                    } else {
                        if (isAnyHW) {
                            if (isHeightPred) {
                                PredHeightGivenWeightGivenFeatDL.push_back(tf_result[0]);
                            } else if (isWeightPred) {
                                PredWeightGivenHeightGivenFeatDL.push_back(tf_result[0]);
                            } else if (isHeightWeightPred) {
                                PredHeightGivenFeatDL.push_back(tf_result[0]);
                                PredWeightGivenFeatDL.push_back(tf_result[1]);
                            }
                        } else if (count == 1) {
                            FatDL.push_back(tf_result[0]);
                        }

                    }

                }
            }
        }

        // Move loaded tf model back into map
        loadedTfModels[currModelFileName] = std::move(classifyFT.mInterpreter);
        classifyFT.mInputs.clear();
    }


    bool isShapeForResults = to_lowerStr(modelScanType).find("shape") != std::string::npos;

    bool isCompositionForResults = to_lowerStr(modelScanType).find("comp") != std::string::npos;

    if (isShapeForResults) {
        if (!ChestDL.empty()) {
            classResultsRawPairs.push_back({"ChestDLCurrent", ChestDL});
        }
        if (!WaistDL.empty()) {
            classResultsRawPairs.push_back({"WaistDLCurrent", WaistDL});
        }
        if (!HipsDL.empty()) {
            classResultsRawPairs.push_back({"HipsDLCurrent", HipsDL});
        }

        // we collate the same measure under one name. First we start with the longitudinal extra measurements that are validated
        std::map<std::string, float> allRemainingMeasurementsDict = allMeasurementsDict;

        std::vector<std::string> currExtraMeasNames =
                {
                        "chest_M1_uwa_ibvFeat",
                        "chest_uwa_imgBased",
                        "bustOrChestGirth_M1_ibvFeat",
                        "chest_M1_mhFeat",
                        "chest_mkh_imgBased",
                        "chest_M2_uwa_ibvFeat",
                        "bustOrChestGirth_M2_ibvFeat",
                        "chest_M1_uwa_mhFeat",
                        "bustOrChestGirth_ibv_imgBased"
                };

        classResultsRawPairs.push_back({"ChestExtra", getMeasVect(allMeasurementsDict, currExtraMeasNames, allRemainingMeasurementsDict)});

        currExtraMeasNames = {
                "waist_mkh_imgBased",
                "waistGirth_ibv_imgBased",
                "waist_M2_uwa_ibvFeat",
                "waist_uwa_imgBased",
                "waist_M1_uwa_mhFeat",
                "waistGirth_M2_ibvFeat",
                "waist_M1_uwa_ibvFeat",
                "waistGirth_M1_ibvFeat",
        };

        classResultsRawPairs.push_back({"WaistExtra", getMeasVect(allMeasurementsDict, currExtraMeasNames, allRemainingMeasurementsDict)});

        currExtraMeasNames = {
                "hips_uwa_imgBased",
                "hips_M1_uwa_mhFeat",
                "hips_M1_mhFeat",
                "hipGirthButtock_M1_ibvFeat",
                "hips_M2_uwa_ibvFeat",
                "hipGirthButtock_ibv_imgBased",
                "hips_M1_uwa_ibvFeat",
                "hips_mkh_imgBased",
                "hipGirthButtock_M2_ibvFeat",
        };

        classResultsRawPairs.push_back({"HipsExtra", getMeasVect(allMeasurementsDict, currExtraMeasNames, allRemainingMeasurementsDict)});

        currExtraMeasNames = {
                "thighs_M2_uwa_ibvFeat",
                "thighCirc_mkh_imgBased",
                "thighs_M1_uwa_ibvFeat",
                "thighCirc_M1_mhFeat",
                "thighs_M1_uwa_mhFeat",
                "maxThighGirth_M2_ibvFeat",
                "maxThighGirth_M1_ibvFeat",
                "maxThighGirth_ibv_imgBased",
        };

        classResultsRawPairs.push_back({"ThighsExtra", getMeasVect(allMeasurementsDict, currExtraMeasNames, allRemainingMeasurementsDict)});

        currExtraMeasNames = {
                "inseam_M2_uwa_ibvFeat",
                "inseam_mkh_imgBased",
                "inseam_M1_uwa_ibvFeat",
                "inseam_M1_mhFeat",
                "inseam_M1_uwa_mhFeat",
                "inseam_M2_ibvFeat",
                "inseam_M1_ibvFeat",
                "inseam_ibv_imgBased",
                "inseam_uwa_imgBased"
        };

        classResultsRawPairs.push_back({"InseamExtra", getMeasVect(allMeasurementsDict, currExtraMeasNames, allRemainingMeasurementsDict)});

        currExtraMeasNames = {
                "calfCirc_M1_mhFeat",
                "calfCirc_mkh_imgBased",
        };

        classResultsRawPairs.push_back({"CalfExtra", getMeasVect(allMeasurementsDict, currExtraMeasNames, allRemainingMeasurementsDict)});

        currExtraMeasNames = {
                "upperArmGirth_ibv_imgBased",
                "upperArmCirc_M1_mhFeat",
                "upperArmGirth_M1_ibvFeat",
                "upperArmGirth_M2_ibvFeat",
                "upperArmCirc_mkh_imgBased",
        };

        classResultsRawPairs.push_back({"BicepExtra", getMeasVect(allMeasurementsDict, currExtraMeasNames, allRemainingMeasurementsDict)});

        currExtraMeasNames = {
                "upperArmLength_ibv_imgBased",
                "upperArmLength_M1_mhFeat",
                "upperArmLength_mkh_imgBased",
                "upperArmLength_M1_ibvFeat",
                "upperArmLength_M2_ibvFeat",
        };

        classResultsRawPairs.push_back({"UpperArmLengthExtra", getMeasVect(allMeasurementsDict, currExtraMeasNames, allRemainingMeasurementsDict)});
        currExtraMeasNames = {
                "lowerArmLength_M1_mhFeat",
                "lowerArmLength_mkh_imgBased",
                "foreArmLength_M1_ibvFeat",
                "foreArmLength_M2_ibvFeat",
                "foreArmLength_ibv_imgBased",
        };

        classResultsRawPairs.push_back({"LowerArmLengthExtra", getMeasVect(allMeasurementsDict, currExtraMeasNames, allRemainingMeasurementsDict)});

        currExtraMeasNames = {
                "upperLegLength_mkh_imgBased",
                "upperLegLength_M1_mhFeat",
                "upperLegLength_ibv_imgBased",
                "upperLegLength_ibv_imgBased_",
                "upperLegLength_M1_ibvFeat",
                "upperLegLength_M1_ibvFeat_",
                "upperLegLength_M2_ibvFeat",
                "upperLegLength_M2_ibvFeat_",
        };

        classResultsRawPairs.push_back({"UpperLegLengthExtra", getMeasVect(allMeasurementsDict, currExtraMeasNames, allRemainingMeasurementsDict)});

        currExtraMeasNames = {
                "lowerLegLength_mkh_imgBased",
                "lowerLegLength_M1_mhFeat",
                "lowerLegLength_ibv_imgBased",
                "lowerLegLength_M1_ibvFeat",
                "lowerLegLength_M2_ibvFeat",
        };

        classResultsRawPairs.push_back({"LowerLegLengthExtra", getMeasVect(allMeasurementsDict, currExtraMeasNames, allRemainingMeasurementsDict)});

        currExtraMeasNames = {
                "wristCirc_mkh_imgBased",
                "wristGirth_M1_ibvFeat",
                "wristCirc_M1_mhFeat",
                "wristGirth_M2_ibvFeat",
                "wristGirth_ibv_imgBased",
        };

        classResultsRawPairs.push_back({"WristCircExtra", getMeasVect(allMeasurementsDict, currExtraMeasNames, allRemainingMeasurementsDict)});

        currExtraMeasNames = {
                "horizontalShoulderWidthBetweenAcromions_ibv_imgBased",
                "horizontalShoulderWidthBetweenAcromions_M2_ibvFeat",
                "horizontalShoulderWidthBetweenAcromions_M1_ibvFeat",
        };

        classResultsRawPairs.push_back({"ShoulderExtra", getMeasVect(allMeasurementsDict, currExtraMeasNames, allRemainingMeasurementsDict)});

        currExtraMeasNames = {
                "midNeckGirth_M1_ibvFeat",
                "midNeckGirth_M2_ibvFeat",
                "midNeckGirth_ibv_imgBased",
        };

        classResultsRawPairs.push_back({"NeckExtra", getMeasVect(allMeasurementsDict, currExtraMeasNames, allRemainingMeasurementsDict)});

        currExtraMeasNames = {
                "ankleGirth_M1_ibvFeat",
                "ankleGirth_M2_ibvFeat",
                "ankleGirth_ibv_imgBased",
                "ankleCirc_mkh_imgBased",
                "ankleCirc_M1_mhFeat",
        };

        classResultsRawPairs.push_back({"AnkleExtra", getMeasVect(allMeasurementsDict, currExtraMeasNames, allRemainingMeasurementsDict)});

        currExtraMeasNames = {
                "kneeGirth_M1_ibvFeat",
                "kneeGirth_M2_ibvFeat",
                "kneeGirth_ibv_imgBased",
                "kneeCirc_mkh_imgBased",
                "kneeCirc_M1_mhFeat",
        };

        classResultsRawPairs.push_back({"KneeExtra", getMeasVect(allMeasurementsDict, currExtraMeasNames, allRemainingMeasurementsDict)});

        currExtraMeasNames = {
                "fullBodyVolume_M1_ibvFeat",
                "fullBodyVolume_M2_ibvFeat",
                "fullBodyVolume_ibv_imgBased",
                "bodyVol_M1_ibvFeat",
                "bodyVol_M2_ibvFeat",
                "bodyVol_ibv_imgBased",
                "bodyVol_mkh_imgBased",
                "bodyVol_M1_mhFeat",
        };

        classResultsRawPairs.push_back({"VolumeExtra", getMeasVect(allMeasurementsDict, currExtraMeasNames, allRemainingMeasurementsDict)});

        currExtraMeasNames = {
                "bodyArea_M1_ibvFeat",
                "bodyArea_M2_ibvFeat",
                "bodyArea_ibv_imgBased",
                "bodyArea_mkh_imgBased",
                "bodyArea_M1_mhFeat",
        };
        classResultsRawPairs.push_back({"AreaExtra", getMeasVect(allMeasurementsDict, currExtraMeasNames, allRemainingMeasurementsDict)});


//store and colate rest of the extra meas
        for (auto measIter = allRemainingMeasurementsDict.begin(); measIter != allRemainingMeasurementsDict.end(); measIter++) {
            std::string key = measIter->first;
            float value = measIter->second;
            if (value > 0) {
                key = eraseMeasSite(key);
                std::vector<float> measVect;
                for (auto measSubIter = allRemainingMeasurementsDict.begin(); measSubIter != allRemainingMeasurementsDict.end(); measSubIter++) {
                    std::string subKey = "_" + measSubIter->first;
                    float subValue = measSubIter->second;
                    bool isFound = subKey.find("_" + key + "_") != string::npos;
                    if (isFound && subValue > 0) {
                        measVect.push_back(subValue);
                        allRemainingMeasurementsDict[subKey] = -1;
                    }
                }
                if (!measVect.empty()) {
                    classResultsRawPairs.push_back({key + "Extra", measVect});
                }

            }

        }
    }

    if (isCompositionForResults) {
        if (!FatDL.empty()) {
            classResultsRawPairs.push_back({"FatDLCurrent", FatDL});
        }

        if (!PredWeightGivenHeightGivenFeatDL.empty()) {
            classResultsRawPairs.push_back({"PredWeightGivenHeightGivenFeatExtra", PredWeightGivenHeightGivenFeatDL});
        }

        if (!PredHeightGivenWeightGivenFeatDL.empty()) {
            classResultsRawPairs.push_back({"PredHeightGivenWeightGivenFeatExtra", PredHeightGivenWeightGivenFeatDL});
        }

        if (!PredHeightGivenFeatDL.empty()) {
            classResultsRawPairs.push_back({"PredHeightGivenFeatExtra", PredHeightGivenFeatDL});
        }

        if (!PredWeightGivenFeatDL.empty()) {
            classResultsRawPairs.push_back({"PredWeightGivenFeatExtra", PredWeightGivenFeatDL});
        }
    }


    return true;
}

std::string ahiFactoryClassify::transformClassificationResultsToJson(std::map<std::string, float> resultsMap) {
    Json::Reader reader;
    Json::Value json;
    Json::FastWriter writer;
    std::string ahiClassResultsAsJsonStr;
    Json::Value childJson;

    for (auto iter = resultsMap.begin(); iter != resultsMap.end(); iter++) {
        childJson[iter->first] = iter->second;
    }
    json["ahiClassInfo"] = childJson;
    ahiClassResultsAsJsonStr = writer.write(json);
    return ahiClassResultsAsJsonStr;
}

// inference for 1 front and 1 side using all models.
ahiClassifyInfo ahiFactoryClassify::getClassifyOutInfo(double height,
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
    ahiClassifyInfo classInfo;
    // or can be fed as  "shape" or "comp"
    modelScanType = "shape_and_comp";
    // this to store all prediction which are then averaged etc.
    // I would use this outside this function and iterate over  all 4 images then average later but in this example I'm only averaging over the results of 1 front and 1 side
    std::vector<std::pair<std::string, std::vector<float>>> classResultsRawPairs;

    std::vector<double> sil_features_for_DL;
    std::vector<double> svr_class_results;
    bool isSVRSucess = ahiSVRClassification(height, weight, gender, frontSilhouette, sideSilhouette, front_joints_vector, side_joints_vector,
                                            modelScanType, sil_features_for_DL, svr_class_results, svrModels, classResultsRawPairs);

    if (!isSVRSucess || sil_features_for_DL.empty() || svr_class_results.empty()) {
        return classInfo;
    }

    // Load all TF models into memory
    auto loadedTfModels = ahiFactoryClassify::loadAllTfModels(tfModels);

    bool isDLSucess = ahiDLClassification(height, weight, gender, frontSilhouette, sideSilhouette, sil_features_for_DL, modelScanType, loadedTfModels,
                                          classResultsRawPairs);

    // Release all loaded TF models from memory
    for (auto &model: loadedTfModels) {
        model.second.release();
    }

    if (!isDLSucess || classResultsRawPairs.empty()) {
        return classInfo;
    }

    // here we take mean and stdDev and clean results
    // it is highly preferred to do these over all 4 front and 4 side images  but here we do it for a single front and a single side image
    std::string addKey = "Current";
    std::map<std::string, float> classificationResultsCurrent;
    double chestCM = classification_helper.mean_stddevPairs(classResultsRawPairs, "Chest", addKey,useAverage);
    classificationResultsCurrent[AHI_RAW_CHEST] = chestCM;
    double waistCM = classification_helper.mean_stddevPairs(classResultsRawPairs, "Waist", addKey,useAverage);
    classificationResultsCurrent[AHI_RAW_WAIST] = waistCM;
    double hipsCM = classification_helper.mean_stddevPairs(classResultsRawPairs, "Hip", addKey,useAverage);
    classificationResultsCurrent[AHI_RAW_HIPS] = hipsCM;
    double inseamCM = classification_helper.mean_stddevPairs(classResultsRawPairs, "Inseam", addKey,useAverage);
    classificationResultsCurrent[AHI_RAW_INSEAM] = inseamCM;
    double thighsCM = classification_helper.mean_stddevPairs(classResultsRawPairs, "Thigh", addKey,useAverage);
    classificationResultsCurrent[AHI_RAW_THIGH] = thighsCM;
    double weightPredicKG = classification_helper.mean_stddevPairs(classResultsRawPairs, "Weight", addKey,useAverage);
    classificationResultsCurrent[AHI_RAW_WEIGHTPRED] = weightPredicKG;
    double fatKG = classification_helper.mean_stddevPairs(classResultsRawPairs, "Fat", addKey,useAverage);
    double fatPCT = (fatKG / weight) * 100;
    classificationResultsCurrent[AHI_RAW_BODYFAT] = fatPCT;
    double ffmKG = classification_helper.mean_stddevPairs(classResultsRawPairs, "FFM", addKey,useAverage);
    classificationResultsCurrent[AHI_GEN_FFM] = ffmKG;
    double gynoidKG = classification_helper.mean_stddevPairs(classResultsRawPairs, "Gynoid", addKey,useAverage);
    classificationResultsCurrent[AHI_GEN_GYNOID] = gynoidKG;
    double androidKG = classification_helper.mean_stddevPairs(classResultsRawPairs, "Android", addKey,useAverage);
    classificationResultsCurrent[AHI_GEN_ANDROID] = androidKG;
    double vatKG = classification_helper.mean_stddevPairs(classResultsRawPairs, "VAT", addKey,useAverage);
    classificationResultsCurrent[AHI_GEN_VAT] = vatKG;

    // Confirmed by Labs to be hard coded for now.
    classificationResultsCurrent[AHI_GEN_FITNESS] = 0.8;

    classInfo.classificationResultsCurrent = classificationResultsCurrent;
    classInfo.currentClassResultsAsJson = transformClassificationResultsToJson(classificationResultsCurrent);
    return classInfo;
}


ahiClassifyInfo
ahiFactoryClassify::getClassifyOutInfoMultipleImages(double height,
                                                     double weight,
                                                     const std::string &gender,
                                                     std::vector<cv::Mat> frontSilhouettes,
                                                     std::vector<cv::Mat> sideSilhouettes,
                                                     std::vector<std::map<std::string, cv::Point2f>> frontJoints,
                                                     std::vector<std::map<std::string, cv::Point2f>> sideJoints,
                                                     std::string modelScanType,
                                                     std::map<std::string, std::pair<char *, std::size_t>> &tfModels,
                                                     std::map<std::string, std::pair<char *, std::size_t>> &svrModels,
                                                     bool useAverage) {
    ahiClassifyInfo classInfo;
    // or can be fed as  "shape" or "comp"
    modelScanType = "shape_and_comp";
    // this to store all prediction which are then averaged etc.
    // I would use this outside this function and iterate over  all 4 images then average later but in this example I'm only averaging over the results of 1 front and 1 side
    std::vector<std::pair<std::string, std::vector<float>>> classResultsRawPairs;

    // Load all TF models into memory
    auto loadedTfModels = ahiFactoryClassify::loadAllTfModels(tfModels);

    for (int idx = 0; idx < frontSilhouettes.size(); idx++) {
        cv::Mat frontSilhouette = frontSilhouettes[idx];
        cv::Mat sideSilhouette = sideSilhouettes[idx];
        map<std::string, cv::Point2f> front_joints_vector = frontJoints[idx];
        map<std::string, cv::Point2f> side_joints_vector = sideJoints[idx];

        std::vector<double> sil_features_for_DL;
        std::vector<double> svr_class_results;
        bool isSVRSucess = ahiSVRClassification(height, weight, gender, frontSilhouette,
                                                sideSilhouette, front_joints_vector,
                                                side_joints_vector, modelScanType,
                                                sil_features_for_DL, svr_class_results,
                                                svrModels, classResultsRawPairs);

        if (!isSVRSucess || sil_features_for_DL.empty() || svr_class_results.empty()) {
            return classInfo;
        }

        bool isDLSucess = ahiDLClassification(height, weight, gender, frontSilhouette,
                                              sideSilhouette, sil_features_for_DL, modelScanType,
                                              loadedTfModels, classResultsRawPairs);

        if (!isDLSucess || classResultsRawPairs.empty()) {
            return classInfo;
        }
    }

    // Release all loaded TF models from memory
    for (auto &model: loadedTfModels) {
        model.second.release();
    }

    // here we take mean and stdDev and clean results
    // it is highly preferred to do these over all 4 front and 4 side images  but here we do it for a single front and a single side image
    std::string addKey = "Current";
    std::map<std::string, float> classificationResultsCurrent;
    double chestCM = classification_helper.mean_stddevPairs(classResultsRawPairs, "Chest", addKey,useAverage);
    classificationResultsCurrent[AHI_RAW_CHEST] = chestCM;
    double waistCM = classification_helper.mean_stddevPairs(classResultsRawPairs, "Waist", addKey,useAverage);
    classificationResultsCurrent[AHI_RAW_WAIST] = waistCM;
    double hipsCM = classification_helper.mean_stddevPairs(classResultsRawPairs, "Hip", addKey,useAverage);
    classificationResultsCurrent[AHI_RAW_HIPS] = hipsCM;
    double inseamCM = classification_helper.mean_stddevPairs(classResultsRawPairs, "Inseam", addKey,useAverage);
    classificationResultsCurrent[AHI_RAW_INSEAM] = inseamCM;
    double thighsCM = classification_helper.mean_stddevPairs(classResultsRawPairs, "Thigh", addKey,useAverage);
    classificationResultsCurrent[AHI_RAW_THIGH] = thighsCM;
    double weightPredicKG = classification_helper.mean_stddevPairs(classResultsRawPairs, "Weight", addKey,useAverage);
    classificationResultsCurrent[AHI_RAW_WEIGHTPRED] = weightPredicKG;
    double fatKG = classification_helper.mean_stddevPairs(classResultsRawPairs, "Fat", addKey,useAverage);
    double fatPCT = (fatKG / weight) * 100;
    classificationResultsCurrent[AHI_RAW_BODYFAT] = fatPCT;
    double ffmKG = classification_helper.mean_stddevPairs(classResultsRawPairs, "FFM", addKey,useAverage);
    classificationResultsCurrent[AHI_GEN_FFM] = ffmKG;
    double gynoidKG = classification_helper.mean_stddevPairs(classResultsRawPairs, "Gynoid", addKey,useAverage);
    classificationResultsCurrent[AHI_GEN_GYNOID] = gynoidKG;
    double androidKG = classification_helper.mean_stddevPairs(classResultsRawPairs, "Android", addKey,useAverage);
    classificationResultsCurrent[AHI_GEN_ANDROID] = androidKG;
    double vatKG = classification_helper.mean_stddevPairs(classResultsRawPairs, "VAT", addKey,useAverage);
    classificationResultsCurrent[AHI_GEN_VAT] = vatKG;

    // Confirmed by Labs to be hard coded for now.
    classificationResultsCurrent[AHI_GEN_FITNESS] = 0.8;

    classInfo.classificationResultsCurrent = classificationResultsCurrent;
    classInfo.currentClassResultsAsJson = transformClassificationResultsToJson(classificationResultsCurrent);
    return classInfo;
}

