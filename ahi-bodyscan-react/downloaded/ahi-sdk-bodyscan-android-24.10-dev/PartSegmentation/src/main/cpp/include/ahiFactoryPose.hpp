//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#ifndef ahiFactoryPose_H_
#define ahiFactoryPose_H_

#include "ahiFactoryInspection.hpp"
#include "ahiFactoryTensor.hpp"

class ahiFactoryPose {
public:
    ahiFactoryPose() = default;

    void initPose();

    bool isPoseInit;

    void getFactorTensorInstant();

    ahiFactoryTensor poseFT;

    bool ahiMoveNetPose(ahiPoseInfo &jointsPrediction);

    bool ahiPoseLight(ahiPoseInfo &jointsPrediction);

    std::vector<float> mlkitPoseData;

    bool mlkitPose(ahiPoseInfo &jointsPrediction);

    bool loadTensorFlowPoseModelFromBufferOrFile(const char *buffer, std::size_t buffer_size,
                                                 std::string modelFileName);

    bool feedInputBufferImageToCppToPose(const void *data, cv::Mat mat);

    bool getPoseInfoOutputs(cv::Mat image, ahiPoseInfo &poseInfoPredictions);

    std::string modelFileName;
    cv::Mat origImageMat;
    int originalImageHeight;
    int originalImageWidth;
    int originalImageNumOfChannels;
    bool isPaddedForResize;
    cv::Rect faceRect; // from mlkit or can be used for others
};

#endif
