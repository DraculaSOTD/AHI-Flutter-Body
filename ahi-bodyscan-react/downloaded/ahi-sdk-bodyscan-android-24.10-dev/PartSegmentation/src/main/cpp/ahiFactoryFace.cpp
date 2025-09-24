//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.

#include "ahiFactoryFace.hpp"

#include "AHIAvatarGenHaarcascade_frontalface_alt2.hpp"
#include "AHIAvatarGenHaarcascade_profileface.hpp"
#include "CameraConstants.hpp"

std::string ahiFactoryFace::to_lowerStr(std::string str) {
    std::for_each(str.begin(), str.end(), [](char &c) {
        c = ::tolower(c);
    });
    return str;
}

void ahiFactoryFace::getFactorTensorInstant() {
    ahiFactoryTensor faceFT;
    isFaceInit = true;

}

// function for face detection using opencv (Viola-Jones)
void ahiFactoryFace::detectFaceUsingOpencv(cv::Mat const &faceImage, cv::CascadeClassifier &model,
                                           bool isGray,
                                           bool doFlipping, std::vector <cv::Rect> &outputFaces) {
    std::vector <cv::Rect> faceRects;
    cv::Size minFaceSize(30, 30);
    cv::Size maxFaceSize(220, 220);
    cv::Mat inputFrame;
    if (isGray) {
        cv::Mat grayFrame;
        cv::cvtColor(faceImage, grayFrame, cv::COLOR_GRAY2BGR);
        equalizeHist(grayFrame, grayFrame);
        inputFrame = grayFrame;
    } else {
        inputFrame = faceImage;
    }
    if (doFlipping)
        cv::flip(inputFrame, inputFrame, 1);
    model.detectMultiScale(inputFrame, faceRects, 1.1, 3, 0,
                           minFaceSize,
                           maxFaceSize
    );
    if (faceRects.size() < 1) {
        cv::cvtColor(faceImage, inputFrame, cv::COLOR_BGR2RGB);
        model.detectMultiScale(inputFrame, faceRects, 1.1, 3, 0,
                               minFaceSize,
                               maxFaceSize
        );
    }
    // Important Note by Amar: if the face is flipped we will have to return 720 - .x position of
    // each detected face, but I will not do this in the future when we need the actual face cord
    cv::Rect topRect;
    int y = CAMERA_HEIGHT;
    for (auto &faceRect:faceRects) {
        if (faceRect.y < y) {
            y = faceRect.y;
            topRect = faceRect;
        }
    }
    if (topRect.area() >= minFaceSize.area()) {
        outputFaces.push_back(topRect);
    };
}

// function for validating face with multiple face detector models
void ahiFactoryFace::detectFaceCV(cv::Mat const &faceImage,
                                  std::vector <cv::Rect> &outputFaces) {
    //Stopwatch detectTime("Detect Face");

    cv::Rect faceROI;//(0, 640, CAMERA_WIDTH, CAMERA_HEIGHT);
    faceROI.x = (faceImage.cols / 4);//(faceImage.cols / 2) - 200;
    faceROI.y = 1;//30;
    faceROI.width = (faceImage.cols / 2); //400;
    faceROI.height = faceImage.rows / 3;//CAMERA_HEIGHT / 3;
    cv::FileStorage fs;
    cv::CascadeClassifier mFrontFaceCascadeDetector;
    cv::CascadeClassifier mProfileFaceCascadeDetector;
    fs.open(ahi_avatar_gen::front_face_features::data,
            cv::FileStorage::READ | cv::FileStorage::FORMAT_XML | cv::FileStorage::MEMORY);
    mFrontFaceCascadeDetector.read(fs.getFirstTopLevelNode()); //TODO: check result & handle.
    fs.open(ahi_avatar_gen::side_face_features::data,
            cv::FileStorage::READ | cv::FileStorage::FORMAT_XML | cv::FileStorage::MEMORY);
    mProfileFaceCascadeDetector.read(fs.getFirstTopLevelNode()); //TODO: check result & handle.
    if (outputFaces.empty()) {
        detectFaceUsingOpencv(faceImage(faceROI), mFrontFaceCascadeDetector, false, false,
                              outputFaces);
    }
    //-- try opencv profile face detector
    if (outputFaces.empty()) {
        detectFaceUsingOpencv(faceImage(faceROI), mProfileFaceCascadeDetector, false, false,
                              outputFaces);
    }
    if (outputFaces.empty()) {
        detectFaceUsingOpencv(faceImage(faceROI), mProfileFaceCascadeDetector, false, true,
                              outputFaces);
    }
    if (!outputFaces.empty()) // below only if we feed 1280x720 and without Amar22 changes above
    {
        for (int n = 0; n < outputFaces.size(); n++) {
            outputFaces[n].x += 160;
            outputFaces[n].y += 30;
        }
    }
    //detectTime.print();
}

