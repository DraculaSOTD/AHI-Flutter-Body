//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#ifndef ahiFactoryFace_H_
#define ahiFactoryFace_H_

#include <opencv2/core/mat.hpp>
#include <opencv2/opencv.hpp>

#include "ahiFactoryTensor.hpp"

typedef struct {
    std::vector <cv::Rect> detectedFaces;
    cv::Rect detectedBiggestFaces;
    int numOfDetectedFaces;
    std::string detectionMethodOrModel;
    std::vector<float> faceLandmarks;
    std::string view = "";
} ahiFaceInfo;

typedef struct {
    int faceX = 0;
    int faceY = 0;
    int faceHeight = 0;
    int faceWidth = 0;
    int numOfDetectedFaces = 0;
    cv::Rect faceRect;
    std::vector <cv::Rect> detectedFaces;
    cv::Rect detectedBiggestFaces;
    std::string detectionMethodOrModel;
    std::vector<float> faceLandmarks;
    std::string view = "";
} mlKitFaceInfo;

class ahiFactoryFace {
public:

    ahiFactoryFace() = default;

    void initFace();

    bool isFaceInit;

    void getFactorTensorInstant();

    ahiFactoryTensor faceFT;

    std::string to_lowerStr(std::string str);

    bool loadTensorFlowFaceModelFromBufferOrFile(const char *buffer, std::size_t buffer_size,
                                                 std::string modelFileName);

    bool feedInputBufferImageToCppToFace(const void *data, cv::Mat mat);

    bool ahiDLFace(ahiFaceInfo &faceInfoPredictions);

    bool mlkitFace();

    std::vector<float> mlkitFaceData;
    mlKitFaceInfo mlkitFaceDataInfo;

    bool getFaceInfoOutputs(cv::Mat image, ahiFaceInfo &faceInfoPredictions);

    void detectFaceUsingOpencv(cv::Mat const &faceImage, cv::CascadeClassifier &model,
                               bool isGray,
                               bool doFlipping, std::vector <cv::Rect> &outputFaces);

    void detectFaceCV(cv::Mat const &faceImage, std::vector <cv::Rect> &outputFaces);

    std::string modelFileName;
    cv::Mat origImageMat;
    int originalImageHeight;
    int originalImageWidth;
    int originalImageNumOfChannels;
    bool isPaddedForResize;
    cv::Rect faceRect; // from mlkit or can be used for others
    int numOfDetectedFaces; // from mlkit or can be used for others
};

#endif
