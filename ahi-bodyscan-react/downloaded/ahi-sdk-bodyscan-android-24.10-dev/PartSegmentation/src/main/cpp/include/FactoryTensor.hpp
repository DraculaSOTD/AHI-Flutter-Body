//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#pragma once

// typedef for joints centroid
typedef struct {
    cv::Point CentroidHeadTop, CentroidNeck, CentroidRightShoulder;
    cv::Point CentroidLeftShoulder, CentroidRightElbow, CentroidLeftElbow;
    cv::Point CentroidRightHand, CentroidLeftHand, CentroidRightHip, CentroidLeftHip;
    cv::Point CentroidRightKnee, CentroidLeftKnee, CentroidRightAnkle, CentroidLeftAnkle;
    cv::Rect Face;
    bool threeOutOfFiveRule = false;

    inline Json::Value vectorToJson() {
        Json::Value childJson;
        childJson["FaceH"] = Face.height;
        childJson["FaceW"] = Face.width;
        childJson["FaceX"] = Face.x;
        childJson["FaceY"] = Face.y;
        childJson["HeadTopX"] = CentroidHeadTop.x;
        childJson["HeadTopY"] = CentroidHeadTop.y;
        childJson["NeckX"] = CentroidNeck.x;
        childJson["NeckY"] = CentroidNeck.y;
        childJson["RightShoulderX"] = CentroidRightShoulder.x;
        childJson["RightShoulderY"] = CentroidRightShoulder.y;
        childJson["LeftShoulderX"] = CentroidLeftShoulder.x;
        childJson["LeftShoulderY"] = CentroidLeftShoulder.y;
        childJson["RightElbowX"] = CentroidRightElbow.x;
        childJson["RightElbowY"] = CentroidRightElbow.y;
        childJson["LeftElbowX"] = CentroidLeftElbow.x;
        childJson["LeftElbowY"] = CentroidLeftElbow.y;
        childJson["RightHandX"] = CentroidRightHand.x;
        childJson["RightHandY"] = CentroidRightHand.y;
        childJson["LeftHandX"] = CentroidLeftHand.x;
        childJson["LeftHandY"] = CentroidLeftHand.y;
        childJson["RightHipX"] = CentroidRightHip.x;
        childJson["RightHipY"] = CentroidRightHip.y;
        childJson["LeftHipX"] = CentroidLeftHip.x;
        childJson["LeftHipY"] = CentroidLeftHip.y;
        childJson["RightKneeX"] = CentroidRightKnee.x;
        childJson["RightKneeY"] = CentroidRightKnee.y;
        childJson["LeftKneeX"] = CentroidLeftKnee.x;
        childJson["LeftKneeY"] = CentroidLeftKnee.y;
        childJson["RightAnkleX"] = CentroidRightAnkle.x;
        childJson["RightAnkleY"] = CentroidRightAnkle.y;
        childJson["LeftAnkleX"] = CentroidLeftAnkle.x;
        childJson["LeftAnkleY"] = CentroidLeftAnkle.y;
        childJson["RightAnkleX"] = CentroidRightAnkle.x;
        childJson["RightAnkleY"] = CentroidRightAnkle.y;
        return childJson;
    }

} Joints;

// typedef for silhouette extremas
typedef struct {
    int xLeft, yLeft, xRight, yRight, xCenter, yCenter;
    int xBottom, yBottom, xTop, yTop, xBottomLeft, xBottomRight, yBottomLeft, yBottomRight;
    int xContourFaceCenter, yContourFaceCenter, ContourFaceWidth, ContourFaceHeight, FaceDistThrdForInspection;
} Extremas;

// typedef for the front Pose
typedef struct {
    float HeadTopOverlapRatio, LeftHandOverlapRatio, RightHandOverlapRatio, LeftLegOverlapRatio, RightLegOverlapRatio;
    cv::Rect HeadTopFromML, HeadTopFromTL, FaceDetectedFromML;
    cv::Rect LeftHandFromML, RightHandFromML, LeftLegFromML, RightLegFromML;
    cv::Rect LeftHandFromTL, RightHandFromTL, LeftLegFromTL, RightLegFromTL;
    bool HeadInContour = false, LeftHandInContour = false, RightHandInContour = false, LeftLegInContour = false, RightLegInContour = false;
    bool FaceInExpectedContour = false, CameraIsPotentiallyHigh = false, CameraIsPotentiallyLow = false;
} FrontPose;

