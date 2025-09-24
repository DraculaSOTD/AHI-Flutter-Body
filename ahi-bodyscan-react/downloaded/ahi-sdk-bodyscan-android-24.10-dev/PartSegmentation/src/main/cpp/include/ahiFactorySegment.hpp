//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#ifndef ahiFactorySegment_H_
#define ahiFactorySegment_H_

#include "ahiFactoryInspection.hpp"
#include "ahiFactoryTensor.hpp"

typedef struct {
    cv::Mat segmentMask;
    cv::Mat segmentDLMask;
    cv::Mat segmentMlKitMask;
    std::string view;
    std::string segErrMsg;
    std::string segUsed;
} ahiSegmentInfo;

class ahiFactorySegment {
public:
    ahiFactorySegment() = default;

    void initSegment();

    bool isSegmentInit;

    void getFactorTensorInstant();

    ahiFactoryTensor segmentFT;

    std::string to_lowerStr(std::string str);

    bool loadTensorFlowSegmentModelFromBufferOrFile(const char *buffer, std::size_t buffer_size,
                                                    std::string modelFileName);

    bool feedInputBufferImageToCppToSegment(const void *data, cv::Mat mat);

    bool ahiDLSegment(ahiSegmentInfo &segInfo);

    bool getSegmentOutInfo(cv::Mat image, cv::Mat contourMask, ahiPoseInfo poseInfoPredictions,
                           std::string viewStr, ahiSegmentInfo &segInfo);

    cv::Mat getBiggestBlob(cv::Mat const matImage_orig);

    bool mlkitSegment(ahiSegmentInfo &segInfo);

    std::string modelFileName;
    cv::Mat origImageMat;
    int originalImageHeight;
    int originalImageWidth;
    int originalImageNumOfChannels;
    bool isPaddedForResize;

    cv::Mat mlkitSegmentData;
};

#endif
