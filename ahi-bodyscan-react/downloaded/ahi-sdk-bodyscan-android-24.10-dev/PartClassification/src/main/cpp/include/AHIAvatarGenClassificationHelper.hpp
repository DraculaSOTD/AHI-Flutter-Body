//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#ifndef AHIAvatarGenClassificationHelper_hpp
#define AHIAvatarGenClassificationHelper_hpp

#include "AHIAvatarGenMdlBase.hpp"
#include <stdio.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <string>
#include <math.h>
#include "Common.hpp"
#include <AHIBSCereal.hpp>

#define N_FEATURES_svr_image_features 126 //[H W ML's SVR's and number of image features]
#define KERNEL_TYPE 'p' //'l' //define the typr of kernel you are using
#define KERNEL_GAMMA 1.0//0 //define here if you are using non linear kernel
#define KERNEL_COEF 1.0//0.0 //define here if you are using non linear kernel
#define KERNEL_DEGREE 2.0//0

namespace ahi_avatar_gen {

    class classification_helper {
    private:
        std::vector<double> extract_image_features_v1(double height,
                                                      double weight,
                                                      const std::string &gender,
                                                      cv::Mat const &front_silhoutte,
                                                      cv::Mat const &side_silhoutte,
                                                      std::map<std::string, cv::Point2f> const &front_joints_vector,
                                                      std::map<std::string, cv::Point2f> const &side_joints_vector);

        std::vector<double>
        extract_image_features(double height,
                               double weight,
                               const std::string &gender,
                               cv::Mat const &front_silhoutte,
                               cv::Mat const &side_silhoutte,
                               std::map<std::string, cv::Point2f> const &front_joints_vector,
                               std::map<std::string, cv::Point2f> const &side_joints_vector);

        double svr_image_features_predictor_predict(std::map<std::string, AHIModelSVR> &svrModels, double features[], std::string param);

        double male_svr_image_features_predictor_predict(std::map<std::string, AHIModelSVR> &svrModels, double features[], std::string param);

        double
        female_svr_image_features_predictor_predict(std::map<std::string, AHIModelSVR> &svrModels, double features[], const std::string &param);

        double male_svr_image_features_predictor_v2_predict(std::map<std::string, AHIModelSVR> &svrModels, double features[], std::string param);

        double female_svr_image_features_predictor_v2_predict(std::map<std::string, AHIModelSVR> &svrModels, double features[], std::string param);

        double male_svr_image_features_predictor_v3_predict(std::map<std::string, AHIModelSVR> &svrModels, double features[], std::string param);

        double female_svr_image_features_predictor_v3_predict(std::map<std::string, AHIModelSVR> &svrModels, double features[], std::string param);

    public:
        classification_helper(void);

        std::vector<double> classify(double height,
                                     double weight,
                                     const std::string &gender,
                                     cv::Mat const &inp_front_silhoutte,
                                     cv::Mat const &inp_side_silhoutte,
                                     std::map<std::string, cv::Point2f> const &front_joints_vector,
                                     std::map<std::string, cv::Point2f> const &side_joints_vector,
                                     std::vector<double> &sil_features_for_DL,
                                     std::map<std::string, std::pair<char *, std::size_t>> &svrModels,
                                     std::vector<std::pair<std::string, std::vector<float>>> &svr_class_resultsRawPairs);

        double mean(std::vector<float> V);

        double mean_stddev(std::vector<float> V,bool useAverage);

        double mean_stddevPairs(std::vector<std::pair<std::string, std::vector<float>>> svr_class_resultsRawPairs, std::string measurementName,
                                std::string addKey,bool useAverage);
    };

}
#endif /* AHIAvatarGenClassificationHelper_hpp */
