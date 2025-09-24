//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#ifndef ahiFactoryTensor_H_
#define ahiFactoryTensor_H_

#include "AutoLock.hpp"
#include "Mutex.hpp"
#include "AssetManager.hpp"
#include <cstddef>
#include <memory>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <chrono>
#include <sstream>
#include <string>
#include <stdio.h>
#include <fstream>
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/delegates/gpu/delegate.h"
#include "tensorflow/lite/delegates/nnapi/nnapi_delegate.h"
#include "tensorflow/lite/delegates/xnnpack/xnnpack_delegate.h"
#include "tensorflow/lite/error_reporter.h"
#include "tensorflow/lite/interpreter_builder.h"
#include "tensorflow/lite/model_builder.h"
// This is Added to get the gpu info as we used to do it in previous TF, but it seems we can no longer get the info so we can decided if CPU,NNAPI, .. or GPU are better.. TODO
#include "tensorflow/lite/delegates/gpu/common/status.h"
#include "abseil-cpp/absl/status/status.h"
#include "tensorflow/lite/delegates/gpu/common/gpu_info.h"
#include "tensorflow/lite/delegates/gpu/gl/request_gpu_info.h"
#include "tensorflow/lite/delegates/gpu/gl_delegate.h"
#include "tensorflow/lite/experimental/acceleration/compatibility/android_info.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include "tensorflow/lite/delegates/gpu/common/status.h"
#include "abseil-cpp/absl/status/status.h"
#include "tensorflow/lite/delegates/gpu/common/gpu_info.h"
#include "tensorflow/lite/delegates/gpu/gl/request_gpu_info.h"
#include "tensorflow/lite/delegates/gpu/gl_delegate.h"
#include "tensorflow/lite/experimental/acceleration/compatibility/android_info.h"
#include "Types.hpp"
#include "AHIAvatarGenClassificationHelper.hpp"
#include <log2022.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types_c.h>
#include <opencv2/imgproc/types_c.h>
#include "AHIAvatarGenMdlBase.hpp"
#include "math.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <inttypes.h>
#include "chacha20.hpp"
#include <sys/ptrace.h>
#include <random>

// split a flat vector containing nrows*ncols elements into a two dimensional matrix
// T is (a placeholder for) the type of the values in the vector
// for example, for std:vector<int>, T would be int
template<typename T>
std::vector<std::vector<T> > to_2d(const std::vector<T> &flat_vec, std::size_t ncols) {
    // ncols is the number of cols in the matrix
    // the size of the flat vector must be an integral multiple of some non-zero ncols
    // if the check fails, report an error (by throwing an exception)
    if (ncols == 0 || flat_vec.size() % ncols != 0) throw std::domain_error("bad #cols");

    // compute the number of rows in the matrix ie. flat_vec.size() == nrows * ncols
    const auto nrows = flat_vec.size() / ncols;

    // declare an empty matrix. eventually, we build this up to an nrows x ncols matrix
    // the final matrix would be a collection of nrows rows
    // exch row in the matrix would a collection (a vector) containing ncols elements
    std::vector<std::vector<T> > mtx;

    // get an iterator to the beginning of the flat vector
    // http://en.cppreference.com/w/cpp/iterator/begin
    // if you are unfamiliar with iterators, see: https://cal-linux.com/tutorials/STL.html
    const auto begin = std::begin(flat_vec);

    // add rows one by one to the matrix
    for (std::size_t row = 0; row < nrows; ++row) // for each row [0,nrows-1] in the matrix
    {
        // add the row (a vector of ncols elements)
        // for example, if ncols = 12,
        // row 0 would contain elements in positions 0, 1, 2, ...10, 11 ie. [0,12)
        // row 1 would contain elements in positions 12, 13, ...23 ie. [12,24)
        // row 2 would contain elements in positions 24, 25, ...35 ie. [24,36)
        // in general, row r would contain elements at positions [ r*12, (r+1)*12 ]
        mtx.push_back({begin + row * ncols, begin + (row + 1) * ncols});
        // the above as akin to:
        // construct the row containing elements at positions as described earlier
        // const std::vector<T> this_row( begin + row*ncols, begin + (row+1)*ncols ) ;
        // mtx.push_back(this_row) ; // and add this row to the back of the vector
    }

    return mtx; // return the fully populated matrix
}