// typedef for the side pose
typedef struct {
    float HeadTopOverlapRatio, HandOverlapRatio, LegOverlapRatio;
    cv::Rect HeadTopFromML, HeadTopFromTL, FaceDetectedFromML;
    cv::Rect HandFromML, HandFromTL, LegFromML, LegFromTL;
    bool HeadInContour = false, HandInContour = false, LegInContour = false;
    bool FaceInExpectedContour = false, CameraIsPotentiallyHigh = false, CameraIsPotentiallyLow = false;
} SidePose;

// json object structure for pose results
typedef struct {
    bool GE = false;
    bool LA = false;
    bool RA = false;
    bool LL = false;
    bool RL = false;
    bool BG = true;
    bool DP = true;
    bool UB = false;
    bool LB = false;
    bool FaceInExpectedContour = false;
    bool CameraIsPotentiallyHigh = false;
    bool CameraIsPotentiallyLow = false;
    int Face = 0;
    std::string view = "";

} JsonPose;

typedef enum FactoryTensorOpType {
    FactoryTensorOpInit,

    FactoryTensorOpLoadPose,
    FactoryTensorOpInitPose,
    FactoryTensorOpPose,

    FactoryTensorOpLoadJoints,
    FactoryTensorOpInitJoints,
    FactoryTensorOpJoints,

    FactoryTensorOpLoadSegment,
    FactoryTensorOpInitSegment,
    FactoryTensorOpSegment,

    FactoryTensorOpLoadSegment256,
    FactoryTensorOpInitSegment256,
    FactoryTensorOpSegment256,

    FactoryTensorOpLoadSegment512,
    FactoryTensorOpInitSegment512,
    FactoryTensorOpSegment512,

    FactoryTensorOpLoadClassV1,
    FactoryTensorOpInitClassV1,
    FactoryTensorOpClassV1,

    FactoryTensorOpLoadClassV2male,
    FactoryTensorOpInitClassV2male,
    FactoryTensorOpClassV2male,

    FactoryTensorOpLoadClassV2female,
    FactoryTensorOpInitClassV2female,
    FactoryTensorOpClassV2female,

    FactoryTensorOpLoadClassV2p5,
    FactoryTensorOpInitClassV2p5,
    FactoryTensorOpClassV2p5,

    FactoryTensorOpLoadClassV3male,
    FactoryTensorOpInitClassV3male,
    FactoryTensorOpClassV3male,

    FactoryTensorOpLoadClassV3female,
    FactoryTensorOpInitClassV3female,
    FactoryTensorOpClassV3female,

    FactoryTensorOpLoadClassV3p1,
    FactoryTensorOpInitClassV3p1,
    FactoryTensorOpClassV3p1,

    FactoryTensorOpLoadClassTBFIM1,
    FactoryTensorOpInitClassTBFIM1,
    FactoryTensorOpClassTBFIM1,

    FactoryTensorOpLoadClassTBFIM2,
    FactoryTensorOpInitClassTBFIM2,
    FactoryTensorOpClassTBFIM2,

    FactoryTensorOpLoadAHIPose,
    FactoryTensorOpInitAHIPose,
    FactoryTensorOpAHIPose,

    FactoryTensorOpLoadExmeasImageBased,
    FactoryTensorOpInitExmeasImageBased,
    FactoryTensorOpExmeasImageBased,

    FactoryTensorOpRelease
} FactoryTensorOpType;

typedef enum ModelGender {
    male,
    female,
    either
} ModelGender;

class ModelLookup {
public:
    FactoryTensorModelId _id;
    FactoryTensorOpType _load;
    FactoryTensorOpType _init;
    FactoryTensorOpType _run;
    const char *_name;
    bool _bPreload;
    bool _bClassify;
    ModelGender _gender;
    void *_pData;
    size_t _dataSize;

