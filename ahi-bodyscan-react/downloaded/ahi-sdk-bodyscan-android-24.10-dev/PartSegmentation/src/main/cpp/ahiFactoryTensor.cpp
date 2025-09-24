//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#include "ahiFactoryTensor.hpp"

#include <iostream>
#include <random>

#include <opencv2/imgproc/types_c.h>
#include <opencv2/opencv.hpp>
#include <tensorflow/lite/delegates/nnapi/nnapi_delegate.h>

#include "log2022.h"
#include "Logging.hpp"

int openCV_TfLiteTypes[32] = {-100}; // make it large

void ahiFactoryTensor::match_CV_TF_types() {
    openCV_TfLiteTypes[kTfLiteNoType] = -100;
    openCV_TfLiteTypes[kTfLiteFloat32] = CV_32F;
    openCV_TfLiteTypes[kTfLiteInt32] = CV_32S; // ???? no unsigned for 32
    openCV_TfLiteTypes[kTfLiteUInt8] = CV_8U;
    openCV_TfLiteTypes[kTfLiteInt64] = -100;
    openCV_TfLiteTypes[kTfLiteString] = -100;
    openCV_TfLiteTypes[kTfLiteBool] = -100;
    openCV_TfLiteTypes[kTfLiteInt16] = CV_16S;
    openCV_TfLiteTypes[kTfLiteComplex64] = -100;
    openCV_TfLiteTypes[kTfLiteInt8] = CV_8U;
    openCV_TfLiteTypes[kTfLiteFloat16] = CV_16F;
    openCV_TfLiteTypes[kTfLiteFloat64] = CV_64F;
    openCV_TfLiteTypes[kTfLiteComplex128] = -100;
    openCV_TfLiteTypes[kTfLiteUInt64] = -100;
    openCV_TfLiteTypes[kTfLiteResource] = -100;
    openCV_TfLiteTypes[kTfLiteVariant] = -100;
    openCV_TfLiteTypes[kTfLiteUInt32] = CV_32S; // ???? no unsigned for 32
}

std::string ahiFactoryTensor::GetTensorInfo(const TfLiteTensor *tensor) {
    std::stringstream ss;
    ss << tensor->name << " "
       << TfLiteTypeGetName(tensor->type) << " ";

    if (tensor->dims->size == 0) {
        ss << "None";
    } else {
        ss << tensor->dims->data[0];
        for (int i = 1; i < tensor->dims->size; ++i)
            ss << 'x' << tensor->dims->data[i];
    }
    return ss.str();
}

std::string ahiFactoryTensor::to_lowerStr(std::string str) {
    std::for_each(str.begin(), str.end(), [](char &c) {
        c = ::tolower(c);
    });
    return str;
}

void ahiFactoryTensor::PrintModelInfo(const tflite::Interpreter *interpreter) {
    std::stringstream ss;

    ss << "Input Tensor: \n";
    for (const auto id : interpreter->inputs())
        ss << "\t" << GetTensorInfo(interpreter->tensor(id)) << '\n';

    ss << "\nOutput Tensor: \n";
    for (const auto id : interpreter->outputs())
        ss << "\t" << GetTensorInfo(interpreter->tensor(id)) << '\n';
}

void ahiFactoryTensor::GetModelInpOutNames() {
// Below can be added to PrintModelInfo above
    const auto inputs = mInterpreter->inputs();
    mInputNames.clear();
    mInputNames.resize(0);
    for (int inpIdx = 0; inpIdx < inputs.size(); inpIdx++) {
        std::string name = mInterpreter->GetInputName(inpIdx);
        if (!name.empty()) {
            mInputNames.push_back(name);
        }
    }

// Save output names.
    const auto outputs = mInterpreter->outputs();
    mOutputNames.clear();
    mOutputNames.resize(0);
    for (int outIdx = 0; outIdx < outputs.size(); outIdx++) {
        std::string name = mInterpreter->GetOutputName(outIdx);
        if (!name.empty()) {
            mOutputNames.push_back(name);
        }
    }
}

bool ahiFactoryTensor::loadModel(const char *buffer, std::size_t buffer_size) {
    mModel = tflite::FlatBufferModel::BuildFromBuffer(buffer, buffer_size);
    return mModel != nullptr;
}

void ahiFactoryTensor::resetInterpreter() {
    mInterpreter.reset();
}