#ifndef MIN
#define MIN(x, y) ((x)<(y)?(x):(y))
#endif

inline void ahiFactoryTensordummy() {};

#define RETURN_IF_TF_FAIL(expr, ret)  \
    if ((expr) != kTfLiteOk) {        \
      LOG_ERROR_HERE;                 \
      return ret;                     \
    }

#define RETURN_FALSE_IF_TF_FAIL(expr) RETURN_IF_TF_FAIL(expr, false)

#define RETURN_NULL_IF_TF_FAIL(expr) RETURN_IF_TF_FAIL(expr, nullptr)

typedef struct ahiTensorInput {
    cv::Mat _mat;
    bool _isImage;
    bool _isRboustPreprocess;
    cv::Size _target_size;
    std::vector<float> _scales;
    std::vector<float> _biases;

    ahiTensorInput(cv::Mat mat = cv::Mat(), bool isImage = false, bool isRboustPreprocess = false, cv::Size target_size = cv::Size(1, 1),
                   std::vector<float> scales = {1.}, std::vector<float> biases = {0.})
            : _mat(mat), _target_size(target_size), _scales(scales), _biases(biases), _isImage(isImage), _isRboustPreprocess(isRboustPreprocess) {};
} ahiTensorInput;

typedef struct ahiTensorInputMinimal {
    cv::Mat _mat = cv::Mat();
} ahiTensorInputMinimal;


typedef struct ahiTensorOutput {
    cv::Mat _mat = cv::Mat();
} ahiTensorOutput;

typedef std::unordered_map<std::string, ahiTensorInput> ahiTensorInputMap;
typedef std::unordered_map<std::string, ahiTensorOutput> ahiTensorOutputMap;

//typedef std::map<int, int> openCV_TfLiteTypes_EQ;

class ahiFactoryTensor {
public:
    static ahiFactoryTensor *mThis;

    static ahiFactoryTensor *getInstance();

    AssetManager *mAssetMgr;
    std::atomic_bool mQuit{false};
    uint8_t mKeydata[32];
    uint8_t mNoncedata[12];
    bool mLowendDevice;

    Mutex mMutex;

    bool saveModelBufferOnDevice(std::string modelFileName, const char *buffer, std::size_t buffer_size);

    std::string to_lowerStr(std::string str);

    std::string modelFileName;//or modelName, it depends on what how we handle., at the end we just need something to tell us which e.g. pose model we are inferencing
    ahiTensorInputMap mInputs;
    ahiTensorOutputMap mOutputs;

    std::string InferenceMethod;

    void match_CV_TF_types();

    cv::Mat
    preprocess_image(cv::Mat src, cv::Size target_size, int &top, int &bottom, int &left, int &right);

    void processImage(cv::Mat const &srcImage, cv::Mat const &srcContour, cv::Size const &targetSize,
                      cv::Mat &pImage,
                      cv::Mat &pContour, int &top, int &bottom, int &left, int &right,
                      bool &maskImageBg);

    cv::Mat processImageWorWoutPadding(cv::Mat const &srcImage, cv::Size const &targetSize,
                                       int &top, int &bottom, int &left, int &right, bool &toBGR, bool &doPadding, bool toF32);

    int get_perc_idx(float percentage, cv::Mat rows_columns);

    std::vector<int> get_roi_idx(cv::Mat cal_src, int psum, float top_padding_scale, float bottom_padding_scale);

    cv::Mat preprocess_image_robust(cv::Mat const inp_src, cv::Size target_size, bool robust_cropping, float top_padding_scale,
                                    float bottom_padding_scale);