    ModelLookup(const char *n, bool preload, bool classify, ModelGender gen, void *data,
                size_t dataSz)
            : _name(n), _bPreload(preload), _bClassify(classify), _gender(gen), _pData(data),
              _dataSize(dataSz) {};

    ModelLookup(FactoryTensorModelId id, FactoryTensorOpType l, FactoryTensorOpType i,
                FactoryTensorOpType r, const char *n, bool preload, bool classify, ModelGender gen,
                void *data, size_t dataSz)
            : _id(id), _load(l), _init(i), _run(r), _name(n), _bPreload(preload),
              _bClassify(classify), _gender(gen), _pData(data), _dataSize(dataSz) {};
};

typedef enum FactoryTensorOpFlag {
    // On device classification.
    FactoryTensorOpFlagOnDevice = 1
} FactoryTensorOpFlag;

class FactoryTensorOp {
public:
    FactoryTensorOpType mOp;
    uint64_t mFlags;
    TensorInputMap mInputs;
    TensorOutputMap mOutputs;
    bool mInvokeSuccess;
    Queue <std::unique_ptr<FactoryTensorOp>> *mResultQueue;

    FactoryTensorOp(FactoryTensorOpType op)
            : mOp(op), mFlags(0), mInputs(), mOutputs(), mInvokeSuccess(false),
              mResultQueue(nullptr) {

    };

    FactoryTensorOp(FactoryTensorOpType op, uint64_t flags)
            : mOp(op), mFlags(flags), mInputs(), mOutputs(), mInvokeSuccess(false),
              mResultQueue(nullptr) {

    };

    FactoryTensorOp(FactoryTensorOpType op, Queue <std::unique_ptr<FactoryTensorOp>> *resultQueue)
            : mOp(op), mFlags(0), mInputs(), mOutputs(), mInvokeSuccess(false),
              mResultQueue(resultQueue) {

    };

    FactoryTensorOp(FactoryTensorOpType op, cv::Mat tensorMat,
                    Queue <std::unique_ptr<FactoryTensorOp>> *resultQueue)
            : mOp(op), mFlags(0), mInputs(), mOutputs(), mInvokeSuccess(false),
              mResultQueue(resultQueue) {
        // assume single input and ignore name.
        mInputs["0"] = TensorInput(tensorMat);
    };

    FactoryTensorOp(FactoryTensorOpType op, std::string name, cv::Mat tensorMat,
                    Queue <std::unique_ptr<FactoryTensorOp>> *resultQueue)
            : mOp(op), mFlags(0), mInputs(), mOutputs(), mInvokeSuccess(false),
              mResultQueue(resultQueue) {
        // assume single input and ignore name.
        mInputs[name] = TensorInput(tensorMat);
    };

    FactoryTensorOp(FactoryTensorOpType op, std::string name, TensorInput tensorInput,
                    Queue <std::unique_ptr<FactoryTensorOp>> *resultQueue)
            : mOp(op), mFlags(0), mInputs(), mOutputs(), mInvokeSuccess(false),
              mResultQueue(resultQueue) {
        // assume single input and ignore name.
        mInputs[name] = tensorInput;
    };

    TensorInput &getInput(std::string name) {
        return mInputs[name];
    }

    void addInput(std::string name, TensorInput input) {
        mInputs[name] = input;
    }

    bool operator<(const FactoryTensorOp &lhs) {
        return mOp > lhs.mOp;
    }

    uint64_t flags() {
        return mFlags;
    }

    bool hasOutput(int ix) {
        if (ix >= 0 && ix < mOutputs.size())
            return true;

        return false;
    }

    // legacy support - get output by index.
    cv::Mat output(int ix) {
        if (hasOutput(ix)) {
            auto i = 0;
            for (auto iter = mOutputs.begin(); iter != mOutputs.end(); iter++) {
                if (i == ix) {
                    return iter->second._mat;
                }
                i++;
            }
        }

        return cv::Mat();
    }

    TensorOutput output(std::string name) {
        auto iter = mOutputs.find(name);
        if (iter != mOutputs.end()) {
            return mOutputs.at(name);
        }

        return TensorOutput();
    }
};