bool ahiFactoryTensor::buildInterpreter() {
    RETURN_FALSE_IF_TF_FAIL(tflite::InterpreterBuilder(*mModel, mResolver)(&mInterpreter))
    mInterpreter->SetNumThreads(num_thread_);

    if (build_type_ == kNNAPI) {
        nnapi_delegate_ = std::make_unique<tflite::StatefulNnApiDelegate>();
        RETURN_FALSE_IF_TF_FAIL(mInterpreter->ModifyGraphWithDelegate(nnapi_delegate_.get()));
    } else if (build_type_ == kGPU) {
        gpu_delegate_.reset(TfLiteGpuDelegateV2Create(&gpu_options_));
        RETURN_FALSE_IF_TF_FAIL(mInterpreter->ModifyGraphWithDelegate(gpu_delegate_.get()));
    }
    else if (build_type_ == kXNNPack) {
        xnn_delegate_.reset(TfLiteXNNPackDelegateCreate(&xnn_options_));
        RETURN_FALSE_IF_TF_FAIL(mInterpreter->ModifyGraphWithDelegate(xnn_delegate_.get()));
    }

    RETURN_FALSE_IF_TF_FAIL(mInterpreter->AllocateTensors())
    PrintModelInfo(mInterpreter.get());
    return true;
}

bool ahiFactoryTensor::buildOptimalInterpreter() {
    RETURN_FALSE_IF_TF_FAIL(tflite::InterpreterBuilder(*mModel, mResolver)(&mInterpreter))
    mInterpreter->SetNumThreads(num_thread_);

    TfLiteStatus Status;
//////////////// GPU
    try {
        gpu_delegate_.reset(TfLiteGpuDelegateV2Create(&gpu_options_));
        Status = mInterpreter->ModifyGraphWithDelegate(gpu_delegate_.get());
        if (Status == kTfLiteOk)
            return true;
    }
    catch (std::exception e) {
        Status = kTfLiteError;
        LOG_GUARD(std::cout << "Could not us the GPU: " << e.what() << std::endl)
    }
///////////// NNAPI
    if (Status != kTfLiteOk)
        try {
            nnapi_delegate_ = std::make_unique<tflite::StatefulNnApiDelegate>();
            Status = mInterpreter->ModifyGraphWithDelegate(nnapi_delegate_.get());
            if (Status == kTfLiteOk)
                return true;
        }
        catch (std::exception e) {
            Status = kTfLiteError;
            LOG_GUARD(std::cout << "Could not us NNAPI: " << e.what() << std::endl)
        }

//////////// XNNPACK
//    if(Status != kTfLiteOk)
//        try {
//            xnn_delegate_.reset(TfLiteXNNPackDelegateCreate(&xnn_options_));
//            Status = mInterpreter->ModifyGraphWithDelegate(xnn_delegate_.get());
//            if(Status == kTfLiteOk)
//                return true;
//        }
//        catch(std::exception e)
//        {
//            Status = kTfLiteError;
//            LOG_GUARD(std::cout << "Could not us XNNAPACK: " << e.what() << std::endl)
//        }

    if (Status != kTfLiteOk) { // still fails, then go to the default (CPU)
        mInterpreter.release();
        buildInterpreter();
    }

    RETURN_FALSE_IF_TF_FAIL(mInterpreter->AllocateTensors())
    PrintModelInfo(mInterpreter.get());
    return true;
}

void ahiFactoryTensor::setInput(std::size_t index, const void *data, std::size_t data_size) {
    std::memcpy(mInterpreter->input_tensor(index)->data.data, data, data_size);
}

void ahiFactoryTensor::setNumThreads(int num) {
    num_thread_ = num;
}

void ahiFactoryTensor::setUseCPU() {
    build_type_ = kCPU;
}

void ahiFactoryTensor::setUseNnApi() {
    build_type_ = kNNAPI;
}

void ahiFactoryTensor::setUseGpu() {
    build_type_ = kGPU;
}

void ahiFactoryTensor::setUseXNNPack() {
    build_type_ = kXNNPack;
}

int ahiFactoryTensor::invoke() {
    namespace chrono = std::chrono;
    using clock = chrono::steady_clock;

    const auto t1 = clock::now();
    {
        const auto status = mInterpreter->Invoke();
        assert(status == kTfLiteOk);
    }
    const auto t2 = clock::now();

    return chrono::duration_cast<chrono::milliseconds>(t2 - t1).count();
}

std::size_t ahiFactoryTensor::input_bytes(std::size_t index) const {
    return mInterpreter->input_tensor(index)->bytes;
}

std::size_t ahiFactoryTensor::output_bytes(std::size_t index) const {
    return mInterpreter->output_tensor(index)->bytes;
}

void ahiFactoryTensor::copy_output(void *dst, std::size_t index) const {
    std::memcpy(dst, mInterpreter->output_tensor(index)->data.data, output_bytes(index));
}