void ahiFactoryFace::initFace() {
    getFactorTensorInstant();
}

bool ahiFactoryFace::mlkitFace() {
    return false;// TODO
}

bool ahiFactoryFace::ahiDLFace(ahiFaceInfo &faceInfoPredictions) {
    // TODO not finished yet
    if (!isFaceInit) {
        initFace();
    }
    // now we use ML to get the face/joints, in this case this is a movenet type model
    std::vector <std::string> OutputNames = faceFT.mOutputNames;
    ahiTensorOutputMap outputs;
    faceFT.invokeMIMO(faceFT.mInputs, outputs);
    std::vector<float> regressionResults;
    std::vector<float> classificationResults;
    float scoreThreshold = 0.8;
    float sigmoidScoreThreshold = log(scoreThreshold / (1 - scoreThreshold));
    std::vector<int> candIdx;
    int lC;
    for (auto outIter = outputs.begin(); outIter != outputs.end(); outIter++) {
        std::string currModelOutNodeName = outIter->first;
        std::cout << "Current output node name :" << currModelOutNodeName << "\n";
        cv::Mat currOutResult = outIter->second._mat;
        if (currModelOutNodeName.find("regres") != std::string::npos) {
            regressionResults.assign(currOutResult.begin<float>(), currOutResult.end<float>());
        }
        if (currModelOutNodeName.find("class") != std::string::npos) {
            classificationResults.assign(currOutResult.begin<float>(), currOutResult.end<float>());
            lC = classificationResults.size();
            for (int idx = 0; idx < lC; idx++) {
                if (classificationResults[idx] > sigmoidScoreThreshold) {
                    candIdx.push_back(idx);
                }
            }
        }
    }
    int numGoodDetections = candIdx.size();
    if (candIdx.empty()) {
        faceInfoPredictions.numOfDetectedFaces = 0;
        return true;
    }
    // if we reach here then we got detected faces
    static constexpr int32_t
    kElementNumOfAnchor = 16; // lC   /* x, y, w, h, [x, y] */
    std::array<std::pair<int32_t, int32_t>, 2> kAnchorGridSize = {
            std::pair<int32_t, int32_t>(16, 16), std::pair<int32_t, int32_t>(8, 8)};
    std::array<int32_t, 2> kAnchorNum = {2, 6};
    int32_t crop_x = 0;
    int32_t crop_y = 0;
    int32_t crop_w = originalImageWidth;
    int32_t crop_h = originalImageHeight;
    std::vector <std::pair<float, float>> anchor_list;
    int width = faceFT.getInputDim(0, 1);
    int height = faceFT.getInputDim(0, 2);
    float scale_x = originalImageWidth / width;
    float scale_y = originalImageHeight / height;
    for (size_t i = 0; i < kAnchorGridSize.size(); i++) {
        int32_t grid_cols = kAnchorGridSize[i].first;
        int32_t grid_rows = kAnchorGridSize[i].second;
        float stride_x = static_cast<float>(width) / grid_cols;
        float stride_y = static_cast<float>(height) / grid_rows;
        int anchor_num = kAnchorNum[i];
        std::pair<float, float> anchor;
        for (int grid_y = 0; grid_y < grid_rows; grid_y++) {
            anchor.second = stride_y * (grid_y + 0.5f);
            for (int grid_x = 0; grid_x < grid_cols; grid_x++) {
                anchor.first = stride_x * (grid_x + 0.5f);
                for (int n = 0; n < anchor_num; n++) {
                    anchor_list.push_back(anchor);
                }
            }
        }
    }
    faceInfoPredictions.numOfDetectedFaces = numGoodDetections; // 1 in this case
    std::vector <cv::Rect> bBoxes; // should be one box as I returned above if more than one face is detected
    for (size_t i = 0; i < anchor_list.size(); i++) {
        if (classificationResults[i] > sigmoidScoreThreshold) {
            int32_t index_regressor = i * kElementNumOfAnchor;
            float cx = regressionResults[index_regressor + 0] + anchor_list[i].first;
            float cy = regressionResults[index_regressor + 1] + anchor_list[i].second;
            float w = regressionResults[index_regressor + 2];
            float h = regressionResults[index_regressor + 3];
            cv::Rect bbox;
            bbox.x = static_cast<int32_t>((cx - w / 2) * scale_x);
            bbox.y = static_cast<int32_t>((cy - h / 2) * scale_y);
            bbox.width = static_cast<int32_t>(w * scale_x);
            bbox.height = static_cast<int32_t>(h * scale_y);
            bBoxes.push_back(bbox);
        }
    }
    if (numGoodDetections > 1) {
        faceInfoPredictions.numOfDetectedFaces = candIdx.size();
        return true;
    }
    return true;
}

