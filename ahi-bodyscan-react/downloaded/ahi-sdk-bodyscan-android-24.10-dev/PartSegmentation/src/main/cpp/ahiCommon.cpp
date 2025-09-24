//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#include "ahiCommon.hpp"

#include "ahiFactoryFace.hpp"
#include "Logging.hpp"

inline void dummy() {}

#define RETURN_IF_TF_FAIL(expr, ret)  \
    if ((expr) != kTfLiteOk) {        \
      LOG_ERROR_HERE;                 \
      return ret;                     \
    }
#define RETURN_FALSE_IF_TF_FAIL(expr) RETURN_IF_TF_FAIL(expr, false)

std::string to_lower(std::string str) {
    std::for_each(str.begin(), str.end(), [](char &c) {
        c = ::tolower(c);
    });
    return str;
}

ahiFactoryFace FFd;
ahiFactoryPose FP;
ahiFactorySegment FSeg;

bool ahiCommon::loadTensorFlowModelFromBuffer(const char *buffer, std::size_t buffer_size,
                                              std::string modelFileName) {
    currModelFileName = modelFileName;
    if (to_lower(modelFileName).find("mlkitface") != std::string::npos) {
        FFd.initFace();
        FFd.modelFileName = "mlkitFace";
        FFd.mlkitFaceDataInfo = mlkitFaceData;
        return true;
    }
    if (to_lower(modelFileName).find("mlkitpose") != std::string::npos) {
        FP.initPose();
        FP.modelFileName = "mlkitPose";
        FP.mlkitPoseData = mlkitPoseData;
        return true;
    }
    if (to_lower(modelFileName).find("mlkitseg") != std::string::npos) {
        FSeg.initSegment();
        FSeg.modelFileName = "mlkitSeg";
        FSeg.mlkitSegmentData = mlkitSegmentData;
        return true;
    }
    std::string modelPath = "/data/data/io.ahi.example/"; // just for the moment until we decide where to store it
    if (to_lower(modelFileName).find("face") != std::string::npos ||
        to_lower(modelFileName).find("opencv") != std::string::npos) {
        FFd.initFace();
        if (to_lower(modelFileName).find("opencv") != std::string::npos) {
            FFd.modelFileName = modelFileName;
            return true; // load is success because we assume opencv will handle this later in face detection
        }
        if (buffer_size <
            10) // if kotlin or java does not feed the model buffer, use one of ahi models from the list (assuming it was saved previously)
        {
            std::vector<std::string> generalModelList = FFd.faceFT.getModelFilesList("", "");
            modelFileName = FFd.faceFT.pickModelFromList(generalModelList, "face");
            // we check if the model exist, if not then we can download it and save it then
        }
        if (modelFileName.find("/") ==
            std::string::npos) { // if the filename does not have the full path. I'm only checking for "/" but you can do any other test here
            modelFileName = modelPath + modelFileName; // update
        }
        FFd.modelFileName = modelFileName;
        bool loadModelSucess = FFd.loadTensorFlowFaceModelFromBufferOrFile(buffer, buffer_size,
                                                                           modelFileName);
        return loadModelSucess;
    }
    if (to_lower(modelFileName).find("pose") != std::string::npos) {
        FP.initPose();
        if (buffer_size <
            10) // if kotlin or java does not feed the model buffer, use one of ahi models from the list
        {
            std::vector<std::string> generalModelList = FP.poseFT.getModelFilesList("", "");
            modelFileName = FP.poseFT.pickModelFromList(generalModelList, "pose");
            // we check if the model exist, if not then we can download it
        }
        if (modelFileName.find("/") ==
            std::string::npos) { // if the filename does not have the full path. I'm only checking for "/" but you can do any other test here
            modelFileName = modelPath + modelFileName; // update
        }
        FP.modelFileName = modelFileName;
        bool loadModelSucess = FP.loadTensorFlowPoseModelFromBufferOrFile(buffer, buffer_size,
                                                                          modelFileName);
        return loadModelSucess;
    }
    if (to_lower(modelFileName).find("seg") != std::string::npos) {
        FSeg.initSegment();
        if (buffer_size <
            10) // if kotlin or java does not feed the model buffer, use one of ahi models from the list
        {
            std::vector<std::string> generalModelList = FSeg.segmentFT.getModelFilesList("", "");
            modelFileName = FSeg.segmentFT.pickModelFromList(generalModelList, "seg");
            // we check if the model exist, if not then we can download it
        }
        if (modelFileName.find("/") ==
            std::string::npos) { // if the filename does not have the full path. I'm only checking for "/" but you can do any other test here
            modelFileName = modelPath + modelFileName; // update
        }
        FSeg.modelFileName = modelFileName;
        bool loadModelSucess = FSeg.loadTensorFlowSegmentModelFromBufferOrFile(buffer, buffer_size,
                                                                               modelFileName);
        return loadModelSucess;
    }
    return false;
}