int ahiFactoryTensor::getInputDim(int tensorId, int offset) {
    if (tensorId >= 0 && tensorId < mInterpreter->inputs().size()) {
        const int inputIx = mInterpreter->inputs()[tensorId];
        TfLiteTensor *tensor = mInterpreter->tensor(inputIx);

        if (nullptr != tensor && offset < tensor->dims->size) {
            return tensor->dims->data[offset];
        }
    }

    return 0;
}

ahiTensorInput &ahiFactoryTensor::getInput(std::string name) {
    return mInputs[name];
}

void ahiFactoryTensor::addInput(std::string name, ahiTensorInput input) {
    mInputs[name] = input;
}

bool ahiFactoryTensor::saveModelBufferOnDevice(std::string modelFileName, const char *buffer,
                                               std::size_t buffer_size) {
    FILE *pFile;
    pFile = fopen(modelFileName.c_str(), "wb");
    if (pFile != nullptr) {
        fwrite(buffer, 1, buffer_size, pFile);
        fclose(pFile);
        return true;
    }
    return false;
}

std::string
ahiFactoryTensor::pickModelFromList(std::vector<std::string> mlModelsList, std::string keyword) {
    for (auto currModel : mlModelsList) {
        if (to_lowerStr(currModel).find("pose") != std::string::npos) {
            return currModel;
        }
    }
    return "";
}

std::vector<std::string>
ahiFactoryTensor::getModelFilesList(std::string gender, std::string measCatagory) {
    std::vector<std::string> result;
    ahiModelGender genderType = ahiModelGender::Either;

    if (gender == "male" || gender == "Male" || gender == "M") {
        genderType = ahiModelGender::Male;
    } else if (gender == "female" || gender == "Female" || gender == "F") {
        genderType = ahiModelGender::Female;
    }

    ahiModelNameGender_map ModelNameGenderCatagoryMap;
    if (measCatagory.find("shape") != std::string::npos) {
        ModelNameGenderCatagoryMap = ahiShapeModelGenderMap;
    } else if (measCatagory.find("comp") != std::string::npos) {
        ModelNameGenderCatagoryMap = ahiCompositionModelGenderMap;
    } else if (measCatagory.find("general") != std::string::npos || measCatagory.size() < 1) {
        ModelNameGenderCatagoryMap = ahiModelGenderMap;
    }

    for (auto it = ModelNameGenderCatagoryMap.begin();
         it != ModelNameGenderCatagoryMap.end(); it++) {
        if (genderType == ahiModelGender::Either || it->second == ahiModelGender::Either ||
            it->second == genderType) {
            std::string filename = it->first;
            filename.append(".tflite");
            result.push_back(filename);
        }
    }

    //add SVR models to the TF models
    getSvrModelList(result, measCatagory, false);
    return result;
}

void ahiFactoryTensor::getSvrModelList(std::vector<std::string> &result, std::string measCatagory,
                                       bool isEncoded) {
    // TODO: move to static struct...
    std::string ext = ".tflite";
    if (isEncoded) {
        ext = ".bin";
    }

    if (measCatagory.find("shape") != std::string::npos) {
        result.push_back("andriod_svr_image_features_UWA_all" + ext);
        result.push_back("chest_svr_image_features" + ext);
        result.push_back("waist_svr_image_features" + ext);
        result.push_back("fat_svr_image_features" + ext);
        result.push_back("fat_svr_image_features_UWA_all" + ext);
        result.push_back("female_chest_svr_image_features" + ext);
        result.push_back("female_chest_svr_image_features_v2" + ext);
        result.push_back("female_chest_svr_image_features_v2_UWA_all" + ext);
        result.push_back("female_chest_svr_image_features_v3" + ext);
        result.push_back("female_hip_svr_image_features" + ext);
        result.push_back("female_hip_svr_image_features_v2" + ext);
        result.push_back("female_hip_svr_image_features_v2_UWA_all" + ext);
        result.push_back("female_hip_svr_image_features_v3" + ext);
        result.push_back("female_inseam_svr_image_features" + ext);
        result.push_back("female_inseam_svr_image_features_v2" + ext);
        result.push_back("female_inseam_svr_image_features_v2_UWA_all" + ext);
        result.push_back("female_inseam_svr_image_features_v3" + ext);
        result.push_back("female_waist_svr_image_features" + ext);
        result.push_back("female_waist_svr_image_features_v2" + ext);
        result.push_back("female_waist_svr_image_features_v2_UWA_all" + ext);
        result.push_back("female_waist_svr_image_features_v3" + ext);
        result.push_back("hip_svr_image_features" + ext);
        result.push_back("inseam_svr_image_features" + ext);
        result.push_back("male_chest_svr_image_features" + ext);
        result.push_back("male_chest_svr_image_features_v2" + ext);
        result.push_back("male_chest_svr_image_features_v2_UWA_all" + ext);
        result.push_back("male_chest_svr_image_features_v3" + ext);
        result.push_back("male_hip_svr_image_features" + ext);
        result.push_back("male_hip_svr_image_features_v2" + ext);
        result.push_back("male_hip_svr_image_features_v2_UWA_all" + ext);
        result.push_back("male_hip_svr_image_features_v3" + ext);
        result.push_back("male_inseam_svr_image_features" + ext);
        result.push_back("male_inseam_svr_image_features_v2" + ext);
        result.push_back("male_inseam_svr_image_features_v2_UWA_all" + ext);
        result.push_back("male_inseam_svr_image_features_v3" + ext);
        result.push_back("male_waist_svr_image_features" + ext);
        result.push_back("male_waist_svr_image_features_v2" + ext);
        result.push_back("male_waist_svr_image_features_v2_UWA_all" + ext);
        result.push_back("male_waist_svr_image_features_v3" + ext);
        result.push_back("thigh_svr_image_features_UWA_all" + ext);
    }
    if (measCatagory.find("comp") != std::string::npos) {
        result.push_back("FFM_svr_image_features_UWA_all" + ext);
        result.push_back("gynoid_svr_image_features_UWA_all" + ext);
        result.push_back("visceral_svr_image_features_UWA_all" + ext);
        result.push_back("weight_svr_image_features" + ext);
        result.push_back("weight_svr_image_features_UWA_all" + ext);
    }
}

