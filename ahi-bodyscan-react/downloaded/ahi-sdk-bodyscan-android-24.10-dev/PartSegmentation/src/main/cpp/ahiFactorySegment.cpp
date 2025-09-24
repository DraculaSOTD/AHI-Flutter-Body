//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#include "ahiFactorySegment.hpp"

#include <iostream>
#include <opencv2/imgproc/types_c.h>

#include "AHIAvatarGenSegmentationJointsHelper.hpp"

std::string ahiFactorySegment::to_lowerStr(std::string str) {
    std::for_each(str.begin(), str.end(), [](char &c) {
        c = ::tolower(c);
    });
    return str;
}

void ahiFactorySegment::getFactorTensorInstant() {
    ahiFactoryTensor segmentFT;
    isSegmentInit = true;
}

void ahiFactorySegment::initSegment() {
    getFactorTensorInstant();
}

bool ahiFactorySegment::mlkitSegment(ahiSegmentInfo &segInfo) {
    segInfo.segmentMlKitMask = mlkitSegmentData;
    if (mlkitSegmentData.empty()) {
        return false;
    }
    return true;
}

bool ahiFactorySegment::ahiDLSegment(ahiSegmentInfo &segInfo) {
    if (!isSegmentInit) {
        initSegment();
    }
    // now we use ML to get the pose/joints, in this case this is a movenet type model
    std::vector<std::string> OutputNames = segmentFT.mOutputNames;
    ahiTensorOutputMap outputs;
    bool invokeSuccess = segmentFT.invokeMIMO(segmentFT.mInputs, outputs);
    if (!invokeSuccess) {
        segInfo.segErrMsg = "Segmentation Invoke Failed";
        return false;
    }
    int ix = 0; // we have one output here hence 0
    auto outputName = segmentFT.mOutputNames[ix];
    int pWidth = 256;
    int pHeight = 256;
    cv::Mat OutResult;
    for (auto outIter = outputs.begin(); outIter != outputs.end(); outIter++) {
        std::string currModelOutNodeName = outIter->first;
        std::cout << "Current output node name :" << currModelOutNodeName << "\n";
        OutResult = outIter->second._mat;
    }
    cv::Mat segMask(pHeight, pWidth, CV_32F);
    int t = 0;
    for (int y = 0; y < pHeight; ++y) {
        for (int x = 0; x < pWidth; ++x) {
            segMask.at<float>(cv::Point(x, y)) = (float) OutResult.at<cv::Vec4f>(0, y, x)[t];
        }
    }
    cv::Point min_loc, max_loc;
    double min_val, max_val;
    cv::minMaxLoc(segMask.clone(), &min_val, &max_val, &min_loc, &max_loc);// get peak location
    segMask = 255. * segMask / (1.0e-10 + max_val);
    segMask.convertTo(segMask, CV_8U);
    int threshold = 0;
    cv::threshold(segMask, segMask, threshold, 255, cv::THRESH_BINARY + cv::THRESH_OTSU);
    cv::Size origSize(originalImageWidth, originalImageHeight);
    cv::resize(segMask, segMask, origSize, CV_INTER_CUBIC);
    segInfo.segmentDLMask = segMask;
    return true;
}

bool ahiFactorySegment::loadTensorFlowSegmentModelFromBufferOrFile(const char *buffer,
                                                                   std::size_t buffer_size,
                                                                   std::string modelFileName) {
    if (!isSegmentInit) {
        initSegment();
    }

    segmentFT.modelFileName = modelFileName;

    if (to_lowerStr(modelFileName).find("mlkit") != std::string::npos) {
        return true;
    }
    if (buffer_size > 10)  // model as buffer from Java or elsewhere
    {
        segmentFT.mModel = tflite::FlatBufferModel::BuildFromBuffer(buffer, buffer_size);
        bool saveSucess = segmentFT.saveModelBufferOnDevice(modelFileName, buffer, buffer_size);
    } else if (modelFileName.size() > 0 && buffer_size < 10) // full model name path is supplied
    {
        // here I can use the list of pose tflite models and load them. We need the full path of the model here
        segmentFT.mModel = tflite::FlatBufferModel::BuildFromFile(
                modelFileName.c_str());// VerifyAndBuildFromFile
        //if above doesn't pass then we can try to download it here, save to cache/data folder and update the class modelFilename (full path)
    } else if (modelFileName.size() < 0 && buffer_size < 10) // no buffer or file name
    {
        return false;
    }

    segmentFT.buildOptimalInterpreter();
    if (segmentFT.mModel != nullptr) {
        segmentFT.GetModelInpOutNames();
    }
    return segmentFT.mModel != nullptr;
}