bool
ahiCommon::feedInputBufferImageToCpp(const void *data, cv::Mat mat, std::string toMLProcessName) {
    try {
        if (!FFd.isFaceInit && !FP.isPoseInit &&
            !FSeg.isSegmentInit) // just for this POC but if you are doing face detection only then check FFd
        {
            return false;
        }
        origImageMat = mat.clone();
        originalImageHeight = mat.rows;
        originalImageWidth = mat.cols;
        originalImageNumOfChannels = mat.channels();
        bool feedImageSuccess;
        if (toMLProcessName.find("face") != std::string::npos) {
            FFd.modelFileName = toMLProcessName;
            feedImageSuccess = FFd.feedInputBufferImageToCppToFace(data, mat);
            return feedImageSuccess;
        }
        if (to_lower(toMLProcessName).find("pose") != std::string::npos) {
            FP.modelFileName = toMLProcessName;
            feedImageSuccess = FP.feedInputBufferImageToCppToPose(data, mat);
            return feedImageSuccess;
        }
        if (to_lower(toMLProcessName).find("seg") != std::string::npos) {
            FSeg.modelFileName = toMLProcessName;
            feedImageSuccess = FSeg.feedInputBufferImageToCppToSegment(data, mat);
            return feedImageSuccess;
        }
    }
    catch (std::exception e) {
        return false;
    }
    return false;
}

bool ahiCommon::feedMlKitPoseToCpp(std::vector<float> mlkitPoseResultsToCpp) {
    try {
        mlkitPoseData = mlkitPoseResultsToCpp;
        poseDetectionTypeName = "mlkitPose";
        FP.mlkitPoseData = mlkitPoseData;
        FP.modelFileName = "mlkitPose";
        return true;
    }
    catch (std::exception e) {
        return false;
    }
}

bool ahiCommon::feedMlKitSegmentToCpp(cv::Mat mlkitSegResultsToCpp) {
    try {
        mlkitSegmentData = mlkitSegResultsToCpp;
        segmentTypeName = "mlkit";
        FSeg.mlkitSegmentData = mlkitSegResultsToCpp;
        FSeg.modelFileName = "mlkitSeg";
        return true;
    }
    catch (std::exception e) {
        return false;
    }
}

bool ahiCommon::feedMlKitFaceToCpp(int faceX, int faceY, int faceHeight, int faceWidth,
                                   int numOfDetectedFaces) {
    try {
        mlkitFaceData.numOfDetectedFaces = numOfDetectedFaces;
        mlkitFaceData.faceX = faceX;
        mlkitFaceData.faceY = faceY;
        mlkitFaceData.faceWidth = faceWidth;
        mlkitFaceData.faceHeight = faceHeight;
        mlkitFaceData.faceRect = cv::Rect(faceX, faceY, faceHeight, faceWidth);
        mlkitFaceData.detectionMethodOrModel = "mlkitFace";
        return true;
    }
    catch (std::exception e) {
        return false;
    }
}