cv::Mat
ahiFactoryTensor::preprocess_image(cv::Mat src, cv::Size target_size, int &top, int &bottom,
                                   int &left, int &right) {
    try {
        if ((int) src.rows < 1)
            return cv::Mat(0, 0, 0);

        if (src.channels() == 1) // make it color
        {
            cv::cvtColor(src, src, CV_GRAY2BGR);
        }
        cv::Mat dst;
        int borderType = cv::BORDER_CONSTANT;
        int cols = src.cols;
        int rows = src.rows;
        if (cols < rows) {
            int to_padd_value = (rows - cols) / 2;
            left = to_padd_value;
            right = to_padd_value;
            top = 0;
            bottom = 0;
        } else {
            int to_padd_value = (cols - rows) / 2;
            left = 0;
            right = 0;
            top = to_padd_value;
            bottom = to_padd_value;
        }
        cv::copyMakeBorder(src, dst, top, bottom, left, right, borderType);
        cv::resize(dst, dst, target_size, cv::INTER_AREA);
        return dst;
    }
    catch (cv::Exception &e) {
        LOG_GUARD(std::cout << e.what() << std::endl)
        return cv::Mat();
    }
}

void ahiFactoryTensor::processImage(cv::Mat const &srcImage, cv::Mat const &srcContour,
                                    cv::Size const &targetSize,
                                    cv::Mat &pImage,
                                    cv::Mat &pContour, int &top, int &bottom, int &left, int &right,
                                    bool &maskImageBg) {
    maskImageBg = false;
    pImage.release();
    pContour.release();
    //Stopwatch imageTime("Process image");

    int maxHW = std::max(srcImage.cols, srcImage.rows);
    cv::Mat nImage;
    nImage = srcImage;
    //Threshold the binary contour map
    if (srcContour.channels() > 1) {
        cv::cvtColor(srcContour, srcContour, cv::COLOR_BGR2GRAY);
    }

    int borderType = cv::BORDER_CONSTANT;
    int cols = nImage.cols;
    int rows = nImage.rows;

    if (cols < rows) {
        int to_padd_value = (rows - cols) / 2;
        left = to_padd_value;
        right = to_padd_value;
        top = 0;
        bottom = 0;
    } else {
        int to_padd_value = (cols - rows) / 2;
        left = 0;
        right = 0;
        top = to_padd_value;
        bottom = to_padd_value;
    }
    copyMakeBorder(nImage, nImage, top, bottom, left, right, borderType);
    cv::resize(nImage, nImage, targetSize, cv::INTER_AREA);

    copyMakeBorder(srcContour, pContour, top, bottom, left, right, borderType);

    if (pContour.size().empty()) {
        printf("\bpContour empty.");
        return;
    }

    cv::resize(pContour, pContour, targetSize, cv::INTER_AREA);

    // Maks the image background relative to the contour with
    cv::Mat structElement = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(11, 11));
    cv::dilate(pContour, pContour, structElement);

    std::vector<std::vector<cv::Point>> _contours;
    // filling the contour
    cv::findContours(pContour.clone(), _contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    for (int i = 0; i < (int) _contours.size(); i++) {
        cv::drawContours(pContour, _contours, i, cv::Scalar(255), cv::FILLED);
    }
    _contours.clear();
    _contours.resize(0);

    if ((maskImageBg)) {
        nImage.copyTo(pImage, pContour);
    } else {
        nImage.copyTo(pImage);
    }
    //imageTime.print();
}