bool ahiFactorySegment::feedInputBufferImageToCppToSegment(const void *data, cv::Mat mat) {
    try {
        if (!isSegmentInit) {
            return false;
        }
        origImageMat = mat.clone();
        originalImageHeight = mat.rows;
        originalImageWidth = mat.cols;
        originalImageNumOfChannels = mat.channels();
        if (to_lowerStr(modelFileName).find("mlkit") != std::string::npos) {
            return true;
        }
        std::vector<std::string> InputNames = segmentFT.mInputNames;
        ahiTensorInputMap mInputs;
        if (InputNames.empty()) {
            return false;
        }
        int pWidth = segmentFT.getInputDim(0, 1);
        int pHeight = segmentFT.getInputDim(0, 2);
        cv::Size targetSize(pWidth, pHeight); // TF model input image size

        int top, bottom, left, right;
        bool toBGR;
        bool doPadding;
        bool toF32;

        toBGR = true;
        doPadding = false;
        toF32 = true; // this can done automatic by comparing type
        isPaddedForResize = doPadding;
        segmentFT.isPaddedForResize = doPadding;
        cv::Mat preprocessed_image = segmentFT.processImageWorWoutPadding(mat, targetSize, top,
                                                                          bottom, left, right,
                                                                          toBGR, doPadding, toF32);
        preprocessed_image = preprocessed_image / 255.0;
        segmentFT.addInput(InputNames[0],
                           ahiTensorInput(preprocessed_image, true, false, targetSize, {1.0 / 255},
                                          {0.}));
        return true;
    }
    catch (std::exception e) {
        return false;
    }
    return false;
}

cv::Mat ahiFactorySegment::getBiggestBlob(cv::Mat const matImage_orig) {
    try {
        cv::Mat matImage = matImage_orig.clone();
        if (matImage.channels() > 1) {
            cv::cvtColor(matImage, matImage, cv::COLOR_BGR2GRAY);
        }
        double largest_area = 0;
        int largest_contour_index = 0;
        cv::Mat BiggestBlob = cv::Mat::zeros(matImage.size(), matImage.type());
        std::vector<std::vector<cv::Point> > contours; // Vector for storing contour
        std::vector<cv::Vec4i> hierarchy;
        findContours(matImage.clone(), contours, hierarchy, cv::RETR_CCOMP,
                     cv::CHAIN_APPROX_SIMPLE); // Find the contours in the image
        for (int i = 0; i < contours.size(); i++) { // iterate through each contour.
            double a = contourArea(contours[i]);  // Find the area of contour
            if (a > largest_area) {
                largest_area = a;
                largest_contour_index = i;                // Store the index of largest contour
            }
        }
        drawContours(BiggestBlob, contours, largest_contour_index, cv::Scalar(255),
                     -1); // Draw the largest contour using previously stored index.
        return BiggestBlob;
    }
    catch (...) {
        return matImage_orig;
    }
}

bool ahiFactorySegment::getSegmentOutInfo(cv::Mat image, cv::Mat contourMask,
                                          ahiPoseInfo poseInfoPredictions, std::string viewStr,
                                          ahiSegmentInfo &segInfo) {
    bool segSuccess = false;
    // here is the actual in device segmentation
    ahi_avatar_gen::joints_helper AHI_JH;

    cv::Mat silhouette;

    BodyScanCommon::Profile viewType;
    if (to_lowerStr(viewStr) == "front") {
        viewType = BodyScanCommon::Profile::front;
        poseInfoPredictions.view = "front";
    } else {
        viewType = BodyScanCommon::Profile::side;
        poseInfoPredictions.view = "side";
    }

    if (to_lowerStr(segInfo.segUsed).find("mlkit") != std::string::npos) {
        bool successMlkit = mlkitSegment(segInfo);
        segInfo.segmentMask = segInfo.segmentMlKitMask;
        return successMlkit; // below is grabCut seems working but for testing I will return mlkit Mask

        if (successMlkit) {
            //iOS ver
            //silhoutte = AHI_JH.segment_using_net_joints_and_grabcut(image, viewType, segInfo.segmentMlkitMask,poseInfoPredictions.tranformToCvJoints());

            // Android ver
            silhouette = AHI_JH.segment_using_net_joints_and_grabcut_and_contourmask(image,
                                                                                     viewType,
                                                                                     segInfo.segmentMlKitMask,
                                                                                     poseInfoPredictions.tranformToCvJoints(),
                                                                                     contourMask);
            if (!silhouette.empty()) {
                segInfo.segmentMask = getBiggestBlob(silhouette);
            } else {
                segInfo.segmentMask = getBiggestBlob(segInfo.segmentDLMask);
            }
        }
        segSuccess = successMlkit && !segInfo.segmentMask.empty();
        return segSuccess;
    } else {
        if (!isSegmentInit) {
            initSegment();
        }
        if (origImageMat.empty() || !image.empty()) {
            feedInputBufferImageToCppToSegment(nullptr, image);
        }
        bool segDLSuccess = ahiDLSegment(segInfo);
        segInfo.segmentMask = segInfo.segmentDLMask; // just the default
        if (segDLSuccess) {
            // iOS ver
            // silhouette = MFZ_JH.segment_using_net_joints_and_grabcut(image, viewType, segInfo.segmentDLMask,poseInfoPredictions.tranformToCvJoints());
            // Android ver
            silhouette = AHI_JH.segment_using_net_joints_and_grabcut_and_contourmask(image,
                                                                                     viewType,
                                                                                     segInfo.segmentDLMask,
                                                                                     poseInfoPredictions.tranformToCvJoints(),
                                                                                     contourMask);
            if (!silhouette.empty()) {
                segInfo.segmentMask = getBiggestBlob(silhouette);
            } else {
                segInfo.segmentMask = getBiggestBlob(segInfo.segmentDLMask);
            }
        }
        segSuccess = segDLSuccess & !segInfo.segmentMask.empty();
        return segSuccess;
    }
}