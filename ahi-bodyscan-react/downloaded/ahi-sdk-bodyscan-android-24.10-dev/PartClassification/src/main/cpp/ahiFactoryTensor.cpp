//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#include "ahiFactoryTensor.hpp"

#if defined(ANDROID) || defined(__ANDROID__)

#include <jni.h>
#include <android/bitmap.h>
#include "ndk/sources/android/cpufeatures/cpu-features.h"
#include "AHILogging.hpp"

#endif


// make it large
int openCV_TfLiteTypes[32] = {-100};

void ahiFactoryTensor::match_CV_TF_types() {
    openCV_TfLiteTypes[kTfLiteNoType] = -100;
    openCV_TfLiteTypes[kTfLiteFloat32] = CV_32F;
    openCV_TfLiteTypes[kTfLiteInt32] = CV_32S;
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
    openCV_TfLiteTypes[kTfLiteUInt32] = CV_32S;
}

Mutex gInstanceMutex_;
ahiFactoryTensor *ahiFactoryTensor::mThis = nullptr;

ahiFactoryTensor *ahiFactoryTensor::getInstance() {
    AutoLock lock(gInstanceMutex_);

    if (nullptr == mThis) {
        mThis = new ahiFactoryTensor();
    }

    return mThis;
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
    for (const auto id: interpreter->inputs())
        ss << "\t" << GetTensorInfo(interpreter->tensor(id)) << '\n';

    ss << "\nOutput Tensor: \n";
    for (const auto id: interpreter->outputs())
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
    } else if (build_type_ == kXNNPack) {
        xnn_delegate_.reset(TfLiteXNNPackDelegateCreate(&xnn_options_));
        RETURN_FALSE_IF_TF_FAIL(mInterpreter->ModifyGraphWithDelegate(xnn_delegate_.get()));
    }

    RETURN_FALSE_IF_TF_FAIL(mInterpreter->AllocateTensors())


    PrintModelInfo(mInterpreter.get());

    return true;
}

std::unique_ptr<tflite::Interpreter> ahiFactoryTensor::buildInterpreter(std::unique_ptr<tflite::FlatBufferModel> model) {
    std::unique_ptr<tflite::Interpreter> interpreter;
    RETURN_NULL_IF_TF_FAIL(tflite::InterpreterBuilder(*model, mResolver)(&interpreter))
    interpreter->SetNumThreads(num_thread_);

    if (build_type_ == kNNAPI) {
        nnapi_delegate_ = std::make_unique<tflite::StatefulNnApiDelegate>();
        RETURN_NULL_IF_TF_FAIL(interpreter->ModifyGraphWithDelegate(nnapi_delegate_.get()));
    } else if (build_type_ == kGPU) {
        gpu_delegate_.reset(TfLiteGpuDelegateV2Create(&gpu_options_));
        RETURN_NULL_IF_TF_FAIL(interpreter->ModifyGraphWithDelegate(gpu_delegate_.get()));
    } else if (build_type_ == kXNNPack) {
        xnn_delegate_.reset(TfLiteXNNPackDelegateCreate(&xnn_options_));
        RETURN_NULL_IF_TF_FAIL(interpreter->ModifyGraphWithDelegate(xnn_delegate_.get()));
    }

    RETURN_NULL_IF_TF_FAIL(interpreter->AllocateTensors())


    PrintModelInfo(interpreter.get());

    return interpreter;
}


std::unique_ptr<tflite::Interpreter> ahiFactoryTensor::buildOptimalInterpreter(std::unique_ptr<tflite::FlatBufferModel> model) {
    std::unique_ptr<tflite::Interpreter> interpreter;
    RETURN_NULL_IF_TF_FAIL(tflite::InterpreterBuilder(*model, mResolver)(&interpreter))
    interpreter->SetNumThreads(num_thread_);
    InferenceMethod = "";
    TfLiteStatus Status;
    // GPU
    try {
        InferenceMethod = "gpu_delegate";
        gpu_delegate_.reset(TfLiteGpuDelegateV2Create(&gpu_options_));
        Status = interpreter->ModifyGraphWithDelegate(gpu_delegate_.get());
        if (Status == kTfLiteOk)
            return interpreter;
    }
    catch (std::exception e) {
        Status = kTfLiteError;
        LOG_GUARD(std::cout << "Could not use the GPU: " << e.what() << std::endl)
    }
    // NNAPI
    if (Status != kTfLiteOk)
        try {
            InferenceMethod = "nnapi_delegate";
            nnapi_delegate_ = std::make_unique<tflite::StatefulNnApiDelegate>();
            Status = interpreter->ModifyGraphWithDelegate(nnapi_delegate_.get());
            if (Status == kTfLiteOk)
                return interpreter;
        }
        catch (std::exception e) {
            Status = kTfLiteError;
            LOG_GUARD(std::cout << "Could not use NNAPI: " << e.what() << std::endl)
        }

    // XNNPACK
    if (Status != kTfLiteOk)
        try {
            InferenceMethod = "xnn_delegate";
            xnn_delegate_.reset(TfLiteXNNPackDelegateCreate(&xnn_options_));
            Status = interpreter->ModifyGraphWithDelegate(xnn_delegate_.get());
            if (Status == kTfLiteOk)
                return interpreter;
        }
        catch (std::exception e) {
            Status = kTfLiteError;
            LOG_GUARD(std::cout << "Could not use XNNAPACK: " << e.what() << std::endl)
        }

    // Still fails, then go to the default (CPU)
    if (Status != kTfLiteOk) {
        interpreter.release();
        InferenceMethod = "cpu";
        interpreter = buildInterpreter(std::move(model));
    }

    RETURN_NULL_IF_TF_FAIL(interpreter->AllocateTensors())

    PrintModelInfo(interpreter.get());

    return interpreter;
}