// Preprocess image with/out padding
cv::Mat
ahiFactoryTensor::processImageWorWoutPadding(cv::Mat const &srcImage, cv::Size const &targetSize,
                                             int &top, int &bottom, int &left, int &right,
                                             bool &toBGR, bool &doPadding, bool toF32) {

    //Stopwatch imageTime("Process image");

    int maxHW = std::max(srcImage.cols, srcImage.rows);
    cv::Mat normImage = srcImage.clone();
    // fix color channels
    if (toBGR) {
        if (srcImage.channels() > 3) {
            cv::cvtColor(srcImage, normImage, cv::COLOR_RGBA2BGR);
        } else {
            cv::cvtColor(srcImage, normImage, cv::COLOR_RGB2BGR);
        }
    } else {
        if (srcImage.channels() > 3) {
            cv::cvtColor(srcImage, normImage, cv::COLOR_RGBA2RGB);
        }
    }

    cv::Mat dst = normImage.clone();
    int cols = normImage.cols;
    int rows = normImage.rows;
    top = 0;
    bottom = 0;
    left = 0;
    right = 0;
    if (doPadding) {
        int borderType = cv::BORDER_CONSTANT;
        if (cols < rows) {
            int to_padd_value = (rows - cols) / 2;
            left = to_padd_value;
            right = to_padd_value;
            top = 0;
            bottom = 0;
        } else {
            int to_padd_value = (cols - rows) / 2;
            left = 0;
            right = 0;
            top = to_padd_value;
            bottom = to_padd_value;
        }
        copyMakeBorder(normImage, dst, top, bottom, left, right, borderType);
    }
    cv::resize(dst, dst, targetSize, cv::INTER_CUBIC);

    if (toF32) {
        dst.convertTo(dst, CV_32F);
    }
    return dst;
}

int ahiFactoryTensor::get_perc_idx(float percentage, cv::Mat rows_columns) {
    cv::Size row_size = rows_columns.size();
    int left_idx = 0;
    int right_idx = row_size.height;

    int current_idx = 0;
    int intervall_length = 0;
    float added_ratio_sums = 0.;
    while (true) {
        intervall_length = (right_idx - left_idx);
        current_idx = left_idx + intervall_length / 2;
        added_ratio_sums = 0;
        for (int k = 0; k < current_idx; k++) {
            added_ratio_sums += rows_columns.at<float>(k);
        }
        if (intervall_length < 3)
            break;

        if (added_ratio_sums > percentage)
            right_idx = current_idx;
        else
            left_idx = current_idx;
    }
    return current_idx;
}

std::vector<int> ahiFactoryTensor::get_roi_idx(cv::Mat cal_src, int psum, float top_padding_scale,
                                               float bottom_padding_scale) {
    std::vector<int> retvector;
    cv::Mat column_sum;
    cv::reduce(cal_src, column_sum, 0, cv::REDUCE_SUM);
    cv::Mat row_sum;
    cv::reduce(cal_src, row_sum, 1, cv::REDUCE_SUM);
    column_sum /= psum;
    row_sum /= psum;
    column_sum = column_sum.t();
    cv::Mat reverse_column_sum;
    cv::flip(column_sum.t(), reverse_column_sum, 1);
    cv::Mat reverse_row_sum;

    cv::flip(row_sum.t(), reverse_row_sum, 1);

    int top_perc_idx = get_perc_idx(0.1, row_sum);
    int bottom_perc_idx = row_sum.size().height - get_perc_idx(0.1, reverse_row_sum.t());
    int left_perc_idx = get_perc_idx(0.2, column_sum);
    int right_perc_idx = column_sum.size().height - get_perc_idx(0.2, reverse_column_sum.t());
    int middle = (0.5 * (top_perc_idx + bottom_perc_idx));
    int top_idx = middle - int(top_padding_scale * (middle - top_perc_idx));
    int bottom_idx = middle + int(bottom_padding_scale * (bottom_perc_idx - middle));

    middle = (0.5 * (left_perc_idx + right_perc_idx));

    int left_idx = middle - int(5 * (middle - left_perc_idx));
    int right_idx = middle + int(5 * (right_perc_idx - middle));

    retvector = {top_idx, bottom_idx, left_idx, right_idx};
    return retvector;
}

