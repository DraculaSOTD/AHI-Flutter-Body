//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#ifndef ahiFactoryTensor_H_
#define ahiFactoryTensor_H_

#include <opencv2/core/mat.hpp>
#include <tensorflow/lite/delegates/gpu/delegate.h>
#include <tensorflow/lite/delegates/xnnpack/xnnpack_delegate.h>
#include <tensorflow/lite/interpreter.h>
#include <tensorflow/lite/kernels/register.h>
#include <tensorflow/lite/model_builder.h>

typedef enum ahiModelGender {
    Male,
    Female,
    Either
} ahiModelGender;

#ifndef MIN
#define MIN(x, y) ((x)<(y)?(x):(y))
#endif

/////////////// Amar22
//"pose_light"
//"AHI_pose"
//"segnet"
////"seg_model_256"
////"seg_model_512"
//"jointnet"
//"classV1Model"
//"classV2Modelmale"
//"classV2Modelfemale"
//"classV2p5Model",
//"classV3Modelmale"
//"classV3Modelfemale"
//"classV3p1Model"
//"classTBFIM1Model"
//"classTBFIM2Model"
////"classTBFNG2" //needs cropped images
//"ExmeasImageBased"

#define RETURN_IF_TF_FAIL(expr, ret)  \
    if ((expr) != kTfLiteOk) {        \
      LOG_ERROR_HERE;                 \
      return ret;                     \
    }

#define RETURN_FALSE_IF_TF_FAIL(expr) RETURN_IF_TF_FAIL(expr, false)

typedef std::map<std::string, ahiModelGender> ahiModelNameGender_map;
typedef struct ahiTensorInput {
    cv::Mat _mat;
    bool _isImage;
    bool _isRboustPreprocess;
    cv::Size _target_size;
    std::vector<float> _scales;
    std::vector<float> _biases;

    ahiTensorInput(cv::Mat mat = cv::Mat(), bool isImage = false, bool isRboustPreprocess = false,
                   cv::Size target_size = cv::Size(1, 1),
                   std::vector<float> scales = {1.}, std::vector<float> biases = {0.})
            : _mat(mat), _target_size(target_size), _scales(scales), _biases(biases),
              _isImage(isImage), _isRboustPreprocess(isRboustPreprocess) {};
} ahiTensorInput;

typedef struct ahiTensorOutput {
    cv::Mat _mat = cv::Mat();
} ahiTensorOutput;

typedef std::unordered_map<std::string, ahiTensorInput> ahiTensorInputMap;
typedef std::unordered_map<std::string, ahiTensorOutput> ahiTensorOutputMap;

typedef enum FactoryTensorModelType {
    ModelTypeUnknown,
    ModelTypeTF,
    ModelTypeStub
} FactoryTensorModelType;

class ahiFactoryTensor {
public:
    std::atomic_bool mQuit{false};
    uint8_t mKeydata[32];
    uint8_t mNoncedata[12];
    bool mLowendDevice;

    FactoryTensorModelType checkModel(const uint8_t *bytes);

    ahiModelNameGender_map ahiModelGenderMap = {
            {"faceDetectionFront",  ahiModelGender::Either},
            {"faceDetectionBack",   ahiModelGender::Either},
            {"ahiPoseLight",        ahiModelGender::Either},
            {"ahiMoveNetPoseModel", ahiModelGender::Either},
            {"segnet",              ahiModelGender::Either},
            //{"seg_model_256",ahiModelGender::Either},
            //{"seg_model_512",ahiModelGender::Either},
            //{"jointnet",ahiModelGender::Either},
    };

    ahiModelNameGender_map ahiShapeModelGenderMap = {
            {"classV1Model",       ahiModelGender::Either},
            {"classV2Modelmale",   ahiModelGender::Male},
            {"classV2Modelfemale", ahiModelGender::Female},
            {"classV2p5Model",     ahiModelGender::Either},
            {"classV3Modelmale",   ahiModelGender::Male},
            {"classV3Modelfemale", ahiModelGender::Female},
            {"classV3p1Model",     ahiModelGender::Either},
            {"ExmeasImageBased",   ahiModelGender::Either},
    };

    ahiModelNameGender_map ahiCompositionModelGenderMap = {

            {"classTBFIM1Model", ahiModelGender::Either},
            {"classTBFIM2Model", ahiModelGender::Either},
            //{"classTBFNG2",ahiModelGender::Either}, // needs cropped images (extra new staff compared to our current Prod.)
    };

    bool
    saveModelBufferOnDevice(std::string modelFileName, const char *buffer, std::size_t buffer_size);

    std::string to_lowerStr(std::string str);

    std::string modelFileName;// or modelName, it depends on what how we handle., at the end we just need something to tell us which e.g. pose model we are inferencing
    ahiTensorInputMap mInputs;
    ahiTensorOutputMap mOutputs;

    void match_CV_TF_types();
    bool isPaddedForResize;

    cv::Mat preprocess_image(cv::Mat src, cv::Size target_size, int &top, int &bottom, int &left,
                             int &right);

    void
    processImage(cv::Mat const &srcImage, cv::Mat const &srcContour, cv::Size const &targetSize,
                 cv::Mat &pImage,
                 cv::Mat &pContour, int &top, int &bottom, int &left, int &right,
                 bool &maskImageBg);

    cv::Mat processImageWorWoutPadding(cv::Mat const &srcImage, cv::Size const &targetSize,
                                       int &top, int &bottom, int &left, int &right, bool &toBGR,
                                       bool &doPadding, bool toF32);

    int get_perc_idx(float percentage, cv::Mat rows_columns);

    std::vector<int>
    get_roi_idx(cv::Mat cal_src, int psum, float top_padding_scale, float bottom_padding_scale);

    cv::Mat
    preprocess_image_robust(cv::Mat const inp_src, cv::Size target_size, bool robust_cropping,
                            float top_padding_scale,
                            float bottom_padding_scale);

    cv::Mat
    preprocess_image_resize_crop_or_pad(cv::Mat const inp_src, cv::Size target_size, double scale,
                                        int offset);

    cv::Mat
    preprocess_image_for_exmeas(cv::Mat const inp_src, cv::Size target_size, int &top, int &bottom,
                                int &left, int &right);

    std::vector<double> generateUniformMLSamples(int numSamples);

    bool prepareInputsForImageBasedExtraMeas(cv::Mat front_silhouette, cv::Mat side_silhouette,
                                             double height, double weight, std::string gender,
                                             cv::Mat &inputImageFeat,
                                             cv::Mat &inputHWGFeat,
                                             cv::Mat &inputNormalDist);

    std::vector<std::string> mInputNames;
    std::vector<std::string> mOutputNames;

    bool loadModel(const char *buffer, std::size_t buffer_size);

    bool buildInterpreter();

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

    // below are refactored FactoryTensor and TensorMap
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

    bool invokeMIMO(ahiTensorInputMap &inputs, ahiTensorOutputMap &outputs);

    std::vector<std::string> getModelFilesList(std::string gender, std::string measCatagory);

    std::string pickModelFromList(std::vector<std::string> mlModelsList, std::string keyword);

    void rcv(char);

    static void
    getSvrModelList(std::vector<std::string> &result, std::string measCatagory, bool isEncoded);

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

    int num_thread_ = 2; // default
    BUILD_TYPE build_type_ = kCPU;
};

#endif