    cv::Mat preprocess_image_gray(cv::Mat const inp_src, cv::Size target_size, int &top, int &bottom, int &left, int &right);

    cv::Mat
    preprocess_image_std_or_robust(cv::Mat src, cv::Size target_size, int &top, int &bottom, int &left, int &right, bool robust_cropping,
                                   float top_padding_scale, float bottom_padding_scale);

    cv::Mat preprocess_image_resize_crop_or_pad(cv::Mat const inp_src, cv::Size target_size, double scale,
                                                int offset);

    cv::Mat preprocess_image_for_exmeas(cv::Mat const inp_src, cv::Size target_size, int &top, int &bottom, int &left, int &right);

    std::vector<double> generateUniformMLSamples(int numSamples);

    bool prepareInputsForImageBasedExtraMeas(cv::Mat front_silhoutte, cv::Mat side_silhoutte, double height, double weight, std::string gender,
                                             cv::Mat &inputImageFeat,
                                             cv::Mat &inputHWGFeat,
                                             cv::Mat &inputNormalDist);

    std::vector<std::string> mInputNames;
    std::vector<std::string> mOutputNames;

    bool loadModel(const char *buffer, std::size_t buffer_size);

    bool buildInterpreter();

    std::unique_ptr<tflite::Interpreter> buildInterpreter(std::unique_ptr<tflite::FlatBufferModel> model);

    std::unique_ptr<tflite::Interpreter> buildOptimalInterpreter(std::unique_ptr<tflite::FlatBufferModel> model);

    bool buildOptimalInterpreter();

    void resetInterpreter();

    void setNumThreads(int num);

    void setUseCPU();

    void setUseNnApi();

    void setUseGpu();

    void setUseXNNPack();

    void setInput(std::size_t index, const void *data, std::size_t data_size);

    /**
     * Invoke the model
     * @return Inference time in milliseconds
     */
    int invoke();

    std::size_t input_bytes(std::size_t index) const;

    std::size_t output_bytes(std::size_t index) const;

    void copy_output(void *dst, std::size_t index) const;

    std::string GetTensorInfo(const TfLiteTensor *tensor);

    void PrintModelInfo(const tflite::Interpreter *interpreter);

    void GetModelInpOutNames();

    TfLiteDelegate *mDelegate;
    std::unique_ptr<tflite::FlatBufferModel> mModel;
    std::unique_ptr<tflite::Interpreter> mInterpreter;
    tflite::ops::builtin::BuiltinOpResolver mResolver;

    std::string mModelName;
    const char *mModelData;
    size_t mModelDataLen;
    bool mInitSuccess;

    using ModelMap = std::unordered_map<std::string, std::unique_ptr<tflite::FlatBufferModel>>;

    ahiTensorInput &getInput(std::string name);

    void addInput(std::string name, ahiTensorInput input);

    int getInputDim(int, int);

    std::vector<int> getOutputDim(int);

    bool invokeMIMO(ahiTensorInputMap &inputs, ahiTensorOutputMap &outputs);

#if 0

    void releaseDelegate();

    void getJointParts(ahiPoseInfo const &poseInfo, std::vector<cv::Point>&);
    void putJointParts(ahiPoseInfo &poseInfo, std::vector<cv::Point>&);
    void putJniParts(ahiPoseInfo &poseInfo, std::vector<float> &jnijoints);

    bool pose(cv::Mat& tensorMat, cv::Mat& contourMat, ahi_avatar_gen::view_t viewType, std::string &result, ahiPoseInfo& pojoJoints);
    bool joints(cv::Mat const &tensorMat, cv::Mat const &contourMat, ahi_avatar_gen::view_t viewType, std::vector<cv::Point>&, ahiPoseInfo &poseInfo, float& centerY);
    cv::Mat segment(cv::Mat const &tensorMat, cv::Mat contourMat, ahi_avatar_gen::view_t viewType, ahiPoseInfo &poseInfo, bool bUseJoints, int index);