cv::Mat ahiFactoryTensor::preprocess_image_robust(cv::Mat const inp_src, cv::Size target_size,
                                                  bool robust_cropping, float top_padding_scale,
                                                  float bottom_padding_scale) {
    if (0 == inp_src.size) {
        return inp_src;
    }

    cv::Mat src = inp_src.clone();

    if (src.channels() > 1) {
        cv::cvtColor(inp_src, src, cv::COLOR_BGR2GRAY);
    }
    cv::Mat dst;
    int borderType = cv::BORDER_CONSTANT;
    cv::Mat help_src = src.clone();

    if (robust_cropping) {
        float resize_factor = 1.1 * target_size.height / (float(std::max(src.rows, src.cols)));
        cv::resize(src, src, cv::Size(0, 0), resize_factor, resize_factor);
        cv::Mat cal_src = src.clone();
        cal_src.convertTo(cal_src, CV_32FC1);
        cal_src /= 255.;

        double psum = cv::sum(cal_src)[0];
        if (psum == 0) {
            LOG_GUARD(std::cout << "[preprocess_image_robust] ERROR: psum == 0" << std::endl)
            return src;//todo add succescode
        }

        auto pre_roi_idx = get_roi_idx(cal_src.clone(), psum, top_padding_scale,
                                       bottom_padding_scale);

        int top_idx, left_idx, bottom_idx, right_idx;
        int top_pad, left_pad, bottom_pad, right_pad;

        top_pad = std::max(0, -pre_roi_idx[0]);
        top_idx = std::max(0, pre_roi_idx[0]);
        bottom_pad = std::max(0, pre_roi_idx[1] - src.rows);
        bottom_idx = pre_roi_idx[1] + top_pad;
        left_pad = std::max(0, -pre_roi_idx[2]);
        left_idx = std::max(0, pre_roi_idx[2]);
        right_pad = std::max(0, pre_roi_idx[3] - src.cols);
        right_idx = pre_roi_idx[3] + left_pad;
        cv::copyMakeBorder(src, src, top_pad, bottom_pad, left_pad, right_pad, borderType);
        cv::Rect myROI(left_idx, top_idx, right_idx - left_idx, bottom_idx - top_idx);
        help_src = src(myROI).clone();
    }
    src = help_src.clone();
    int cols = src.cols;
    int rows = src.rows;
    int top, bottom, left, right;

    if (cols < rows) {
        int to_padd_value = (rows - cols) / 2;
        left = to_padd_value;
        right = to_padd_value;
        top = 0;
        bottom = 0;
        right += rows - (2 * to_padd_value + cols);
    } else {
        int to_padd_value = (cols - rows) / 2;
        left = 0;
        right = 0;
        top = to_padd_value;
        bottom = to_padd_value;
        bottom += cols - (2 * to_padd_value + rows);
    }
    cv::copyMakeBorder(src, dst, top, bottom, left, right, borderType);
    if (dst.rows > 0 && dst.cols > 0) {
        cv::resize(dst, dst, target_size);
        return dst;
    }
    return inp_src;
}

// The below was done because coreml doesn't have some of the used layers yet. I kept it just in case tflite needs too
cv::Mat
ahiFactoryTensor::preprocess_image_resize_crop_or_pad(cv::Mat const inp_src, cv::Size target_size,
                                                      double scale,
                                                      int offset)// this is for slice/zoom in models
{
    try {
        cv::Mat src = inp_src.clone();
        if ((int) src.rows < 1) {
            return cv::Mat();
        }
        if (src.channels() > 1) {
            cv::cvtColor(inp_src, src, cv::COLOR_BGR2GRAY);
        }
        if ((src.rows != target_size.height) & (src.cols != target_size.width)) {
            int top, bottom, left, right;
            src = preprocess_image(src, target_size, top, bottom, left, right);
        }

        cv::resize(src, src, cv::Size(), scale, scale, cv::INTER_LINEAR);
        cv::Rect ROI;

        ROI.x = offset;
        ROI.y = offset;
        ROI.width = target_size.width;
        ROI.height = target_size.height;

        return src(ROI);
    }
    catch (cv::Exception &e) {
        return cv::Mat();
    }
}