bool
ahiFactoryFace::loadTensorFlowFaceModelFromBufferOrFile(const char *buffer, std::size_t buffer_size,
                                                        std::string modelFileName) {
    if (!isFaceInit) {
        initFace();
    }
    faceFT.modelFileName = modelFileName;
    if (modelFileName.find("mlkit") != std::string::npos) {
        return true;
    }
    if (buffer_size > 10)  // model as buffer from Java or elsewhere
    {
        faceFT.mModel = tflite::FlatBufferModel::BuildFromBuffer(buffer, buffer_size);
        bool saveSucess = faceFT.saveModelBufferOnDevice(modelFileName, buffer, buffer_size);
    } else if (modelFileName.size() > 0 && buffer_size < 10) {
        // here I can use the list of pose tflite models and load them. We need the full path of the model here
        faceFT.mModel = tflite::FlatBufferModel::BuildFromFile(
                modelFileName.c_str());// VerifyAndBuildFromFile
        // if above doesn't pass then we can try to download it here, save to cache/data folder and update the class modelFilename (full path)
    } else if (modelFileName.size() < 0 && buffer_size < 10) // no buffer or file name
    {
        return false;
    }
    faceFT.buildOptimalInterpreter();
    if (faceFT.mModel != nullptr) {
        faceFT.GetModelInpOutNames();
    }
    return faceFT.mModel != nullptr;
}

bool ahiFactoryFace::feedInputBufferImageToCppToFace(const void *data, cv::Mat mat) {
    try {
        if (!isFaceInit) {
            return false;
        }
        origImageMat = mat.clone();
        originalImageHeight = mat.rows;
        originalImageWidth = mat.cols;
        originalImageNumOfChannels = mat.channels();
        if (modelFileName.empty() || modelFileName.find("opencv") != std::string::npos ||
            modelFileName.find("mlkit") != std::string::npos) // empty is the same as opencv
        {
            return true;
        }
        std::vector <std::string> InputNames = faceFT.mInputNames;
        ahiTensorInputMap mInputs;
        if (InputNames.empty()) {
            return false;
        }
        int pWidth = faceFT.getInputDim(0, 1);
        int pHeight = faceFT.getInputDim(0, 2);
        cv::Size targetSize(pWidth, pHeight); // TF model input image size
        int top, bottom, left, right;
        bool toBGR;
        bool doPadding;
        bool toF32;
        toBGR = false;
        doPadding = false;
        toF32 = true; // this can done automatic by comparing type
        isPaddedForResize = doPadding;
        faceFT.isPaddedForResize = doPadding;
        cv::Mat matFace = faceFT.processImageWorWoutPadding(mat.clone(), targetSize, top, bottom,
                                                            left, right,
                                                            toBGR,
                                                            doPadding, toF32);
        matFace = 2 * (matFace / 255.0 - 0.5);
        faceFT.addInput(InputNames[0],
                        ahiTensorInput(matFace, true, false, targetSize, {1.0f}, {0.}));
        return true;
    }
    catch (std::exception e) {
        return false;
    }
}

bool ahiFactoryFace::getFaceInfoOutputs(cv::Mat image, ahiFaceInfo &faceInfoPredictions) {
    if (!isFaceInit) {
        initFace();
    }
    if(origImageMat.empty() || !image.empty()) {
        feedInputBufferImageToCppToFace(nullptr, image);
    }
    bool faceDLSuccess = ahiDLFace(faceInfoPredictions);
    return faceDLSuccess;
}