std::vector<cv::Point>
ahiCommon::calcScaledContourPoints(std::vector<cv::Point> originalContourPoints, float headTopY,
                                   float ankleY, cv::Mat &scaledContourMat) {
    float minContourX = 10000;
    float maxContourX = -1;
    float minContourY = 10000;
    float maxContourY = -1;
    int L = originalContourPoints.size();
    for (int n = 0; n < L; n += 2) {
        minContourX = MIN(minContourX, originalContourPoints[n].x);
        maxContourX = MAX(maxContourX, originalContourPoints[n].x);
        minContourY = MIN(minContourY, originalContourPoints[n].y);
        maxContourY = MAX(maxContourY, originalContourPoints[n].y);
    }
    float contourAnkleY = 0.9f * maxContourY + 0.1f * minContourY;
    float scale = 1.02 * (ankleY - headTopY) / (contourAnkleY - minContourY);
    if (scale * (maxContourX - 720.0f / 2) + 720.0f / 2 > 720) {
        scale = (720.0f / 2) / (maxContourX - 720.0f / 2);
    }
    std::vector<cv::Point> scaledContourPoints;
    for (int n = 0; n < L; n++) {
        float Px = scale * (originalContourPoints[n].x - scaledContourMat.cols / 2) +
                   scaledContourMat.cols / 2;
        float Py = scale * (originalContourPoints[n].y - minContourY) + 0.9f * headTopY;
        scaledContourPoints.push_back(cv::Point(Px, Py));
    }
    std::vector<std::vector<cv::Point> > contours(1);
    contours[0] = scaledContourPoints;
    cv::drawContours(scaledContourMat, contours, 0, 255, 3);
    return scaledContourPoints;
}

bool ahiCommon::detectFace(cv::Mat image, ahiFaceInfo &faceInfo) {
    originalImageWidth = image.cols;
    originalImageHeight = image.rows;
    // face detection
    std::string viewStr = faceInfo.view;
    faceInfo.detectedFaces.clear();
    if (FFd.modelFileName.find("mlkit") != std::string::npos ||
        faceInfo.detectionMethodOrModel.find("mlkit") != std::string::npos) {
        faceInfo.numOfDetectedFaces = mlkitFaceData.numOfDetectedFaces;
        faceInfo.detectedFaces.push_back(mlkitFaceData.faceRect);
        faceInfo.detectionMethodOrModel = "mlkit";
        return true;
    }
    if (to_lower(FFd.modelFileName).find("opencv") != std::string::npos ||
        FFd.modelFileName.size() < 1) // opencv
    {
        std::vector<cv::Rect> outputFaces;
        FFd.detectFaceCV(image, outputFaces);
        faceInfo.detectedFaces = outputFaces;
        faceInfo.numOfDetectedFaces = outputFaces.size();
        faceInfo.detectionMethodOrModel = "openCV";
        FFd.modelFileName = "openCV";
        return true;
    }
    // below is for any additional face detection or landmarks models that we can run on C++
    if (FFd.modelFileName.size() > 0) {
        if (origImageMat.empty() || FFd.origImageMat.empty() || !image.empty()) {
            FFd.feedInputBufferImageToCppToFace(nullptr, image);
        }
        faceInfo.detectionMethodOrModel = FFd.modelFileName;
        return true;
    }
    return false;
}

bool
ahiCommon::detectPose(cv::Mat image, std::string genderStr, std::string viewStr,
                      ahiPoseInfo &poseInfoPredictions) {
    if(image.empty()) {
        originalImageWidth = image.cols;
        originalImageHeight = image.rows;
    }
    // init
    poseInfoPredictions.GE = false;
    poseInfoPredictions.numOfDetectedFaces = 0;
    poseInfoPredictions.headFound = false;
    poseInfoPredictions.RA = false;
    poseInfoPredictions.LA = false;
    poseInfoPredictions.RL = false;
    poseInfoPredictions.LL = false;
    poseInfoPredictions.UB = false;
    poseInfoPredictions.LB = false;
    BodyScanCommon::SexType gender;
    BodyScanCommon::Profile view;
    if (to_lower(viewStr) == "front") {
        view = BodyScanCommon::Profile::front;
        poseInfoPredictions.view = "front";
    } else {
        view = BodyScanCommon::Profile::side;
        poseInfoPredictions.view = "side";
    }
    if (to_lower(genderStr) == "male") {
        gender = BodyScanCommon::SexType::male;
        poseInfoPredictions.gender = "male";
    } else {
        gender = BodyScanCommon::SexType::female;
        poseInfoPredictions.gender = "female";
    }
    // lets do the pose now
    // check for a face?
    // face detection
    if (poseInfoPredictions.Face.empty()) {
        ahiFaceInfo faceInfo;
        faceInfo.detectionMethodOrModel = FFd.modelFileName;// "openCV";
        faceInfo.view = viewStr;
        std::vector<cv::Rect> facesFound;
        bool faceDetectSucess = detectFace(image, faceInfo);
        facesFound = faceInfo.detectedFaces;
        if (!faceDetectSucess || facesFound.empty()) {
            LOG_GUARD(std::cout << "Face Not Found.." << std::endl)
            poseInfoPredictions.numOfDetectedFaces = 0;
            return true;
        }
        if (facesFound.size() > 1) {
            LOG_GUARD(std::cout << "Face Not Found.." << std::endl)
            poseInfoPredictions.numOfDetectedFaces = 2;
            return true;
        }
        if (facesFound.size() == 1) { // more can be issues or more than one person
            poseInfoPredictions.numOfDetectedFaces = 1;
            poseInfoPredictions.Face = facesFound[0];
            poseInfoPredictions.FaceConfidence = 1.0;
        }
    }
    bool poseSuccess;
    poseSuccess = FP.getPoseInfoOutputs(image, poseInfoPredictions);
    return poseSuccess;
}