// Extra Meas image preprocessing
cv::Mat
ahiFactoryTensor::preprocess_image_for_exmeas(cv::Mat const inp_src, cv::Size target_size, int &top,
                                              int &bottom, int &left, int &right) {
    try {
        cv::Mat src = inp_src;
        if ((int) src.rows < 1) {
            return cv::Mat();
        }
        if (src.channels() > 1) {
            cv::cvtColor(inp_src, src, cv::COLOR_BGR2GRAY);
        }
        cv::threshold(inp_src, inp_src, 100, 255, CV_8U);
        std::vector<std::vector<cv::Point>> points;
        std::vector<cv::Vec4i> hierarchy;
        cv::findContours(inp_src, points, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
        float largestArea = 0;
        int largestIndex = 0;
        for (int i = 0; i < points.size(); i++) {
            // Find the area of contour
            double area = cv::contourArea(points[i]);
            if (area > largestArea) {
                largestArea = area;
                //Store the index of largest contour
                largestIndex = i;
            }
        }
        int imageHeight = inp_src.rows;
        int imageWidth = inp_src.cols;
        cv::Rect imageRect = cv::boundingRect(points[largestIndex]);
        imageRect.x = MAX(0, imageRect.x - 30);
        imageRect.y = MAX(0, imageRect.y - 30);
        imageRect.width = MIN(imageRect.width + 60, imageWidth - imageRect.x - 1); //check this
        imageRect.height = MIN(imageRect.height + 60, imageHeight - imageRect.y - 1); //check this
        cv::Mat croppedSrc = src(imageRect);
        cv::Mat dst;
        int borderType = cv::BORDER_CONSTANT;
        int cols = croppedSrc.cols;
        int rows = croppedSrc.rows;
        if (cols < rows) {
            int to_padd_value = MAX(0, (rows - cols) / 2);
            left = to_padd_value;
            right = to_padd_value;
            top = 0;
            bottom = 0;
        } else {
            int to_padd_value = MAX(0, (cols - rows) / 2);
            left = 0;
            right = 0;
            top = to_padd_value;
            bottom = to_padd_value;
        }
        cv::copyMakeBorder(croppedSrc, dst, top, bottom, left, right, borderType);
        cv::resize(dst, dst, target_size, cv::INTER_CUBIC);
        return dst;
    }
    catch (cv::Exception &e) {
        return cv::Mat();
    }
}

std::vector<double> ahiFactoryTensor::generateUniformMLSamples(int numSamples) {
    std::vector<double> outputSamples(numSamples, 0);
    std::default_random_engine generator;
    generator.seed(10);
    std::normal_distribution<double> distribution(0.0, 1.0);
    for (int i = 0; i < numSamples; ++i) {
        outputSamples[i] = distribution(generator);
    }
    return outputSamples;
}

bool ahiFactoryTensor::prepareInputsForImageBasedExtraMeas(cv::Mat front_silhouette,
                                                           cv::Mat side_silhouette, double height,
                                                           double weight, std::string gender,
                                                           cv::Mat &inputImageFeat,
                                                           cv::Mat &inputHWGFeat,
                                                           cv::Mat &inputNormalDist
) {
    try {
        cv::Size target_size(256, 256);  // preprate the input for the model to correctly
        int top;
        int bottom;
        int left;
        int right;
        cv::Mat prep_front = preprocess_image_for_exmeas(front_silhouette, target_size, top, bottom,
                                                         left, right);
        cv::Mat prep_side = preprocess_image_for_exmeas(side_silhouette, target_size, top, bottom,
                                                        left, right);
        cv::Mat prep_fside;
        cv::flip(prep_side, prep_fside, 1); // 1 corresponds to mirror image

        cv::Mat tempimg[3];
        tempimg[0] = prep_front;
        tempimg[1] = prep_side;
        tempimg[2] = prep_fside;
        cv::Mat silhouettes;
        cv::merge(tempimg, 3, silhouettes);
        silhouettes.convertTo(inputImageFeat, CV_32F);
        silhouettes.release();

        std::vector<double> heightWeightGender(3);
        heightWeightGender[0] = 1.; // Male
        if (gender.find("f") != std::string::npos || gender.find("F") != std::string::npos) {
            heightWeightGender[0] = 0.; // Female
        }
        heightWeightGender[1] = height;
        heightWeightGender[2] = weight;

        inputHWGFeat = cv::Mat(3, 1, CV_64F, heightWeightGender.data());
        inputHWGFeat.convertTo(inputHWGFeat, CV_32F);

        inputNormalDist = cv::Mat(256, 1, CV_64F, (generateUniformMLSamples(256)).data());
        inputNormalDist.convertTo(inputNormalDist, CV_32F);

        return true;
    }
    catch (cv::Exception &e) {
        return false;
    }
}

bool ahiFactoryTensor::invokeMIMO(ahiTensorInputMap &inputs, ahiTensorOutputMap &outputs) {
    match_CV_TF_types();
    //Stopwatch tfStopwatch(mModelName.c_str());
    auto inputCount = inputs.size();
    auto iter = inputs.begin();

    while (iter != inputs.end()) {
        // Get the iterator to the first named input - allows for unnamed input.
        auto name = mInputNames.begin();
        if (inputCount > 1) {
            // If there's more than one input supplied -> search for input by name.
            name = std::find(mInputNames.begin(), mInputNames.end(), iter->first);
        }
        if (name != mInputNames.end()) {
            auto nameIx = std::distance(mInputNames.begin(), name);
            const int inputIx = mInterpreter->inputs()[nameIx];
            TfLiteTensor *tensor = mInterpreter->tensor(inputIx);
            cv::Mat inputMat = iter->second._mat;
            auto type = mInterpreter->input_tensor(nameIx)->type;

            if (inputMat.rows > 1 && inputMat.cols > 1) {
                int CV_type = openCV_TfLiteTypes[type];
                if (CV_type < 0) {
                    LOG_GUARD(std::cout << "TF input has unsupported CV type" << std::endl);
                    return false;
                }
                inputMat.convertTo(inputMat, CV_type);
            }
            size_t cvSizeInBytes = inputMat.total() * inputMat.elemSize();
            //LOG_GUARD(std::cout << "TensorModel::invoke cvSize:" << cvSizeInBytes << " Tensor bytes:" << tensor->bytes << std::endl)
            if (cvSizeInBytes <= tensor->bytes) {
                std::memcpy(mInterpreter->input_tensor(nameIx)->data.data, inputMat.data,
                            cvSizeInBytes);
            // original but not always good  memcpy(mInterpreter->typed_input_tensor<float>((int)nameIx), inputMat.data, cvSizeInBytes);
            } else {
                LOG_GUARD(std::cout << "[TensorModel::invoke]:" << mModelName << " error ("
                                    << mInputNames[nameIx] << ") - invalid input mat size "
                                    << cvSizeInBytes << " vs " << tensor->bytes << std::endl)
                return false;
            }
        }
        iter++;
    }

    //TODO: check if all inputs succeeded.
    //if (bAllInputsDone)
    {
        const TfLiteStatus status = mInterpreter->Invoke();

        if (status != kTfLiteOk) {
            return false;
        }

        auto ix = 0;
        size_t nOutputs = mInterpreter->outputs().size();
        //LOG_GUARD(std::cout << "[TensorModel::invoke]:" << mModelName << " nOutputs:" << nOutputs << std::endl)

        while (ix < nOutputs) {
            auto outputName = mOutputNames[ix];
            const int outputIx = mInterpreter->outputs()[ix];
            TfLiteTensor *tensor = mInterpreter->tensor(outputIx);
            cv::Mat output(tensor->dims->size, tensor->dims->data, CV_32F);
            size_t cvSizeInBytes = output.total() * output.elemSize();
            //printTensor("Output", tensor);
            //LOG_GUARD(std::cout << "[TensorModel::invoke]:" << mModelName << " cvSizeInBytes:" << cvSizeInBytes << std::endl)
            memcpy(output.data, mInterpreter->typed_output_tensor<float>(ix), cvSizeInBytes);
            ahiTensorOutput outputStruct;
            outputStruct._mat = output;
            outputs[outputName] = outputStruct;
            ix++;
        }

        //LOG_GUARD(tfStopwatch.print())
        return true;
    }
}

FactoryTensorModelType ahiFactoryTensor::checkModel(const uint8_t *bytes) {
    // ....TFL3.....
    // ....SFL3..... (stubbed model)
    // Check that the model header exists.
    // We could check for more characters... but this should suffice.
    if (bytes[4] == 'T' && bytes[5] == 'F')
        return FactoryTensorModelType::ModelTypeTF;
    if (bytes[4] == 'S' && bytes[4] == 'F')
        return FactoryTensorModelType::ModelTypeStub;
    return FactoryTensorModelType::ModelTypeUnknown;
}

struct membuf : std::streambuf {
    membuf(char *begin, char *end) : begin(begin), end(end) {
        this->setg(begin, begin, end);
    }

    virtual pos_type seekoff(off_type off, std::ios_base::seekdir dir,
                             std::ios_base::openmode which = std::ios_base::in) override {
        if (dir == std::ios_base::cur)
            gbump(off);
        else if (dir == std::ios_base::end)
            setg(begin, end + off, end);
        else if (dir == std::ios_base::beg)
            setg(begin, begin + off, end);
        return gptr() - eback();
    }

    virtual pos_type seekpos(std::streampos pos, std::ios_base::openmode mode) override {
        return seekoff(pos - pos_type(off_type(0)), std::ios_base::beg, mode);
    }

    char *begin, *end;
};