    cv::Mat scaleContour(bool bUseJoints, bool bSmooth, bool& inZone, cv::Mat const &tensorMat, cv::Mat contourMat, std::string contourId,
                         ahi_avatar_gen::view_t viewType, std::string gender, float height, float weight,
                         ahiPoseInfo& poseInfo);


    bool est_joints_old(const cv::Mat orig_image, const cv::Mat contourMat, ahi_avatar_gen::view_t viewType, ahiPoseInfo &poseInfo, vector<cv::Point> &JointLocs, float& centerY);
    bool est_joints(const cv::Mat orig_image, const cv::Mat contourMat, ahi_avatar_gen::view_t viewType, ahiPoseInfo &poseInfo, vector<cv::Point> &JointLocs, float& centerY);

    bool predictFromHeatMap(cv::Mat const &oimage, cv::Mat const &ocontour, std::string const &view,
                            cv::Mat &binaryImageJoints, bool &isMask, std::vector<cv::Point>&, ahiPoseInfo&);

    bool predictFromHeatMapOpt(cv::Mat const &oimage, cv::Mat const &ocontour, std::string const &view,
                               bool &isMask, std::vector<cv::Point>&, ahiPoseInfo&);

    bool predictFromAHIPose(cv::Mat const &oimage, cv::Mat const &ocontour, std::string const &view,
                            bool &isMask, std::vector<cv::Point>&, ahiPoseInfo&);
    std::vector<bool> pose_inspect_usingsegnet(cv::Mat const &origImage, cv::Mat const &contour_mask, string const &view, vector<cv::Point> &approx_joints);

    ahiJsonPose performImageInspectionUsingSegnet(cv::Mat const &oimage, cv::Mat const &ocontour,
                                               std::string const &view, ahiPoseInfo &centroidJoints);

    bool predictFromJoints(cv::Mat const &oimage, cv::Mat const &ocontour, std::string const &view,
                           cv::Mat &binaryImageJoints, bool &isMask, ahiPoseInfo&);

    ahiJsonPose performImageInspection(cv::Mat const &oimage, cv::Mat const &ocontour, std::string const &view,
                                    bool isMask, std::vector<cv::Point>& pojo_joints, ahiPoseInfo &centroidJoints);

    cv::Mat performImageNetSegmentation(cv::Mat const &tensorMat);

    ahiJsonPose performPoseInspection(cv::Mat const &oimage, cv::Mat const &ocontour, std::string const &view,
                                   bool isMask, ahiPoseInfo &centroidJoints);

    cv::Mat performImageSegmentation(cv::Mat const &tensorMat, ahi_avatar_gen::view_t viewType,
                                     cv::Mat const &contourMat, vector<cv::Point> joints, int index);
#endif

    std::string pickModelFromList(std::vector<std::string> mlModelsList, std::string keyword);

    ahiFactoryTensor() = default;

private:

    enum BUILD_TYPE {
        kCPU,
        kGPU,
        kNNAPI,
        kXNNPack,
    };

    // 'delegate' must outlive the interpreter.

    // NNAPI
    std::unique_ptr<TfLiteDelegate> nnapi_delegate_;

    // GPU
    TfLiteGpuDelegateOptionsV2 gpu_options_ = TfLiteGpuDelegateOptionsV2Default();
    std::unique_ptr<TfLiteDelegate, decltype(&TfLiteGpuDelegateV2Delete)>
            gpu_delegate_{nullptr, &TfLiteGpuDelegateV2Delete};

    // XNNPack
    TfLiteXNNPackDelegateOptions xnn_options_ = TfLiteXNNPackDelegateOptionsDefault();
    std::unique_ptr<TfLiteDelegate, decltype(&TfLiteXNNPackDelegateDelete)>
            xnn_delegate_{nullptr, &TfLiteXNNPackDelegateDelete};

    int num_thread_ = 2;
    BUILD_TYPE build_type_ = kCPU;

    bool decode(unsigned char *, size_t);

    bool decodeSvr(unsigned char *, size_t);
};

#endif