bool ahiFactoryTensor::buildOptimalInterpreter() {
    RETURN_FALSE_IF_TF_FAIL(tflite::InterpreterBuilder(*mModel, mResolver)(&mInterpreter))
    mInterpreter->SetNumThreads(num_thread_);
    InferenceMethod = "";
    TfLiteStatus Status;
    // GPU
    try {
        InferenceMethod = "gpu_delegate";
        gpu_delegate_.reset(TfLiteGpuDelegateV2Create(&gpu_options_));
        Status = mInterpreter->ModifyGraphWithDelegate(gpu_delegate_.get());
        if (Status == kTfLiteOk)
            return true;
    }
    catch (std::exception e) {
        Status = kTfLiteError;
        LOG_GUARD(std::cout << "Could not use the GPU: " << e.what() << std::endl)
    }
    // NNAPI
    if (Status != kTfLiteOk)
        try {
            InferenceMethod = "nnapi_delegate";
            nnapi_delegate_ = std::make_unique<tflite::StatefulNnApiDelegate>();
            Status = mInterpreter->ModifyGraphWithDelegate(nnapi_delegate_.get());
            if (Status == kTfLiteOk)
                return true;
        }
        catch (std::exception e) {
            Status = kTfLiteError;
            LOG_GUARD(std::cout << "Could not use NNAPI: " << e.what() << std::endl)
        }

    // XNNPACK
    if (Status != kTfLiteOk)
        try {
            InferenceMethod = "xnn_delegate";
            xnn_delegate_.reset(TfLiteXNNPackDelegateCreate(&xnn_options_));
            Status = mInterpreter->ModifyGraphWithDelegate(xnn_delegate_.get());
            if (Status == kTfLiteOk)
                return true;
        }
        catch (std::exception e) {
            Status = kTfLiteError;
            LOG_GUARD(std::cout << "Could not use XNNAPACK: " << e.what() << std::endl)
        }

    // Still fails, then go to the default (CPU)
    if (Status != kTfLiteOk) {
        mInterpreter.release();
        InferenceMethod = "cpu";
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

bool ahiFactoryTensor::saveModelBufferOnDevice(std::string modelFileName, const char *buffer, std::size_t buffer_size) {
    FILE *pFile;
    pFile = fopen(modelFileName.c_str(), "wb");
    if (pFile != nullptr) {
        fwrite(buffer, 1, buffer_size, pFile);
        fclose(pFile);
        return true;
    }
    return false;
}

std::string ahiFactoryTensor::pickModelFromList(std::vector<std::string> mlModelsList, std::string keyword) {
    for (auto currModel: mlModelsList) {
        if (to_lowerStr(currModel).find(keyword) != std::string::npos) {
            return currModel;
        }
    }
    return "";
}

cv::Mat
ahiFactoryTensor::preprocess_image(cv::Mat src, cv::Size target_size, int &top, int &bottom, int &left, int &right) {
    try {
        if ((int) src.rows < 1)
            return cv::Mat(0, 0, 0);

        // make it color
        if (src.channels() == 1) {
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
        return cv::Mat();
    }
}

void ahiFactoryTensor::processImage(cv::Mat const &srcImage, cv::Mat const &srcContour, cv::Size const &targetSize,
                                    cv::Mat &pImage,
                                    cv::Mat &pContour, int &top, int &bottom, int &left, int &right,
                                    bool &maskImageBg) {
    maskImageBg = false;
    pImage.release();
    pContour.release();

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
        AHILog(ANDROID_LOG_ERROR, "Contour empty.");
        return;
    }

    cv::resize(pContour, pContour, targetSize, cv::INTER_AREA);

    //Maks the image background relative to the contour with
    cv::Mat structElement = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(11, 11));
    cv::dilate(pContour, pContour, structElement);

    std::vector<std::vector<cv::Point> > _contours;
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
}


// Preprocess image with/out padding
cv::Mat ahiFactoryTensor::processImageWorWoutPadding(cv::Mat const &srcImage, cv::Size const &targetSize,
                                                     int &top, int &bottom, int &left, int &right, bool &toBGR, bool &doPadding, bool toF32) {
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

// Classification  main file/utils begins here
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

std::vector<int> ahiFactoryTensor::get_roi_idx(cv::Mat cal_src, int psum, float top_padding_scale, float bottom_padding_scale) {
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

cv::Mat ahiFactoryTensor::preprocess_image_robust(cv::Mat const inp_src, cv::Size target_size, bool robust_cropping, float top_padding_scale,
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

        auto pre_roi_idx = get_roi_idx(cal_src.clone(), psum, top_padding_scale, bottom_padding_scale);


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

cv::Mat ahiFactoryTensor::preprocess_image_gray(cv::Mat const inp_src, cv::Size target_size, int &top, int &bottom, int &left, int &right) {
    try {
        cv::Mat src = inp_src;

        if ((int) src.rows < 1)
            return cv::Mat();

        if (src.channels() > 1) {
            cv::cvtColor(inp_src, src, cv::COLOR_BGR2GRAY);
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
        cv::resize(dst, dst, target_size);

        return dst;
    }
    catch (cv::Exception &e) {
        return cv::Mat();
    }
}

cv::Mat
ahiFactoryTensor::preprocess_image_std_or_robust(cv::Mat src, cv::Size target_size, int &top, int &bottom, int &left, int &right,
                                                 bool robust_cropping,
                                                 float top_padding_scale, float bottom_padding_scale) {
    cv::Mat processed_image;
    if (robust_cropping) {
        processed_image = preprocess_image_robust(src, target_size, robust_cropping, top_padding_scale, bottom_padding_scale);
    } else {
        processed_image = preprocess_image(src, target_size, top, bottom, left, right);
    }
    return processed_image;
}

// The below was done because coreml doesn't have some of the used layers yet. I kept it just in case tflite needs too
cv::Mat ahiFactoryTensor::preprocess_image_resize_crop_or_pad(cv::Mat const inp_src, cv::Size target_size, double scale, int offset) {
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
cv::Mat ahiFactoryTensor::preprocess_image_for_exmeas(cv::Mat const inp_src, cv::Size target_size, int &top, int &bottom, int &left, int &right) {
    try {
        cv::Mat src = inp_src;
        if ((int) src.rows < 1) {
            return cv::Mat();
        }
        if (src.channels() > 1) {
            cv::cvtColor(inp_src, src, cv::COLOR_BGR2GRAY);
        }
        cv::threshold(inp_src, inp_src, 100, 255, CV_8U);
        std::vector<std::vector<cv::Point> > points;
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
        // just initialization
        cv::Mat croppedSrc = src.clone();
        if (!points.empty()) {
            cv::Rect imageRect = cv::boundingRect(points[largestIndex]);
            imageRect.x = MAX(0, imageRect.x - 30);
            imageRect.y = MAX(0, imageRect.y - 30);
            imageRect.width = MIN(imageRect.width + 60, imageWidth - imageRect.x - 1);
            imageRect.height = MIN(imageRect.height + 60,
                                   imageHeight - imageRect.y - 1);
            croppedSrc = src(imageRect);
        }

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

bool ahiFactoryTensor::prepareInputsForImageBasedExtraMeas(cv::Mat front_silhoutte, cv::Mat side_silhoutte, double height, double weight,
                                                           std::string gender,
                                                           cv::Mat &inputImageFeat,
                                                           cv::Mat &inputHWGFeat,
                                                           cv::Mat &inputNormalDist
) {
    try {

        //preprate the input for the model to correctly
        cv::Size target_size(256, 256);
        int top;
        int bottom;
        int left;
        int right;
        cv::Mat prep_front = preprocess_image_for_exmeas(front_silhoutte, target_size, top, bottom,
                                                         left, right);
        cv::Mat prep_side = preprocess_image_for_exmeas(side_silhoutte, target_size, top, bottom,
                                                        left, right);
        cv::Mat prep_fside;
        //1 correcponds to mirror image
        cv::flip(prep_side, prep_fside, 1);

        cv::Mat tempimg[3];
        tempimg[0] = prep_front;
        tempimg[1] = prep_side;
        tempimg[2] = prep_fside;
        cv::Mat silhouttes;
        cv::merge(tempimg, 3, silhouttes);
        silhouttes.convertTo(inputImageFeat, CV_32F);
        silhouttes.release();

        std::vector<double> heightWeightGender(3);
        //Male
        heightWeightGender[0] = 1.;
        if (gender.find("f") != string::npos || gender.find("F") != string::npos) {
            // Female
            heightWeightGender[0] = 0.;
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

////////////////////////////////////////////////////////
bool ahiFactoryTensor::invokeMIMO(ahiTensorInputMap &inputs, ahiTensorOutputMap &outputs) {
    match_CV_TF_types();
    {
        LOG_GUARD(std::cout << "[TensorModel::invoke]: Now invoking model " << mModelName << std::endl);
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
                if (cvSizeInBytes <= tensor->bytes) {
                    std::memcpy(mInterpreter->input_tensor(nameIx)->data.data, inputMat.data, cvSizeInBytes);
                } else {
                    LOG_GUARD(
                            std::cout << "[TensorModel::invoke]:" << mModelName << " error (" << mInputNames[nameIx] << ") - invalid input mat size "
                                      << cvSizeInBytes << " vs " << tensor->bytes << std::endl)
                    return false;
                }
            }
            iter++;
        }

        //TODO: check if all inputs succeeded.
        {
            const TfLiteStatus status = mInterpreter->Invoke();

            if (status != kTfLiteOk) {
                return false;
            }

            auto ix = 0;
            size_t nOutputs = mInterpreter->outputs().size();

            while (ix < nOutputs) {
                auto outputName = mOutputNames[ix];
                const int outputIx = mInterpreter->outputs()[ix];
                TfLiteTensor *tensor = mInterpreter->tensor(outputIx);
                cv::Mat output(tensor->dims->size, tensor->dims->data, CV_32F);
                size_t cvSizeInBytes = output.total() * output.elemSize();
                memcpy(output.data, mInterpreter->typed_output_tensor<float>(ix), cvSizeInBytes);
                ahiTensorOutput outputStruct;
                outputStruct._mat = output;
                outputs[outputName] = outputStruct;
                ix++;
            }

            return true;
        }
    }

    return false;
}

#if 0
#if 0
//This function inspect the given pose to the template pose
ahiFrontPose ahiFactoryTensor::inspectFrontPoseWithTemplatePose(cv::Mat const &dPose, cv::Mat const &tPose,
                                                                cv::Rect const faceRect, ahiPoseInfo const centroids)
{
    ahiFrontPose finalResult{};

    ahiExtremas pointsTemplatePose = getExtremePointsFromBinaryImage(tPose, true);
    // pointsTemplatePose.ContourFaceWidth and pointsTemplatePose.ContourFaceHeight are available but not implemented here yet
    int yContourFaceCenter = pointsTemplatePose.yContourFaceCenter;
    int FaceDistThrdForInspection = pointsTemplatePose.FaceDistThrdForInspection;
    int yDetectedFaceCenter = faceRect.y + faceRect.height / 2;
    bool isFaceInAcceptableLocation;

    // x co-ordinate does not affect the height of the persons face, so don't factor it into our thresholding
    float heightDifference = abs(yContourFaceCenter - yDetectedFaceCenter);

    isFaceInAcceptableLocation = bool(heightDifference < FaceDistThrdForInspection);

    finalResult.FaceInExpectedContour = false;
    finalResult.CameraIsPotentiallyHigh = false;
    finalResult.CameraIsPotentiallyLow = false;

    if (isFaceInAcceptableLocation)
    {
        finalResult.FaceInExpectedContour = true;
    }
    else
    {
        LOG_GUARD(std::cout << THREAD_STR << "Face is not in expected location ..\n")
        if (yDetectedFaceCenter > yContourFaceCenter)
        {
            // detected face is appearing below the expected location
            // Should check ankle here too in the future
            finalResult.CameraIsPotentiallyHigh = true;
            LOG_GUARD(std::cout << THREAD_STR << "Camera is potentially too high ..\n")
        }
        else
        {
            // detected face is appearing above the expected location
            // Camera is lower than expected
            finalResult.CameraIsPotentiallyLow = true;
            LOG_GUARD(std::cout << THREAD_STR << "Camera is potentially too low ..\n")
        }
    }

    ahiExtremas pointsDetectedPose = getExtremePointsFromBinaryImage(dPose & tPose, false);

    cv::Rect headTopML, leftHandML, rightHandML, leftLegML, rightLegML;
    cv::Rect headTopTL, leftHandTL, rightHandTL, leftLegTL, rightLegTL;
    int lengthBox = faceRect.height;
    int widthBox = faceRect.width;
    int originx, originy;
    int lengthTemplateBox = 0, widthTemplateBox = 0;

    double tHandBoxConstH = 0.75 * (0.5 * 0.6); //RND
    double tHandBoxConstW = 0.75 * (0.5 * 0.6); //RND
    double tLegBoxConstH = 0.8 * (0.183); //RND
    double tLegBoxConstW = 2.0 * 0.123; //RND

    cv::Rect binaryImageBoxML, binaryImageBoxTL;

    for (int countJoints = 0; countJoints < 5; countJoints++)
    {
        switch (countJoints)
        {
            case 0: //head
                originx = (int) std::max(0.0, pointsTemplatePose.xTop - 0.8 * lengthBox);
                originy = (int) std::max(0.0, (double) pointsTemplatePose.yTop);
                headTopTL.x = originx;
                headTopTL.y = originy;
                headTopTL.height = (int) (1.2 * lengthBox);
                headTopTL.width = (int) (1.2 * widthBox);

                originx = (int) std::max(0, pointsDetectedPose.xTop - lengthBox / 2);
                originy = pointsTemplatePose.yTop;
                headTopML.x = originx;
                headTopML.y = originy;
                headTopML.height = lengthBox;
                headTopML.width = widthBox;

                //binaryImageBoxML = headTopML;//if using the output from posenet
                binaryImageBoxML = faceRect;
                binaryImageBoxTL = headTopTL;
                break;
            case 1: //left arm
                lengthTemplateBox = std::abs(pointsTemplatePose.yTop - pointsTemplatePose.yBottom);
                widthTemplateBox = std::abs(pointsTemplatePose.xLeft - pointsTemplatePose.xRight);
                lengthTemplateBox = (int) (tHandBoxConstH * lengthTemplateBox);
                widthTemplateBox = (int) (tHandBoxConstW * widthTemplateBox);

                originx = (int) std::max(0.0, (double) pointsTemplatePose.xLeft) - widthTemplateBox;
                originy = (int) std::max(0.0, (double) pointsTemplatePose.yRight) - widthTemplateBox;
                leftHandTL.x = originx;
                leftHandTL.y = originy;
                leftHandTL.width = widthTemplateBox;
                leftHandTL.height = widthTemplateBox;

                if (pointsDetectedPose.yLeft < 800)
                {
                    //originx = (int) std::max(0.0, pointsDetectedPose.xLeft - 0.5 * lengthBox);
                    //originy = (int) std::max(0.0, (double) pointsDetectedPose.yLeft);
                    originx = (int) std::max(0.0, (double) pointsDetectedPose.xLeft - lengthBox);
                    originy = (int) std::max(0.0, (double) pointsDetectedPose.yLeft - 0.5 * lengthBox);
                    leftHandML.x = originx;
                    leftHandML.y = originy;
                    leftHandML.height = lengthBox;
                    leftHandML.width = widthBox;
                }
                else
                {
                    leftHandML.x = (int) ((double)CAMERA_WIDTH - lengthBox);
                    leftHandML.y = 1;
                    leftHandML.height = lengthBox;
                    leftHandML.width = widthBox;
                }
                //
                binaryImageBoxML = leftHandML;
                binaryImageBoxTL = leftHandTL;
                break;
            case 2: //right arm
                lengthTemplateBox = std::abs(pointsTemplatePose.yTop - pointsTemplatePose.yBottom);
                widthTemplateBox = std::abs(pointsTemplatePose.xLeft - pointsTemplatePose.xRight);
                lengthTemplateBox = (int) (tHandBoxConstW * lengthTemplateBox);
                widthTemplateBox = (int) (tHandBoxConstH * widthTemplateBox);

                originx = (int) std::max(0.0, (double) pointsTemplatePose.xRight);
                originy =
                        (int) std::max(0.0, (double) pointsTemplatePose.yRight) - widthTemplateBox;
                rightHandTL.x = originx;
                rightHandTL.y = originy;
                rightHandTL.width = widthTemplateBox;
                rightHandTL.height = widthTemplateBox;


                if (pointsDetectedPose.yRight < 800)
                {
                    //originx = (int) std::max(0.0, pointsDetectedPose.xRight - 0.8 * lengthBox);
                    //originy = (int) std::max(0.0, (double) pointsDetectedPose.yRight);
                    originx = (int) std::max(0.0, (double) pointsDetectedPose.xRight);
                    originy = (int) std::max(0.0, (double) pointsDetectedPose.yRight - 0.5 * lengthBox);
                    rightHandML.x = originx;
                    rightHandML.y = originy;
                    rightHandML.height = lengthBox;
                    rightHandML.width = widthBox;
                }
                else
                {
                    rightHandML.x = 1;
                    rightHandML.y = 1;
                    rightHandML.height = lengthBox;
                    rightHandML.width = widthBox;
                }


                binaryImageBoxML = rightHandML;
                binaryImageBoxTL = rightHandTL;
                break;
            case 3: //left leg
                lengthTemplateBox = std::abs(pointsTemplatePose.yTop - pointsTemplatePose.yBottom);
                widthTemplateBox = std::abs(pointsTemplatePose.xLeft - pointsTemplatePose.xRight);

                lengthTemplateBox = (int) (tLegBoxConstH * lengthTemplateBox);
                widthTemplateBox = (int) (tLegBoxConstW * widthTemplateBox);

                originx = (int) (std::max(0, pointsTemplatePose.xBottomLeft) -
                                 widthTemplateBox);
                originy = (int) (std::max(0, pointsTemplatePose.yBottom) -
                                 1.05 * lengthTemplateBox);
                leftLegTL.x = originx;// - originx/10;
                leftLegTL.y = originy;
                leftLegTL.height = lengthTemplateBox;
                leftLegTL.width = widthTemplateBox;


                originx = (int) (std::max(0, pointsDetectedPose.xBottomLeft) - 1.2 * lengthBox);
                //originy = (int) (std::max(0, pointsDetectedPose.yBottom) - widthBox);
                originy = (int) (std::max(0, pointsDetectedPose.yBottomLeft) - 0.6 * widthBox);
                leftLegML.x = originx;
                leftLegML.y = originy;
                leftLegML.height = (int) (1.5 * lengthBox);//1.5
                leftLegML.width = (int) (1.2 * widthBox);//1.2

                binaryImageBoxML = leftLegML;
                binaryImageBoxTL = leftLegTL;
                break;
            case 4: //right leg
                lengthTemplateBox = std::abs(pointsTemplatePose.yTop - pointsTemplatePose.yBottom);
                widthTemplateBox = std::abs(pointsTemplatePose.xLeft - pointsTemplatePose.xRight);

                lengthTemplateBox = (int) (tLegBoxConstH * lengthTemplateBox);
                widthTemplateBox = (int) (tLegBoxConstW * widthTemplateBox);

                originx = (int) std::max(0, pointsTemplatePose.xBottomRight);
                originy = (int) (std::max(0, pointsTemplatePose.yBottom) -
                                 1.05 * lengthTemplateBox);
                rightLegTL.x = originx;
                rightLegTL.y = originy;
                rightLegTL.height = lengthTemplateBox;
                rightLegTL.width = widthTemplateBox;

                //originx = (int) (std::max(0.0, (double) pointsDetectedPose.xBottomRight) - 0.5 * lengthBox);
                //originy = (int) (std::max(0.0, (double) pointsDetectedPose.yBottom) - widthBox);
                originx = (int) (std::max(0.0, (double) pointsDetectedPose.xBottomRight));
                originy = (int) (std::max(0.0, (double) pointsDetectedPose.yBottomRight) - 0.6 * widthBox);
                rightLegML.x = originx;
                rightLegML.y = originy;
                rightLegML.height = (int) (1.5 * lengthBox);//1.5
                rightLegML.width = (int) (1.2 * widthBox);//1.3

                binaryImageBoxML = rightLegML;
                binaryImageBoxTL = rightLegTL;
                break;
            default:
                continue;
        }

        //get the template parts with the contour pose
        cv::Mat matBoxBinaryImageTL = getBinaryImageWithRect(tPose.size(), binaryImageBoxTL);
        cv::Mat matBoxBinaryImageML = getBinaryImageWithRect(dPose.size(), binaryImageBoxML);

        cv::Mat templateBinaryImage;
        cv::Mat matTemplatePose;

        cv::bitwise_and(matBoxBinaryImageTL, tPose, matBoxBinaryImageTL);
        cv::bitwise_and(matBoxBinaryImageML, dPose, matBoxBinaryImageML);
        cv::Mat intersectionBinaryImage;
        cv::bitwise_and(matBoxBinaryImageTL, matBoxBinaryImageML, intersectionBinaryImage);
        cv::Scalar sumIntersectionImage = cv::sum(intersectionBinaryImage / 255.0);
        cv::Scalar sumBinaryImageTL = cv::sum(matBoxBinaryImageTL / 255.0);
        cv::Scalar sumBinaryImageML = cv::sum(matBoxBinaryImageML / 255.0);
        double overlapratio1 = sumIntersectionImage.val[0] / (sumBinaryImageTL.val[0] + 1.e-10);
        double overlapratio2 = sumIntersectionImage.val[0] / (sumBinaryImageML.val[0] + 1.e-10);
        double overlapratio = cv::max(overlapratio1, overlapratio2); //debug and needed
        //cv::Scalar overlapratio = overlapratio2[0];

        //std::cout << "[inspectFrontPoseWithTemplatePose] joint:" << countJoints <<" overlap:" << overlapratio << std::endl;

        double threshold = 0.58;
        switch (countJoints)
        {
            case 0: //headTop
                finalResult.HeadTopOverlapRatio = (float) overlapratio;
                finalResult.HeadTopFromML = headTopML;
                finalResult.HeadTopFromTL = headTopTL;
                finalResult.FaceDetectedFromML = faceRect;
                if ((overlapratio > threshold) ||
                    (headTopTL.contains(centroids.CentroidHeadTop)))
                {
                    finalResult.HeadInContour = true;
                }
                break;
            case 1: // Let Hand
                finalResult.LeftHandOverlapRatio = (float) overlapratio;
                finalResult.LeftHandFromML = leftHandML;
                finalResult.LeftHandFromTL = leftHandTL;
                if ((overlapratio >= threshold) ||
                    (leftHandTL.contains(centroids.CentroidLeftHand)))
                    finalResult.LeftHandInContour = true;
                LOG_GUARD(std::cout << "Left hand overlap ratio: " << finalResult.LeftHandOverlapRatio << ". Threshold: " << threshold
                                    << ". Contains centroid: " << leftHandTL.contains(centroids.CentroidLeftHand) << ". From ML: "
                                    << finalResult.LeftHandFromML << ". From TL: " << finalResult.LeftHandFromTL << "." << std::endl)
                break;
            case 2: //right Hand
                finalResult.RightHandOverlapRatio = (float) overlapratio;
                finalResult.RightHandFromML = rightHandML;
                finalResult.RightHandFromTL = rightHandTL;
                if ((overlapratio >= threshold) ||
                    (rightHandTL.contains(centroids.CentroidRightHand)))
                    finalResult.RightHandInContour = true;
                LOG_GUARD(std::cout << "Right hand overlap ratio: " << finalResult.RightHandOverlapRatio << ". Threshold: " << threshold
                                    << ". Contains centroid: " << rightHandTL.contains(centroids.CentroidRightHand) << ". From ML: "
                                    << finalResult.RightHandFromML << ". From TL: " << finalResult.RightHandFromTL << "." << std::endl)
                break;
            case 3:
                finalResult.LeftLegOverlapRatio = (float) overlapratio;
                finalResult.LeftLegFromML = leftLegML;
                finalResult.LeftLegFromTL = leftLegTL;
                if ((overlapratio >= threshold) &&
                    (leftLegTL.contains(centroids.CentroidLeftAnkle)))
                    finalResult.LeftLegInContour = true;
                break;
            case 4:
                finalResult.RightLegOverlapRatio = (float) overlapratio;
                finalResult.RightLegFromML = rightLegML;
                finalResult.RightLegFromTL = rightLegTL;
                if ((overlapratio >= threshold) &&
                    (rightLegTL.contains(centroids.CentroidRightAnkle)))
                    finalResult.RightLegInContour = true;
                break;
            default:
                continue;
        }
    }
    return finalResult;
}
//This function performss the Side Pose Inspection
ahiSidePose
ahiFactoryTensor::inspectSidePoseWithTemplatePose(cv::Mat &dPose, cv::Mat const &tPose, cv::Rect faceRect,
                                                  ahiPoseInfo centroids)
{
    ahiSidePose finalResult{};
    faceRect.x = faceRect.x - faceRect.width / 2;
    faceRect.width = (int) (1.5 * faceRect.width);

    ahiExtremas pointsTemplatePose = getExtremePointsFromBinaryImage(tPose, true);
    std::vector<cv::Point> contourPoints;
    std::vector<cv::Vec4i> hierarchy;
    std::vector<std::vector<cv::Point> > contours;
    cv::Mat dPoseMasked = dPose & tPose;
    cv::findContours(dPoseMasked, contours, hierarchy, CV_RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    std::vector<cv::Rect> mlBoxes;

    for (auto &contour:contours)
    {
        cv::Rect boundRect = cv::boundingRect(contour);
        if (boundRect.y > (faceRect.y + faceRect.height + 50))
        {
            if (pointsTemplatePose.yBottom > boundRect.y)
            {
                mlBoxes.push_back(boundRect);
            }
        }

    }

    cv::Rect headML, handML, legML;
    cv::Rect headTL, handTL, legTL;
    int lengthTemplateBox, widthTemplateBox;
    auto tHandBoxConstH = (float) (0.5 * 0.2); //RND
    auto tHandBoxConstW = (float) 0.4; //RND
    auto tLegBoxConstH = (float) (0.35 * 0.5); //RND
    auto tLegBoxConstW = (float) 0.8; //RND

    if (!mlBoxes.empty())
    {
        int originx, originy;
        cv::Rect mlRect1, mlRect2;
        if (mlBoxes.size() == 1)
        {
            mlRect1 = mlBoxes[0];
            if (mlRect1.y > CAMERA_HEIGHT / 2 + 100)
            {
                legML = mlRect1;
            }
            else
            {
                handML = mlRect1;
            }
        }
        if (mlBoxes.size() >= 2)
        {
            mlRect1 = mlBoxes[0];
            mlRect2 = mlBoxes[1];

            if (mlRect1.y > mlRect2.y)
            {
                handML = mlRect2;
                legML = mlRect1;
            }
            else
            {
                handML = mlRect1;
                legML = mlRect2;
            }
        }

        cv::Rect binaryImageBoxML, binaryImageBoxTL;
        for (int cnt = 0; cnt < 3; cnt++)
        {
            switch (cnt)
            {
                case 0:
                    originx = (int) (std::max(0.0,
                                              pointsTemplatePose.xTop - 0.8 * faceRect.height));
                    originy = (int) (std::max(0.0, (double) pointsTemplatePose.yTop));
                    headTL.x = originx;
                    headTL.y = originy;
                    headTL.height = (int) (1.2 * faceRect.height);
                    headTL.width = (int) (1.2 * faceRect.width);
                    binaryImageBoxML = faceRect;
                    binaryImageBoxTL = headTL;
                    break;
                case 1:
                    lengthTemplateBox = std::abs(
                            pointsTemplatePose.yTop - pointsTemplatePose.yBottom);
                    widthTemplateBox = std::abs(
                            pointsTemplatePose.xLeft - pointsTemplatePose.xRight);
                    lengthTemplateBox = (int) (tHandBoxConstH * lengthTemplateBox);
                    widthTemplateBox = (int) (tHandBoxConstW * widthTemplateBox);
                    originx = (int) std::max(0.0, (CAMERA_WIDTH / 2.0 - widthTemplateBox));
                    originy = (int) std::max(0.0, (CAMERA_HEIGHT / 2.0 - lengthTemplateBox));

                    handTL.x = originx;
                    handTL.y = originy;
                    handTL.height = 2 * lengthTemplateBox;
                    handTL.width = 2 * widthTemplateBox;
                    binaryImageBoxML = handML;
                    binaryImageBoxTL = handTL;
                    break;
                case 2:
                    lengthTemplateBox = std::abs(
                            pointsTemplatePose.yTop - pointsTemplatePose.yBottom);
                    widthTemplateBox = std::abs(
                            pointsTemplatePose.xLeft - pointsTemplatePose.xRight);
                    lengthTemplateBox = (int) (tLegBoxConstH * lengthTemplateBox);
                    widthTemplateBox = (int) (tLegBoxConstW * widthTemplateBox);
                    originx = (int) std::max(0.0, (double) pointsTemplatePose.xBottomRight);
                    originy = (int) std::max(0.0, (double) pointsTemplatePose.yBottom -
                                                  lengthTemplateBox);

                    legTL.x = originx;
                    legTL.y = originy;
                    legTL.height = lengthTemplateBox;
                    legTL.width = widthTemplateBox;
                    binaryImageBoxML = legML;
                    binaryImageBoxTL = legTL;
                    break;
                default:
                    continue;
            }

            //get the binary rect image from the rect for IoU computation
            cv::Mat matBoxBinaryImageTL = getBinaryImageWithRect(tPose.size(), binaryImageBoxTL);
            cv::Mat matBoxBinaryImageML = getBinaryImageWithRect(dPose.size(), binaryImageBoxML);

            cv::Mat templateBinaryImage;
            cv::Mat matTemplatePose;

            cv::Mat intersectionBinaryImage;
            cv::bitwise_and(matBoxBinaryImageTL, matBoxBinaryImageML, intersectionBinaryImage);
            cv::Scalar sumIntersectionImage = cv::sum(intersectionBinaryImage / 255.0);
            cv::Scalar sumBinaryImageTL = cv::sum(matBoxBinaryImageTL / 255.0);
            cv::Scalar sumBinaryImageML = cv::sum(matBoxBinaryImageML / 255.0);
            double overlapratio1 = sumIntersectionImage.val[0] / (sumBinaryImageTL.val[0] + 0.00001);
            double overlapratio2 = sumIntersectionImage.val[0] / (sumBinaryImageML.val[0] + 0.00001);
            double overlapratio = cv::max(overlapratio1, overlapratio2); //debug and needed
            double threshold = 0.60;
            switch (cnt)
            {
                case 0:
                    finalResult.HeadTopOverlapRatio = (float) overlapratio;
                    finalResult.HeadTopFromML = headML;
                    finalResult.HeadTopFromTL = headTL;
                    finalResult.FaceDetectedFromML = faceRect;
                    if ((overlapratio > threshold) ||
                        (headTL.contains(centroids.CentroidHeadTop)))
                        finalResult.HeadInContour = true;
                    else
                        finalResult.HeadInContour = false;
                    break;
                case 1:
                    finalResult.HandOverlapRatio = (float) overlapratio;
                    finalResult.HandFromML = handML;
                    finalResult.HandFromTL = handTL;
                    std::cout << "Hand overlap ratio B: " << overlapratio << ". Threshold: " << threshold << "." << std::endl;
                    if ((overlapratio >= threshold) ||
                        (handTL.contains(centroids.CentroidRightHand)))
                        finalResult.HandInContour = true;
                    else
                        finalResult.HandInContour = false;
                    break;
                case 2:
                    finalResult.LegOverlapRatio = (float) overlapratio;
                    finalResult.LegFromML = legML;
                    finalResult.LegFromTL = legTL;
                    if ((overlapratio >= threshold) &&
                        (legTL.contains(centroids.CentroidRightAnkle)))
                        finalResult.LegInContour = true;
                    else
                        finalResult.LegInContour = false;
                    break;
                default:
                    continue;
            }
        }
    }
    else
    {
        finalResult.HandOverlapRatio = 0.0;
        finalResult.LegOverlapRatio = 0.0;
        finalResult.HeadTopOverlapRatio = 0.0;
        finalResult.HandInContour = false;
        finalResult.LegInContour = false;
        finalResult.HeadInContour = false;
    }

    return finalResult;
}
#endif
///////////////////////////////////////////////
bool isRectContainsPoint(cv::Rect rect, cv::Point P)
{
    try {
        return ((P.x > rect.x) && (P.y > rect.y) && (P.x < rect.x + rect.width) &&
                (P.y < rect.y + rect.height));
    }
    catch (cv::Exception &e)
    {
        return false;
    }
}
/////////////////////////////////////////////////////////
//This function inspect the given pose to the template pose
ahiFrontPose ahiFactoryTensor::inspectFrontPoseWithDetectedPosePlusContour(cv::Mat const &tPose,
                                           cv::Rect const faceRect, ahiPoseInfo& frontPoseInfo, int yTopUp, int yTopLow, int yBotUp, int yBotLow)
{
    ahiFrontPose finalResult{};
    ahiExtremas pointsTemplatePose = getExtremePointsFromBinaryImage(tPose, true);
    // pointsTemplatePose.ContourFaceWidth and pointsTemplatePose.ContourFaceHeight are available but not implemented here yet
    int yContourFaceCenter = pointsTemplatePose.yContourFaceCenter;
    int FaceDistThrdForInspection = pointsTemplatePose.FaceDistThrdForInspection;
    int yDetectedFaceCenter = faceRect.y + faceRect.height / 2;
    bool isFaceInAcceptableLocation;

    // x co-ordinate does not affect the height of the persons face, so don't factor it into our thresholding
    float heightDifference = abs(yContourFaceCenter - yDetectedFaceCenter);

    isFaceInAcceptableLocation = bool(heightDifference < FaceDistThrdForInspection);

    finalResult.FaceInExpectedContour = false;
    finalResult.CameraIsPotentiallyHigh = false;
    finalResult.CameraIsPotentiallyLow = false;

    if (isFaceInAcceptableLocation)
    {
        finalResult.FaceInExpectedContour = true;
    }
    else
    {
        LOG_GUARD(std::cout << THREAD_STR << "Face is not in expected location ..\n")
        if (yDetectedFaceCenter > yContourFaceCenter)
        {
            // detected face is appearing below the expected location
            // Should check ankle here too in the future
            finalResult.CameraIsPotentiallyHigh = true;
            LOG_GUARD(std::cout << THREAD_STR << "Camera is potentially too high ..\n")
        }
        else
        {
            // detected face is appearing above the expected location
            // Camera is lower than expected
            finalResult.CameraIsPotentiallyLow = true;
            LOG_GUARD(std::cout << THREAD_STR << "Camera is potentially too low ..\n")
        }
    }

//    cv::Rect headTopML, leftHandML, rightHandML, leftLegML, rightLegML;
//    cv::Rect headTopTL, leftHandTL, rightHandTL, leftLegTL, rightLegTL;
//    int lengthBox = faceRect.height;
//    int widthBox = faceRect.width;

    int widthBox = faceRect.width * 1.2; // just init, it must be contour dep
    int heightBox = faceRect.height * 1.2; //just init, it must be contour dep

    int originx, originy;

    int lengthTemplateBox = 0, widthTemplateBox = 0;

    double tHandBoxConstH = 0.75 * (0.5 * 0.6); //RND
    double tHandBoxConstW = 0.75 * (0.5 * 0.6); //RND
    double tLegBoxConstH = 0.8 * (0.183); //RND
    double tLegBoxConstW = 2.0 * 0.123; //RND

    cv::Rect binaryImageBoxML, binaryImageBoxTL;
    cv::Point posePoint;
    cv::Mat matPoseInspecROI = tPose.clone();
    for (int countJoints = 0; countJoints < 5; countJoints++)
    {
        switch (countJoints)
        {
            case 0: //head
            {
                int ContourFaceWidth = 2* (pointsTemplatePose.yContourFaceCenter - pointsTemplatePose.yTop);
                if( abs(pointsTemplatePose.xTop- tPose.cols/2) > 5)
                {
                    pointsTemplatePose.xTop = tPose.cols/2;
                }
                widthBox = ContourFaceWidth;
                heightBox = ContourFaceWidth;
                originx = MAX(0, pointsTemplatePose.xTop) - 0.5 * ContourFaceWidth;

                int yPoint = pointsTemplatePose.yTop;
                //posePoint = cv::Point(faceRect.x + faceRect.width / 2,faceRect.y + faceRect.height / 2);

//                if (!CGRectEqualToRect(CGRectZero, idealZoneHead)) {
//                    yPoint = [[NSNumber numberWithFloat:idealZoneHead.origin.y] integerValue];
//                    heightBox = idealZoneHead.size.height;
//                }
//                if ([poseDictionary valueForKey:@"CentroidHeadTop"]) {
//            CGPoint headTopPoint = [poseDictionary[@"CentroidHeadTop"] CGPointValue];
//            posePoint = headTopPoint;
//        }
                posePoint = frontPoseInfo.CentroidHeadTop;

                originy = MAX(0, pointsTemplatePose.yTop - 20);//yPoint);
                binaryImageBoxTL = cv::Rect(originx, originy, widthBox, heightBox);
                finalResult.FaceInExpectedContour = isRectContainsPoint(binaryImageBoxTL, posePoint);
                break;
            }
            case 1: //left arm
            {
                //iOS
                // LeftHand
//                resultPart = @"LA";
//                NSInteger lengthTemplateBox = MFZ_INSPECT_tHandBoxConstH_front * ABS([[pointsTemplatePose objectForKey:@"yTop"] integerValue] - [[pointsTemplatePose objectForKey:@"yBottom"] integerValue]);
//                NSInteger widthTemplateBox = MFZ_INSPECT_tHandBoxConstW_front * ABS([[pointsTemplatePose objectForKey:@"xLeft"] integerValue] - [[pointsTemplatePose objectForKey:@"xRight"] integerValue]);
//                NSInteger originx = MAX(0, [[pointsTemplatePose objectForKey:@"xLeft"] integerValue] - widthTemplateBox);
//                NSInteger originy = MAX(0, [[pointsTemplatePose objectForKey:@"yRight"] intValue] - (lengthTemplateBox * 0.7));
//                binaryImageBoxTL = CGRectMake(originx, originy, widthTemplateBox, lengthTemplateBox);
//                posePoint = [poseDictionary[@"CentroidLeftHand"] CGPointValue];

                //left arm
                lengthTemplateBox = std::abs(pointsTemplatePose.yTop - pointsTemplatePose.yBottom);
                widthTemplateBox = std::abs(pointsTemplatePose.xLeft - pointsTemplatePose.xRight);
                lengthTemplateBox = (int) (tHandBoxConstH * lengthTemplateBox);
                widthTemplateBox = (int) (tHandBoxConstW * widthTemplateBox);

                originx = (int) std::max(0.0, (double) pointsTemplatePose.xLeft) - widthTemplateBox;
                originy = (int) std::max(0.0, (double) pointsTemplatePose.yRight) - widthTemplateBox;
//                leftHandTL.x = originx;
//                leftHandTL.y = originy;
//                leftHandTL.width = widthTemplateBox;
//                leftHandTL.height = widthTemplateBox;

                binaryImageBoxTL = cv::Rect(originx, originy, widthTemplateBox, lengthTemplateBox);
                posePoint = frontPoseInfo.CentroidLeftHand;
                finalResult.LA = isRectContainsPoint(binaryImageBoxTL, posePoint);
                break;
            }
            case 2: //right arm
            {
                //iOS
                // RightHand
//                resultPart = @"RA";
//                NSInteger lengthTemplateBox = MFZ_INSPECT_tHandBoxConstH_front * ABS([[pointsTemplatePose objectForKey:@"yTop"] integerValue] - [[pointsTemplatePose objectForKey:@"yBottom"] integerValue]);
//                NSInteger widthTemplateBox = MFZ_INSPECT_tHandBoxConstW_front * ABS([[pointsTemplatePose objectForKey:@"xLeft"] integerValue] - [[pointsTemplatePose objectForKey:@"xRight"] integerValue]);
//                NSInteger originx = MAX(0, [[pointsTemplatePose objectForKey:@"xRight"] integerValue]);
//                NSInteger originy = MAX(0, [[pointsTemplatePose objectForKey:@"yRight"] intValue] - (lengthTemplateBox * 0.7));
//                binaryImageBoxTL = CGRectMake(originx, originy, widthTemplateBox, lengthTemplateBox);
//                posePoint = [poseDictionary[@"CentroidRightHand"] CGPointValue];
                //right arm

                lengthTemplateBox = std::abs(pointsTemplatePose.yTop - pointsTemplatePose.yBottom);
                widthTemplateBox = std::abs(pointsTemplatePose.xLeft - pointsTemplatePose.xRight);
                lengthTemplateBox = (int) (tHandBoxConstW * lengthTemplateBox);
                widthTemplateBox = (int) (tHandBoxConstH * widthTemplateBox);

                originx = (int) std::max(0.0, (double) pointsTemplatePose.xRight);
                originy =
                        (int) std::max(0.0, (double) pointsTemplatePose.yRight) - widthTemplateBox;

//                rightHandTL.x = originx;
//                rightHandTL.y = originy;
//                rightHandTL.width = widthTemplateBox;
//                rightHandTL.height = widthTemplateBox;

                binaryImageBoxTL = cv::Rect(originx, originy, widthTemplateBox, lengthTemplateBox);
                posePoint = frontPoseInfo.CentroidRightHand;
                finalResult.RA = isRectContainsPoint(binaryImageBoxTL, posePoint);

                break;
            }
            case 3: //left leg
            {
                //iOS
//                resultPart = @"LL";
//                NSInteger lengthTemplateBox = MFZ_INSPECT_tLegBoxConstH_front * ABS([[pointsTemplatePose objectForKey:@"yTop"] integerValue] - [[pointsTemplatePose objectForKey:@"yBottom"] integerValue]);
//                NSInteger widthTemplateBox = MFZ_INSPECT_tLegBoxConstW_front * ABS([[pointsTemplatePose objectForKey:@"xLeft"] integerValue] - [[pointsTemplatePose objectForKey:@"xRight"] integerValue]);
//                NSInteger originx = MAX(0, [[pointsTemplatePose objectForKey:@"xBottomLeft"] integerValue] - widthTemplateBox);
//                NSInteger originy = MAX(0, [[pointsTemplatePose objectForKey:@"yBottom"] integerValue] - (1.05 * lengthTemplateBox));
//                originy = originy - (originy / 20);
//                if (!CGRectEqualToRect(CGRectZero, idealZoneAnkles)) {
//                    originy = [[NSNumber numberWithFloat:idealZoneAnkles.origin.y] integerValue];;
//                    lengthTemplateBox = idealZoneAnkles.size.height;
//                }
//                binaryImageBoxTL = CGRectMake(originx, originy, widthTemplateBox, lengthTemplateBox);
//                posePoint = [poseDictionary[@"CentroidLeftAnkle"] CGPointValue];

                lengthTemplateBox = std::abs(pointsTemplatePose.yTop - pointsTemplatePose.yBottom);
                widthTemplateBox = std::abs(pointsTemplatePose.xLeft - pointsTemplatePose.xRight);

                lengthTemplateBox = (int) (tLegBoxConstH * lengthTemplateBox);
                widthTemplateBox = (int) (tLegBoxConstW * widthTemplateBox);

                originx = (int) (std::max(0, pointsTemplatePose.xBottomLeft) - widthTemplateBox);
                originy = (int) (std::max(0, pointsTemplatePose.yBottom) - 1.05 * lengthTemplateBox);

                binaryImageBoxTL = cv::Rect(originx, originy, widthTemplateBox, lengthTemplateBox);
                posePoint = frontPoseInfo.CentroidLeftAnkle;
                finalResult.LL = isRectContainsPoint(binaryImageBoxTL, posePoint);

                break;
            }
            case 4: //right leg
            {
                lengthTemplateBox = std::abs(pointsTemplatePose.yTop - pointsTemplatePose.yBottom);
                widthTemplateBox = std::abs(pointsTemplatePose.xLeft - pointsTemplatePose.xRight);

                lengthTemplateBox = (int) (tLegBoxConstH * lengthTemplateBox);
                widthTemplateBox = (int) (tLegBoxConstW * widthTemplateBox);

                originx = (int) std::max(0, pointsTemplatePose.xBottomRight);
                originy = (int) (std::max(0, pointsTemplatePose.yBottom) - 1.05 * lengthTemplateBox);

                binaryImageBoxTL = cv::Rect(originx, originy, widthTemplateBox, lengthTemplateBox);
                posePoint = frontPoseInfo.CentroidRightAnkle;
                finalResult.RL = isRectContainsPoint(binaryImageBoxTL, posePoint);
                break;
            }
            default:
                continue;
        }
        cv::Scalar COLOR(255/(countJoints+2),255/(countJoints+2), 255/(countJoints+2)); // just RND color
        cv::rectangle(matPoseInspecROI, binaryImageBoxTL, COLOR, 2);
        cv::circle(matPoseInspecROI, posePoint, 25, COLOR, -1);
    }


    cv::imwrite("/data/data/io.ahi.example/frontPoseInspecROI.bmp", matPoseInspecROI);
    return finalResult;
}

///////////////////////////////////
ahiSidePose ahiFactoryTensor::inspectSidePoseWithDetectedPosePlusContour(cv::Mat const &tPose, cv::Rect const faceRect, ahiPoseInfo& sidePoseInfo, int yTopUp, int yTopLow, int yBotUp, int yBotLow)
{
    ahiSidePose finalResult{};
//    faceRect.x = faceRect.x - faceRect.width / 2;
//    faceRect.width = (int) (1.5 * faceRect.width);

    if(faceRect.x < 0) //
    {
        finalResult.HandInContour = false;
        finalResult.LegInContour = false;
        finalResult.HeadInContour = false;
        finalResult.Face = 0;
        finalResult.UB = false;
        finalResult.LB = false;

        return finalResult;
    }

    ahiExtremas pointsTemplatePose = getExtremePointsFromBinaryImage(tPose, true);


    cv::Rect headML, handML, legML;
    cv::Rect headTL, handTL, legTL;
    int lengthTemplateBox, widthTemplateBox;
    auto tHandBoxConstH = (float) (0.5 * 0.2); //RND
    auto tHandBoxConstW = (float) 0.4; //RND
    auto tLegBoxConstH = (float) (0.35 * 0.5); //RND
    auto tLegBoxConstW = (float) 0.8; //RND

    int originx, originy;
    cv::Point posePoint;

    cv::Rect binaryImageBoxML, binaryImageBoxTL;
    for (int cnt = 0; cnt < 5; cnt++) {
        switch (cnt) {
            case 0: {
//                originx = (int) (std::max(0.0, pointsTemplatePose.xTop - 0.8 * faceRect.height));
                int ContourFaceWidth =
                        2.2 * (pointsTemplatePose.yContourFaceCenter - pointsTemplatePose.yTop);

                originx = (int) (std::max(0.0, pointsTemplatePose.xTop - 0.5 * ContourFaceWidth));
                originy = (int) (std::max(0.0, (double) pointsTemplatePose.yTop) - 20);

//                headTL.x = originx;
//                headTL.y = originy;
//                headTL.height = ContourFaceWidth;//(int) (1.2 * faceRect.height);
//                headTL.width = ContourFaceWidth;//(int) (1.2 * faceRect.width);
                binaryImageBoxTL = cv::Rect(originx, originy, ContourFaceWidth, ContourFaceWidth);
                posePoint = sidePoseInfo.CentroidHeadTop;
                finalResult.FaceInExpectedContour = isRectContainsPoint(binaryImageBoxTL,posePoint);
                break;
            }
            case 1: // LA
            {
                lengthTemplateBox = std::abs(pointsTemplatePose.yTop - pointsTemplatePose.yBottom);
                widthTemplateBox = std::abs(pointsTemplatePose.xLeft - pointsTemplatePose.xRight);
                lengthTemplateBox = (int) (tHandBoxConstH * lengthTemplateBox);
                widthTemplateBox = (int) (tHandBoxConstW * widthTemplateBox);
                originx = (int) std::max(0.0, (CAMERA_WIDTH / 2.0 - widthTemplateBox));
                originy = (int) std::max(0.0, (CAMERA_HEIGHT / 2.0 - lengthTemplateBox));

                binaryImageBoxTL = cv::Rect(originx, originy, widthTemplateBox, lengthTemplateBox);
                posePoint = sidePoseInfo.CentroidLeftHand;
                finalResult.LA = isRectContainsPoint(binaryImageBoxTL, posePoint);
                break;
            }
            case 2: // RA
            {
                lengthTemplateBox = std::abs(pointsTemplatePose.yTop - pointsTemplatePose.yBottom);
                widthTemplateBox = std::abs(pointsTemplatePose.xLeft - pointsTemplatePose.xRight);
                lengthTemplateBox = (int) (tHandBoxConstH * lengthTemplateBox);
                widthTemplateBox = (int) (tHandBoxConstW * widthTemplateBox);
                originx = (int) std::max(0.0, (CAMERA_WIDTH / 2.0 - widthTemplateBox));
                originy = (int) std::max(0.0, (CAMERA_HEIGHT / 2.0 - lengthTemplateBox));

                binaryImageBoxTL = cv::Rect(originx, originy, widthTemplateBox, lengthTemplateBox);
                posePoint = sidePoseInfo.CentroidRightHand;
                finalResult.LA = isRectContainsPoint(binaryImageBoxTL, posePoint);
                break;
            }
            case 3://LL
            {
                lengthTemplateBox = std::abs(pointsTemplatePose.yTop - pointsTemplatePose.yBottom);
                widthTemplateBox = std::abs(pointsTemplatePose.xLeft - pointsTemplatePose.xRight);
                lengthTemplateBox = (int) (tLegBoxConstH * lengthTemplateBox);
                widthTemplateBox = (int) (tLegBoxConstW * widthTemplateBox);
                originx = (int) std::max(0.0, (double) pointsTemplatePose.xBottomRight);
                originy = (int) std::max(0.0,
                                         (double) pointsTemplatePose.yBottom - lengthTemplateBox);

                binaryImageBoxTL = cv::Rect(originx, originy, widthTemplateBox, lengthTemplateBox);
                posePoint = sidePoseInfo.CentroidLeftAnkle;
                finalResult.LL = isRectContainsPoint(binaryImageBoxTL, posePoint);
                break;
            }
            case 4://RL
            {
                lengthTemplateBox = std::abs(pointsTemplatePose.yTop - pointsTemplatePose.yBottom);
                widthTemplateBox = std::abs(pointsTemplatePose.xLeft - pointsTemplatePose.xRight);
                lengthTemplateBox = (int) (tLegBoxConstH * lengthTemplateBox);
                widthTemplateBox = (int) (tLegBoxConstW * widthTemplateBox);
                originx = (int) std::max(0.0, (double) pointsTemplatePose.xBottomRight);
                originy = (int) std::max(0.0, (double) pointsTemplatePose.yBottom - lengthTemplateBox);

                binaryImageBoxTL = cv::Rect(originx, originy, widthTemplateBox, lengthTemplateBox);
                posePoint = sidePoseInfo.CentroidRightAnkle;
                finalResult.RL = isRectContainsPoint(binaryImageBoxTL, posePoint);
                break;
            }
            default:
                continue;
        }
    }
    finalResult.UB = finalResult.RA || finalResult.LA;
    finalResult.LB = finalResult.RL || finalResult.LL;
    return finalResult;
}


void ahiFactoryTensor::releaseDelegate()
{
#if defined(ANDROID) || defined(__ANDROID__)
//    switch (mDelegateType)
//    {
//        case FactoryTensorDelegateCPU:
//            break;
//        case FactoryTensorDelegateGPU:
//            if (nullptr != mDelegate)
//            {
//                TfLiteGpuDelegateV2Delete(mDelegate);//DelegateDelete(mDelegate);
//                mDelegate = nullptr;
//            }
//            break;
//        case FactoryTensorDelegateNNAPI:
//            if (nullptr != mDelegate)
//            {
//                delete mDelegate;
//                mDelegate = nullptr;
//            }
//            break;
//    }
#endif
}

//function that returns the joints
bool ahiFactoryTensor::predictFromHeatMap(cv::Mat const &oimage, cv::Mat const &ocontour,
                                       std::string const &view,
                                       cv::Mat &binaryImageJoints, bool &isMask,
                                       std::vector<cv::Point> &pojo_joints,
                                       ahiPoseInfo &jointsPrediction)
{
    //declare the constant for the heatmap
    int pWidth = 192;
    int pHeight = 192;
    int const numChanel = 15;
    int const numRow = 96;//40;
    int const numCol = 96;//40;
    cv::Rect rect;
    int countHeatmapResults = 0;
    bool headFound = false;
    bool leftHandFound = false;
    bool rightHandFound = false;
    bool leftLegFound = false;
    bool rightLegFound = false;
    //bool bVisualise = (nullptr != mVisualize);

    int const ds = 5;
    cv::Mat pimage, pcontour;
    int top, bottom, left, right;


    for (auto it = ahiModelGenderMap.begin(); it != ahiModelGenderMap.end(); it++)
    {
        std::string filename = it->first;
        if (filename == "pose_light")
        {
            std::string filename = it->first;
            filename.append(".bin");
        }
    }
    //TODO
    //Amar22 modelDecode(filename); // to get .tflite model

//    loadModel(const char buffer, size_t buffer_size )
    //set/init/define the ahiFactoryTensor class
    //load the model and get the mInterpretor

//    ModelMap::iterator iter = mModelMap.find(FactoryTensorModelId::ModelPose);
//    if (mModelMap.end() != iter)
//    {
        pWidth = getInputDim(0, 1);
        pHeight = getInputDim(0, 2);
//    }

    processImage(oimage, ocontour, cv::Size(pWidth, pHeight), pimage, pcontour, top, bottom, left,
                 right, isMask);

    pimage.convertTo(pimage, CV_32F);

    if (pcontour.size().empty())
    {
        AHILog(ANDROID_LOG_ERROR, "pcontour size was empty.");
        return false;
    }

    // Queue the TFLite operation and wait for the result...
    unique_ptr<FactoryTensorOp> op;
    FactoryTensorOpQueue resultQueue(0);
    mQueue.push(move(unique_ptr<FactoryTensorOp>(new FactoryTensorOp(
            FactoryTensorOpPose, pimage, &resultQueue))));
    send(FactoryTensorOpPose);
    if (resultQueue.popWait(op) && op->hasOutput(0))
    {
        cv::Mat outputBlob = op->output(0);

        if (!op->mInvokeSuccess)
        {
            LOG_GUARD(std::cout << "predictFromHeatMap() received an empty result" << std::endl)
            return false;
        }

        //perform the post processing of the heatMap
        int inputImageMaxHW = std::max(oimage.rows, oimage.cols);
        cv::Size inputImageMaxSize = cv::Size(inputImageMaxHW, inputImageMaxHW);
        cv::Size heatMapSize = cv::Size(inputImageMaxHW / ds, inputImageMaxHW / ds);
        //cv::Point2f pc(numRow / 2.0, numCol / 2.0);
        cv::Mat heatMapGlobal = cv::Mat::zeros(heatMapSize, CV_32F);
        cv::Mat binaryImageGlobal = cv::Mat::zeros(inputImageMaxHW / ds, inputImageMaxHW / ds,
                                                   CV_8U);
        rect.x = right;
        rect.y = top;
        rect.width = inputImageMaxHW - 2 * right;
        rect.height = inputImageMaxHW - 2 * top;

        cv::Mat pcontour_new;
        cv::resize(pcontour, pcontour_new, cv::Size(40, 40), cv::INTER_NEAREST);

        for (int channel = 1; channel < numChanel; channel++)
        {
            cv::Mat heatMapEach = cv::Mat::zeros(numRow, numCol, CV_32F);

            for (int i = 0; i < numRow; i++)
            {
                for (int j = 0; j < numCol; j++)
                {
                    auto featureValue = (float) (outputBlob.at<cv::Vec4f>(0, i, j)[channel - 1]);
                    if (featureValue > 1.0)
                    { featureValue = 1.0; }
                    if (featureValue < 0.1) // 0.1 was for iOs based
                    { featureValue = 0.0; }
                    heatMapEach.at<float>(cv::Point(j, i)) = featureValue;
                }
            }

            cv::resize(heatMapEach, heatMapEach, cv::Size(40, 40), cv::INTER_AREA);
            cv::Point min_loc, max_loc;
            double min_val, max_val;
            minMaxLoc(heatMapEach, &min_val, &max_val, &min_loc, &max_loc);

            //refine the heatmap based on the contours
            heatMapEach = refineHeatMap(heatMapEach, pcontour_new, view, channel);
            cv::resize(heatMapEach, heatMapEach, heatMapSize, cv::INTER_CUBIC);
            heatMapEach = 255. * heatMapEach / (max_val + 1.0e-10);
            heatMapGlobal = heatMapGlobal + heatMapEach;

            cv::Mat binaryImageEachHeatMap = clusterHeatmap(heatMapEach, 2);
            if (channel == 1 || channel == 5 || channel == 8 || channel == 11 || channel == 14)
            {
                binaryImageGlobal += binaryImageEachHeatMap;
            }

            heatMapEach.convertTo(heatMapEach, CV_8U);
            cv::Point jointCentroid_ = getCentreHeatMap(heatMapEach,
                                                        binaryImageEachHeatMap,
                                                        channel);
            //map jointCentroid to actual image size;
            cv::Point jointCentroid;
            jointCentroid.x = (int) (jointCentroid_.x * 4.986 - 278.222);
            jointCentroid.y = (int) (jointCentroid_.y * 5.0314960 - 4.031496);
            if (channel == 1)
            {
                jointsPrediction.CentroidHeadTop = jointCentroid;
                headFound = (jointCentroid.x > 0 && jointCentroid.y > 0);
            }
            if (channel == 2)
            { jointsPrediction.CentroidNeck = jointCentroid; }
            if (channel == 3)
            { jointsPrediction.CentroidRightShoulder = jointCentroid; }
            if (channel == 6)
            { jointsPrediction.CentroidLeftShoulder = jointCentroid; }
            if (channel == 4)
            { jointsPrediction.CentroidRightElbow = jointCentroid; }
            if (channel == 7)
            { jointsPrediction.CentroidLeftElbow = jointCentroid; }
            if (channel == 5)
            {
                jointsPrediction.CentroidRightHand = jointCentroid;
                rightHandFound = (jointCentroid.x > 0 && jointCentroid.y > 0);
            }
            if (channel == 8)
            {
                jointsPrediction.CentroidLeftHand = jointCentroid;
                leftHandFound = (jointCentroid.x > 0 && jointCentroid.y > 0);
            }
            if (channel == 9)
            { jointsPrediction.CentroidRightHip = jointCentroid; }
            if (channel == 12)
            { jointsPrediction.CentroidLeftHip = jointCentroid; }
            if (channel == 10)
            { jointsPrediction.CentroidRightKnee = jointCentroid; }
            if (channel == 13)
            { jointsPrediction.CentroidLeftKnee = jointCentroid; }
            if (channel == 11)
            {
                jointsPrediction.CentroidRightAnkle = jointCentroid;
                rightLegFound = (jointCentroid.x > 0 && jointCentroid.y > 0);
            }
            if (channel == 14)
            {
                jointsPrediction.CentroidLeftAnkle = jointCentroid;
                leftLegFound = (jointCentroid.x > 0 && jointCentroid.y > 0);
            }
        }
        cv::Point minLoc, maxLoc;
        double minVal, maxVal;
        //LOG_INFO(LOG_GLOBAL, "predictFromHeatMap - minMaxLoc");
        minMaxLoc(heatMapGlobal, &minVal, &maxVal, &minLoc, &maxLoc);
        heatMapGlobal = heatMapGlobal / (maxVal + 1e-10) * 255.0;
        heatMapGlobal.convertTo(heatMapGlobal, CV_8U);
        binaryImageGlobal.convertTo(binaryImageGlobal, CV_8U);

        cv::resize(binaryImageGlobal, binaryImageGlobal, inputImageMaxSize, cv::INTER_CUBIC);// resize to net test size
        cv::threshold(binaryImageGlobal, binaryImageGlobal, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
        binaryImageJoints = binaryImageGlobal(rect);

        //This is for verying if retattempt process is needed or not
        if (view == "side")
        {
            if (headFound)
            { countHeatmapResults++; }
            if (rightHandFound || leftHandFound)
            { countHeatmapResults += 2; }
            if (rightLegFound || leftLegFound)
            { countHeatmapResults += 2; }
        }
        else
        {
            if (headFound)
            { countHeatmapResults++; }
            if (rightHandFound)
            { countHeatmapResults++; }
            if (leftHandFound)
            { countHeatmapResults++; }
            if (rightLegFound)
            { countHeatmapResults++; }
            if (leftLegFound)
            { countHeatmapResults++; }

        }

        if ((countHeatmapResults >= 2) && (countHeatmapResults <= 5))
        {
            jointsPrediction.threeOutOfFiveRule = true;
        }

        return true;
    }

    return false;
}

bool ahiFactoryTensor::predictFromHeatMapOpt(cv::Mat const &oimage, cv::Mat const &ocontour,
                                          std::string const &view,
                                          bool &isMask,
                                          std::vector<cv::Point> &pojo_joints,
                                          ahiPoseInfo &jointsPrediction)
{
    //declare the constant for the heatmap
    int pWidth = 192;
    int pHeight = 192;
    int const numChanel = 15;
    int const numRow = 96;//40;
    int const numCol = 96;//40;
    cv::Rect rect;
    int countHeatmapResults = 0;
    bool headFound = false;
    bool leftHandFound = false;
    bool rightHandFound = false;
    bool leftLegFound = false;
    bool rightLegFound = false;
    //bool bVisualise = (nullptr != mVisualize);

    int const ds = 5;
    cv::Mat pimage, pcontour;
    int top, bottom, left, right;

    //Stopwatch predictFromHeatMap_pre("predictFromHeatMapOpt pre");

    ModelMap::iterator iter = mModelMap.find(FactoryTensorModelId::ModelPose);
    if (mModelMap.end() != iter)
    {
        pWidth = getInputDim(0, 1);
        pHeight = getInputDim(0, 2);
    }

    processImage(oimage, ocontour, cv::Size(pWidth, pHeight), pimage, pcontour, top, bottom, left,
                 right, isMask);

    pimage.convertTo(pimage, CV_32F);

    if (pcontour.size().empty())
    {
        std::cout << "predictFromHeatMapOpt: pcontour size was empty." << std::endl;
        return false;
    }

    //predictFromHeatMap_pre.print();
    //Stopwatch predictFromHeatMap_tensor("predictFromHeatMapOpt tensor");

    // Queue the TFLite operation and wait for the result...
    unique_ptr<FactoryTensorOp> op;
    FactoryTensorOpQueue resultQueue(0);
    mQueue.push(move(unique_ptr<FactoryTensorOp>(new FactoryTensorOp(
            FactoryTensorOpPose, pimage, &resultQueue))));
    send(FactoryTensorOpPose);

    if (resultQueue.popWait(op) && op->hasOutput(0))
    {
        //ahiTensorOutputMap PoseOutputs = op-> mOutputs;// name and values
        cv::Mat outputBlob = op->output(0);

        if (!op->mInvokeSuccess)
        {
            LOG_GUARD(std::cout << "predictFromHeatMapOpt() received an empty result" << std::endl)
            return false;
        }

        //perform the post processing of the heatMap
//        int inputImageMaxHW = std::max(oimage.rows, oimage.cols);
//        cv::Size inputImageMaxSize = cv::Size(inputImageMaxHW, inputImageMaxHW);
//        cv::Size heatMapSize = cv::Size(inputImageMaxHW / ds, inputImageMaxHW / ds);
//        //cv::Point2f pc(numRow / 2.0, numCol / 2.0);
//        rect.x = right;
//        rect.y = top;
//        rect.width = inputImageMaxHW - 2 * right;
//        rect.height = inputImageMaxHW - 2 * top;
//
//        cv::Mat pcontour_new;
        //cv::resize(pcontour, pcontour_new, cv::Size(40, 40), cv::INTER_NEAREST);

        //predictFromHeatMap_tensor.print();
        //Stopwatch predictFromHeatMap_proc("predictFromHeatMapOpt proc");

        std::vector<float> heatMapEachSum(14,-1);
        std::vector<float> heatMapEachRadius(14,-1);
        for (int channel = 1; channel < numChanel; channel++)
        {
            cv::Mat heatMapEach = cv::Mat::zeros(numRow, numCol, CV_32F);

            for (int i = 0; i < numRow; i++)
            {
                for (int j = 0; j < numCol; j++)
                {
                    auto featureValue = (float) (outputBlob.at<cv::Vec4f>(0, i, j)[channel - 1]);
                    if (featureValue > 1.0)
                    { featureValue = 1.0; }
                    if (featureValue < 0.1) // 0.1 was for iOs based
                    { featureValue = 0.0; }
                    heatMapEach.at<float>(cv::Point(j, i)) = featureValue;
                }
            }

//            heatMapEach = refineHeatMapRelaxed(heatMapEach, pcontour_new, view, channel);

            cv::Point min_loc, max_loc;
            double min_val, max_val;
            cv::minMaxLoc(heatMapEach, &min_val, &max_val, &min_loc, &max_loc);

            //refine the heatmap based on the contours
            heatMapEach = 255. * heatMapEach / (max_val + 1.0e-10);

            heatMapEachSum[channel-1] = cv::sum(heatMapEach)[0];
            // TODO: try to do moments as float and convert after.
            heatMapEach.convertTo(heatMapEach, CV_8U);
            cv::blur(heatMapEach, heatMapEach, cv::Size(3, 3)); // you may remove/keep but it helps


            float cx=-1;
            float cy=-1;
            float cxElip=-1;
            float cyElip=-1;
            float cxbx=-1;
            float cybx=-1;
            cv::Moments Mom;
            float cx_mom = -1;
            float cy_mom =-1;
            float cxMinMax= -1;
            float cyMinMax= -1;
            heatMapEachRadius[channel-1]=0;
            if (max_val > 0)
            {
                cxMinMax= max_loc.x * 1280/96.0 - 280;
                cyMinMax= max_loc.y * 1280/96.0-6;
                Mom = moments(heatMapEach, 0);
                cx_mom = Mom.m10 / (Mom.m00 + 1e-10) * 1280/96.0 - 280;
                cy_mom = Mom.m01 / (Mom.m00 + 1e-10) * 1280/96-6 ;
                std::vector<vector<cv::Point> > Blobcontours;
                cv::findContours(heatMapEach.clone(), Blobcontours, cv::RETR_EXTERNAL,
                                 cv::CHAIN_APPROX_SIMPLE);// cv::RETR_TREE
                std::vector<cv::Rect> boundRect(Blobcontours.size());
                int biggest_blob_idx = 0;
                float biggest_blob_area = -1;
                for (size_t i = 0; i < Blobcontours.size(); i++) {
                    boundRect[i] = boundingRect(Blobcontours[i]);
                    float boundRect_area = boundRect[i].area();
                    if (boundRect_area > biggest_blob_area) {
                        biggest_blob_area = boundRect_area;
                        biggest_blob_idx = i;
                    }
                }
                // Head and ankle heatmaps is expected to have  the same or a wider width when cropped, but we can have a general
                // a general method for all other joints if needed
                int bW = boundRect[biggest_blob_idx].width;
                int bH = boundRect[biggest_blob_idx].height;
                cxbx = boundRect[biggest_blob_idx].x + bW/2;
                cybx = boundRect[biggest_blob_idx].y + bH/2;
                float radius=1; //
                cv::Point2f center;
                if(Blobcontours[biggest_blob_idx].size() > 4) // too close image anyway
                {
                    cv::minEnclosingCircle(Blobcontours[biggest_blob_idx], center, radius);
                    cx = center.x;
                    cy = center.y;
                //LOG_GUARD(std::cout << "[predictFromHeatMapOpt] circ x: " << cx << " y:" << cy << std::endl);
            }
            else
            {
                cx = Mom.m10 / (Mom.m00 + 1e-10);
                cy = Mom.m01 / (Mom.m00 + 1e-10);
                }
                heatMapEachRadius[channel-1] = radius;
                //Ellipse (it helps head an ankles but sometimes over estimates)
                int eRadiusMin=1;
                int eRadiusMax=1;
                cv::Size2f ElRadius;
                cv::RotatedRect RotRect;
                if(Blobcontours[biggest_blob_idx].size() > 5) {
                    RotRect = cv::fitEllipse(Blobcontours[biggest_blob_idx]);
                    ElRadius = RotRect.size;
                    // Head and ankles heatmaps is expected to have the same or a wider width when cropped, but we can have a general
                    eRadiusMax = (max(ElRadius.width, ElRadius.height)) / 2;
                    eRadiusMin = (min(ElRadius.width, ElRadius.height)) / 2;
                    cxElip = RotRect.center.x;
                    cyElip = RotRect.center.y;
                    //cv::circle(drawing, cv::Point((int) cx, (int) cy), 2, color4, -1);
                }
                else // use mom or bbox // too close image anyway
                {
                    cxElip = Mom.m10 / (Mom.m00 + 1e-10);
                    cxElip = Mom.m01 / (Mom.m00 + 1e-10);
                    RotRect.center.x = cxElip;
                    RotRect.center.y = cyElip;
                    RotRect.size = cv::Size2f(1,1);
                }


                if(channel==1) // head
                {
                    // option 0
                    // (Circle)
                    //cy = cy-0.5;

                    //Mom
                    //cy_mom = cy_mom -0.5;

                    //option1
                    cv::Point headTopAdjusted_opt1 = cv::Point(boundRect[biggest_blob_idx].x + bW / 2,
                                                               boundRect[biggest_blob_idx].y + bH -
                                                               bW / 2);

                    //use this or the below from the Circle fitting
                    cxbx = headTopAdjusted_opt1.x;
                    cybx = headTopAdjusted_opt1.y-0.5;

                    //option2
                    cv::Point headTopAdjusted_opt2 = cv::Point(RotRect.center.x, RotRect.center.y + eRadiusMin -  eRadiusMax);
                    // use this or the above from boundingbox
                    cxElip = headTopAdjusted_opt2.x;
                    cyElip = headTopAdjusted_opt2.y;
                }
                else if(channel ==11 || channel ==14) // ankles
                {
                    // option 0
                    // (Circle)
                    //cy = cy+0.5;

                    //Mom
                    //cy_mom = cy_mom +0.5;

                    //option1
                    cv::Point ankleAdjusted_opt1 = cv::Point(boundRect[biggest_blob_idx].x + bW / 2,
                                                             boundRect[biggest_blob_idx].y + bW / 2);
                    cxbx = ankleAdjusted_opt1.x;
                    cybx = ankleAdjusted_opt1.y;//+0.5;

                    //option2
                    cv::Point ankleAdjusted_opt2 = cv::Point(RotRect.center.x,RotRect.center.y - eRadiusMin + eRadiusMax);
                    // use this or the above from boundingbox
                    cxElip = ankleAdjusted_opt2.x;
                    cyElip = ankleAdjusted_opt2.y;
                }
            }

            float cxbx_ = cxbx * 1280.0/96.0 -280;
            float cybx_ = cybx * 1280.0/96.0;

            float cx_ = cx * 1280.0/96.0 -280;
            float cy_ = cy * 1280.0/96.0;

            float cxElip_ = cxElip * 1280.0/96.0 -280;
            float cyElip_ = cyElip * 1280.0/96.0;

            // I have choosen the Circle as a good candidate, but moment will do
//            pojo_joints[channel - 1] = cv::Point(cx_, cy_); // correction made later down but saved in jointsPrediction
//            cout <<"\n" + view +": joint[" << channel-1 << "] =" << pojo_joints[channel - 1] << "\n";

            //map jointCentroid to actual image size;
            cv::Point jointCentroid;
            jointCentroid.x = (int) cx_;
            jointCentroid.y = (int) cy_;

            if (channel == 1)
            {
                jointsPrediction.CentroidHeadTop = jointCentroid;
                headFound = (jointCentroid.x > 0 && jointCentroid.y > 0);
            }
            if (channel == 2)
            { jointsPrediction.CentroidNeck = jointCentroid; }
            if (channel == 3)
            { jointsPrediction.CentroidRightShoulder = jointCentroid; }
            if (channel == 6)
            { jointsPrediction.CentroidLeftShoulder = jointCentroid; }
            if (channel == 4)
            { jointsPrediction.CentroidRightElbow = jointCentroid; }
            if (channel == 7)
            { jointsPrediction.CentroidLeftElbow = jointCentroid; }
            if (channel == 5)
            {
                jointsPrediction.CentroidRightHand = jointCentroid;
                rightHandFound = (jointCentroid.x > 0 && jointCentroid.y > 0);
            }
            if (channel == 8)
            {
                jointsPrediction.CentroidLeftHand = jointCentroid;
                leftHandFound = (jointCentroid.x > 0 && jointCentroid.y > 0);
            }
            if (channel == 9)
            { jointsPrediction.CentroidRightHip = jointCentroid; }
            if (channel == 12)
            { jointsPrediction.CentroidLeftHip = jointCentroid; }
            if (channel == 10)
            { jointsPrediction.CentroidRightKnee = jointCentroid; }
            if (channel == 13)
            { jointsPrediction.CentroidLeftKnee = jointCentroid; }
            if (channel == 11)
            {
                jointsPrediction.CentroidRightAnkle = jointCentroid;
                rightLegFound = (jointCentroid.x > 0 && jointCentroid.y > 0);
            }
            if (channel == 14)
            {
                jointsPrediction.CentroidLeftAnkle = jointCentroid;
                leftLegFound = (jointCentroid.x > 0 && jointCentroid.y > 0);
            }
        }

        //predictFromHeatMap_proc.print();

        //This is for verying if retattempt process is needed or not
        if (view == "side")
        {
            if (headFound)
            { countHeatmapResults++; }
            if (rightHandFound || leftHandFound)
            { countHeatmapResults += 2; }
            if (rightLegFound || leftLegFound)
            { countHeatmapResults += 2; }
        }
        else
        {
            if (headFound)
            { countHeatmapResults++; }
            if (rightHandFound)
            { countHeatmapResults++; }
            if (leftHandFound)
            { countHeatmapResults++; }
            if (rightLegFound)
            { countHeatmapResults++; }
            if (leftLegFound)
            { countHeatmapResults++; }
        }

        if ((countHeatmapResults >= 2) && (countHeatmapResults <= 5))
        {
            jointsPrediction.threeOutOfFiveRule = true;
        }

        //////////////////////////////////////////////////////////////////////
        //Another fix attempt from the heatmap size itself. This is similar confidence scoring w.r.t others
        //This seems working fine to adjust head, ankles and even wrists/hands
        int counter=0;
        float heatmapAvg =0;
        float heatmapAvgRadious =0;
        for(int ch=0; ch < 14; ch++)
        {
            if(heatMapEachSum[ch] > 0)
            {
                counter=counter+1;
                heatmapAvg = heatmapAvg + heatMapEachSum[ch];
                heatmapAvgRadious = heatmapAvgRadious + heatMapEachRadius[ch];
            }
        }
        heatmapAvg = heatmapAvg/counter;
        heatmapAvgRadious= (1280.0/96.0) * heatmapAvgRadious/counter;

        float ratioHead = std::min(1.0, heatMapEachSum[0]/(1.0e-10+heatmapAvg));
        float ratioRightAnkle = std::min(1.0, heatMapEachSum[10]/(1.0e-10+heatmapAvg));
        float ratioLeftAnkle = std::min(1.0, heatMapEachSum[13]/(1.0e-10+heatmapAvg));

//        float ratioRightHand = std::min(1.0, heatMapEachSum[4]/(1.0e-10+heatmapAvg));
//        float ratioLeftHand = std::min(1.0, heatMapEachSum[7]/(1.0e-10+heatmapAvg));

        //if any of the above ratios is < 0.6, it simply means the joints is partly visable or not there at all

        std::cout<< "\n heatmapAvg = " << heatmapAvg <<  "\n";
        std::cout<< "\n heatmapAvg Head ratio = " << ratioHead << "\n";
        std::cout<< "\n heatmapAvg RAnkle ratio = " << ratioRightAnkle << "\n";
        std::cout<< "\n heatmapAvg LAnkle ratio = " << ratioLeftAnkle << "\n";

        //Now headtop correction
        if(ratioHead > 0 && jointsPrediction.CentroidHeadTop.y < 60)
        {
            jointsPrediction.CentroidHeadTop.y =  jointsPrediction.CentroidHeadTop.y -  2.0*(1.-ratioHead) * heatmapAvgRadious;
        }

        //Now RightAnkle correction
        float ScaleRadiusAnkleX = 1.0;
        if(view =="side")
        {
            ScaleRadiusAnkleX = 0;
        }
        if(ratioRightAnkle > 0 && jointsPrediction.CentroidRightAnkle.y > (1280-60))
        {
            jointsPrediction.CentroidRightAnkle =  jointsPrediction.CentroidRightAnkle +  2.0*(1.-ratioRightAnkle) * (cv::Point(-heatmapAvgRadious/4*ScaleRadiusAnkleX,heatmapAvgRadious));
        }

        //Now LeftAnkle correction
        if(ratioLeftAnkle > 0 && jointsPrediction.CentroidLeftAnkle.y > (1280-60))
        {
            jointsPrediction.CentroidLeftAnkle =  jointsPrediction.CentroidLeftAnkle +  2.0*(1.-ratioLeftAnkle) * (cv::Point(heatmapAvgRadious/4*ScaleRadiusAnkleX, heatmapAvgRadious));
        }

        /////////////////////////////////////////////////////////////////
        //neck fix(up)
        if (headFound)
        {
            std::cout <<"\n neck fix(up) using head" << "\n";
            jointsPrediction.CentroidNeck.y = 0.85*jointsPrediction.CentroidNeck.y + 0.15 * jointsPrediction.CentroidHeadTop.y;
        }
        else
        {
            std::cout <<"\n neck fix(up) using shoulders" << "\n";
            jointsPrediction.CentroidNeck.y = 1.4*jointsPrediction.CentroidNeck.y -0.4*(jointsPrediction.CentroidRightShoulder.y + jointsPrediction.CentroidLeftShoulder.y)/2.0;
        }

        // (POJO) knee x adjustment for th side image as it does affect the seg.,
        if (view == "side")
        {
            float kneex = 0.5*(jointsPrediction.CentroidLeftKnee.x + jointsPrediction.CentroidRightKnee.x);
            jointsPrediction.CentroidLeftKnee.x = kneex;
            jointsPrediction.CentroidRightKnee.x = kneex;
        }


//        // Ankles check/fix
//        if (jointsPrediction.CentroidLeftAnkle.y > 0  || jointsPrediction.CentroidRightAnkle.y > 0)
//        {
//            float ankley = std::max(jointsPrediction.CentroidLeftAnkle.y,
//                                    jointsPrediction.CentroidRightAnkle.y);
//            float kneey = 0.5 * (jointsPrediction.CentroidLeftKnee.y + jointsPrediction.CentroidRightKnee.y);
//            float hipy = 0.5 * (jointsPrediction.CentroidLeftHip.y + jointsPrediction.CentroidRightHip.y);
//            float ankle_to_knee_y = ankley - kneey;
//            float hips_to_knee_y = kneey - hipy;
//            if (ankle_to_knee_y / (hips_to_knee_y) < 0.8)
//            {
//                std::cout <<"\n" + view + " case of a short ankle to knees \n";
//                jointsPrediction.CentroidLeftAnkle.y = kneey + 0.85 * hips_to_knee_y;
//                jointsPrediction.CentroidRightAnkle.y = kneey + 0.85 * hips_to_knee_y;
//            }
//            else
//            {
//                jointsPrediction.CentroidLeftAnkle.y = 1.04*jointsPrediction.CentroidLeftAnkle.y-0.04*kneey;
//                jointsPrediction.CentroidRightAnkle.y = 1.04*jointsPrediction.CentroidRightAnkle.y -0.04*kneey ;
//            }
//        }


//        update pojo joints too (or delete the one above and save it here)

        pojo_joints[0] = jointsPrediction.CentroidHeadTop;
        pojo_joints[1] = jointsPrediction.CentroidNeck;
        pojo_joints[2] = jointsPrediction.CentroidRightShoulder;
        pojo_joints[3] = jointsPrediction.CentroidRightElbow;
        pojo_joints[4] = jointsPrediction.CentroidRightHand;
        pojo_joints[5] = jointsPrediction.CentroidLeftShoulder;
        pojo_joints[6] = jointsPrediction.CentroidLeftElbow;
        pojo_joints[7] = jointsPrediction.CentroidLeftHand;
        pojo_joints[8] = jointsPrediction.CentroidRightHip;
        pojo_joints[9] = jointsPrediction.CentroidRightKnee;
        pojo_joints[10] = jointsPrediction.CentroidRightAnkle;
        pojo_joints[11] = jointsPrediction.CentroidLeftHip;
        pojo_joints[12] = jointsPrediction.CentroidLeftKnee;
        pojo_joints[13] = jointsPrediction.CentroidLeftAnkle;
        return true;
    }
    else
    {
        LOG_GUARD(std::cout << "predictFromHeatMapOpt() failed to run model" << std::endl)
    }

    return false;
}
// AHI pose (sample as apple)
bool ahiFactoryTensor::predictFromAHIPose(cv::Mat const &oimage, cv::Mat const &ocontour,
                                       std::string const &view,
                                       bool &isMask,
                                       std::vector<cv::Point> &AHI_joints,
                                       ahiPoseInfo &jointsPrediction)
{
    //declare the constant for the heatmap
    int pWidth = 192;
    int pHeight = 192;
    int const numChanel = 17;

    cv::Rect rect;
    int countHeatmapResults = 0;
    bool headFound = false;
    bool leftHandFound = false;
    bool rightHandFound = false;
    bool leftLegFound = false;
    bool rightLegFound = false;
    //bool bVisualise = (nullptr != mVisualize);



    int top, bottom, left, right;

    ModelMap::iterator iter = mModelMap.find(FactoryTensorModelId::ModelAHIPose);
    if (mModelMap.end() != iter)
    {
        pWidth = iter->second->getInputDim(0, 1);
        pHeight = iter->second->getInputDim(0, 2);
    }
    cv::Mat pimage=cv::Mat::zeros(192,192,CV_8U);
    cv::Size targetSize(pWidth, pHeight);
    bool toBGR = false;
    bool doPadding = false;
    pimage = processImageWorWoutPadding(oimage,targetSize,top,bottom,left,right,toBGR,doPadding);

    pimage.convertTo(pimage, CV_8U);

//    postInitModel(FactoryTensorModelId::ModelAHIPose, true);
//    FactoryTensorOpQueue resultQueue(0);
//    FactoryTensorOp *pOp = new FactoryTensorOp(FactoryTensorOpAHIPose, &resultQueue);
//    pOp->addInput("serving_default_input:0",
//                  TensorInput( pimage, true, false, targetSize,
//                              {1.0f}, {0.}));
//    unique_ptr<FactoryTensorOp> op(pOp);
//    mQueue.push(move(op));
//    send(FactoryTensorOpAHIPose);

    // Queue the TFLite operation and wait for the result...
    unique_ptr<FactoryTensorOp> op;
    FactoryTensorOpQueue resultQueue(0);
//    mQueue.push(move(unique_ptr<FactoryTensorOp>(new FactoryTensorOp(
//            FactoryTensorOpAHIPose, "serving_default_input:0", pimage, &resultQueue))));
    mQueue.push(move(unique_ptr<FactoryTensorOp>(new FactoryTensorOp(
            FactoryTensorOpAHIPose, pimage, &resultQueue))));
    send(FactoryTensorOpAHIPose);

    if (resultQueue.popWait(op) && op->hasOutput(0))
    {
        cv::Mat outputBlob = op->output(0);

        if (!op->mInvokeSuccess)
        {
            LOG_GUARD(std::cout << "predictFromAHIPose() received an empty result" << std::endl)
            return false;
        }

//        std::vector<float> heatMapEachSum(14,-1);
//        std::vector<float> heatMapEachRadius(14,-1);
//        for (int channel = 1; channel < numChanel; channel++)
//        {
//            cv::Mat heatMapEach = cv::Mat::zeros(numRow, numCol, CV_32F);
//
//            for (int i = 0; i < numRow; i++)
//            {
//                for (int j = 0; j < numCol; j++)
//                {
//                    auto featureValue = (float) (outputBlob.at<cv::Vec4f>(0, i, j)[channel - 1]);
//                    if (featureValue > 1.0)
//                    { featureValue = 1.0; }
//                    if (featureValue < 0.1) // 0.1 was for iOs based
//                    { featureValue = 0.0; }
//                    heatMapEach.at<float>(cv::Point(j, i)) = featureValue;
//                }
//            }


        //predictFromHeatMap_proc.print();

        //This is for verying if retattempt process is needed or not
        if (view == "side")
        {
            if (headFound)
            { countHeatmapResults++; }
            if (rightHandFound || leftHandFound)
            { countHeatmapResults += 2; }
            if (rightLegFound || leftLegFound)
            { countHeatmapResults += 2; }
        }
        else
        {
            if (headFound)
            { countHeatmapResults++; }
            if (rightHandFound)
            { countHeatmapResults++; }
            if (leftHandFound)
            { countHeatmapResults++; }
            if (rightLegFound)
            { countHeatmapResults++; }
            if (leftLegFound)
            { countHeatmapResults++; }
        }

        if ((countHeatmapResults >= 2) && (countHeatmapResults <= 5))
        {
            jointsPrediction.threeOutOfFiveRule = true;
        }

        //////////////////////////////////////////////////////////////////////

        return true;
    }
    else
    {
        LOG_GUARD(std::cout << "predictFromHeatMapOpt() failed to run model" << std::endl)
    }

    return false;
}


//function that returns the joints
bool ahiFactoryTensor::predictFromJoints(cv::Mat const &oimage, cv::Mat const &ocontour,
                                      std::string const &view,
                                      cv::Mat &binaryImageJoints, bool &isMask,
                                      ahiPoseInfo &jointsPrediction)
{
    //declare the constant for the heatmap
    int pWidth = 320;
    int pHeight = 320;

    int const ds = 5;
    cv::Mat pimage, pcontour;
    int top, bottom, left, right;

    ModelMap::iterator iter = mModelMap.find(FactoryTensorModelId::ModelPose);
    if (mModelMap.end() != iter)
    {
        pWidth = iter->second->getInputDim(0, 1);
        pHeight = iter->second->getInputDim(0, 2);
    }

    processImage(oimage, ocontour, cv::Size(pWidth, pHeight), pimage, pcontour, top, bottom, left,
                 right, isMask);

    pimage.convertTo(pimage, CV_32F);

    // Queue the TFLite operation and wait for the result...
    unique_ptr<FactoryTensorOp> op;
    FactoryTensorOpQueue resultQueue(0);
    mQueue.push(move(unique_ptr<FactoryTensorOp>(new FactoryTensorOp(
            FactoryTensorOpPose, "image", pimage, &resultQueue))));
    send(FactoryTensorOpPose);
    if (resultQueue.popWait(op))
    {
        /* TODO: fix for named outputs
        if (op->mOutputs.size() >= 4)
        {
            cv::Mat scores = op->output(0);
            cv::Mat offsets = op->output(1);
            cv::Mat displacements_fwd = op->output(2);
            cv::Mat displacements_bwd = op->output(3);

            if (!op->mInvokeSuccess)
            {
                LOG_GUARD(std::cout << "predictFromJoints() received an empty result" << std::endl)
                return false;
            }

            return true;
        }
        */
    }

    return false;
}


ahiJsonPose
ahiFactoryTensor::performImageInspectionUsingSegnet(cv::Mat const &oimage, cv::Mat const &ocontour,
                                                 std::string const &view, ahiPoseInfo &centroidJoints)
{

    //  {
    //      cv::Point CentroidHeadTop, CentroidNeck, CentroidRightShoulder;
    //      cv::Point CentroidLeftShoulder, CentroidRightElbow, CentroidLeftElbow;
    //      cv::Point CentroidRightHand, CentroidLeftHand, CentroidRightHip, CentroidLeftHip;
    //       cv::Point CentroidRightKnee, CentroidLeftKnee, CentroidRightAnkle, CentroidLeftAnkle;
    //       bool threeOutOfFiveRule = false;
    //  } Joints;

    std::vector<bool> isPartDetected;
    std::vector<cv::Point> approx_centriods;
    isPartDetected = pose_inspect_usingsegnet(oimage, ocontour, view, approx_centriods);

    ahiJsonPose result{};
    result.view = view;
    std::vector<cv::Rect> facesFound;

    detectFace(oimage, ocontour, facesFound);
    result.Face = (int) facesFound.size();

    //if (facesFound.empty())
    //{
    //    LOG_GUARD(std::cout << "Face Not Found.." << std::endl)
    //}

    if (facesFound.size() > 0)
    {
        //cv::Mat dPose;
        //bool heatmapResult = predictFromHeatMap(oimage, ocontour, view, dPose, isMask, centroidJoints);

        if (isPartDetected.size() < 1)
        {
            LOG_GUARD(std::cout << "pose_inspect_usingsegnet() was unsuccessful" << std::endl)
            return result;
        }

        if ((view == "front") || (view == "Front"))
        {
            //FrontPose poseInspectionOutput = inspectFrontPoseWithTemplatePose(dPose, ocontour,
            //                                                                  facesFound[0],
            //                                                                  centroidJoints);
            if (isPartDetected[4])
            {
                result.RA = true;
            }
            if (isPartDetected[7])
            {
                result.LA = true;
            }
            if (isPartDetected[10])
            {
                result.RL = true;
            }
            if (isPartDetected[13])
            {
                result.LL = true;
            }


            //LOG_GUARD(std::cout << THREAD_STR << "size of detected pose is " << dPose.size() << "\n")
            //LOG_GUARD(std::cout << THREAD_STR << "size of template pose is " << ocontour.size() << "\n")
            //LOG_GUARD(std::cout << THREAD_STR << "Face Rectangle is " << facesFound[0] << "\n")
            //LOG_GUARD(std::cout << THREAD_STR << "Head Top Overlap = "
            //                    << poseInspectionOutput.HeadTopOverlapRatio
            //                    << "\n")
            //LOG_GUARD(std::cout << THREAD_STR << "Left Hand  Overlap = "
            //                    << poseInspectionOutput.LeftHandOverlapRatio << "\n")
            //LOG_GUARD(std::cout << THREAD_STR << "Right Hand Overlap = "
            //                    << poseInspectionOutput.RightHandOverlapRatio << "\n")
            //LOG_GUARD(std::cout << THREAD_STR << "Left Leg Overlap = "
            //                   << poseInspectionOutput.LeftLegOverlapRatio
            //                    << "\n")
            //LOG_GUARD(std::cout << THREAD_STR << "Right Leg Overlap = "
            //                    << poseInspectionOutput.RightLegOverlapRatio
            //                    << "\n")
            //LOG_GUARD(std::cout << THREAD_STR << "Head In Contour  = " << poseInspectionOutput.HeadInContour
            //                    << "\n")
            //LOG_GUARD(std::cout << THREAD_STR << "Right Hand  In Contour  = "
            //                    << isPartDetected[4] << "\n")
            //LOG_GUARD(std::cout << THREAD_STR << "Left hand In Contour  = "
            //                    << isPartDetected[7] << "\n")
            //LOG_GUARD(std::cout << THREAD_STR << "Right Leg In Contour  = "
            //                    << isPartDetected[10] << "\n")
            //LOG_GUARD(std::cout << THREAD_STR << "Left Leg In Contour  = "
            //                    << isPartDetected[13]
            //                    << "\n")
            //LOG_GUARD(std::cout << THREAD_STR << "Right Leg In Contour  = "
            //                    << poseInspectionOutput.RightLegInContour << "\n")
            //visualize("front_pose", dPose);
        }
        else if ((view == "side") || (view == "Side"))
        {
            /*SidePose poseInspectionOutput = inspectSidePoseWithTemplatePose(dPose, ocontour,
                                                                            facesFound[0],
                                                                            centroidJoints);*/
            if (isPartDetected[2] || isPartDetected[3] || isPartDetected[5] || isPartDetected[6])
            { result.UB = true; }
            if (isPartDetected[10] || isPartDetected[13])
            { result.LB = true; }

            //LOG_GUARD(std::cout << THREAD_STR << "size of detected pose is " << dPose.size() << "\n")
//            LOG_GUARD(std::cout << THREAD_STR << "size of template pose is " << ocontour.size() << "\n")
//            LOG_GUARD(std::cout << THREAD_STR << "Face Rectangle is " << facesFound[0] << "\n")
//            LOG_GUARD(std::cout << THREAD_STR << "Right Shoulder = " << isPartDetected[2] << "\n")
//            LOG_GUARD(std::cout << THREAD_STR << "Right Elbow = " << isPartDetected[3] << "\n")
//            LOG_GUARD(std::cout << THREAD_STR << "Left Shoulder = " << isPartDetected[5] << "\n")
//            LOG_GUARD(std::cout << THREAD_STR << "Left Elbow = " << isPartDetected[6] << "\n")
//            LOG_GUARD(std::cout << THREAD_STR << "Right Ankle/Foot = " << isPartDetected[10] << "\n")
//            LOG_GUARD(std::cout << THREAD_STR << "Left Ankle/Foot = " << isPartDetected[13] << "\n")

            //visualize("side_pose", dPose);
        }
        else
        {
            LOG_GUARD(std::cout << THREAD_STR << "Incorect Pose Mode, only support front and side ..\n")
        }
    }

    centroidJoints.CentroidHeadTop = approx_centriods[0];
    centroidJoints.CentroidNeck = approx_centriods[1];
    centroidJoints.CentroidRightShoulder = approx_centriods[2];
    centroidJoints.CentroidLeftShoulder = approx_centriods[5];
    centroidJoints.CentroidRightElbow = approx_centriods[3];
    centroidJoints.CentroidLeftElbow = approx_centriods[6];
    centroidJoints.CentroidRightHand = approx_centriods[4];
    centroidJoints.CentroidLeftHand = approx_centriods[7];
    centroidJoints.CentroidRightHip = approx_centriods[8];
    centroidJoints.CentroidLeftHip = approx_centriods[11];
    centroidJoints.CentroidRightKnee = approx_centriods[9];
    centroidJoints.CentroidLeftKnee = approx_centriods[12];
    centroidJoints.CentroidRightAnkle = approx_centriods[10];
    centroidJoints.CentroidLeftAnkle = approx_centriods[13];


    return result;
}

// this function performs the image inspection
ahiJsonPose
ahiFactoryTensor::performImageInspection(cv::Mat const &oimage, cv::Mat const &ocontour,
                                      std::string const &view,
                                      bool isMask, std::vector<cv::Point> &pojo_joints, Joints &centroidJoints)
{
    ahiJsonPose result{};
    result.view = view;
    std::vector<cv::Rect> facesFound;

    detectFace(oimage, ocontour, facesFound);
    result.Face = (int) facesFound.size();

    if (facesFound.empty())
    {
        LOG_GUARD(std::cout << "Face Not Found.." << std::endl)
    }

    //cv::imwrite("/data/data/com.myfiziq.myfiziq_android/files/1567734143430_image.bmp", oimage);
    //cv::imwrite("/data/data/com.myfiziq.myfiziq_android/files/1567734143430_contour.bmp", ocontour);

    if (facesFound.size() == 1)
    {
        cv::Mat dPose;
        centroidJoints.Face = facesFound[0];

        bool heatmapResult = predictFromHeatMap(oimage, ocontour, view, dPose, isMask, pojo_joints, centroidJoints);

        if (!heatmapResult)
        {
            LOG_GUARD(std::cout << "predictFromHeatmap() was unsuccessful" << std::endl)
            return result;
        }

        if ((abs(facesFound[0].x - CAMERA_WIDTH/2) < 160) && (abs(facesFound[0].y) < 160))
        {
            /*
            if ((centroidJoints.CentroidHeadTop.x < 100) || (centroidJoints.CentroidHeadTop.y < 8))
            {
                centroidJoints.CentroidHeadTop.x = facesFound[0].x + facesFound[0].width / 2;
                centroidJoints.CentroidHeadTop.y = facesFound[0].y;
            }
            if ((centroidJoints.CentroidNeck.x < 100) || (centroidJoints.CentroidNeck.y < centroidJoints.CentroidHeadTop.y))
            {
                centroidJoints.CentroidNeck.x = facesFound[0].x + facesFound[0].width / 2;
                centroidJoints.CentroidNeck.y = facesFound[0].y + facesFound[0].heiclassght;
            }
            */
            if ((centroidJoints.CentroidHeadTop.x < 100) || (centroidJoints.CentroidHeadTop.y < 8) ||
                (centroidJoints.CentroidNeck.x < 100) || (centroidJoints.CentroidNeck.y < centroidJoints.CentroidHeadTop.y))
            {
                centroidJoints.CentroidHeadTop.x = facesFound[0].x + facesFound[0].width / 2;
                centroidJoints.CentroidHeadTop.y = facesFound[0].y;
                centroidJoints.CentroidNeck.x = facesFound[0].x + facesFound[0].width / 2;
                centroidJoints.CentroidNeck.y = facesFound[0].y + facesFound[0].height;
            }
        }

        if ((view == "front") || (view == "Front"))
        {
            FrontPose poseInspectionOutput = inspectFrontPoseWithTemplatePose(dPose, ocontour,
                                                                              facesFound[0],
                                                                              centroidJoints);
            if (poseInspectionOutput.RightHandInContour)
            {
                result.RA = true;
            }
            if (poseInspectionOutput.LeftHandInContour)
            {
                result.LA = true;
            }
            if (poseInspectionOutput.RightLegInContour)
            {
                result.RL = true;
            }
            if (poseInspectionOutput.LeftLegInContour)
            {
                result.LL = true;
            }
            if (poseInspectionOutput.FaceInExpectedContour)
            {
                result.FaceInExpectedContour = true;
            }
            if (poseInspectionOutput.CameraIsPotentiallyHigh)
            {
                result.CameraIsPotentiallyHigh = true;
            }
            if (poseInspectionOutput.CameraIsPotentiallyLow)
            {
                result.CameraIsPotentiallyLow = true;
            }

            /*
            LOG_GUARD(std::cout << THREAD_STR << "size of detected pose is " << dPose.size() << "\n")
            LOG_GUARD(std::cout << THREAD_STR << "size of template pose is " << ocontour.size() << "\n")
            LOG_GUARD(std::cout << THREAD_STR << "Face Rectangle is " << facesFound[0] << "\n")
            LOG_GUARD(std::cout << THREAD_STR << "Head Top Overlap = "
                      << poseInspectionOutput.HeadTopOverlapRatio
                      << "\n")
            LOG_GUARD(std::cout << THREAD_STR << "Left Hand  Overlap = "
                      << poseInspectionOutput.LeftHandOverlapRatio << "\n")
            LOG_GUARD(std::cout << THREAD_STR << "Right Hand Overlap = "
                      << poseInspectionOutput.RightHandOverlapRatio << "\n")
            LOG_GUARD(std::cout << THREAD_STR << "Left Leg Overlap = "
                      << poseInspectionOutput.LeftLegOverlapRatio
                      << "\n")
            LOG_GUARD(std::cout << THREAD_STR << "Right Leg Overlap = "
                      << poseInspectionOutput.RightLegOverlapRatio
                      << "\n")
            LOG_GUARD(std::cout << THREAD_STR << "Head In Contour  = " << poseInspectionOutput.HeadInContour
                      << "\n")
            LOG_GUARD(std::cout << THREAD_STR << "Right Hand In Contour  = "
                      << poseInspectionOutput.RightHandInContour << "\n")
            LOG_GUARD(std::cout << THREAD_STR << "Left Hand In Contour  = "
                      << poseInspectionOutput.LeftHandInContour << "\n")
            LOG_GUARD(std::cout << THREAD_STR << "Right Leg In Contour  = "
                      << poseInspectionOutput.RightLegInContour << "\n")
            LOG_GUARD(std::cout << THREAD_STR << "Left Leg In Contour  = "
                      << poseInspectionOutput.LeftLegInContour
                      << "\n")
             */
            //visualize("front_pose", dPose);
        }
        else if ((view == "side") || (view == "Side"))
        {
            SidePose poseInspectionOutput = inspectSidePoseWithTemplatePose(dPose, ocontour,
                                                                            facesFound[0],
                                                                            centroidJoints);
            if (poseInspectionOutput.HandInContour)
            { result.UB = true; }
            if (poseInspectionOutput.LegInContour)
            { result.LB = true; }

            /*
            LOG_GUARD(std::cout << THREAD_STR << "size of detected pose is " << dPose.size() << "\n")
            LOG_GUARD(std::cout << THREAD_STR << "size of template pose is " << ocontour.size() << "\n")
            LOG_GUARD(std::cout << THREAD_STR << "Face Rectangle is " << facesFound[0] << "\n")
            LOG_GUARD(std::cout << THREAD_STR << "Head Top Overlap = "
                      << poseInspectionOutput.HeadTopOverlapRatio
                      << "\n")
            LOG_GUARD(std::cout << THREAD_STR << "Hand  Overlap = " << poseInspectionOutput.HandOverlapRatio
                      << "\n")
            LOG_GUARD(std::cout << THREAD_STR << "Leg Overlap = " << poseInspectionOutput.LegOverlapRatio
                      << "\n")
            LOG_GUARD(std::cout << THREAD_STR << "Head In Contour  = " << poseInspectionOutput.HeadInContour
                      << "\n")
            LOG_GUARD(std::cout << THREAD_STR << "Hand  In Contour  = " << poseInspectionOutput.HandInContour
                      << "\n")
            LOG_GUARD(std::cout << THREAD_STR << "Leg In Contour  = " << poseInspectionOutput.LegInContour
                      << "\n")
            */
            //visualize("side_pose", dPose);
        }
        else
        {
            LOG_GUARD(std::cout << THREAD_STR << "Incorect Pose Mode, only support front and side ..\n")
        }
    }

    return result;
}

ahiJsonPose
ahiFactoryTensor::performPoseInspection(cv::Mat const &oimage, cv::Mat const &ocontour,
                                     std::string const &view,
                                     bool isMask, Joints &centroidJoints)
{
    ahiJsonPose result{};
    result.view = view;
    std::vector<cv::Rect> facesFound;

    detectFace(oimage, ocontour, facesFound);
    result.Face = (int) facesFound.size();

    if (facesFound.empty())
    {
        LOG_GUARD(std::cout << "Face Not Found.." << std::endl)
    }

    if (facesFound.size() == 1)
    {
        cv::Mat dPose;
        bool heatmapResult = predictFromJoints(oimage, ocontour, view, dPose, isMask, centroidJoints);

        if (!heatmapResult)
        {
            LOG_GUARD(std::cout << "predictFromHeatmap() was unsuccessful" << std::endl)
            return result;
        }

        if ((view == "front") || (view == "Front"))
        {
            FrontPose poseInspectionOutput = inspectFrontPoseWithTemplatePose(dPose, ocontour,
                                                                              facesFound[0],
                                                                              centroidJoints);
            if (poseInspectionOutput.RightHandInContour)
            {
                result.RA = true;
            }
            if (poseInspectionOutput.LeftHandInContour)
            {
                result.LA = true;
            }
            if (poseInspectionOutput.RightLegInContour)
            {
                result.RL = true;
            }
            if (poseInspectionOutput.LeftLegInContour)
            {
                result.LL = true;
            }

            /*
            LOG_GUARD(std::cout << THREAD_STR << "size of detected pose is " << dPose.size() << "\n")
            LOG_GUARD(std::cout << THREAD_STR << "size of template pose is " << ocontour.size() << "\n")
            LOG_GUARD(std::cout << THREAD_STR << "Face Rectangle is " << facesFound[0] << "\n")
            LOG_GUARD(std::cout << THREAD_STR << "Head Top Overlap = "
                                << poseInspectionOutput.HeadTopOverlapRatio
                                << "\n")
            LOG_GUARD(std::cout << THREAD_STR << "Left Hand  Overlap = "
                                << poseInspectionOutput.LeftHandOverlapRatio << "\n")
            LOG_GUARD(std::cout << THREAD_STR << "Right Hand Overlap = "
                                << poseInspectionOutput.RightHandOverlapRatio << "\n")
            LOG_GUARD(std::cout << THREAD_STR << "Left Leg Overlap = "
                                << poseInspectionOutput.LeftLegOverlapRatio
                                << "\n")
            LOG_GUARD(std::cout << THREAD_STR << "Right Leg Overlap = "
                                << poseInspectionOutput.RightLegOverlapRatio
                                << "\n")
            LOG_GUARD(std::cout << THREAD_STR << "Head In Contour  = " << poseInspectionOutput.HeadInContour
                                << "\n")
            LOG_GUARD(std::cout << THREAD_STR << "Right Hand In Contour  = "
                                << poseInspectionOutput.RightHandInContour << "\n")
            LOG_GUARD(std::cout << THREAD_STR << "Left Hand In Contour  = "
                                << poseInspectionOutput.LeftHandInContour << "\n")
            LOG_GUARD(std::cout << THREAD_STR << "Right Leg In Contour  = "
                                << poseInspectionOutput.RightLegInContour << "\n")
            LOG_GUARD(std::cout << THREAD_STR << "Left Leg In Contour  = "
                                << poseInspectionOutput.LeftLegInContour
                                << "\n")
            */
            //visualize("front_pose", dPose);
        }
        else if ((view == "side") || (view == "Side"))
        {
            SidePose poseInspectionOutput = inspectSidePoseWithTemplatePose(dPose, ocontour,
                                                                            facesFound[0],
                                                                            centroidJoints);
            if (poseInspectionOutput.HandInContour)
            { result.UB = true; }
            if (poseInspectionOutput.LegInContour)
            { result.LB = true; }
            /*
            LOG_GUARD(std::cout << THREAD_STR << "size of detected pose is " << dPose.size() << "\n")
            LOG_GUARD(std::cout << THREAD_STR << "size of template pose is " << ocontour.size() << "\n")
            LOG_GUARD(std::cout << THREAD_STR << "Face Rectangle is " << facesFound[0] << "\n")
            LOG_GUARD(std::cout << THREAD_STR << "Head Top Overlap = "
                                << poseInspectionOutput.HeadTopOverlapRatio
                                << "\n")
            LOG_GUARD(std::cout << THREAD_STR << "Hand  Overlap = " << poseInspectionOutput.HandOverlapRatio
                                << "\n")
            LOG_GUARD(std::cout << THREAD_STR << "Leg Overlap = " << poseInspectionOutput.LegOverlapRatio
                                << "\n")
            LOG_GUARD(std::cout << THREAD_STR << "Head In Contour  = " << poseInspectionOutput.HeadInContour
                                << "\n")
            LOG_GUARD(std::cout << THREAD_STR << "Hand  In Contour  = " << poseInspectionOutput.HandInContour
                                << "\n")
            LOG_GUARD(std::cout << THREAD_STR << "Leg In Contour  = " << poseInspectionOutput.LegInContour
                                << "\n")
            */
            //visualize("side_pose", dPose);
        }
        else
        {
            LOG_GUARD(std::cout << THREAD_STR << "Incorect Pose Mode, only support front and side ..\n")
        }
    }

    return result;
}

void ahiFactoryTensor::getJointParts(Joints const &poseInfo, std::vector<cv::Point> &jointParts)
{
    jointParts.clear();
    jointParts.push_back(poseInfo.CentroidHeadTop);
    jointParts.push_back(poseInfo.CentroidNeck);
    jointParts.push_back(poseInfo.CentroidRightShoulder);
    jointParts.push_back(poseInfo.CentroidRightElbow);
    jointParts.push_back(poseInfo.CentroidRightHand);
    jointParts.push_back(poseInfo.CentroidLeftShoulder);
    jointParts.push_back(poseInfo.CentroidLeftElbow);
    jointParts.push_back(poseInfo.CentroidLeftHand);
    jointParts.push_back(poseInfo.CentroidRightHip);
    jointParts.push_back(poseInfo.CentroidRightKnee);
    jointParts.push_back(poseInfo.CentroidRightAnkle);
    jointParts.push_back(poseInfo.CentroidLeftHip);
    jointParts.push_back(poseInfo.CentroidLeftKnee);
    jointParts.push_back(poseInfo.CentroidLeftAnkle);
}

void ahiFactoryTensor::putJointParts(Joints &poseInfo, std::vector<cv::Point> &jointParts)
{
    poseInfo.CentroidHeadTop = jointParts[0];
    poseInfo.CentroidNeck = jointParts[1];
    poseInfo.CentroidRightShoulder = jointParts[2];
    poseInfo.CentroidRightElbow = jointParts[3];
    poseInfo.CentroidRightHand = jointParts[4];
    poseInfo.CentroidLeftShoulder = jointParts[5];
    poseInfo.CentroidLeftElbow = jointParts[6];
    poseInfo.CentroidLeftHand = jointParts[7];
    poseInfo.CentroidRightHip = jointParts[8];
    poseInfo.CentroidRightKnee = jointParts[9];
    poseInfo.CentroidRightAnkle = jointParts[10];
    poseInfo.CentroidLeftHip = jointParts[11];
    poseInfo.CentroidLeftKnee = jointParts[12];
    poseInfo.CentroidLeftAnkle = jointParts[13];
}

void ahiFactoryTensor::putJniParts(Joints &poseInfo, std::vector<float> &jnijoints)
{
    size_t ix = 0;
    poseInfo.CentroidHeadTop = cv::Point(jnijoints[ix++], jnijoints[ix++]);
    poseInfo.CentroidNeck = cv::Point(jnijoints[ix++], jnijoints[ix++]);
    poseInfo.CentroidRightShoulder = cv::Point(jnijoints[ix++], jnijoints[ix++]);
    poseInfo.CentroidRightElbow = cv::Point(jnijoints[ix++], jnijoints[ix++]);
    poseInfo.CentroidRightHand = cv::Point(jnijoints[ix++], jnijoints[ix++]);
    poseInfo.CentroidLeftShoulder = cv::Point(jnijoints[ix++], jnijoints[ix++]);
    poseInfo.CentroidLeftElbow = cv::Point(jnijoints[ix++], jnijoints[ix++]);
    poseInfo.CentroidLeftHand = cv::Point(jnijoints[ix++], jnijoints[ix++]);
    poseInfo.CentroidRightHip = cv::Point(jnijoints[ix++], jnijoints[ix++]);
    poseInfo.CentroidRightKnee = cv::Point(jnijoints[ix++], jnijoints[ix++]);
    poseInfo.CentroidRightAnkle = cv::Point(jnijoints[ix++], jnijoints[ix++]);
    poseInfo.CentroidLeftHip = cv::Point(jnijoints[ix++], jnijoints[ix++]);
    poseInfo.CentroidLeftKnee = cv::Point(jnijoints[ix++], jnijoints[ix++]);
    poseInfo.CentroidLeftAnkle = cv::Point(jnijoints[ix++], jnijoints[ix++]);
}

void printJoints(const char *header, std::vector<cv::Point> &jointPoints)
{
#ifndef NDEBUG
    if (jointPoints.size() > 0)
    {
        int ix = 0;
        for (cv::Point point : jointPoints)
        {
            std::cout << header << "(" << ix << ") X:" << point.x << " Y:" << point.y << std::endl;
            ix++;
        }
    }
    else
    {
        std::cout << header << "empty" << std::endl;
    }
#endif
}

bool ahiFactoryTensor::est_joints_old(const cv::Mat orig_image, const cv::Mat contourMat, ahi_avatar_gen::view_t viewType, Joints &poseInfo, vector<cv::Point> &JointLocs,
                                   float &centerY)
{

    try
    {
        bool bUseJoints = true;
        cv::Mat inp_image = orig_image.clone();

        cv::blur(inp_image, inp_image, cv::Size(3, 3));
        string BW = "";

        if ((int) inp_image.channels() < 3) // for grayscale images
        {
            cv::cvtColor(inp_image, inp_image, CV_GRAY2BGR);
            BW = "Gray";
        }

        cv::Mat preprocessed_image;
        cv::Size inp_size, inp_image_maxsize;
        int inp_maxHW;
        int net_image_HorW = 368;// 376
        int net_out_HorW = 46;//47
        cv::Size target_size(net_image_HorW, net_image_HorW); // net size
        int top, bottom, left, right;
        top = 0;
        bottom = 0;
        left = 0;
        right = 0;
        cv::Rect cropROI;
        int cropTop = 0;
        int cropLeft = 0;

        try
        {
            inp_size = inp_image.size();
            inp_maxHW = max(inp_image.rows, inp_image.cols);
            inp_image_maxsize = cv::Size(inp_maxHW, inp_maxHW);
            if (!bUseJoints)
            {
                preprocessed_image = preprocess_image(inp_image, target_size, top, bottom, left, right);
            }
            else
            {
                preprocessed_image = preprocess_image_joints(inp_image, viewType, poseInfo, target_size, cropROI, top, bottom, left, right);
            }
            if (preprocessed_image.empty())
            {
                return false;
            }
        }
        catch (const exception &e)
        {

            LOG_GUARD(std::cout << e.what() << endl)
            return false;
        }
        cv::Mat im;
        //preprocessed_image.convertTo(im, CV_8U);
        //imwrite("pre_im.png", im);
        preprocessed_image.convertTo(preprocessed_image, CV_32F);
        preprocessed_image = preprocessed_image / 255.0 - 0.5;

        cv::Mat center_map = cv::Mat::zeros(preprocessed_image.rows, preprocessed_image.cols,
                                            CV_32F);

        float sigma = 21;
        for (int r = 0; r < center_map.rows; r++)
            for (int c = 0; c < center_map.cols; c++)
            {
                float dist_sq = (r - center_map.rows / 2.) * (r - center_map.rows / 2.) +
                                (c - center_map.cols / 2.) * (c - center_map.cols / 2.);
                float exponent = dist_sq / 2.0 / sigma / sigma;
                center_map.at<float>(cv::Point(c, r)) = (float) exp(-exponent);
            }

        center_map.convertTo(im, CV_8U);

        center_map.convertTo(center_map, CV_32F);
        vector<cv::Mat> input_image_channels;
        cv::split(preprocessed_image, input_image_channels);
        input_image_channels.push_back(center_map);

        cv::Mat input_image;
        cv::merge(input_image_channels, input_image);

        initJoints(true);
        unique_ptr<FactoryTensorOp> op;
        FactoryTensorOpQueue resultQueue(0);
        mQueue.push(move(unique_ptr<FactoryTensorOp>(new FactoryTensorOp(
                FactoryTensorOpJoints, input_image, &resultQueue))));
        send(FactoryTensorOpJoints);
        if (resultQueue.popWait(op))
        {
            cv::Mat outputBlob = op->output(0);
            //auto t = Interpreter->typed_output_tensor<float>(0);
            //float (&Txyzw)[1][46][46][15] = *reinterpret_cast<float (*)[1][46][46][15]>(&(t[0]));
            cv::Rect rect;
            rect.x = right;
            rect.y = top;
            rect.width = inp_maxHW - 2 * right;
            rect.height = inp_maxHW - 2 * top;

            cv::Mat HeatMapsAdded = cv::Mat::zeros(inp_size, CV_32F);

            for (int t = 0; t < 15; t++)
            {
                cv::Mat HeatMapEach(46, 46, CV_32F);
                cv::Point min_loc, max_loc;
                double min_val, max_val;

                for (int y = 0; y < 46; ++y)
                {
                    for (int x = 0; x < 46; ++x)
                    {
                        HeatMapEach.at<float>(cv::Point(x, y)) = outputBlob.at<cv::Vec4f>(0, y, x)[t];
                    }
                }

                cv::resize(HeatMapEach, HeatMapEach, inp_image_maxsize, 0, 0,
                           cv::INTER_CUBIC); // resize to net test size

                HeatMapEach = HeatMapEach(rect); // crop to remove zero-padding

                cv::resize(HeatMapEach, HeatMapEach, inp_size, 0, 0,
                           cv::INTER_CUBIC); // resize to original input image size

                cv::GaussianBlur(HeatMapEach, HeatMapEach, cv::Size(5, 5), 0.5,
                                 0.5); // smooth to get max paks

                cv::minMaxLoc(HeatMapEach.clone(), &min_val, &max_val, &min_loc,
                              &max_loc);// get peak location

                JointLocs.push_back(max_loc);
                HeatMapEach = 255. * HeatMapEach / max_val;
                HeatMapsAdded = HeatMapsAdded + HeatMapEach;

                HeatMapEach.convertTo(HeatMapEach, CV_8U);

                visualize("joints", HeatMapEach);
            }

            return true;
        }
    }
    catch (cv::Exception &e)
    {
        LOG_GUARD(std::cout << "\n Error: " << e.what() << "\n")
        return false;
    }

    return false;
}

bool ahiFactoryTensor::est_joints(const cv::Mat orig_image, const cv::Mat contourMat, ahi_avatar_gen::view_t viewType, Joints &poseInfo, vector<cv::Point> &JointLocs,
                               float &centerY)
{
    try
    {
        bool bUseJoints = (poseInfo.CentroidHeadTop.x > 0 && poseInfo.CentroidHeadTop.y > 0);
        cv::Mat inp_image = orig_image.clone();
        cv::blur(inp_image, inp_image, cv::Size(3, 3));
        string BW = "";
        if ((int) inp_image.channels() < 3) // for grayscale images
        {
            cv::cvtColor(inp_image, inp_image, CV_GRAY2BGR);
            BW = "Gray";
        }
        cv::Mat preprocessed_image;
        cv::Size inp_size, inp_image_maxsize;
        int inp_maxHW;
        int net_image_HorW = 376;
        int net_out_HorW = 47;
        cv::Size target_size(net_image_HorW, net_image_HorW); // net size
        int top, bottom, left, right;
        top = 0;
        bottom = 0;
        left = 0;
        right = 0;
        cv::Rect cropROI;
        int cropTop = 0;
        int cropLeft = 0;
        try
        {
            inp_size = inp_image.size();
            inp_maxHW = max(inp_image.rows, inp_image.cols);
            inp_image_maxsize = cv::Size(inp_maxHW, inp_maxHW);
            if (!bUseJoints)
            {
                preprocessed_image = preprocess_image(inp_image, target_size, top, bottom, left, right);
            }
            else
            {
                preprocessed_image = preprocess_image_joints(inp_image, viewType, poseInfo, target_size, cropROI, top, bottom, left, right);
            }
            if (preprocessed_image.empty())
            {
                return false;
            }
        }
        catch (const exception &e)
        {
            LOG_GUARD(std::cout << e.what() << endl)
            return false;
        }
        cv::Mat im;
        //preprocessed_image.convertTo(im, CV_8U);
        //imwrite("pre_im.png", im);
        preprocessed_image.convertTo(preprocessed_image, CV_32F);
        preprocessed_image = preprocessed_image / 255.0 - 0.5;
        cv::Mat center_map = cv::Mat::zeros(preprocessed_image.rows, preprocessed_image.cols,
                                            CV_32F);
        float sigma = 21;
        float centerX = center_map.cols / 2.;

        //TODO: fixme
        centerY = center_map.rows / 2.;

        /*
        if (centerY <= 0 && poseInfo.CentroidHeadTop.y > 0 && (poseInfo.CentroidLeftAnkle.y > 0 || poseInfo.CentroidRightAnkle.y > 0))
        {
            float scale = net_image_HorW / (float) orig_image.rows;
            float height = ((poseInfo.CentroidLeftAnkle.y + poseInfo.CentroidRightAnkle.y) / 2.) - poseInfo.CentroidHeadTop.y;
            float offset = poseInfo.CentroidHeadTop.y + height / 2;
            sigma = (float) orig_image.rows / height * 18;
            centerY = offset * scale;

            //center_map = preprocess_image(contourMat, target_size, top, bottom, left, right);
            //cv::floodFill(center_map, cv::Point(centerX, scaledTop+10), cv::Scalar(255));
            //center_map.convertTo(center_map, CV_32F);
            LOG_GUARD(std::cout << "[est_joints] centerY:" << centerY << std::endl)
        }
        else if (centerY <= 0)
        {
            centerY = center_map.rows / 2.;
        }
        else
        {
            LOG_GUARD(std::cout << "[est_joints] centerY:" << centerY << std::endl)
        }
        */

        {
            for (int r = 0; r < center_map.rows; r++)
            {
                for (int c = 0; c < center_map.cols; c++)
                {
                    float dist_sq = (r - centerY) * (r - centerY) +
                                    (c - centerX) * (c - centerX);
                    float exponent = dist_sq / 2.0 / sigma / sigma;
                    center_map.at<float>(cv::Point(c, r)) = (float) exp(-exponent);
                }
            }
        }

        FactoryTensorOpQueue resultQueue(0);
        FactoryTensorOp *pOp = new FactoryTensorOp(FactoryTensorOpJoints, &resultQueue);
        pOp->addInput("MYQ/Placeholder_1", TensorInput(preprocessed_image, true));
        pOp->addInput("MYQ/Placeholder_2", TensorInput(center_map, true));

        unique_ptr<FactoryTensorOp> op(pOp);
        mQueue.push(move(op));
        send(FactoryTensorOpJoints);
        if (resultQueue.popWait(op))
        {
            cv::Mat outputBlob = op->output(0);

            int croppedPaddedW = cropROI.width+left+right;
            int croppedPaddedH = cropROI.height+top+bottom;

            cv::Rect rect;

            if (!bUseJoints)
            {
                rect.x = right;
                rect.y = top;
                rect.width = inp_maxHW - 2 * right;
                rect.height = inp_maxHW - 2 * top;
            }
            else
            {
                rect.x = left;
                rect.y = top;
                rect.width = cropROI.width;
                rect.height = cropROI.height;
            }

            //cv::Mat HeatMapsAdded = cv::Mat::zeros(inp_size, CV_32F);
            for (int t = 0; t < 15; t++)
            {
                cv::Mat HeatMapEach(net_out_HorW, net_out_HorW, CV_32F);
                cv::Point min_loc, max_loc;
                double min_val, max_val;
                for (int y = 0; y < net_out_HorW; ++y)
                {
                    for (int x = 0; x < net_out_HorW; ++x)
                    {
                        HeatMapEach.at<float>(cv::Point(x, y)) = outputBlob.at<cv::Vec4f>(0, y, x)[t];
                    }
                }

                if (!bUseJoints)
                {
                    // resize to net test size
                    cv::resize(HeatMapEach, HeatMapEach, inp_image_maxsize, 0, 0, cv::INTER_CUBIC);
                    // crop to remove zero-padding
                    HeatMapEach = HeatMapEach(rect);
                    // resize to original input image size
                    cv::resize(HeatMapEach, HeatMapEach, inp_size, 0, 0, cv::INTER_CUBIC);
                    // smooth to get max peaks
                    cv::GaussianBlur(HeatMapEach, HeatMapEach, cv::Size(5, 5), 0.5, 0.5);
                    // get peak location
                    cv::minMaxLoc(HeatMapEach.clone(), &min_val, &max_val, &min_loc, &max_loc);
                }
                else
                {
                    // resize to net input size
                    cv::resize(HeatMapEach, HeatMapEach, cv::Size(croppedPaddedW, croppedPaddedH), 0, 0, cv::INTER_CUBIC);
                    // crop to remove zero-padding
                    HeatMapEach = HeatMapEach(rect);

                    // resize to original input image size
                    cv::Mat HeatMapScaled(inp_size, CV_32F);
                    cv::Mat HeatMapScaledROI(HeatMapScaled, cropROI);
                    HeatMapScaledROI += HeatMapEach;
                    //cv::resize(HeatMapEach, HeatMapEach, inp_size, 0, 0, cv::INTER_CUBIC);

                    // smooth to get max peaks
                    cv::GaussianBlur(HeatMapScaled, HeatMapScaled, cv::Size(5, 5), 0.5, 0.5);
                    // get peak location
                    cv::minMaxLoc(HeatMapScaled, &min_val, &max_val, &min_loc, &max_loc);
                }

                JointLocs.push_back(max_loc);

                // debugging...
                //HeatMapEach = 255. * HeatMapEach / max_val;
                //HeatMapsAdded = HeatMapsAdded + HeatMapEach;
                //HeatMapEach.convertTo(HeatMapEach, CV_8U);
                //visualize("joints", HeatMapEach);
            }

            printJoints("est_joints", JointLocs);
            return true;
        }
    }
    catch (cv::Exception &e)
    {
        LOG_GUARD(std::cout << "\n Error: " << e.what() << "\n")
        return false;
    }
    return false;
}

/////////////
vector<bool> ahiFactoryTensor::pose_inspect_usingsegnet(cv::Mat const &origImage, cv::Mat const &contour_mask, string const &view,
                                                     vector<cv::Point> &approx_joints)
{
    approx_joints.clear();
    approx_joints.resize(14);


    if (!mInitDone)
        return vector<bool>(14, 1);;

    cv::Mat net_mask = ahiFactoryTensor::performImageNetSegmentation(origImage);
    if (net_mask.empty())
    {
        cout << " Segmentation net failed net_mask is empty " << "\n";
        return vector<bool>(14, 1);
    }

    try
    {

        approx_joints.clear();
        approx_joints.resize(14);
        // Fill the contour_mask, but if it is already filled then
        cv::Mat contour_mask_filled = contour_mask.clone();

        std::vector<cv::Vec4i> contourHierarchy;
        std::vector<std::vector<cv::Point> > contours;

        // filling the contour mask
        cv::findContours(contour_mask_filled, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

        for (int i = 0; i < (int) contours.size(); i++)
        {
            cv::drawContours(contour_mask_filled, contours, i, cv::Scalar(255), cv::FILLED);
        }

        //imwrite("contour_mask_filled.png", contour_mask_filled);

        cv::Mat net_mask_masked;
        net_mask_masked = net_mask & contour_mask_filled;

        //imwrite("net_mask_masked.png", net_mask_masked);

        //cv::Mat maskToPlot = contour_mask | net_mask_masked;
        //cv::cvtColor(maskToPlot,maskToPlot,CV_GRAY2BGR);

// mimicing POJO
        /*
        std::vector<std::string> get_joint_names()
        {
           std::vector<std::string> joint_names;
           joint_names.push_back("Head");//0
           joint_names.push_back("Neck");//1
           joint_names.push_back("RightShoulder");//2
           joint_names.push_back("RightElbow");//3
           joint_names.push_back("RightWrist"); //4
           joint_names.push_back("LeftShoulder");//5
           joint_names.push_back("LeftElbow");//6
           joint_names.push_back("LeftWrist");//7
           joint_names.push_back("RightHip");//8
           joint_names.push_back("RightKnee");//9
           joint_names.push_back("RightAnkle");//10
           joint_names.push_back("LeftHip");//11
           joint_names.push_back("LeftKnee");//12
           joint_names.push_back("LeftAnkle");//13
           joint_names.push_back("BackGround");//14
           return joint_names;
        }
        */
        vector<bool> isPartDetected(14, 0);
        float sumTotal = (cv::sum(net_mask_masked).val[0]) + 0.0000001;
        cv::Rect PartRect;
        float PartInRatio, PartRatio;
        float thrd = 0.03; // can be redujusted
        int w = net_mask_masked.cols;
        int h = net_mask_masked.rows;
        int cx = w / 2;
        int cy = h / 2;

        cv::Rect ContourMaskROI = cv::boundingRect(contours[0]);
        int cxMask = ContourMaskROI.x + ContourMaskROI.width / 2;
        int cyMask = ContourMaskROI.y + ContourMaskROI.height / 2;

        //vector<Point> hull;
        //convexHull(Mat(contours[0]), hull, false);
        //cout<< hull;
        //exit(0);

        std::vector<std::vector<cv::Point> > TopContours;
        cv::Rect rect;
        rect.x = contour_mask_filled.cols / 2. - contour_mask_filled.cols / 6.;
        rect.width = 2 * contour_mask_filled.cols / 6.;
        rect.y = ContourMaskROI.y - 10;
        rect.height = ContourMaskROI.height / 7.5;
        //rectangle(maskToPlot,rect,cv::Scalar(0,0,255),3*maskToPlot.rows/1860);
        //imwrite("maskToPlot.png", maskToPlot);
        //exit(0);
        cv::findContours(contour_mask_filled(rect), TopContours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
        cv::Rect HeadRect = boundingRect(TopContours[0]);

        int FWH = min((int) contour_mask_filled.cols / 6, (int) HeadRect.width);//max((int) faceRect.width, (int) faceRect.height);
        cout << "\n FWH = " << FWH << "\n";
        cv::Mat croppedPart;
        cv::Point joint;
        int Partidx;
        cv::Moments Mom;

        //cv::RNG &rng = cv::theRNG();
        //rng.state = cv::getTickCount();
        //cv::Scalar Clr; // color
        if (view == "front")
        {
            cout << "\n View is entered as front\n";
            /// Head 0
            Partidx = 0;
            PartRect.x = cxMask - 0.75 * FWH; //cx-FWH/2;
            PartRect.width = 1.5 * FWH;
            PartRect.y = ContourMaskROI.y;
            PartRect.height = 1.5 * FWH;
            //cout << "PartRect : " << PartRect <<"\n";
            croppedPart = net_mask_masked(PartRect);
            PartInRatio = sum(croppedPart).val[0] / sumTotal;
            isPartDetected[Partidx] = bool(PartInRatio > thrd); //Head
            if (PartInRatio > thrd)
            {
                Mom = moments(croppedPart, true);
                joint.x = Mom.m10 / (Mom.m00 + 1e-8) + PartRect.x;
                joint.y = Mom.m01 / (Mom.m00 + 1e-8) + PartRect.y;
                //joint.x = cxMask;
                //joint.y = cyMask;
                approx_joints[Partidx] = joint; //Head
            }
            //Clr = cv::Scalar(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255)); rectangle(maskToPlot,PartRect,Clr,3*maskToPlot.rows/1860);
            //circle(maskToPlot,joint,10*maskToPlot.rows/1860,Clr,-1);

            /// Neck 1
            Partidx = 1;
            PartRect.x = cxMask - 0.75 * FWH;
            PartRect.width = 1.5 * FWH;
            PartRect.y = ContourMaskROI.y + 1.25 * FWH;
            PartRect.height = 1.25 * FWH;

            croppedPart = net_mask_masked(PartRect);
            PartInRatio = sum(croppedPart).val[0] / sumTotal;
            PartRatio = sum(net_mask(PartRect)).val[0] / sumTotal;
            //cout << "PartRatio, PartInRatio " << PartRatio <<" " << PartInRatio << "\n";
            //cout << "PartRatio-PartInRatio " << PartRatio-PartInRatio << "\n";
            //cout << "bool(PartInRatio > 0.6*thrd) " << bool(PartInRatio > 0.6*thrd) <<"\n";
            //cout << "bool(PartInRatio > (PartRatio-PartInRatio)) " << bool(PartInRatio > (PartRatio-PartInRatio)) <<"\n";
            isPartDetected[Partidx] = bool(PartInRatio > thrd) & bool(PartInRatio > (PartRatio - PartInRatio));

            if (isPartDetected[Partidx])
            {
                Mom = moments(croppedPart, true);

                joint.x = Mom.m10 / Mom.m00 + PartRect.x;
                joint.y = Mom.m01 / Mom.m00 + PartRect.y;
                approx_joints[Partidx] = joint;
            }

            //Clr = cv::Scalar(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255)); rectangle(maskToPlot,PartRect,Clr,3*maskToPlot.rows/1860);
            //circle(maskToPlot,joint,10*maskToPlot.rows/1860,Clr,-1);


            /// Rshoulder 2
            Partidx = 2;
            PartRect.x = cxMask - 1.55 * FWH;//cx-FWH/2;
            PartRect.width = 1.25 * FWH;
            PartRect.y = ContourMaskROI.y + 1.5 * FWH;
            PartRect.height = 1.6 * FWH;

            croppedPart = net_mask_masked(PartRect);
            PartInRatio = sum(croppedPart).val[0] / sumTotal;
            PartRatio = sum(net_mask(PartRect)).val[0] / sumTotal;
            //cout << "PartRatio, PartInRatio " << PartRatio <<" " << PartInRatio << "\n";
            //cout << "PartRatio-PartInRatio " << PartRatio-PartInRatio << "\n";
            //cout << "bool(PartInRatio > 0.6*thrd) " << bool(PartInRatio > 0.6*thrd) <<"\n";
            //cout << "bool(PartInRatio > (PartRatio-PartInRatio)) " << bool(PartInRatio > (PartRatio-PartInRatio)) <<"\n";
            isPartDetected[Partidx] = bool(PartInRatio > thrd) & bool(PartInRatio > (PartRatio - PartInRatio));

            if (isPartDetected[Partidx])
            {
                Mom = moments(croppedPart, true);

                joint.x = Mom.m10 / Mom.m00 + PartRect.x;
                joint.y = Mom.m01 / Mom.m00 + PartRect.y;
                approx_joints[Partidx] = joint;
            }

            //Clr = cv::Scalar(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255)); rectangle(maskToPlot,PartRect,Clr,3*maskToPlot.rows/1860);
            //circle(maskToPlot,joint,10*maskToPlot.rows/1860,Clr,-1);

            /// Relbow 3
            Partidx = 3;
            PartRect.x = ContourMaskROI.x + FWH;//cx-FWH/2;
            PartRect.width = FWH;
            PartRect.y = ContourMaskROI.y + 2 * FWH;
            PartRect.height = ContourMaskROI.height / 4;

            croppedPart = net_mask_masked(PartRect);
            PartInRatio = sum(croppedPart).val[0] / sumTotal;
            PartRatio = sum(net_mask(PartRect)).val[0] / sumTotal;
            //cout << "PartRatio, PartInRatio " << PartRatio <<" " << PartInRatio << "\n";
            //cout << "PartRatio-PartInRatio " << PartRatio-PartInRatio << "\n";
            //cout << "bool(PartInRatio > 0.6*thrd) " << bool(PartInRatio > 0.6*thrd) <<"\n";
            //cout << "bool(PartInRatio > (PartRatio-PartInRatio)) " << bool(PartInRatio > (PartRatio-PartInRatio)) <<"\n";
            isPartDetected[Partidx] = bool(PartInRatio > thrd) & bool(PartInRatio > (PartRatio - PartInRatio));

            if (isPartDetected[Partidx])
            {
                Mom = moments(croppedPart, true);

                joint.x = Mom.m10 / Mom.m00 + PartRect.x;
                joint.y = Mom.m01 / Mom.m00 + PartRect.y;
                approx_joints[Partidx] = joint;
            }

            //Clr = cv::Scalar(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255)); rectangle(maskToPlot,PartRect,Clr,3*maskToPlot.rows/1860);
            //circle(maskToPlot,joint,10*maskToPlot.rows/1860,Clr,-1);


            /// Rwrist 4
            Partidx = 4;
            PartRect.x = ContourMaskROI.x;//cx-FWH/2;
            PartRect.width = 1.25 * FWH;
            PartRect.y = ContourMaskROI.y + ContourMaskROI.height / 2 - 1. * FWH;
            PartRect.height = 2 * FWH;

            croppedPart = net_mask_masked(PartRect);
            PartInRatio = sum(croppedPart).val[0] / sumTotal;
            PartRatio = sum(net_mask(PartRect)).val[0] / sumTotal;
            //cout << "PartRatio, PartInRatio " << PartRatio <<" " << PartInRatio << "\n";
            //cout << "PartRatio-PartInRatio " << PartRatio-PartInRatio << "\n";
            //cout << "bool(PartInRatio > 0.6*thrd) " << bool(PartInRatio > 0.6*thrd) <<"\n";
            //cout << "bool(PartInRatio > (PartRatio-PartInRatio)) " << bool(PartInRatio > (PartRatio-PartInRatio)) <<"\n";
            isPartDetected[Partidx] = bool(PartInRatio > 0.6 * thrd) & bool(PartInRatio > (PartRatio - PartInRatio));

            if (isPartDetected[Partidx])
            {
                Mom = moments(croppedPart, true);

                joint.x = Mom.m10 / Mom.m00 + PartRect.x;
                joint.y = Mom.m01 / Mom.m00 + PartRect.y;
                approx_joints[Partidx] = joint;
            }

            //Clr = cv::Scalar(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255)); rectangle(maskToPlot,PartRect,Clr,3*maskToPlot.rows/1860);
            //circle(maskToPlot,joint,10*maskToPlot.rows/1860,Clr,-1);

            /// Lshoulder 5
            Partidx = 5;
            PartRect.x = cxMask + 0.30 * FWH;//cx-FWH/2;
            PartRect.width = 1.25 * FWH;
            PartRect.y = ContourMaskROI.y + 1.5 * FWH;
            PartRect.height = 1.6 * FWH;

            croppedPart = net_mask_masked(PartRect);
            PartInRatio = sum(croppedPart).val[0] / sumTotal;
            PartRatio = sum(net_mask(PartRect)).val[0] / sumTotal;
            //cout << "PartRatio, PartInRatio " << PartRatio <<" " << PartInRatio << "\n";
            //cout << "PartRatio-PartInRatio " << PartRatio-PartInRatio << "\n";
            //cout << "bool(PartInRatio > 0.6*thrd) " << bool(PartInRatio > 0.6*thrd) <<"\n";
            //cout << "bool(PartInRatio > (PartRatio-PartInRatio)) " << bool(PartInRatio > (PartRatio-PartInRatio)) <<"\n";
            isPartDetected[Partidx] = bool(PartInRatio > thrd) & bool(PartInRatio > (PartRatio - PartInRatio));

            if (isPartDetected[Partidx])
            {
                Mom = moments(croppedPart, true);

                joint.x = Mom.m10 / Mom.m00 + PartRect.x;
                joint.y = Mom.m01 / Mom.m00 + PartRect.y;
                approx_joints[Partidx] = joint;
            }

            //Clr = cv::Scalar(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255)); rectangle(maskToPlot,PartRect,Clr,3*maskToPlot.rows/1860);
            //circle(maskToPlot,joint,10*maskToPlot.rows/1860,Clr,-1);

            /// Lelbow 6
            Partidx = 6;
            PartRect.x = ContourMaskROI.x + ContourMaskROI.width - 2 * FWH;//cx-FWH/2;
            PartRect.width = FWH;
            PartRect.y = ContourMaskROI.y + 2 * FWH;
            PartRect.height = ContourMaskROI.height / 4;

            croppedPart = net_mask_masked(PartRect);
            PartInRatio = sum(croppedPart).val[0] / sumTotal;
            PartRatio = sum(net_mask(PartRect)).val[0] / sumTotal;
            //cout << "PartRatio, PartInRatio " << PartRatio <<" " << PartInRatio << "\n";
            //cout << "PartRatio-PartInRatio " << PartRatio-PartInRatio << "\n";
            //cout << "bool(PartInRatio > 0.6*thrd) " << bool(PartInRatio > 0.6*thrd) <<"\n";
            //cout << "bool(PartInRatio > (PartRatio-PartInRatio)) " << bool(PartInRatio > (PartRatio-PartInRatio)) <<"\n";
            isPartDetected[Partidx] = bool(PartInRatio > thrd) & bool(PartInRatio > (PartRatio - PartInRatio));

            if (isPartDetected[Partidx])
            {
                Mom = moments(croppedPart, true);

                joint.x = Mom.m10 / Mom.m00 + PartRect.x;
                joint.y = Mom.m01 / Mom.m00 + PartRect.y;
                approx_joints[Partidx] = joint;
            }

            //Clr = cv::Scalar(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255)); rectangle(maskToPlot,PartRect,Clr,3*maskToPlot.rows/1860);
            //circle(maskToPlot,joint,10*maskToPlot.rows/1860,Clr,-1);

            /// Lwrist 7
            Partidx = 7;
            PartRect.x = ContourMaskROI.x + ContourMaskROI.width - 1.25 * FWH;//cx-FWH/2;
            PartRect.width = 1.25 * FWH - 1;
            PartRect.y = ContourMaskROI.y + ContourMaskROI.height / 2 - 1 * FWH;
            PartRect.height = 2 * FWH;

            croppedPart = net_mask_masked(PartRect);
            PartInRatio = sum(croppedPart).val[0] / sumTotal;
            PartRatio = sum(net_mask(PartRect)).val[0] / sumTotal;
            //cout << "PartRatio, PartInRatio " << PartRatio <<" " << PartInRatio << "\n";
            //cout << "PartRatio-PartInRatio " << PartRatio-PartInRatio << "\n";
            //cout << "bool(PartInRatio > 0.6*thrd) " << bool(PartInRatio > 0.6*thrd) <<"\n";
            //cout << "bool(PartInRatio > (PartRatio-PartInRatio)) " << bool(PartInRatio > (PartRatio-PartInRatio)) <<"\n";
            isPartDetected[Partidx] = bool(PartInRatio > 0.6 * thrd) & bool(PartInRatio > (PartRatio - PartInRatio));

            if (isPartDetected[Partidx])
            {
                Mom = moments(croppedPart, true);

                joint.x = Mom.m10 / Mom.m00 + PartRect.x;
                joint.y = Mom.m01 / Mom.m00 + PartRect.y;
                approx_joints[Partidx] = joint;
            }

            //Clr = cv::Scalar(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255)); rectangle(maskToPlot,PartRect,Clr,3*maskToPlot.rows/1860);
            //circle(maskToPlot,joint,10*maskToPlot.rows/1860,Clr,-1);


            /// Rhip 8
            Partidx = 8;
            PartRect.x = cxMask - 1.25 * FWH;//cx-FWH/2;
            PartRect.width = 1.25 * FWH;
            PartRect.y = cyMask - 0.5 * FWH;
            PartRect.height = 2 * FWH;

            croppedPart = net_mask_masked(PartRect);
            PartInRatio = sum(croppedPart).val[0] / sumTotal;
            PartRatio = sum(net_mask(PartRect)).val[0] / sumTotal;
            //cout << "PartRatio, PartInRatio " << PartRatio <<" " << PartInRatio << "\n";
            //cout << "PartRatio-PartInRatio " << PartRatio-PartInRatio << "\n";
            //cout << "bool(PartInRatio > 0.6*thrd) " << bool(PartInRatio > 0.6*thrd) <<"\n";
            //cout << "bool(PartInRatio > (PartRatio-PartInRatio)) " << bool(PartInRatio > (PartRatio-PartInRatio)) <<"\n";
            isPartDetected[Partidx] = bool(PartInRatio > thrd) & bool(PartInRatio > (PartRatio - PartInRatio));

            if (isPartDetected[Partidx])
            {
                Mom = moments(croppedPart, true);

                joint.x = Mom.m10 / Mom.m00 + PartRect.x;
                joint.y = Mom.m01 / Mom.m00 + PartRect.y;
                approx_joints[Partidx] = joint;
            }

            //Clr = cv::Scalar(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255)); rectangle(maskToPlot,PartRect,Clr,3*maskToPlot.rows/1860);
            //circle(maskToPlot,joint,10*maskToPlot.rows/1860,Clr,-1);

            /// Rknee 9
            Partidx = 9;
            PartRect.x = cxMask - 1.5 * FWH;//cx-FWH/2;
            PartRect.width = 1.5 * FWH;
            PartRect.y = cyMask + 1.5 * FWH;
            PartRect.height = 1.5 * FWH;

            croppedPart = net_mask_masked(PartRect);
            PartInRatio = sum(croppedPart).val[0] / sumTotal;
            PartRatio = sum(net_mask(PartRect)).val[0] / sumTotal;
            //cout << "PartRatio, PartInRatio " << PartRatio <<" " << PartInRatio << "\n";
            //cout << "PartRatio-PartInRatio " << PartRatio-PartInRatio << "\n";
            //cout << "bool(PartInRatio > 0.6*thrd) " << bool(PartInRatio > 0.6*thrd) <<"\n";
            //cout << "bool(PartInRatio > (PartRatio-PartInRatio)) " << bool(PartInRatio > (PartRatio-PartInRatio)) <<"\n";
            isPartDetected[Partidx] = bool(PartInRatio > thrd) & bool(PartInRatio > (PartRatio - PartInRatio));

            if (isPartDetected[Partidx])
            {
                Mom = moments(croppedPart, true);

                joint.x = Mom.m10 / Mom.m00 + PartRect.x;
                joint.y = Mom.m01 / Mom.m00 + PartRect.y;
                approx_joints[Partidx] = joint;
            }

            //Clr = cv::Scalar(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255)); rectangle(maskToPlot,PartRect,Clr,3*maskToPlot.rows/1860);
            //circle(maskToPlot,joint,10*maskToPlot.rows/1860,Clr,-1);


            /// Rankle 10
            Partidx = 10;
            PartRect.x = cxMask - 1.75 * FWH;//cx-FWH/2;
            PartRect.width = 1.75 * FWH;
            PartRect.y = cyMask + 3 * FWH;
            PartRect.height = min(int(1.75 * FWH), int(ContourMaskROI.y + ContourMaskROI.height - PartRect.y) - 1);

            croppedPart = net_mask_masked(PartRect);
            PartInRatio = sum(croppedPart).val[0] / sumTotal;
            PartRatio = sum(net_mask(PartRect)).val[0] / sumTotal;
            //cout << "PartRatio, PartInRatio " << PartRatio <<" " << PartInRatio << "\n";
            //cout << "PartRatio-PartInRatio " << PartRatio-PartInRatio << "\n";
            //cout << "bool(PartInRatio > 0.6*thrd) " << bool(PartInRatio > 0.6*thrd) <<"\n";
            //cout << "bool(PartInRatio > (PartRatio-PartInRatio)) " << bool(PartInRatio > (PartRatio-PartInRatio)) <<"\n";
            isPartDetected[Partidx] = bool(PartInRatio > 0.6 * thrd) & bool(PartInRatio > (PartRatio - PartInRatio));

            if (isPartDetected[Partidx])
            {
                Mom = moments(croppedPart, true);

                joint.x = Mom.m10 / Mom.m00 + PartRect.x;
                joint.y = Mom.m01 / Mom.m00 + PartRect.y;
                approx_joints[Partidx] = joint;
            }

            //Clr = cv::Scalar(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255)); rectangle(maskToPlot,PartRect,Clr,3*maskToPlot.rows/1860);
            //circle(maskToPlot,joint,10*maskToPlot.rows/1860,Clr,-1);


            /// Lhip 11
            Partidx = 11;
            PartRect.x = cxMask;//cx-FWH/2;
            PartRect.width = 1.25 * FWH;
            PartRect.y = cyMask - 0.5 * FWH;
            PartRect.height = 2 * FWH;

            croppedPart = net_mask_masked(PartRect);
            PartInRatio = sum(croppedPart).val[0] / sumTotal;
            PartRatio = sum(net_mask(PartRect)).val[0] / sumTotal;
            //cout << "PartRatio, PartInRatio " << PartRatio <<" " << PartInRatio << "\n";
            //cout << "PartRatio-PartInRatio " << PartRatio-PartInRatio << "\n";
            //cout << "bool(PartInRatio > 0.6*thrd) " << bool(PartInRatio > 0.6*thrd) <<"\n";
            //cout << "bool(PartInRatio > (PartRatio-PartInRatio)) " << bool(PartInRatio > (PartRatio-PartInRatio)) <<"\n";
            isPartDetected[Partidx] = bool(PartInRatio > thrd) & bool(PartInRatio > (PartRatio - PartInRatio));

            if (isPartDetected[Partidx])
            {
                Mom = moments(croppedPart, true);

                joint.x = Mom.m10 / Mom.m00 + PartRect.x;
                joint.y = Mom.m01 / Mom.m00 + PartRect.y;
                approx_joints[Partidx] = joint;
            }

            //Clr = cv::Scalar(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255)); rectangle(maskToPlot,PartRect,Clr,3*maskToPlot.rows/1860);
            //circle(maskToPlot,joint,10*maskToPlot.rows/1860,Clr,-1);


            /// Lknee 12
            Partidx = 12;
            PartRect.x = cxMask;//cx-FWH/2;
            PartRect.width = 1.5 * FWH;
            PartRect.y = cyMask + 1.5 * FWH;
            PartRect.height = 1.5 * FWH;

            croppedPart = net_mask_masked(PartRect);
            PartInRatio = sum(croppedPart).val[0] / sumTotal;
            PartRatio = sum(net_mask(PartRect)).val[0] / sumTotal;
            //cout << "PartRatio, PartInRatio " << PartRatio <<" " << PartInRatio << "\n";
            //cout << "PartRatio-PartInRatio " << PartRatio-PartInRatio << "\n";
            //cout << "bool(PartInRatio > 0.6*thrd) " << bool(PartInRatio > 0.6*thrd) <<"\n";
            //cout << "bool(PartInRatio > (PartRatio-PartInRatio)) " << bool(PartInRatio > (PartRatio-PartInRatio)) <<"\n";
            isPartDetected[Partidx] = bool(PartInRatio > thrd) & bool(PartInRatio > (PartRatio - PartInRatio));

            if (isPartDetected[Partidx])
            {
                Mom = moments(croppedPart, true);

                joint.x = Mom.m10 / Mom.m00 + PartRect.x;
                joint.y = Mom.m01 / Mom.m00 + PartRect.y;
                approx_joints[Partidx] = joint;
            }

            //Clr = cv::Scalar(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255)); rectangle(maskToPlot,PartRect,Clr,3*maskToPlot.rows/1860);
            //circle(maskToPlot,joint,10*maskToPlot.rows/1860,Clr,-1);


            /// Lankle 13
            Partidx = 13;
            PartRect.x = cxMask;//cx-FWH/2;
            PartRect.width = 1.75 * FWH;
            PartRect.y = cyMask + 3 * FWH;
            PartRect.height = min(int(1.75 * FWH), int(ContourMaskROI.y + ContourMaskROI.height - PartRect.y) - 1);

            croppedPart = net_mask_masked(PartRect);
            PartInRatio = sum(croppedPart).val[0] / sumTotal;
            PartRatio = sum(net_mask(PartRect)).val[0] / sumTotal;
            //cout << "PartRatio, PartInRatio " << PartRatio <<" " << PartInRatio << "\n";
            //cout << "PartRatio-PartInRatio " << PartRatio-PartInRatio << "\n";
            //cout << "bool(PartInRatio > 0.6*thrd) " << bool(PartInRatio > 0.6*thrd) <<"\n";
            //cout << "bool(PartInRatio > (PartRatio-PartInRatio)) " << bool(PartInRatio > (PartRatio-PartInRatio)) <<"\n";
            isPartDetected[Partidx] = bool(PartInRatio > 0.6 * thrd) & bool(PartInRatio > (PartRatio - PartInRatio));

            if (isPartDetected[Partidx])
            {
                Mom = moments(croppedPart, true);

                joint.x = Mom.m10 / Mom.m00 + PartRect.x;
                joint.y = Mom.m01 / Mom.m00 + PartRect.y;
                approx_joints[Partidx] = joint;
            }

            //Clr = cv::Scalar(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255));
            //rectangle(maskToPlot,PartRect,Clr,3*maskToPlot.rows/1860);
            //circle(maskToPlot,joint,10*maskToPlot.rows/1860,Clr,-1);
        }
        else if (view == "side") // in progress
        {
            cxMask = cx;
            /// Head 0
            Partidx = 0;
            PartRect.x = cxMask - 0.75 * FWH; //cx-FWH/2;
            PartRect.width = 1.5 * FWH;
            PartRect.y = ContourMaskROI.y;
            PartRect.height = 1.5 * FWH;
            //cout << "PartRect : " << PartRect <<"\n";
            //exit(0);
            croppedPart = net_mask_masked(PartRect);
            PartInRatio = sum(croppedPart).val[0] / sumTotal;
            isPartDetected[Partidx] = bool(PartInRatio > thrd); //Head
            if (PartInRatio > thrd)
            {
                Mom = moments(croppedPart, true);
                joint.x = Mom.m10 / Mom.m00 + PartRect.x;
                joint.y = Mom.m01 / Mom.m00 + PartRect.y;
                //joint.x = cxMask;
                //joint.y = cyMask;
                approx_joints[Partidx] = joint; //Head
            }
            //Clr = cv::Scalar(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255));
            //rectangle(maskToPlot,PartRect,Clr,3*maskToPlot.rows/1860);
            //circle(maskToPlot,joint,10*maskToPlot.rows/1860,Clr,-1);
            //imwrite("SideMaskToPlot.png", maskToPlot);
            //exit(0);
            /// Neck 1
            Partidx = 1;
            PartRect.x = cxMask - 0.75 * FWH;
            PartRect.width = 1.5 * FWH;
            PartRect.y = ContourMaskROI.y + 1.25 * FWH;
            PartRect.height = 1.25 * FWH;

            croppedPart = net_mask_masked(PartRect);
            PartInRatio = sum(croppedPart).val[0] / sumTotal;
            PartRatio = sum(net_mask(PartRect)).val[0] / sumTotal;
            //cout << "PartRatio, PartInRatio " << PartRatio <<" " << PartInRatio << "\n";
            //cout << "PartRatio-PartInRatio " << PartRatio-PartInRatio << "\n";
            //cout << "bool(PartInRatio > 0.6*thrd) " << bool(PartInRatio > 0.6*thrd) <<"\n";
            //cout << "bool(PartInRatio > (PartRatio-PartInRatio)) " << bool(PartInRatio > (PartRatio-PartInRatio)) <<"\n";
            isPartDetected[Partidx] = bool(PartInRatio > thrd) & bool(PartInRatio > (PartRatio - PartInRatio));

            if (isPartDetected[Partidx])
            {
                Mom = moments(croppedPart, true);

                joint.x = Mom.m10 / Mom.m00 + PartRect.x;
                joint.y = Mom.m01 / Mom.m00 + PartRect.y;
                approx_joints[Partidx] = joint;
            }

            //Clr = cv::Scalar(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255));
            //rectangle(maskToPlot,PartRect,Clr,3*maskToPlot.rows/1860);
            //circle(maskToPlot,joint,10*maskToPlot.rows/1860,Clr,-1);

            //imwrite("SideMaskToPlot.png", maskToPlot);
            //exit(0);
            /// Rshoulder 2
            Partidx = 2;
            PartRect.x = cxMask - 0.75 * FWH;
            PartRect.width = 1.5 * FWH;
            PartRect.y = ContourMaskROI.y + 1.5 * FWH;
            PartRect.height = 1.6 * FWH;

            croppedPart = net_mask_masked(PartRect);
            PartInRatio = sum(croppedPart).val[0] / sumTotal;
            PartRatio = sum(net_mask(PartRect)).val[0] / sumTotal;
            //cout << "PartRatio, PartInRatio " << PartRatio <<" " << PartInRatio << "\n";
            //cout << "PartRatio-PartInRatio " << PartRatio-PartInRatio << "\n";
            //cout << "bool(PartInRatio > 0.6*thrd) " << bool(PartInRatio > 0.6*thrd) <<"\n";
            //cout << "bool(PartInRatio > (PartRatio-PartInRatio)) " << bool(PartInRatio > (PartRatio-PartInRatio)) <<"\n";
            isPartDetected[Partidx] = bool(PartInRatio > thrd) & bool(PartInRatio > (PartRatio - PartInRatio));

            if (isPartDetected[Partidx])
            {
                Mom = moments(croppedPart, true);

                joint.x = Mom.m10 / Mom.m00 + PartRect.x;
                joint.y = Mom.m01 / Mom.m00 + PartRect.y;
                approx_joints[Partidx] = joint;
            }

            //Clr = cv::Scalar(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255));
            //rectangle(maskToPlot,PartRect,Clr,3*maskToPlot.rows/1860);
            //circle(maskToPlot,joint,20*maskToPlot.rows/1860,Clr,-1);

            /// Relbow 3
            Partidx = 3;
            PartRect.x = cxMask - 0.75 * FWH;
            PartRect.width = 1.5 * FWH;
            PartRect.y = ContourMaskROI.y + 2 * FWH;
            PartRect.height = ContourMaskROI.height / 4;

            croppedPart = net_mask_masked(PartRect);
            PartInRatio = sum(croppedPart).val[0] / sumTotal;
            PartRatio = sum(net_mask(PartRect)).val[0] / sumTotal;
            //cout << "PartRatio, PartInRatio " << PartRatio <<" " << PartInRatio << "\n";
            //cout << "PartRatio-PartInRatio " << PartRatio-PartInRatio << "\n";
            //cout << "bool(PartInRatio > 0.6*thrd) " << bool(PartInRatio > 0.6*thrd) <<"\n";
            //cout << "bool(PartInRatio > (PartRatio-PartInRatio)) " << bool(PartInRatio > (PartRatio-PartInRatio)) <<"\n";
            isPartDetected[Partidx] = bool(PartInRatio > thrd) & bool(PartInRatio > (PartRatio - PartInRatio));

            if (isPartDetected[Partidx])
            {
                Mom = moments(croppedPart, true);

                joint.x = Mom.m10 / Mom.m00 + PartRect.x;
                joint.y = Mom.m01 / Mom.m00 + PartRect.y;
                approx_joints[Partidx] = joint;
            }

            //Clr = cv::Scalar(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255));
            //rectangle(maskToPlot,PartRect,Clr,3*maskToPlot.rows/1860);
            //circle(maskToPlot,joint,20*maskToPlot.rows/1860,Clr,-1);


            /// Rwrist 4
            Partidx = 4;
            PartRect.x = cxMask - 0.75 * FWH;
            PartRect.width = 1.5 * FWH;
            PartRect.y = ContourMaskROI.y + ContourMaskROI.height / 2 - 1. * FWH;
            PartRect.height = 2 * FWH;

            croppedPart = net_mask_masked(PartRect);
            PartInRatio = sum(croppedPart).val[0] / sumTotal;
            PartRatio = sum(net_mask(PartRect)).val[0] / sumTotal;
            //cout << "PartRatio, PartInRatio " << PartRatio <<" " << PartInRatio << "\n";
            //cout << "PartRatio-PartInRatio " << PartRatio-PartInRatio << "\n";
            //cout << "bool(PartInRatio > 0.6*thrd) " << bool(PartInRatio > 0.6*thrd) <<"\n";
            //cout << "bool(PartInRatio > (PartRatio-PartInRatio)) " << bool(PartInRatio > (PartRatio-PartInRatio)) <<"\n";
            isPartDetected[Partidx] = bool(PartInRatio > 0.6 * thrd) & bool(PartInRatio > (PartRatio - PartInRatio));

            if (isPartDetected[Partidx])
            {
                Mom = moments(croppedPart, true);

                joint.x = Mom.m10 / Mom.m00 + PartRect.x;
                joint.y = Mom.m01 / Mom.m00 + PartRect.y;
                approx_joints[Partidx] = joint;
            }

            // Clr = cv::Scalar(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255));
            //rectangle(maskToPlot,PartRect,Clr,3*maskToPlot.rows/1860);
            //circle(maskToPlot,joint,20*maskToPlot.rows/1860,Clr,-1);

            /// Lshoulder 5
            Partidx = 5;
            PartRect.x = cxMask - 0.75 * FWH;
            PartRect.width = 1.5 * FWH;
            PartRect.y = ContourMaskROI.y + 1.5 * FWH;
            PartRect.height = 1.6 * FWH;

            croppedPart = net_mask_masked(PartRect);
            PartInRatio = sum(croppedPart).val[0] / sumTotal;
            PartRatio = sum(net_mask(PartRect)).val[0] / sumTotal;
            //cout << "PartRatio, PartInRatio " << PartRatio <<" " << PartInRatio << "\n";
            //cout << "PartRatio-PartInRatio " << PartRatio-PartInRatio << "\n";
            //cout << "bool(PartInRatio > 0.6*thrd) " << bool(PartInRatio > 0.6*thrd) <<"\n";
            //cout << "bool(PartInRatio > (PartRatio-PartInRatio)) " << bool(PartInRatio > (PartRatio-PartInRatio)) <<"\n";
            isPartDetected[Partidx] = bool(PartInRatio > thrd) & bool(PartInRatio > (PartRatio - PartInRatio));

            if (isPartDetected[Partidx])
            {
                Mom = moments(croppedPart, true);

                joint.x = Mom.m10 / Mom.m00 + PartRect.x;
                joint.y = Mom.m01 / Mom.m00 + PartRect.y;
                approx_joints[Partidx] = joint;
            }

            //Clr = cv::Scalar(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255));
            //rectangle(maskToPlot,PartRect,Clr,3*maskToPlot.rows/1860);
            //circle(maskToPlot,joint,10*maskToPlot.rows/1860,Clr,-1);

            /// Lelbow 6
            PartRect.x = cxMask - 0.75 * FWH;
            PartRect.width = 1.5 * FWH;
            PartRect.y = ContourMaskROI.y + 2 * FWH;
            PartRect.height = ContourMaskROI.height / 4;

            croppedPart = net_mask_masked(PartRect);
            PartInRatio = sum(croppedPart).val[0] / sumTotal;
            PartRatio = sum(net_mask(PartRect)).val[0] / sumTotal;
            //cout << "PartRatio, PartInRatio " << PartRatio <<" " << PartInRatio << "\n";
            //cout << "PartRatio-PartInRatio " << PartRatio-PartInRatio << "\n";
            //cout << "bool(PartInRatio > 0.6*thrd) " << bool(PartInRatio > 0.6*thrd) <<"\n";
            //cout << "bool(PartInRatio > (PartRatio-PartInRatio)) " << bool(PartInRatio > (PartRatio-PartInRatio)) <<"\n";
            isPartDetected[Partidx] = bool(PartInRatio > thrd) & bool(PartInRatio > (PartRatio - PartInRatio));

            if (isPartDetected[Partidx])
            {
                Mom = moments(croppedPart, true);

                joint.x = Mom.m10 / Mom.m00 + PartRect.x;
                joint.y = Mom.m01 / Mom.m00 + PartRect.y;
                approx_joints[Partidx] = joint;
            }

            //Clr = cv::Scalar(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255));
            //rectangle(maskToPlot,PartRect,Clr,3*maskToPlot.rows/1860);
            //circle(maskToPlot,joint,10*maskToPlot.rows/1860,Clr,-1);

            /// Lwrist 7
            Partidx = 7;
            PartRect.x = cxMask - 0.75 * FWH;
            PartRect.width = 1.5 * FWH;
            PartRect.y = ContourMaskROI.y + ContourMaskROI.height / 2 - 1 * FWH;
            PartRect.height = 2 * FWH;

            croppedPart = net_mask_masked(PartRect);
            PartInRatio = sum(croppedPart).val[0] / sumTotal;
            PartRatio = sum(net_mask(PartRect)).val[0] / sumTotal;
            //cout << "PartRatio, PartInRatio " << PartRatio <<" " << PartInRatio << "\n";
            //cout << "PartRatio-PartInRatio " << PartRatio-PartInRatio << "\n";
            //cout << "bool(PartInRatio > 0.6*thrd) " << bool(PartInRatio > 0.6*thrd) <<"\n";
            //cout << "bool(PartInRatio > (PartRatio-PartInRatio)) " << bool(PartInRatio > (PartRatio-PartInRatio)) <<"\n";
            isPartDetected[Partidx] = bool(PartInRatio > 0.6 * thrd) & bool(PartInRatio > (PartRatio - PartInRatio));

            if (isPartDetected[Partidx])
            {
                Mom = moments(croppedPart, true);

                joint.x = Mom.m10 / Mom.m00 + PartRect.x;
                joint.y = Mom.m01 / Mom.m00 + PartRect.y;
                approx_joints[Partidx] = joint;
            }

            //Clr = cv::Scalar(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255));
            //rectangle(maskToPlot,PartRect,Clr,3*maskToPlot.rows/1860);
            //circle(maskToPlot,joint,10*maskToPlot.rows/1860,Clr,-1);


            /// Rhip 8
            Partidx = 8;
            PartRect.x = cxMask - 0.75 * FWH;
            PartRect.width = 1.5 * FWH;
            PartRect.y = cyMask - 0.5 * FWH;
            PartRect.height = 2 * FWH;

            croppedPart = net_mask_masked(PartRect);
            PartInRatio = sum(croppedPart).val[0] / sumTotal;
            PartRatio = sum(net_mask(PartRect)).val[0] / sumTotal;
            //cout << "PartRatio, PartInRatio " << PartRatio <<" " << PartInRatio << "\n";
            //cout << "PartRatio-PartInRatio " << PartRatio-PartInRatio << "\n";
            //cout << "bool(PartInRatio > 0.6*thrd) " << bool(PartInRatio > 0.6*thrd) <<"\n";
            //cout << "bool(PartInRatio > (PartRatio-PartInRatio)) " << bool(PartInRatio > (PartRatio-PartInRatio)) <<"\n";
            isPartDetected[Partidx] = bool(PartInRatio > thrd) & bool(PartInRatio > (PartRatio - PartInRatio));

            if (isPartDetected[Partidx])
            {
                Mom = moments(croppedPart, true);

                joint.x = Mom.m10 / Mom.m00 + PartRect.x;
                joint.y = Mom.m01 / Mom.m00 + PartRect.y;
                approx_joints[Partidx] = joint;
            }

            //Clr = cv::Scalar(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255));
            //rectangle(maskToPlot,PartRect,Clr,3*maskToPlot.rows/1860);
            //circle(maskToPlot,joint,20*maskToPlot.rows/1860,Clr,-1);

            /// Rknee 9
            PartRect.x = cxMask - 0.75 * FWH;
            PartRect.width = 1.5 * FWH;
            PartRect.width = 1.5 * FWH;
            PartRect.y = cyMask + 1.5 * FWH;
            PartRect.height = 1.5 * FWH;

            croppedPart = net_mask_masked(PartRect);
            PartInRatio = sum(croppedPart).val[0] / sumTotal;
            PartRatio = sum(net_mask(PartRect)).val[0] / sumTotal;
            //cout << "PartRatio, PartInRatio " << PartRatio <<" " << PartInRatio << "\n";
            //cout << "PartRatio-PartInRatio " << PartRatio-PartInRatio << "\n";
            //cout << "bool(PartInRatio > 0.6*thrd) " << bool(PartInRatio > 0.6*thrd) <<"\n";
            //cout << "bool(PartInRatio > (PartRatio-PartInRatio)) " << bool(PartInRatio > (PartRatio-PartInRatio)) <<"\n";
            isPartDetected[Partidx] = bool(PartInRatio > thrd) & bool(PartInRatio > (PartRatio - PartInRatio));

            if (isPartDetected[Partidx])
            {
                Mom = moments(croppedPart, true);

                joint.x = Mom.m10 / Mom.m00 + PartRect.x;
                joint.y = Mom.m01 / Mom.m00 + PartRect.y;
                approx_joints[Partidx] = joint;
            }

            //Clr = cv::Scalar(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255));
            //rectangle(maskToPlot,PartRect,Clr,3*maskToPlot.rows/1860);
            //circle(maskToPlot,joint,20*maskToPlot.rows/1860,Clr,-1);


            /// Rankle 10
            Partidx = 10;
            PartRect.x = cxMask - 0.75 * FWH;
            PartRect.width = 1.5 * FWH;
            PartRect.y = cyMask + 3 * FWH;
            PartRect.height = min(int(1.75 * FWH), int(ContourMaskROI.y + ContourMaskROI.height - PartRect.y) - 1);

            croppedPart = net_mask_masked(PartRect);
            PartInRatio = sum(croppedPart).val[0] / sumTotal;
            PartRatio = sum(net_mask(PartRect)).val[0] / sumTotal;
            //cout << "PartRatio, PartInRatio " << PartRatio <<" " << PartInRatio << "\n";
            //cout << "PartRatio-PartInRatio " << PartRatio-PartInRatio << "\n";
            //cout << "bool(PartInRatio > 0.6*thrd) " << bool(PartInRatio > 0.6*thrd) <<"\n";
            //cout << "bool(PartInRatio > (PartRatio-PartInRatio)) " << bool(PartInRatio > (PartRatio-PartInRatio)) <<"\n";
            isPartDetected[Partidx] = bool(PartInRatio > 0.6 * thrd) & bool(PartInRatio > (PartRatio - PartInRatio));

            if (isPartDetected[Partidx])
            {
                Mom = moments(croppedPart, true);

                joint.x = Mom.m10 / Mom.m00 + PartRect.x;
                joint.y = Mom.m01 / Mom.m00 + PartRect.y;
                approx_joints[Partidx] = joint;
            }

            //Clr = cv::Scalar(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255));
            //rectangle(maskToPlot,PartRect,Clr,3*maskToPlot.rows/1860);
            //circle(maskToPlot,joint,20*maskToPlot.rows/1860,Clr,-1);


            /// Lhip 11
            Partidx = 11;
            PartRect.x = cxMask - 0.75 * FWH;
            PartRect.width = 1.5 * FWH;
            PartRect.y = cyMask - 0.5 * FWH;
            PartRect.height = 2 * FWH;

            croppedPart = net_mask_masked(PartRect);
            PartInRatio = sum(croppedPart).val[0] / sumTotal;
            PartRatio = sum(net_mask(PartRect)).val[0] / sumTotal;
            //cout << "PartRatio, PartInRatio " << PartRatio <<" " << PartInRatio << "\n";
            //cout << "PartRatio-PartInRatio " << PartRatio-PartInRatio << "\n";
            //cout << "bool(PartInRatio > 0.6*thrd) " << bool(PartInRatio > 0.6*thrd) <<"\n";
            //cout << "bool(PartInRatio > (PartRatio-PartInRatio)) " << bool(PartInRatio > (PartRatio-PartInRatio)) <<"\n";
            isPartDetected[Partidx] = bool(PartInRatio > thrd) & bool(PartInRatio > (PartRatio - PartInRatio));

            if (isPartDetected[Partidx])
            {
                Mom = moments(croppedPart, true);

                joint.x = Mom.m10 / Mom.m00 + PartRect.x;
                joint.y = Mom.m01 / Mom.m00 + PartRect.y;
                approx_joints[Partidx] = joint;
            }

            //Clr = cv::Scalar(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255));
            //rectangle(maskToPlot,PartRect,Clr,3*maskToPlot.rows/1860);
            //circle(maskToPlot,joint,10*maskToPlot.rows/1860,Clr,-1);


            /// Lknee 12
            Partidx = 12;
            PartRect.x = cxMask - 0.75 * FWH;
            PartRect.width = 1.5 * FWH;
            PartRect.y = cyMask + 1.5 * FWH;
            PartRect.height = 1.5 * FWH;

            croppedPart = net_mask_masked(PartRect);
            PartInRatio = sum(croppedPart).val[0] / sumTotal;
            PartRatio = sum(net_mask(PartRect)).val[0] / sumTotal;
            //cout << "PartRatio, PartInRatio " << PartRatio <<" " << PartInRatio << "\n";
            //cout << "PartRatio-PartInRatio " << PartRatio-PartInRatio << "\n";
            //cout << "bool(PartInRatio > 0.6*thrd) " << bool(PartInRatio > 0.6*thrd) <<"\n";
            //cout << "bool(PartInRatio > (PartRatio-PartInRatio)) " << bool(PartInRatio > (PartRatio-PartInRatio)) <<"\n";
            isPartDetected[Partidx] = bool(PartInRatio > thrd) & bool(PartInRatio > (PartRatio - PartInRatio));

            if (isPartDetected[Partidx])
            {
                Mom = moments(croppedPart, true);

                joint.x = Mom.m10 / Mom.m00 + PartRect.x;
                joint.y = Mom.m01 / Mom.m00 + PartRect.y;
                approx_joints[Partidx] = joint;
            }

            //Clr = cv::Scalar(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255));
            //rectangle(maskToPlot,PartRect,Clr,3*maskToPlot.rows/1860);
            //circle(maskToPlot,joint,10*maskToPlot.rows/1860,Clr,-1);


            /// Lankle 13
            Partidx = 13;
            PartRect.x = cxMask - 0.75 * FWH;
            PartRect.width = 1.5 * FWH;
            PartRect.y = cyMask + 3 * FWH;
            PartRect.height = min(int(1.75 * FWH), int(ContourMaskROI.y + ContourMaskROI.height - PartRect.y) - 1);

            croppedPart = net_mask_masked(PartRect);
            PartInRatio = sum(croppedPart).val[0] / sumTotal;
            PartRatio = sum(net_mask(PartRect)).val[0] / sumTotal;
            //cout << "PartRatio, PartInRatio " << PartRatio <<" " << PartInRatio << "\n";
            //cout << "PartRatio-PartInRatio " << PartRatio-PartInRatio << "\n";
            //cout << "bool(PartInRatio > 0.6*thrd) " << bool(PartInRatio > 0.6*thrd) <<"\n";
            //cout << "bool(PartInRatio > (PartRatio-PartInRatio)) " << bool(PartInRatio > (PartRatio-PartInRatio)) <<"\n";
            isPartDetected[Partidx] = bool(PartInRatio > 0.6 * thrd) & bool(PartInRatio > (PartRatio - PartInRatio));

            if (isPartDetected[Partidx])
            {
                Mom = moments(croppedPart, true);

                joint.x = Mom.m10 / Mom.m00 + PartRect.x;
                joint.y = Mom.m01 / Mom.m00 + PartRect.y;
                approx_joints[Partidx] = joint;
            }

            //Clr = cv::Scalar(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255));
            //rectangle(maskToPlot,PartRect,Clr,3*maskToPlot.rows/1860);
            //circle(maskToPlot,joint,10*maskToPlot.rows/1860,Clr,-1);
        }

        //imwrite("SideMaskToPlot.png", maskToPlot);

        return isPartDetected;
    }
    catch (cv::Exception &err_pose_segnet)
    {
        cout << "Exception error in pose using segnet " << err_pose_segnet.what();
        return vector<bool>(14, 0);
    }
}
////////////////////////////////////

cv::Mat
ahiFactoryTensor::performImageNetSegmentation(cv::Mat const &tensorMat)
{
    //cv::Mat silhoutte;
    cv::Mat preprocessed_image;
    int top = 0;
    int bottom = 0;
    int left = 0;
    int right = 0;

    try
    {
        preprocessed_image = preprocess_image(tensorMat, cv::Size(256, 256), top, bottom, left,
                                              right);
        if (preprocessed_image.empty())
        {
            return cv::Mat();
        }
    }
    catch (const exception &e)
    {
        LOG_GUARD(std::cout << e.what() << endl)
        return cv::Mat();
    }

    preprocessed_image.convertTo(preprocessed_image, CV_32F);
    preprocessed_image = preprocessed_image / 255.;

    cv::Mat net_mask;
    unique_ptr<FactoryTensorOp> op;
    FactoryTensorOpQueue resultQueue(0);
    mQueue.push(move(unique_ptr<FactoryTensorOp>(new FactoryTensorOp(
            FactoryTensorOpSegment,
            "input_net",
            TensorInput(preprocessed_image, true, false, cv::Size(256, 256), {1.0 / 255.0}, {0.}),
            &resultQueue))));
    send(FactoryTensorOpSegment);
    if (resultQueue.popWait(op))
    {
        cv::Size inp_size = tensorMat.size();
        int inp_maxHW = max(tensorMat.rows, tensorMat.cols);
        cv::Size inp_image_maxsize = cv::Size(inp_maxHW, inp_maxHW);

        cv::Rect rect;
        rect.x = right;
        rect.y = top;
        rect.width = inp_maxHW - 2 * right;
        rect.height = inp_maxHW - 2 * top;
        cv::Mat outputBlob = op->output(0);
        cv::Mat HeatMapsAdded = cv::Mat::zeros(inp_size, CV_32F);

        int net_out_HorW = 256;
        cv::Size output_size(net_out_HorW, net_out_HorW); // output net size
        cv::Mat Out_HeatMap(output_size, CV_32F);
        //Out_HeatMap.data = (float *) OutputTensor->data.f;
        //float thrld = 0.0;
        //vector<vector<Point> > peak_locationAll(15, vector<Point>());
        int t = 0;

        for (int y = 0; y < net_out_HorW; ++y)
        {
            for (int x = 0; x < net_out_HorW; ++x)
            {
                //if((float) outputBlob.at<Vec4f>(0,y,x)[t] > thrld)
                Out_HeatMap.at<float>(cv::Point(x, y)) = (float) outputBlob.at<cv::Vec4f>(0, y,
                                                                                          x)[t];
                //else
                //    Out_HeatMap.at<float>(Point(x,y)) = 0.0;
            }
        }


        cv::resize(Out_HeatMap, Out_HeatMap, inp_image_maxsize, 0, 0,
                   cv::INTER_CUBIC); // resize to net test size

        cv::Mat Cropped_HeatMap = Out_HeatMap(rect); // crop to remove zero-padding

        cv::Point min_loc, max_loc;
        double min_val, max_val;

        cv::minMaxLoc(Cropped_HeatMap.clone(), &min_val, &max_val, &min_loc,
                      &max_loc);// get peak location

        Cropped_HeatMap = 255. * Cropped_HeatMap / (1.0e-10 + max_val);


        Cropped_HeatMap.convertTo(Cropped_HeatMap, CV_8U);

        cv::threshold(Cropped_HeatMap, net_mask, 0, 255, cv::THRESH_BINARY + cv::THRESH_OTSU);

        return net_mask;
    }
    return cv::Mat(); // to avoid warnnings
}

cv::Mat getBiggestBlob(cv::Mat const matImage_orig)
{
    try
    {

        cv::Mat matImage = matImage_orig.clone();
        if (matImage.channels() > 1)
        {
            cv::cvtColor(matImage, matImage, cv::COLOR_BGR2GRAY);
        }
        double largest_area = 0;
        int largest_contour_index = 0;
        cv::Mat BiggestBlob = cv::Mat::zeros(matImage.size(), matImage.type());
        std::vector<std::vector<cv::Point> > contours; // Vector for storing contour
        std::vector<cv::Vec4i> hierarchy;
        findContours(matImage.clone(), contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE); // Find the contours in the image
        for (int i = 0; i < contours.size(); i++)
        {// iterate through each contour.
            double a = contourArea(contours[i]);  //  Find the area of contour
            if (a > largest_area)
            {
                largest_area = a;
                largest_contour_index = i;                //Store the index of largest contour
                //bounding_rect=boundingRect(contours[i]); // Find the bounding rectangle for biggest contour
            }
        }
        drawContours(BiggestBlob, contours, largest_contour_index, cv::Scalar(255),
                     -1); // Draw the largest contour using previously stored index.
        return BiggestBlob;
    }
    catch (...)
    {
        return matImage_orig;
    }
}

cv::Mat
ahiFactoryTensor::performImageSegmentation(cv::Mat const &tensorMat, ahi_avatar_gen::view_t viewType,
                                        cv::Mat const &contourMat, vector<cv::Point> joints, int index)
{
    bool bInputCrop = false;
    cv::Mat silhoutte;
    cv::Mat preprocessed_image;
    int top = 0;
    int bottom = 0;
    int left = 0;
    int right = 0;
    cv::Rect cropROI;

    try
    {
        if (bInputCrop)
        {
            Joints poseInfo;
            putJointParts(poseInfo, joints);
            preprocessed_image = preprocess_image_joints(tensorMat, viewType, poseInfo, cv::Size(256, 256), cropROI, top, bottom, left, right);
        }
        else
        {
            preprocessed_image = preprocess_image(tensorMat, cv::Size(256, 256), top, bottom, left,
                                                  right);
        }

        if (preprocessed_image.empty())
        {
            return silhoutte;
        }
    }
    catch (const exception &e)
    {
        LOG_GUARD(std::cout << e.what() << endl)
        return silhoutte;
    }

    preprocessed_image.convertTo(preprocessed_image, CV_32F);
    preprocessed_image = preprocessed_image / 255.;

    cv::Mat net_mask;
    unique_ptr<FactoryTensorOp> op;
    FactoryTensorOpQueue resultQueue(0);
    mQueue.push(move(unique_ptr<FactoryTensorOp>(new FactoryTensorOp(
            FactoryTensorOpSegment, "input_net",
            TensorInput(preprocessed_image, true, false, cv::Size(256, 256), {1.0 / 255.0}, {0.}),
            &resultQueue))));
    send(FactoryTensorOpSegment);
    if (resultQueue.popWait(op) && op->hasOutput(0))
    {
        cv::Mat outputBlob = op->output(0);
        cv::Size inp_size = tensorMat.size();
        int inp_maxHW = max(tensorMat.rows, tensorMat.cols);
        cv::Size inp_image_maxsize = cv::Size(inp_maxHW, inp_maxHW);

        cv::Rect rect;
        int croppedPaddedW = cropROI.width+left+right;
        int croppedPaddedH = cropROI.height+top+bottom;

        if (bInputCrop)
        {
            rect.x = left;
            rect.y = top;
            rect.width = cropROI.width;
            rect.height = cropROI.height;
        }
        else
        {
            rect.x = right;
            rect.y = top;
            rect.width = inp_maxHW - 2 * right;
            rect.height = inp_maxHW - 2 * top;
        }

        cv::Mat HeatMapsAdded = cv::Mat::zeros(inp_size, CV_32F);

        int net_out_HorW = 256;
        cv::Size output_size(net_out_HorW, net_out_HorW); // output net size
        cv::Mat Out_HeatMap(output_size, CV_32F);
        //Out_HeatMap.data = (float *) OutputTensor->data.f;
        float thrld = 0.2;
        //vector<vector<Point> > peak_locationAll(15, vector<Point>());
        int t = 0;

        for (int y = 0; y < net_out_HorW; ++y)
        {
            for (int x = 0; x < net_out_HorW; ++x)
            {
                //if((float) outputBlob.at<Vec4f>(0,y,x)[t] > thrld)
                Out_HeatMap.at<float>(cv::Point(x, y)) = (float) outputBlob.at<cv::Vec4f>(0, y,
                                                                                          x)[t];
                //else
                //    Out_HeatMap.at<float>(Point(x,y)) = 0.0;
            }
        }

        cv::Mat Cropped_HeatMap;

        if (bInputCrop)
        {
            Cropped_HeatMap = cv::Mat(inp_size, CV_32F);
            // resize to net input size
            cv::resize(Out_HeatMap, Out_HeatMap, cv::Size(croppedPaddedW, croppedPaddedH), 0, 0, cv::INTER_CUBIC);
            // crop to remove zero-padding
            Out_HeatMap = Out_HeatMap(rect);

            cv::Mat HeatMapScaledROI(Cropped_HeatMap, cropROI);
            HeatMapScaledROI += Out_HeatMap;
        }
        else
        {
            // resize to net test size
            cv::resize(Out_HeatMap, Out_HeatMap, inp_image_maxsize, 0, 0, cv::INTER_CUBIC);
            // crop to remove zero-padding
            Cropped_HeatMap = Out_HeatMap(rect);
        }

        cv::Point min_loc, max_loc;
        double min_val, max_val;

        cv::minMaxLoc(Cropped_HeatMap.clone(), &min_val, &max_val, &min_loc,
                      &max_loc);// get peak location

        Cropped_HeatMap = 255. * Cropped_HeatMap / (1.0e-10 + max_val);


        Cropped_HeatMap.convertTo(Cropped_HeatMap, CV_8U);

        cv::threshold(Cropped_HeatMap, net_mask, 0, 255, cv::THRESH_BINARY + cv::THRESH_OTSU);
    }

//    if (POJO_joints_are_used)
//    {
//        joints = MFZ_JH.fix_POJO_joints(joints, viewType, CAMERA_WIDTH, CAMERA_HEIGHT);
//    }

    if ((!net_mask.empty()) && (net_mask.rows > 0) && (net_mask.cols > 0))
    {
        if (ahi_avatar_gen::view_t::front == viewType)
        {
            process_callback("mask_front", index, net_mask);
        }
        else
        {
            process_callback("mask_side", index, net_mask);
        }

        // here is the actual in device segmentation
        ahi_avatar_gen::joints_helper MFZ_JH;
        //cv::Mat image_plus_joints = MFZ_JH.draw_image_with_joints(origImage.clone(), joints);
        silhoutte = MFZ_JH.segment_using_net_joints_and_grabcut(tensorMat, viewType, net_mask,
                                                                joints, contourMat);
        silhoutte = getBiggestBlob(silhoutte);

        //process_callback("seg", index, silhoutte);
    }
    else
    {
        LOG_GUARD(std::cout << THREAD_STR << "Error getting seg result" << std::endl)
    }

    return silhoutte;
}

FactoryTensor *ahiFactoryTensor::getInstance()
{
    AutoLock lock(gInstanceMutex);

    if (nullptr == mThis)
    {
        mThis = new FactoryTensor();
    }

    return mThis;
}



void ahiFactoryTensor::init(size_t keylen, const uint8_t *key, size_t noncelen, const uint8_t *nonce, bool bOnDevice)
{
    if (!mInitDone)
    {
#if defined(ANDROID) || defined(__ANDROID__)
        //uint8_t *keydata = new uint8_t[32];
        //uint8_t *noncedata = new uint8_t[12];

        memset(mKeydata, 0, 32);
        memset(mNoncedata, 0, 12);

        memcpy(mKeydata, key, MIN(keylen, 32));
        memcpy(mNoncedata, nonce, MIN(noncelen, 12));
#endif

        cv::FileStorage fs;

        fs.open(front_face_features,
                cv::FileStorage::READ | cv::FileStorage::FORMAT_XML | cv::FileStorage::MEMORY);
        mFrontFaceCascadeDetector.read(fs.getFirstTopLevelNode()); //TODO: check result & handle.

        fs.open(side_face_features,
                cv::FileStorage::READ | cv::FileStorage::FORMAT_XML | cv::FileStorage::MEMORY);
        mProfileFaceCascadeDetector.read(fs.getFirstTopLevelNode()); //TODO: check result & handle.

        mInitDone = true;

        if (bOnDevice)
        {
            // Use flags (bit 0 - FactoryTensorOpFlagOnDevice) to indicate on device classification
            mQueue.push(move(unique_ptr<FactoryTensorOp>(new FactoryTensorOp(FactoryTensorOpInit, FactoryTensorOpFlagOnDevice))));
        }
        else
        {
            mQueue.push(move(unique_ptr<FactoryTensorOp>(new FactoryTensorOp(FactoryTensorOpInit))));
        }
        send(FactoryTensorOpInit);
    }
}
#endif


struct membuf : std::streambuf {
    membuf(char *begin, char *end) : begin(begin), end(end) {
        this->setg(begin, begin, end);
    }

    virtual pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which = std::ios_base::in) override {
        if (dir == std::ios_base::cur)
            gbump(off);
        else if (dir == std::ios_base::end)
            setg(begin, end + off, end);
        else if (dir == std::ios_base::beg)
            setg(begin, begin + off, end);

        return gptr() - eback();
    }

    virtual pos_type seekpos(streampos pos, std::ios_base::openmode mode) override {
        return seekoff(pos - pos_type(off_type(0)), std::ios_base::beg, mode);
    }

    char *begin, *end;
};

#if 0
ahi_avatar_gen::MyFiziqSVR *ahiFactoryTensor::getSvr(std::string name)
{
    size_t pos = name.find(".bin");
    if (std::string::npos != pos)
    {
        name.erase(pos);
    }

    SvrMap::iterator iter = mSvrMap.find(name);
    if (mSvrMap.end() != iter)
    {
        //LOG_GUARD(std::cout << "[ahiFactoryTensor::getSvr] asset cache hit " << name << std::endl)
        return iter->second;
    }

    return loadSvr(name);
}

ahi_avatar_gen::MyFiziqSVR *ahiFactoryTensor::loadSvr(std::string name, bool bWaitForDownload)
{
    AutoLock lock(mMutex);
    //LOG_GUARD(std::cout << "[ahiFactoryTensor::loadSvr] " << name << std::endl)

    size_t pos = name.find(".bin");
    if (std::string::npos != pos)
    {
        name.erase(pos);
    }

    // (re)check local cache...
    SvrMap::iterator iter = mSvrMap.find(name);
    if (mSvrMap.end() != iter)
    {
        return iter->second;
    }

    AssetManager *mgr;// = getAssetMgr();
    if (nullptr != mgr)
    {
        std::string asset_name = name;
        asset_name.append(".bin");

        Asset *asset = nullptr;

        if (bWaitForDownload)
        {
            asset = mgr->open(asset_name.c_str());
        }
        else
        {
            asset = mgr->openIfReady(asset_name.c_str());
        }

        if (nullptr != asset)
        {
            int64_t dataSize = mgr->getLength(asset);
            char *pData = (char *) mgr->load(asset);

            if (nullptr != pData)
            {
                //LOG_GUARD(std::cout << "[ahiFactoryTensor::getSvr] got asset " << asset_name << std::endl)
                decodeSvr((unsigned char *) pData, dataSize);

                membuf sbuf(pData, pData + dataSize);
                std::istream in(&sbuf);
                cereal::PortableBinaryInputArchive archive(in);
                // Read into object
                ahi_avatar_gen::MyFiziqSVR *pSvr = new ahi_avatar_gen::MyFiziqSVR;
                archive(*pSvr);

                free(pData);

                mSvrMap[name] = pSvr;

                return pSvr;
            }
        }
    }

    LOG_GUARD(std::cout << "[ahiFactoryTensor::getSvr] failed to get asset " << name << std::endl)
    return nullptr;
}

cv::Mat ahiFactoryTensor::scaleContour(
        bool bUseJoints, bool bSmooth, bool &inZone, cv::Mat const &tensorMat, cv::Mat contourMat, std::string contourId,
        ahi_avatar_gen::view_t viewType, std::string gender, float height, float weight,
        Joints &poseInfo)
{
    if (!mInitDone)
    {
        // Failsafe - return original contour.
        return contourMat;
    }

    std::string viewStr = viewType == ahi_avatar_gen::view_t::front ? "front" : "side";
    std::vector<cv::Point> joints_set(14);

    std::string basicContouridStr = contourId;
    cv::Mat basicContour = FactoryContour::getInstance()->getBasicContourTf(basicContouridStr);

    bool isMask = false;

    Joints mJoints;
    float mCenterY;

    if (poseInfo.CentroidHeadTop.x == 0 && !tensorMat.empty())
    {
        if (bUseJoints)
        {
            // run pose for initial check & image cropping.
            if (!predictFromHeatMapOpt(tensorMat, basicContour, viewStr, isMask, joints_set, poseInfo))
            {
                LOG_GUARD(std::cout << "predictFromHeatMapOpt() was unsuccessful" << std::endl)
                return basicContour;
            }

            //ahi_avatar_gen::joints_helper MFZ_JH;
            //MFZ_JH.fix_POJO_joints(joints_set, viewType, false);

            // Ensure within green zone.
            if (!FactoryContour::getInstance()->isInGreenZone(false, viewType, contourId, joints_set))
            {
                return basicContour;
            }

            if (!joints(tensorMat, basicContour, viewType, joints_set, poseInfo, mCenterY))
            {
                LOG_GUARD(std::cout << "joints() was unsuccessful" << std::endl)
                return basicContour;
            }
        }
        else
        {
            //predictFromAHIPose(tensorMat, basicContour, viewStr, isMask, joints_set, poseInfo);
            if (!predictFromHeatMapOpt(tensorMat, basicContour, viewStr, isMask, joints_set, poseInfo))
            {
                LOG_GUARD(std::cout << "predictFromHeatMapOpt() was unsuccessful" << std::endl)
                return basicContour;
            }

            //ahi_avatar_gen::joints_helper MFZ_JH;
            //MFZ_JH.fix_POJO_joints(joints_set, viewType, false);
        }

        putJointParts(poseInfo, joints_set);
    }
    else
    {
        getJointParts(poseInfo, joints_set);
    }

    if (contourMat.empty())
    {
        contourMat = basicContour;
    }

    //ahiJsonPose poseResult = performImageInspection(tensorMat, contourMat, viewStr, false, pojo_joints, poseInfo);
    return FactoryContour::getInstance()->genUserDynoStaticContour(bSmooth, inZone, gender, height, weight, viewType, contourMat,
                                                                   contourId, joints_set, false);
}

bool ahiFactoryTensor::pose(cv::Mat &tensorMat, cv::Mat &contourMat, ahi_avatar_gen::view_t viewType,
                         std::string &result, Joints &poseInfo)
{
    if (!mInitDone)
        return false;

    //Stopwatch totalTime("pose time");

    bool isMask = false;

    //fill the contour
    std::string viewStr = viewType == ahi_avatar_gen::view_t::front ? "front" : "side";
    std::vector<cv::Point> pojo_joints(14);

    ahiJsonPose poseResult = performImageInspection(tensorMat, contourMat, viewStr, isMask, pojo_joints, poseInfo);
    bool passInspection = checkForInspection(poseResult, viewStr);

    updateResult(poseResult, result);

    //std::vector<cv::Point> JointLocs;
    //getJointParts(poseInfo, JointLocs);

    //ahi_avatar_gen::joints_helper MFZ_JH;
    //JointLocs = MFZ_JH.fix_POJO_joints(JointLocs, viewType, false, CAMERA_WIDTH, CAMERA_HEIGHT);
    //putJointParts(poseInfo, JointLocs);

    if (!passInspection)
    {
        LOG_GUARD(std::cout << THREAD_STR << "Inspection Error:" << result << std::endl)
    }
    else
    {
        LOG_GUARD(std::cout << THREAD_STR << "Inspection Result:" << result << std::endl)
    }

    //totalTime.print();
    return passInspection;
}

bool
ahiFactoryTensor::joints(cv::Mat const &tensorMat, cv::Mat const &contourMat, ahi_avatar_gen::view_t viewType,
                      std::vector<cv::Point> &jointsResult, Joints &poseInfo, float &centerY)
{
    bool bResult = est_joints(tensorMat, contourMat, viewType, poseInfo, jointsResult, centerY);
    return bResult;
}

cv::Mat
ahiFactoryTensor::segment(cv::Mat const &tensorMat, cv::Mat contourMat,
                       ahi_avatar_gen::view_t viewType, Joints &poseInfo, bool bUseJoints, int index)
{
    cv::Mat seg_mask;

    if (!mInitDone)
        return seg_mask;

    //Stopwatch totalTime("Total time");

    std::vector<cv::Point> JointLocs;

    getJointParts(poseInfo, JointLocs);

    seg_mask = performImageSegmentation(tensorMat, viewType, contourMat, JointLocs, index);

    visualize("seg_mask", seg_mask);

    //totalTime.print();

    return seg_mask;
}

//std::vector<float>
//ahiFactoryTensor::classify(FactoryTensorModelId classModelId, double height, double weight, std::string gender, cv::Mat const front_sil,
//                        cv::Mat const side_sil, cv::Mat image_features, std::vector<bool> is_Used)
//{
//    if (!mInitDone)
//        return std::vector<float>();
//
//    std::vector<float> class_results;
//    ahiTensorOutputMap classOutputs;
//    bool isClassResOK = performDLClassification(classModelId, height, weight, gender, front_sil, side_sil, image_features, is_Used,
//                                                class_results, classOutputs);
//
//    if (isClassResOK)
//    {
//        return class_results;
//    }
//    else
//    {
//        return std::vector<float>();
//    }
//}

//ahiTensorOutputMap
//ahiFactoryTensor::classifyMIMO(FactoryTensorModelId classModelId, double height, double weight, std::string gender, cv::Mat const front_sil,
//                        cv::Mat const side_sil, cv::Mat image_features, std::vector<bool> is_Used)
//{
//    ahiTensorOutputMap classOutputs;
//
//    if (!mInitDone)
//        return classOutputs;
//
//    std::vector<float> class_results;
//
//    bool isClassResOK = performDLClassification(classModelId, height, weight, gender, front_sil, side_sil, image_features, is_Used,
//                                                classOutputs);//class_results, classOutputs);
//
//    if (isClassResOK)
//    {
//        return classOutputs;
//    }
//    else
//    {
//        classOutputs.clear();
//        return classOutputs;
//    }
//}

/////////////////

//////
//bool ahiFactoryTensor::performDLClassification(FactoryTensorModelId classModelId, double height, double weight, std::string gender,
//                                            cv::Mat const front_sil, cv::Mat const side_sil, cv::Mat image_features,
//                                            std::vector<bool> is_Used, ahiTensorOutputMap& classOutputs)
//
////bool ahiFactoryTensor::performDLClassification(FactoryTensorModelId classModelId, double height, double weight, std::string gender,
////                                            cv::Mat const front_sil, cv::Mat const side_sil, cv::Mat image_features,
////                                            std::vector<bool> is_Used, std::vector<float> &results, ahiTensorOutputMap& classOutputs)
//{
//
//    if (!mInitDone)
//        return false;
//
//    cv::Mat preprocessed_image;
//    std::vector<cv::Mat> imageChs;
//    cv::Size target_size = cv::Size(256, 256);
//    if (is_Used[0])// height: we tweak the input layers to have the heigh as a Mat so we can still use the FactoryTensor class which now assumes inputs are Mats
//    {
//        cv::Mat hMat = height * (cv::Mat::ones(target_size, CV_32F));
//        imageChs.push_back(hMat);
//    }
//    if (is_Used[1]) // same as above for the weight
//    {
//        cv::Mat wMat = weight * (cv::Mat::ones(target_size, CV_32F));
//        imageChs.push_back(wMat);
//    }
//
//    if (is_Used[3] & is_Used[4]) // front and side
//    {
//        cv::Mat front_preprocessed_image;
//        cv::Mat side_preprocessed_image;
//
//        cv::Size inp_size;
//
//        int inp_maxHW, inp_image_maxsize;
//        int top, bottom, left, right;
//        top = 0;
//        bottom = 0;
//        left = 0;
//        right = 0;
//
//        try
//        {
//            inp_size = front_sil.size();
//            inp_maxHW = max(front_sil.rows, front_sil.cols);
//            //inp_image_maxsize = cv::Size(inp_maxHW, inp_maxHW);
//            front_preprocessed_image = preprocess_image_gray(front_sil, target_size, top, bottom,
//                                                             left, right);
//            side_preprocessed_image = preprocess_image_gray(side_sil, target_size, top, bottom,
//                                                            left, right);
//
//            if (front_preprocessed_image.empty() || side_preprocessed_image.empty())
//            {
//                //return std::vector<double>();
//                return false;
//            }
//        }
//        catch (const exception &e)
//        {
//
//            LOG_GUARD(std::cout << e.what() << endl)
//            //return std::vector<double>();
//            return false;
//        }
//        front_preprocessed_image.convertTo(front_preprocessed_image, CV_32F);
//        side_preprocessed_image.convertTo(side_preprocessed_image, CV_32F);
//
//        imageChs.push_back(front_preprocessed_image);
//        imageChs.push_back(side_preprocessed_image);
//    }
//
//    if (is_Used[5])
//    {
//        imageChs.push_back(image_features);
//    }
//
//    if ((int) imageChs.size() > 1)
//    {
//        cv::merge(imageChs, preprocessed_image);
//    }
//    else if ((int) imageChs.size() == 1)
//    {
//        preprocessed_image = imageChs[0];
//    }
//    else  // no input data of any type
//    {
//        std::cout << "[performDLClassification] ERR: no inputs" << std::endl;
//        return false;
//    }
//
//    //LOG_GUARD(std::cout << "[performDLClassification] about to run:" << classModelId << endl)
//    //long long int now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
//
//    // Queue the TFLite operation and wait for the result...
//    unique_ptr<FactoryTensorOp> op;
//    FactoryTensorOpQueue resultQueue(0);
//
//    // ClassV3p1Models models to be added later once we mange to convert them
//    switch (classModelId)
//    {
//        default:
//        case ModelClassV1:
//            postInitModel(classModelId, true);
//            mQueue.push(move(unique_ptr<FactoryTensorOp>(new FactoryTensorOp(
//                    FactoryTensorOpClassV1, "silhouettes", preprocessed_image, &resultQueue))));
//            send(FactoryTensorOpClassV1);
//            break;
//
//        case ModelClassV2male:
//            postInitModel(classModelId, true);
//            mQueue.push(move(unique_ptr<FactoryTensorOp>(new FactoryTensorOp(
//                    FactoryTensorOpClassV2male, "silhouettes", preprocessed_image, &resultQueue))));
//            send(FactoryTensorOpClassV2male);
//            break;
//
//        case ModelClassV2female:
//            postInitModel(classModelId, true);
//            mQueue.push(move(unique_ptr<FactoryTensorOp>(new FactoryTensorOp(
//                    FactoryTensorOpClassV2female, "silhouettes", preprocessed_image, &resultQueue))));
//            send(FactoryTensorOpClassV2female);
//            break;
//
//        case ModelClassV2p5:
//            postInitModel(classModelId, true);
//            mQueue.push(move(unique_ptr<FactoryTensorOp>(new FactoryTensorOp(
//                    FactoryTensorOpClassV2p5, "silhouettes", preprocessed_image, &resultQueue))));
//            send(FactoryTensorOpClassV2p5);
//            break;
//
//        case ModelClassV3male:
//            postInitModel(classModelId, true);
//            mQueue.push(move(unique_ptr<FactoryTensorOp>(new FactoryTensorOp(
//                    FactoryTensorOpClassV3male, "silhouettes", preprocessed_image, &resultQueue))));
//            send(FactoryTensorOpClassV3male);
//            break;
//
//        case ModelClassV3female:
//            postInitModel(classModelId, true);
//            mQueue.push(move(unique_ptr<FactoryTensorOp>(new FactoryTensorOp(
//                    FactoryTensorOpClassV3female, "silhouettes", preprocessed_image, &resultQueue))));
//            send(FactoryTensorOpClassV3female);
//            break;
//        case ModelClassV3p1:
//        {
//            postInitModel(classModelId, true);
//            FactoryTensorOp *pOp = new FactoryTensorOp(FactoryTensorOpClassV3p1, &resultQueue);
//            std::vector<double> additional_data(2);
//            additional_data[0] = (height / 255.0f - 0.5149143288800009) / 0.1858516016514072;
//            additional_data[1] = (weight / 255.0f - 0.5149143288800009) / 0.1858516016514072;
//            cv::Mat addInputMat = cv::Mat(2, 1, CV_64F, additional_data.data());
//            addInputMat.convertTo(addInputMat, CV_32F);
//            pOp->addInput("additional_data", TensorInput(addInputMat, false));
//            cv::Size target_size = cv::Size(256, 256);
//            int top_, bottom_, left_, right_;
//            cv::Mat prep_front = preprocess_image_std_or_robust(front_sil,
//                                                                target_size, top_,
//                                                                bottom_, left_, right_, true, 2.0,
//                                                                2.0);
//            cv::Mat prep_side = preprocess_image_std_or_robust(side_sil, target_size,
//                                                               top_, bottom_, left_, right_, true,
//                                                               1.5, 1.5);
//
//            if (prep_front.rows != prep_side.rows || prep_front.cols != prep_side.cols)
//            {
//                return false;
//            }
//            std::vector<cv::Mat> front_side_mats(2);
//            front_side_mats[0] = prep_front;
//            front_side_mats[1] = prep_side;
//            cv::Mat front_side_mats_merged;
//            cv::merge(front_side_mats, front_side_mats_merged);
//            front_side_mats_merged.convertTo(front_side_mats_merged, CV_32F);
//            pOp->addInput("silhouettes",
//                          TensorInput(front_side_mats_merged / 255.0f, true, false, target_size,
//                                      {255.0f}, {0.}));
//            mQueue.push(move(unique_ptr<FactoryTensorOp>(pOp)));
//            send(FactoryTensorOpClassV3p1);
//        }
//        break;
//
//        case ModelClassTBFIM1:
//        {
//            postInitModel(classModelId, true);
//            FactoryTensorOp *pOp = new FactoryTensorOp(FactoryTensorOpClassTBFIM1, &resultQueue);
//            std::vector<double> additional_data(2);
//            additional_data[0] = height;
//            additional_data[1] = weight;
//            cv::Mat addInputMat = cv::Mat(2, 1, CV_64F, additional_data.data());//cv::Mat(2, 1, CV_32F,additional_data.data());
//            addInputMat.convertTo(addInputMat, CV_32F);
//            pOp->addInput("additional_data", TensorInput(addInputMat));
//            cv::Size target_size = cv::Size(256, 256);
//            int top_, bottom_, left_, right_;
//            cv::Mat prep_front = preprocess_image_std_or_robust(front_sil,
//                                                                target_size, top_,
//                                                                bottom_, left_, right_, true, 2.0,
//                                                                2.0);
//            cv::Mat prep_side = preprocess_image_std_or_robust(side_sil, target_size,
//                                                               top_, bottom_, left_, right_, true,
//                                                               1.5, 1.5);
//            if (prep_front.rows != prep_side.rows || prep_front.cols != prep_side.cols)
//            {
//                return false;
//            }
//            std::vector<cv::Mat> front_side_mats(2);
//            front_side_mats[0] = prep_front;
//            front_side_mats[1] = prep_side;
//            cv::Mat front_side_mats_merged;
//            cv::merge(front_side_mats, front_side_mats_merged);
//            front_side_mats_merged.convertTo(front_side_mats_merged, CV_32F);
//            pOp->addInput("silhouettes",
//                          TensorInput(front_side_mats_merged, true, false, target_size,
//                                      {1.0f}, {0.}));
//            mQueue.push(move(unique_ptr<FactoryTensorOp>(pOp)));
//            send(FactoryTensorOpClassTBFIM1);
//        }
//        break;
//
//        case ModelClassTBFIM2: // note this uses the same data as the above TBFIM1
//        {
//            postInitModel(classModelId, true);
//            FactoryTensorOp *pOp = new FactoryTensorOp(FactoryTensorOpClassTBFIM2, &resultQueue);
//            std::vector<double> additional_data(2);
//            additional_data[0] = height;
//            additional_data[1] = weight;
//            cv::Mat addInputMat = cv::Mat(2, 1, CV_64F, additional_data.data());
//            addInputMat.convertTo(addInputMat, CV_32F);
//            pOp->addInput("additional_data", TensorInput(addInputMat, false));
//            cv::Size target_size = cv::Size(256, 256);
//            int top_, bottom_, left_, right_;
//            cv::Mat prep_front = preprocess_image_std_or_robust(front_sil,
//                                                                target_size, top_,
//                                                                bottom_, left_, right_, true, 2.0,
//                                                                2.0);
//            cv::Mat prep_side = preprocess_image_std_or_robust(side_sil, target_size,
//                                                               top_, bottom_, left_, right_, true,
//                                                               1.5, 1.5);
//
//            if (prep_front.rows != prep_side.rows || prep_front.cols != prep_side.cols)
//            {
//                return false;
//            }
//            std::vector<cv::Mat> front_side_mats(2);
//            front_side_mats[0] = prep_front;
//            front_side_mats[1] = prep_side;
//            cv::Mat front_side_mats_merged;
//            cv::merge(front_side_mats, front_side_mats_merged);
//            front_side_mats_merged.convertTo(front_side_mats_merged, CV_32F);
//            pOp->addInput("silhouettes",
//                          TensorInput(front_side_mats_merged, true, false, target_size,
//                                      {1.0f}, {0.}));
//            mQueue.push(move(unique_ptr<FactoryTensorOp>(pOp)));
//            send(FactoryTensorOpClassTBFIM2);
//        }
//        break;
//
//        case ModelExmeasImageBased:
//        {
//            postInitModel(classModelId, true);
//            FactoryTensorOp *pOp = new FactoryTensorOp(FactoryTensorOpExmeasImageBased, &resultQueue);
//            cv::Mat inputImageFeat;
//            cv::Mat inputHWGFeat;
//            cv::Mat  inputNormalDist;
//
//            bool isPrepOkay = prepareInputsForImageBasedExtraMeas(front_sil,side_sil,height,weight,gender,inputImageFeat,inputHWGFeat,inputNormalDist);
//            cv::Size target_size = cv::Size(256, 256);
//            pOp->addInput("input_1",
//                          TensorInput(inputImageFeat, true, false, target_size,
//                                      {1.0f}, {0.}));
//            pOp->addInput("input_2", TensorInput(inputHWGFeat, false));
//            pOp->addInput("input_3", TensorInput(inputNormalDist, false));
//
//            mQueue.push(move(unique_ptr<FactoryTensorOp>(pOp)));
//            send(FactoryTensorOpExmeasImageBased);
//        }
//        break;
//
//    }
//
//    //    mQueue.push(move(unique_ptr<FactoryTensorOp>(new FactoryTensorOp(
//    //          FactoryTensorOpClassV1HWFS, front_preprocessed_image, side_preprocessed_image,image_features, 189, 90, 0,  &resultQueue))));
//
//    if (resultQueue.popWait(op) && op->hasOutput(0))
//    {
//        if (!op->mInvokeSuccess)
//        {
//            LOG_GUARD(std::cout << "performDLClassification() received an empty result" << std::endl)
//            //results = std::vector<double>();
//            return false;
//        }
////        ahiTensorOutputMap classOutputs = op-> mOutputs; // All outputs as a map/pair with first name and second is data (mat)
//        classOutputs = op-> mOutputs;
//
////        cv::Mat outputBlob = op->output(0);
////        results.assign(outputBlob.begin<float>(), outputBlob.end<float>());
//        return true;
//    }
//
//    return false;
//}
#endif