bool ahiCommon::segment(cv::Mat image, cv::Mat contourMask, ahiPoseInfo poseInfoPredictions,
                        std::string viewStr, ahiSegmentInfo &segInfo) {
    if(image.empty()) {
        originalImageWidth = image.cols;
        originalImageHeight = image.rows;
    }
    if (to_lower(viewStr) == "front") {
        segInfo.view = "front";
    } else {
        segInfo.view = "side";
    }
    bool segSuccess;
    segSuccess = FSeg.getSegmentOutInfo(image, contourMask, poseInfoPredictions, viewStr, segInfo);
    return segSuccess;
}

bool ahiCommon::inspect(ahiPoseInfo poseInfoPredictions, cv::Mat contour, int yTopUp, int yTopLow,
                        int yBotUp, int yBotLow, bool doFullInspection) {
    poseInfoPredictions.RA = false;
    poseInfoPredictions.LA = false;
    poseInfoPredictions.RL = false;
    poseInfoPredictions.LL = false;
    poseInfoPredictions.UB = false;
    poseInfoPredictions.LB = false;
    poseInfoPredictions.headInGreenZone = false;
    poseInfoPredictions.anklesInGreenZone = false;
    ahiFactoryInspection FI;
    cv::Rect rectTop(0, poseInfoPredictions.yTopUp, originalImageWidth,
                     poseInfoPredictions.yTopLow - poseInfoPredictions.yTopUp);
    cv::Rect rectBot(0, poseInfoPredictions.yBotUp, originalImageWidth,
                     poseInfoPredictions.yBotLow - poseInfoPredictions.yBotUp);
    bool isHeadWithinGzoon = FI.isRectContainsPoint(rectTop, poseInfoPredictions.CentroidHeadTop);
    bool isAnkleWithinGzoon =
            FI.isRectContainsPoint(rectBot, poseInfoPredictions.CentroidRightAnkle) ||
            FI.isRectContainsPoint(rectBot, poseInfoPredictions.CentroidLeftAnkle);
    poseInfoPredictions.headInGreenZone = isHeadWithinGzoon;
    poseInfoPredictions.anklesInGreenZone = isAnkleWithinGzoon;
    if (!isHeadWithinGzoon ||
        !isAnkleWithinGzoon) // we shouldn't bother doing full inspection in this case, we rather wait until head and ankles are withing the green zones
    {
        poseInfoPredictions.GE = false;
        poseInfoPredictions.ErrorMsg = "";
        return true;
    }
    bool fullInspectSucess = false;
    if (isHeadWithinGzoon && isAnkleWithinGzoon && doFullInspection) {
        fullInspectSucess = FI.inspectWithDetectedPosePlusContour(contour, poseInfoPredictions.Face,
                                                                  poseInfoPredictions,
                                                                  poseInfoPredictions.yTopUp, // you can replace these by yTopUp (poseInfoPredictions.yTopUp)
                                                                  poseInfoPredictions.yBotLow, // yTopLow
                                                                  poseInfoPredictions.yBotUp, // yBotUp
                                                                  poseInfoPredictions.yBotLow); // yBotLow
    } else {
        poseInfoPredictions.GE = false;
        poseInfoPredictions.ErrorMsg = "";
        return true;
    }
    if (!fullInspectSucess) {
        poseInfoPredictions.GE = true;
        poseInfoPredictions.ErrorMsg = "Inspection encountered issues";
        return false;
    } else {
        return true;
    }
}