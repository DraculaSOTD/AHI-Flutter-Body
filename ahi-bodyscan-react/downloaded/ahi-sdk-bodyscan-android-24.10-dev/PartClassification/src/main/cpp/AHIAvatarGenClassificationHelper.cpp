//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#include <map>
#include "AHIAvatarGenClassificationHelper.hpp"
#include "AHILogging.hpp"

namespace ahi_avatar_gen {

    classification_helper::classification_helper(void) {
        // nothing to construct
    }

    std::vector<double> classification_helper::extract_image_features_v1(
            double height,
            double weight,
            const std::string &gender,
            cv::Mat const &front_silhoutte,
            cv::Mat const &side_silhoutte,
            std::map<std::string, cv::Point2f> const &front_joints_vector,
            std::map<std::string, cv::Point2f> const &side_joints_vector
    ) {
        std::vector<double> empty_vect(0, 0);
        try {
            std::vector<double> features;
            features.push_back(height);
            features.push_back(weight);
            //This is for the latest way we write joints

            std::string view = "Side";
            if (view == "Side") {

                if (side_joints_vector.size() < 1)
                    return empty_vect;

                cv::Point Head = side_joints_vector.at("CentroidHeadTop");

                cv::Point Neck = side_joints_vector.at("CentroidNeck");

                cv::Point RightShoulder = side_joints_vector.at("CentroidRightShoulder");

                cv::Point RightElbow = side_joints_vector.at("CentroidRightElbow");

                cv::Point RightWrist = side_joints_vector.at("CentroidRightHand");

                cv::Point LeftShoulder = side_joints_vector.at("CentroidLeftShoulder");

                cv::Point LeftElbow = side_joints_vector.at("CentroidLeftElbow");

                cv::Point LeftWrist = side_joints_vector.at("CentroidLeftHand");

                cv::Point RightHip = side_joints_vector.at("CentroidRightHip");

                cv::Point RightKnee = side_joints_vector.at("CentroidRightKnee");

                cv::Point RightAnkle = side_joints_vector.at("CentroidRightAnkle");

                cv::Point LeftHip = side_joints_vector.at("CentroidLeftHip");

                cv::Point LeftKnee = side_joints_vector.at("CentroidLeftKnee");

                cv::Point LeftAnkle = side_joints_vector.at("CentroidLeftAnkle");

                // side image features
                cv::Rect cropping_rect;
                cropping_rect.x = 1;
                cropping_rect.width = side_silhoutte.cols - 1;
                cropping_rect.y = Neck.y;
                int endp = std::min(side_silhoutte.rows - 1.0, 0.8 * (0.5 * (RightAnkle.y + LeftAnkle.y)) + 0.2 * (0.5 * (RightKnee.y + LeftKnee.y)));
                cropping_rect.height = endp - Neck.y;
                cv::Mat cropped_image = side_silhoutte(cropping_rect);
                int Ncols = 1000 * cropped_image.cols / (1.0e-8 + cropped_image.rows);
                resize(cropped_image, cropped_image, cv::Size(Ncols, 1000));

                //imshow("cropped_image", cropped_image);
                //waitKey(0);
                double Sum_Total = sum(cropped_image).val[0];

                int N_levels = 20;
                int level_depth = cropped_image.rows / N_levels;

                for (int n = 0; n < N_levels; n++) {
                    cv::Rect rect;
                    rect.x = 1;
                    rect.width = cropped_image.cols - 1;
                    rect.y = n * level_depth;
                    rect.height = level_depth;
                    cv::Mat slice = cropped_image(rect);
                    //imshow("slice", slice);
                    //waitKey(0);
                    features.push_back(sum(slice).val[0] / (Sum_Total + 1.0e-6));
                }

            }

            // front image features
            view = "Front";
            if (view == "Front") {
                if (front_joints_vector.size() < 1)
                    return empty_vect;

                cv::Point Head = front_joints_vector.at("CentroidHeadTop");

                cv::Point Neck = front_joints_vector.at("CentroidNeck");

                cv::Point RightShoulder = front_joints_vector.at("CentroidRightShoulder");

                cv::Point RightElbow = front_joints_vector.at("CentroidRightElbow");

                cv::Point RightWrist = front_joints_vector.at("CentroidRightHand");

                cv::Point LeftShoulder = front_joints_vector.at("CentroidLeftShoulder");

                cv::Point LeftElbow = front_joints_vector.at("CentroidLeftElbow");

                cv::Point LeftWrist = front_joints_vector.at("CentroidLeftHand");

                cv::Point RightHip = front_joints_vector.at("CentroidRightHip");

                cv::Point RightKnee = front_joints_vector.at("CentroidRightKnee");

                cv::Point RightAnkle = front_joints_vector.at("CentroidRightAnkle");

                cv::Point LeftHip = front_joints_vector.at("CentroidLeftHip");

                cv::Point LeftKnee = front_joints_vector.at("CentroidLeftKnee");

                cv::Point LeftAnkle = front_joints_vector.at("CentroidLeftAnkle");

                //cv::Rect rect_hand;
                //rect_hand.x = 0;
                //rect_hand.width = RightWrist.x;
                //rect_hand.height = front_silhoutte.rows;
                //front_silhoutte(rect_hand) = 0;

                //resize(front_silhoutte,front_silhoutte,cv::Size(),0.5,0.5);


                cv::Mat hand_feet_mask = 255 * (cv::Mat::ones(front_silhoutte.size(), front_silhoutte.type()));

                circle(hand_feet_mask, cv::Point(RightWrist.x / 2, RightWrist.y + RightWrist.x / 2), 0.75 * RightWrist.x, 0, -1);
                circle(hand_feet_mask, cv::Point(LeftWrist.x + RightWrist.x / 2, LeftWrist.y + RightWrist.x / 2), 0.75 * RightWrist.x, 0, -1);

                circle(hand_feet_mask, cv::Point(RightAnkle.x - RightWrist.x / 3, RightAnkle.y + RightWrist.x / 2), 0.75 * RightWrist.x, 0, -1);
                circle(hand_feet_mask, cv::Point(LeftAnkle.x + RightWrist.x / 3, LeftAnkle.y + RightWrist.x / 2), 0.75 * RightWrist.x, 0, -1);
                //imshow("hand_feet_mask", hand_feet_mask);
                //waitKey(0);
                cv::bitwise_and(front_silhoutte, hand_feet_mask, front_silhoutte);
                //imwrite("front_hand_feet_mask.png", front_silhoutte);
                cv::Rect cropping_rect;
                cropping_rect.x = 1;
                cropping_rect.width = side_silhoutte.cols - 1;
                cropping_rect.y = Neck.y;
                int height_Neck_to_Ankles = std::min(front_silhoutte.rows - 1.0, 0.5 * (std::sqrt(
                        (double) (Neck.x - RightAnkle.x) * (Neck.x - RightAnkle.x) + (double) (Neck.y - RightAnkle.y) * (Neck.y - RightAnkle.y)) +
                                                                                        std::sqrt(
                                                                                                (double) (Neck.x - LeftAnkle.x) *
                                                                                                (Neck.x - LeftAnkle.x) +
                                                                                                (double) (Neck.y - LeftAnkle.y) *
                                                                                                (Neck.y - LeftAnkle.y))));

                //cropping_rect.height = height_Neck_to_Ankles;
                cropping_rect.height = std::min(front_silhoutte.rows - cropping_rect.y - 1, height_Neck_to_Ankles);

                cv::Mat cropped_image = front_silhoutte(cropping_rect);
                int Ncols = 1000 * cropped_image.cols / (1.0e-8 + cropped_image.rows);
                resize(cropped_image, cropped_image, cv::Size(Ncols, 1000));

                //imshow("cropped_image", cropped_image);
                //waitKey(0);

                double Sum_Total = sum(cropped_image).val[0];
                int N_levels = 10;
                int level_depth = cropped_image.rows / N_levels;
                int level_radius = cropped_image.cols / 20;

                for (int n = 0; n < N_levels; n++) {
                    /*cv::Rect rect;
                     rect.x = 1;
                     rect.width = cropped_image.cols-1;
                     rect.y = n*level_depth;
                     rect.height = level_depth;
                     cv::Mat slice = cropped_image(rect);
                     imshow("slice", slice);
                     waitKey(0);
                     features.push_back(sum(slice).val[0]/(Sum_Total + 1.0e-6));*/

                    cv::Point slice_center;

                    slice_center.x = cropped_image.cols / 2;
                    slice_center.y = n * level_depth + 0.5 * level_depth;

                    for (int m = 1; m <= 10; m++) {
                        int d = m * level_radius;
                        cv::Mat mask = cv::Mat::zeros(cropped_image.size(), cropped_image.type());
                        circle(mask, slice_center, d, 255, -1);
                        cv::Mat slice;
                        cv::bitwise_and(cropped_image, mask, slice);
                        //imshow("slice", slice);
                        //waitKey(0);
                        features.push_back(sum(slice).val[0] / (Sum_Total + 1.0e-6));
                    }

                }


                //arms angles as features
                double RAa = double(RightElbow.x - RightShoulder.x) / double(RightElbow.y - RightShoulder.y);
                features.push_back(RAa);
                double LAa = double(LeftElbow.x - LeftShoulder.x) / double(LeftElbow.y - LeftShoulder.y);
                features.push_back(LAa);
                //leg angles as featutres
                double RLa = double(RightKnee.x - Neck.x) / double(RightKnee.y - Neck.y);
                features.push_back(RLa);
                double LLa = double(LeftKnee.x - Neck.x) / double(LeftKnee.y - Neck.y);
                features.push_back(LLa);
            }

            //for(int f=0; f < features.size(); f++)
            //    cout << features[f] << " ";
            return features;


        }
        catch (...) {
            return empty_vect;
        }
    }

    std::vector<double> classification_helper::extract_image_features(double height,
                                                                      double weight,
                                                                      const std::string &gender,
                                                                      cv::Mat const &front_silhoutte,
                                                                      cv::Mat const &side_silhoutte,
                                                                      std::map<std::string, cv::Point2f> const &front_joints_vector,
                                                                      std::map<std::string, cv::Point2f> const &side_joints_vector) {
        std::vector<double> empty_vect(0, 0);
        try {
            std::vector<double> features;
            features.push_back(height);
            features.push_back(weight);

            std::string view = "Side";
            if (view == "Side") {

                if (side_joints_vector.empty())
                    return empty_vect;

                cv::Point Head = side_joints_vector.at("CentroidHeadTop");

                cv::Point Neck = side_joints_vector.at("CentroidNeck");

                cv::Point RightShoulder = side_joints_vector.at("CentroidRightShoulder");

                cv::Point RightElbow = side_joints_vector.at("CentroidRightElbow");

                cv::Point RightWrist = side_joints_vector.at("CentroidRightHand");

                cv::Point LeftShoulder = side_joints_vector.at("CentroidLeftShoulder");

                cv::Point LeftElbow = side_joints_vector.at("CentroidLeftElbow");

                cv::Point LeftWrist = side_joints_vector.at("CentroidLeftHand");

                cv::Point RightHip = side_joints_vector.at("CentroidRightHip");

                cv::Point RightKnee = side_joints_vector.at("CentroidRightKnee");

                cv::Point RightAnkle = side_joints_vector.at("CentroidRightAnkle");

                cv::Point LeftHip = side_joints_vector.at("CentroidLeftHip");

                cv::Point LeftKnee = side_joints_vector.at("CentroidLeftKnee");

                cv::Point LeftAnkle = side_joints_vector.at("CentroidLeftAnkle");

                // side image features
                cv::Rect cropping_rect;
                cropping_rect.x = 1;
                cropping_rect.width = side_silhoutte.cols - 1;
#if 1
                cropping_rect.y = Neck.y;
                int endp = std::min(side_silhoutte.rows - 1.0, 0.9 * (0.5 * (RightAnkle.y + LeftAnkle.y)) + 0.1 * (0.5 * (RightKnee.y + LeftKnee.y)));
                cropping_rect.height = endp - Neck.y;
#else

                cropping_rect.y = Head.y;
                int endp = (int)std::min(side_silhoutte.rows - 1.0, (double)std::max(RightAnkle.y, LeftAnkle.y));
                cropping_rect.height = endp - Head.y;
#endif

                cv::Mat cropped_image = side_silhoutte(cropping_rect);
                int Ncols = 1000 * cropped_image.cols / (1.0e-8 + cropped_image.rows);
                resize(cropped_image, cropped_image, cv::Size(Ncols, 1000));


                //imwrite("cropped_resized_side_silhoutte.png",cropped_image);
                //imshow("cropped_image", cropped_image);
                //waitKey(0);
                double Sum_Total = sum(cropped_image).val[0];

                int N_levels = 20;// 30
                int level_depth = cropped_image.rows / N_levels;

                for (int n = 0; n < N_levels; n++) {
                    cv::Rect rect;
                    rect.x = 1;
                    rect.width = cropped_image.cols - 1;
                    rect.y = n * level_depth;
                    rect.height = level_depth;
                    cv::Mat slice = cropped_image(rect);
                    //imshow("slice", slice);
                    //waitKey(0);
                    features.push_back(sum(slice).val[0] / (Sum_Total + 1.0e-6));
                    //features.push_back(sum(slice).val[0]);
                    //features.push_back(countNonZero(slice));
                }

            }

            // front image features ////////////////////

            view = "Front";
            if (view == "Front") {
                if (front_joints_vector.size() < 1)
                    return empty_vect;

                cv::Point Head = front_joints_vector.at("CentroidHeadTop");

                cv::Point Neck = front_joints_vector.at("CentroidNeck");

                cv::Point RightShoulder = front_joints_vector.at("CentroidRightShoulder");

                cv::Point RightElbow = front_joints_vector.at("CentroidRightElbow");

                cv::Point RightWrist = front_joints_vector.at("CentroidRightHand");

                cv::Point LeftShoulder = front_joints_vector.at("CentroidLeftShoulder");

                cv::Point LeftElbow = front_joints_vector.at("CentroidLeftElbow");

                cv::Point LeftWrist = front_joints_vector.at("CentroidLeftHand");

                cv::Point RightHip = front_joints_vector.at("CentroidRightHip");

                cv::Point RightKnee = front_joints_vector.at("CentroidRightKnee");

                cv::Point RightAnkle = front_joints_vector.at("CentroidRightAnkle");

                cv::Point LeftHip = front_joints_vector.at("CentroidLeftHip");

                cv::Point LeftKnee = front_joints_vector.at("CentroidLeftKnee");

                cv::Point LeftAnkle = front_joints_vector.at("CentroidLeftAnkle");

                //cv::Rect rect_hand;
                //rect_hand.x = 0;
                //rect_hand.width = RightWrist.x;
                //rect_hand.height = front_silhoutte.rows;
                //front_silhoutte(rect_hand) = 0;

                //resize(front_silhoutte,front_silhoutte,cv::Size(),0.5,0.5);

#if 1
                cv::Mat hand_feet_mask = 255 * (cv::Mat::ones(front_silhoutte.size(), front_silhoutte.type()));

                circle(hand_feet_mask, cv::Point(RightWrist.x / 2, RightWrist.y + RightWrist.x / 2), 0.75 * RightWrist.x, 0, -1);
                circle(hand_feet_mask, cv::Point(LeftWrist.x + RightWrist.x / 2, LeftWrist.y + RightWrist.x / 2), 0.75 * RightWrist.x, 0, -1);

                circle(hand_feet_mask, cv::Point(RightAnkle.x - RightWrist.x / 3, RightAnkle.y + RightWrist.x / 2), 0.75 * RightWrist.x, 0, -1);
                circle(hand_feet_mask, cv::Point(LeftAnkle.x + RightWrist.x / 3, LeftAnkle.y + RightWrist.x / 2), 0.75 * RightWrist.x, 0, -1);
                //imshow("hand_feet_mask", hand_feet_mask);
                //waitKey(0);
                cv::bitwise_and(front_silhoutte, hand_feet_mask, front_silhoutte);
                //imwrite("front_hand_feet_mask.png", front_silhoutte);

                cv::Rect cropping_rect;
                cropping_rect.x = 1;
                cropping_rect.width = side_silhoutte.cols - 1;

                cropping_rect.y = Neck.y;
                int height_Neck_to_Ankles = std::min(front_silhoutte.rows - 1.0, 0.5 * (std::sqrt(
                        (double) (Neck.x - RightAnkle.x) * (Neck.x - RightAnkle.x) + (double) (Neck.y - RightAnkle.y) * (Neck.y - RightAnkle.y)) +
                                                                                        std::sqrt(
                                                                                                (double) (Neck.x - LeftAnkle.x) *
                                                                                                (Neck.x - LeftAnkle.x) +
                                                                                                (double) (Neck.y - LeftAnkle.y) *
                                                                                                (Neck.y - LeftAnkle.y))));

                cropping_rect.height = std::min(front_silhoutte.rows - cropping_rect.y - 1, height_Neck_to_Ankles);

#else

                cv::Rect cropping_rect;
                cropping_rect.x = 1;
                cropping_rect.width = side_silhoutte.cols-1;

                cropping_rect.y = Head.y;
                int endp = (int)std::min(front_silhoutte.rows - 1.0, (double)std::max(RightAnkle.y, LeftAnkle.y));
                cropping_rect.height = endp - Head.y;
#endif


                cv::Mat cropped_image = front_silhoutte(cropping_rect);
                int Ncols = 1000 * cropped_image.cols / (1.0e-8 + cropped_image.rows);
                resize(cropped_image, cropped_image, cv::Size(Ncols, 1000));

                //imwrite("cropped_resized_front_silhoutte.png",cropped_image);
                //imshow("cropped_image", cropped_image);
                //waitKey(0);

                // this is needed to re-ref the joints x position in the new cropped-resized image
                double a = (cropped_image.cols - 1.) / (front_silhoutte.cols - 1.0);
                double b = 1.0 - a;

                double Sum_Total = sum(cropped_image).val[0];
                int N_levels = 10;//20;
                int level_depth = cropped_image.rows / N_levels;
                int level_radius = cropped_image.cols / 20;

                for (int n = 0; n < N_levels; n++) {

                    cv::Point slice_center;
#if 0
                    slice_center.x = cropped_image.cols/2; // image center
#else
                    slice_center.x = 0.5 * (Neck.x + 0.5 * (RightShoulder.x + LeftShoulder.x)); // body center
                    slice_center.x = a * slice_center.x + b;
#endif
                    slice_center.y = n * level_depth + 0.5 * level_depth;

                    for (int m = 1; m <= 10; m++) {
                        int d = m * level_radius;
                        cv::Mat mask = cv::Mat::zeros(cropped_image.size(), cropped_image.type());
                        circle(mask, slice_center, d, 255, -1);
                        cv::Mat slice;
                        cv::bitwise_and(cropped_image, mask, slice);
                        //imshow("slice", slice);
                        // waitKey(0);
                        features.push_back(sum(slice).val[0] / (Sum_Total + 1.0e-6));
                        //features.push_back(sum(slice).val[0]);

                        //features.push_back(countNonZero(slice));
                    }

                }

                //arms angles as features
                double RAa = double(RightElbow.x - RightShoulder.x) / double(RightElbow.y - RightShoulder.y);
                features.push_back(RAa);
                double LAa = double(LeftElbow.x - LeftShoulder.x) / double(LeftElbow.y - LeftShoulder.y);
                features.push_back(LAa);
                //leg angles as featutres
                double RLa = double(RightKnee.x - Neck.x) / double(RightKnee.y - Neck.y);
                features.push_back(RLa);
                double LLa = double(LeftKnee.x - Neck.x) / double(LeftKnee.y - Neck.y);
                features.push_back(LLa);
            }

            return features;

        }
        catch (...) {
            //error_id ="11";
            return empty_vect;
        }
    }

////
    double classification_helper::mean(std::vector<float> V) {
        int L = (int) V.size();
        if (L < 1) {
            return 0;
        }

        float Mean = 0.;
        size_t N = 0;
        for (int n = 0; n < L; n++) {
            if (isinf(V[n]) || isnan(V[n]) || V[n] <= 0) {
                continue;
            }
            Mean = Mean + V[n];
            N = N + 1;
        }
        return Mean / float(N);
    }

//
    double classification_helper::mean_stddev(std::vector<float> V, bool useAverage) {
        // total size of V
        size_t L = V.size();
        if (L < 1) {
            return 0;
        }
        float Mean = 0.;
        float Variance = 0.;
        size_t N = 0;
        float result = 0;
        if (useAverage) {
            // use mean
            for (int n = 0; n < L; n++) {
                bool bValid = true;
                float val = V[n];

                if (isnan(val)) {
                    bValid = false;
                }

                if (isinf(val)) {
                    bValid = false;
                }

                if (val <= 0) {
                    bValid = false;
                }

                if (bValid) {
                    Mean = Mean + val;
                    N++;
                }
            }

            if (N <= 1) {
                return Mean;
            }
            // Mean average result
            result = Mean / float(N);
            if (N <= 2) {
                return result;
            }
        } else {
            // use median
            // step 1. sort by increase order
            std::sort(V.begin(), V.end());
            // step 2. check is size is odd or even
            if (V.size() % 2 == 0) {
                // step 3. V's size is even, pick 2 of middle value and calculate average.
                float middleValue1 = V[V.size() / 2 - 1];
                float middleValue2 = V[V.size() / 2];
                result = (middleValue1 + middleValue2) / 2;
            } else {
                // step 3. V's size is odd, pick middle value;
                result = V[V.size() / 2];
            }
            if (L <= 1) {
                return result;
            }
            if (L <= 2) {
                return result;
            }
        }

        for (int n = 0; n < L; n++) {
            bool bValid = true;
            float val = V[n];

            if (isnan(val)) {
                bValid = false;
            }

            if (isinf(val)) {
                bValid = false;
            }

            if (val <= 0) {
                bValid = false;
            }

            if (bValid) {
                Variance += (val - result) * (val - result);
            }
        }
        float stddev = sqrt(Variance / float(L));

        Mean = 0.;
        for (int n = 0; n < L; n++) {
            bool bValid = true;
            float val = V[n];

            if (isnan(val)) {
                bValid = false;
            }

            if (isinf(val)) {
                bValid = false;
            }

            if (val <= 0) {
                bValid = false;
            }

            if (bValid) {
                if (abs(result - val) < 1.5 * stddev) {
                    Mean = Mean + val;
                } else {
                    N--;
                }
            }
        }
        if (isnan(Mean) || isinf(Mean)) {
            return result;
        }

        return Mean / float(L);
    }

//
    double classification_helper::mean_stddevPairs(std::vector<std::pair<std::string, std::vector<float>>> svr_class_resultsRawPairs,
                                                   std::string measurementName, std::string addKey, bool useAverage) {
        std::vector<float> Meas;
        for (int n = 0; n < (int) svr_class_resultsRawPairs.size(); n++) {
            //std::vector<std::pair<std::string, std::vector<double>>>
            std::string currMeasurementName = svr_class_resultsRawPairs[n].first;
            if (currMeasurementName.find(measurementName) != std::string::npos && currMeasurementName.find(addKey) != std::string::npos) {
                std::vector<float> measure = svr_class_resultsRawPairs[n].second;
                if (Meas.empty()) {
                    Meas = measure;
                } else {
                    Meas.insert(
                            Meas.end(),
                            std::make_move_iterator(measure.begin()),
                            std::make_move_iterator(measure.end()));

                }
            }
        }
        bool a = useAverage;
        double Mean = classification_helper::mean_stddev(Meas, useAverage);
        return Mean;
    }

//
///#################################################################################
    double
    classification_helper::svr_image_features_predictor_predict(std::map<std::string, AHIModelSVR> &svrModels, double features[], std::string param) {
        // Get ref to SVR
        AHIModelSVR svr;
        if (param == "chest") {
            svr = svrModels.at("chest_svr_image_features");
        } else if (param == "waist") {
            svr = svrModels.at("waist_svr_image_features");
        } else if (param == "hip") {
            svr = svrModels.at("hip_svr_image_features");
        } else if (param == "inseam") {
            svr = svrModels.at("inseam_svr_image_features");
        } else if (param == "thigh_UWA_all") {
            svr = svrModels.at("thigh_svr_image_features_UWA_all");
        } else if (param == "weight") {
            svr = svrModels.at("weight_svr_image_features");
        } else if (param == "weight_UWA_all") {
            svr = svrModels.at("weight_svr_image_features_UWA_all");
        } else if (param == "fat") {
            svr = svrModels.at("fat_svr_image_features");
        } else if (param == "fat_UWA_all") {
            svr = svrModels.at("fat_svr_image_features_UWA_all");
        } else if (param == "gynoid_UWA_all") {
            svr = svrModels.at("gynoid_svr_image_features_UWA_all");
        } else if (param == "andriod_UWA_all") {
            svr = svrModels.at("andriod_svr_image_features_UWA_all");
        } else if (param == "visceral_UWA_all") {
            svr = svrModels.at("visceral_svr_image_features_UWA_all");
        } else if (param == "ffm_UWA_all") {
            svr = svrModels.at("FFM_svr_image_features_UWA_all");
        }

        size_t num_vectors = svr.vectors.size();

        //compute the mapped vectors wrt support vectors
        double kernels[num_vectors];
        double kernel;
        switch (KERNEL_TYPE) {
            case 'l':
                // <x,x'>
                for (int i = 0; i < num_vectors; i++) {
                    kernel = 0.;
                    for (int j = 0; j < N_FEATURES_svr_image_features; j++) {
                        kernel += svr.vectors[i][j] * features[j];
                    }
                    kernels[i] = kernel;
                }
                break;
            case 'p':
                // (y<x,x'>+r)^d
                for (int i = 0; i < num_vectors; i++) {
                    kernel = 0.;
                    for (int j = 0; j < N_FEATURES_svr_image_features; j++) {
                        kernel += svr.vectors[i][j] * features[j];
                    }
                    kernels[i] = pow((KERNEL_GAMMA * kernel) + KERNEL_COEF, KERNEL_DEGREE);// (1.0 + kernel) * (1.0 + kernel); //
                }
                break;
            case 'r':
                // exp(-y|x-x'|^2)
                for (int i = 0; i < num_vectors; i++) {
                    kernel = 0.;
                    for (int j = 0; j < N_FEATURES_svr_image_features; j++) {
                        kernel += pow(svr.vectors[i][j] - features[j], 2);
                    }
                    kernels[i] = exp(-KERNEL_GAMMA * kernel);
                }
                break;
            case 's':
                // tanh(y<x,x'>+r)
                for (int i = 0; i < num_vectors; i++) {
                    kernel = 0.;
                    for (int j = 0; j < N_FEATURES_svr_image_features; j++) {
                        kernel += svr.vectors[i][j] * features[j];
                    }
                    kernels[i] = tanh((KERNEL_GAMMA * kernel) + KERNEL_COEF);
                }
                break;
        }

        //compute the product of kernel and coeff
        double predicted = 0.0;
        for (int i = 0; i < num_vectors; i++) {
            predicted = predicted + kernels[i] * svr.coefficients[i];
            if (i == num_vectors - 1) {
                predicted = predicted + svr.intercepts[0];
            }
        }

        return predicted;
    }


///
    double classification_helper::male_svr_image_features_predictor_predict(std::map<std::string, AHIModelSVR> &svrModels,
                                                                            double features[],
                                                                            std::string param) {
        if (features == NULL) {
            AHILog(ANDROID_LOG_ERROR, "FEATURES for %s is null", param.c_str());
        }

        // Get ref to SVR
        AHIModelSVR svr;
        if (param == "chest") {
            svr = svrModels.at("male_chest_svr_image_features");
        } else if (param == "waist") {
            svr = svrModels.at("male_waist_svr_image_features");
        } else if (param == "hip") {
            svr = svrModels.at("male_hip_svr_image_features");
        } else if (param == "inseam") {
            svr = svrModels.at("male_inseam_svr_image_features");
        }

        // Get boundaries
        size_t num_vectors = svr.vectors.size();

        //compute the mapped vectors wrt support vectors
        double kernels[num_vectors];
        double kernel;
        switch (KERNEL_TYPE) {
            case 'l':
                // <x,x'>
                for (int i = 0; i < num_vectors; i++) {
                    kernel = 0.;
                    for (int j = 0; j < N_FEATURES_svr_image_features; j++) {
                        kernel += svr.vectors[i][j] * features[j];
                    }
                    kernels[i] = kernel;
                }
                break;
            case 'p':
                // (y<x,x'>+r)^d
                for (int i = 0; i < num_vectors; i++) {
                    kernel = 0.;
                    for (int j = 0; j < N_FEATURES_svr_image_features; j++) {
                        double featureJ = 0;
                        if (features != NULL) {
                            featureJ = features[j];
                        }
                        kernel += svr.vectors[i][j] * featureJ;
                    }
                    kernels[i] = pow((KERNEL_GAMMA * kernel) + KERNEL_COEF, KERNEL_DEGREE); //(1.0 + kernel) * (1.0 + kernel);//
                }
                break;
            case 'r':
                // exp(-y|x-x'|^2)
                for (int i = 0; i < num_vectors; i++) {
                    kernel = 0.;
                    for (int j = 0; j < N_FEATURES_svr_image_features; j++) {
                        kernel += pow(svr.vectors[i][j] - features[j], 2);
                    }
                    kernels[i] = exp(-KERNEL_GAMMA * kernel);
                }
                break;
            case 's':
                // tanh(y<x,x'>+r)
                for (int i = 0; i < num_vectors; i++) {
                    kernel = 0.;
                    for (int j = 0; j < N_FEATURES_svr_image_features; j++) {
                        kernel += svr.vectors[i][j] * features[j];
                    }
                    kernels[i] = tanh((KERNEL_GAMMA * kernel) + KERNEL_COEF);
                }
                break;
        }

        //compute the product of kernel and coeff
        double predicted = 0.0;
        for (int i = 0; i < num_vectors; i++) {
            predicted = predicted + kernels[i] * svr.coefficients[i];
            if (i == num_vectors - 1) {
                predicted = predicted + svr.intercepts[0];
            }
        }

        return predicted;
    }


//female predict function definition
    double classification_helper::female_svr_image_features_predictor_predict(std::map<std::string, AHIModelSVR> &svrModels, double features[],
                                                                              const std::string &param) {

        // Get ref to SVR
        AHIModelSVR svr;
        if (param == "chest") {
            svr = svrModels.at("female_chest_svr_image_features");
        } else if (param == "waist") {
            svr = svrModels.at("female_waist_svr_image_features");
        } else if (param == "hip") {
            svr = svrModels.at("female_hip_svr_image_features");
        } else if (param == "inseam") {
            svr = svrModels.at("female_inseam_svr_image_features");
        }

        size_t num_vectors = svr.vectors.size();

        //compute the mapped vectors wrt support vectors
        double kernels[num_vectors];
        double kernel;
        switch (KERNEL_TYPE) {
            case 'l':
                // <x,x'>
                for (int i = 0; i < num_vectors; i++) {
                    kernel = 0.;
                    for (int j = 0; j < N_FEATURES_svr_image_features; j++) {
                        kernel += svr.vectors[i][j] * features[j];
                    }
                    kernels[i] = kernel;
                }
                break;
            case 'p':
                // (y<x,x'>+r)^d
                for (int i = 0; i < num_vectors; i++) {
                    kernel = 0.;
                    for (int j = 0; j < N_FEATURES_svr_image_features; j++) {
                        kernel += svr.vectors[i][j] * features[j];
                    }
                    kernels[i] = pow((KERNEL_GAMMA * kernel) + KERNEL_COEF, KERNEL_DEGREE);//(1.0 + kernel) * (1.0 + kernel);//
                }
                break;
            case 'r':
                // exp(-y|x-x'|^2)
                for (int i = 0; i < num_vectors; i++) {
                    kernel = 0.;
                    for (int j = 0; j < N_FEATURES_svr_image_features; j++) {
                        kernel += pow(svr.vectors[i][j] - features[j], 2);
                    }
                    kernels[i] = exp(-KERNEL_GAMMA * kernel);
                }
                break;
            case 's':
                // tanh(y<x,x'>+r)
                for (int i = 0; i < num_vectors; i++) {
                    kernel = 0.;
                    for (int j = 0; j < N_FEATURES_svr_image_features; j++) {
                        kernel += svr.vectors[i][j] * features[j];
                    }
                    kernels[i] = tanh((KERNEL_GAMMA * kernel) + KERNEL_COEF);
                }
                break;
        }

        //compute the product of kernel and coeff
        double predicted = 0.0;
        for (int i = 0; i < num_vectors; i++) {
            if (param == "waist") {
                predicted = predicted + kernels[i] * svr.coefficients[i];

                if (i == num_vectors - 1) {
                    predicted = predicted + svr.intercepts[0];
                }
            }

            if (param == "chest") {
                predicted = predicted + kernels[i] * svr.coefficients[i];

                if (i == num_vectors - 1) {
                    predicted = predicted + svr.intercepts[0];
                }
            }

            if (param == "hip") {
                predicted = predicted + kernels[i] * svr.coefficients[i];

                if (i == num_vectors - 1) {
                    predicted = predicted + svr.intercepts[0];
                }
            }


            if (param == "inseam") {
                predicted = predicted + kernels[i] * svr.coefficients[i];

                if (i == num_vectors - 1) {
                    predicted = predicted + svr.intercepts[0];
                }
            }


        }

        return predicted;
    }


#if 0 /// this for a model for both male and female

    double classification_helper::svr_image_features_predictor_v2_predict(double features[], std::string param)
    {

        int num_vectors;

        if (param == "chest")
        {
            num_vectors = sizeof(chest_svr_image_features_v2::vectors) / (N_FEATURES_svr_image_features * 8);
            //std::cout <<"\n chest num_vectors = " << num_vectors;
        }
        if (param == "waist")
        {
            num_vectors = sizeof(waist_svr_image_features_v2::vectors) / (N_FEATURES_svr_image_features * 8);
            //std::cout <<"\n num_vectors = " << num_vectors;
        }
        if (param == "hip")
        {
            num_vectors = sizeof(hip_svr_image_features_v2::vectors) / (N_FEATURES_svr_image_features * 8);
            //std::cout <<"\n num_vectors = " << num_vectors;
        }
        if (param == "inseam")
        {
            num_vectors = sizeof(inseam_svr_image_features_v2::vectors) / (N_FEATURES_svr_image_features * 8);
            //std::cout <<"\n num_vectors = " << num_vectors;
        }

        if (param == "weight")
        {
            num_vectors = sizeof(weight_svr_image_features_v2::vectors) / (N_FEATURES_svr_image_features * 8);
            //std::cout <<"\n num_vectors = " << num_vectors;
        }

        if (param == "fat")
        {
            num_vectors = sizeof(fat_svr_image_features_v2::vectors) / (N_FEATURES_svr_image_features * 8);
            //std::cout <<"\n num_vectors = " << num_vectors;
        }

        //compute the mapped vectors wrt support vectors
        double kernels[num_vectors];
        double kernel;
        switch (KERNEL_TYPE)
        {
            case 'l':
                // <x,x'>
                for (int i = 0; i < num_vectors; i++)
                {
                    kernel = 0.;
                    for (int j = 0; j < N_FEATURES_svr_image_features; j++)
                    {
                        if (param == "waist")
                            kernel += waist_svr_image_features_v2::vectors[i][j] * features[j];
                        if (param == "chest")
                            kernel += chest_svr_image_features_v2::vectors[i][j] * features[j];
                        if (param == "hip")
                            kernel += hip_svr_image_features_v2::vectors[i][j] * features[j];
                        if (param == "inseam")
                            kernel += inseam_svr_image_features_v2::vectors[i][j] * features[j];
                        if (param == "inseam")
                            kernel += inseam_svr_image_features_v2::vectors[i][j] * features[j];
                    }
                    kernels[i] = kernel;

                }
                break;
            case 'p':
                // (y<x,x'>+r)^d
                for (int i = 0; i < num_vectors; i++)
                {
                    kernel = 0.;
                    for (int j = 0; j < N_FEATURES_svr_image_features; j++)
                    {
                        if (param == "waist")
                            kernel += waist_svr_image_features_v2::vectors[i][j] * features[j];
                        if (param == "chest")
                            kernel += chest_svr_image_features_v2::vectors[i][j] * features[j];
                        if (param == "hip")
                            kernel += hip_svr_image_features_v2::vectors[i][j] * features[j];
                        if (param == "inseam")
                            kernel += inseam_svr_image_features_v2::vectors[i][j] * features[j];
                        if (param == "weight")
                            kernel += weight_svr_image_features_v2::vectors[i][j] * features[j];
                        if (param == "fat")
                            kernel += fat_svr_image_features_v2::vectors[i][j] * features[j];

                    }
                    kernels[i] = pow((KERNEL_GAMMA * kernel) + KERNEL_COEF, KERNEL_DEGREE);// (1.0 + kernel) * (1.0 + kernel); //
                }
                break;
            case 'r':
                // exp(-y|x-x'|^2)
                for (int i = 0; i < num_vectors; i++)
                {
                    kernel = 0.;
                    for (int j = 0; j < N_FEATURES_svr_image_features; j++)
                    {
                        kernel += pow(waist_svr_image_features_v2::vectors[i][j] - features[j], 2);
                    }
                    kernels[i] = exp(-KERNEL_GAMMA * kernel);
                }
                break;
            case 's':
                // tanh(y<x,x'>+r)
                for (int i = 0; i < num_vectors; i++)
                {
                    kernel = 0.;
                    for (int j = 0; j < N_FEATURES_svr_image_features; j++)
                    {
                        kernel += waist_svr_image_features_v2::vectors[i][j] * features[j];
                    }
                    kernels[i] = tanh((KERNEL_GAMMA * kernel) + KERNEL_COEF);
                }
                break;
        }

        //compute the product of kernel and coeff
        double predicted = 0.0;
        for (int i = 0; i < num_vectors; i++)
        {
            if (param == "waist")
            {
                predicted = predicted + kernels[i] * waist_svr_image_features_v2::coefficients[i];

                if (i == num_vectors - 1)
                {
                    predicted = predicted + waist_svr_image_features_v2::intercepts[0];
                }
            }

            if (param == "chest")
            {
                predicted = predicted + kernels[i] * chest_svr_image_features_v2::coefficients[i];

                if (i == num_vectors - 1)
                {
                    predicted = predicted + chest_svr_image_features_v2::intercepts[0];
                }
            }

            if (param == "hip")
            {
                predicted = predicted + kernels[i] * hip_svr_image_features_v2::coefficients[i];

                if (i == num_vectors - 1)
                {
                    predicted = predicted + hip_svr_image_features_v2::intercepts[0];
                }
            }

            if (param == "inseam")
            {
                predicted = predicted + kernels[i] * inseam_svr_image_features_v2::coefficients[i];

                if (i == num_vectors - 1)
                {
                    predicted = predicted + inseam_svr_image_features_v2::intercepts[0];
                }
            }

            if (param == "weight")
            {
                predicted = predicted + kernels[i] * weight_svr_image_features_v2::coefficients[i];

                if (i == num_vectors - 1)
                {
                    predicted = predicted + weight_svr_image_features_v2::intercepts[0];
                }
            }

            if (param == "fat")
            {
                predicted = predicted + kernels[i] * fat_svr_image_features_v2::coefficients[i];

                if (i == num_vectors - 1)
                {
                    predicted = predicted + fat_svr_image_features_v2::intercepts[0];
                }
            }
        }

        return predicted;

    }

#endif

    double classification_helper::male_svr_image_features_predictor_v2_predict(std::map<std::string, AHIModelSVR> &svrModels, double features[],
                                                                               std::string param) {
        // Get ref to SVR
        AHIModelSVR svr;
        if (param == "chest") {
            svr = svrModels.at("male_chest_svr_image_features_v2");
        } else if (param == "waist") {
            svr = svrModels.at("male_waist_svr_image_features_v2");
        } else if (param == "hip") {
            svr = svrModels.at("male_hip_svr_image_features_v2");
        } else if (param == "inseam") {
            svr = svrModels.at("male_inseam_svr_image_features_v2");
        } else if (param == "chest_UWA_all") {
            svr = svrModels.at("male_chest_svr_image_features_v2_UWA_all");
        } else if (param == "waist_UWA_all") {
            svr = svrModels.at("male_waist_svr_image_features_v2_UWA_all");
        } else if (param == "hip_UWA_all") {
            svr = svrModels.at("male_hip_svr_image_features_v2_UWA_all");
        } else if (param == "inseam_UWA_all") {
            svr = svrModels.at("male_inseam_svr_image_features_v2_UWA_all");
        }

        size_t num_vectors = svr.vectors.size();

        //compute the mapped vectors wrt support vectors
        double kernels[num_vectors];
        double kernel;
        switch (KERNEL_TYPE) {
            case 'l':
                // <x,x'>
                for (int i = 0; i < num_vectors; i++) {
                    kernel = 0.;
                    for (int j = 0; j < N_FEATURES_svr_image_features; j++) {
                        kernel += svr.vectors[i][j] * features[j];
                    }
                    kernels[i] = kernel;
                }
                break;
            case 'p':
                // (y<x,x'>+r)^d
                for (int i = 0; i < num_vectors; i++) {
                    kernel = 0.;
                    for (int j = 0; j < N_FEATURES_svr_image_features; j++) {
                        kernel += svr.vectors[i][j] * features[j];
                    }
                    kernels[i] = pow((KERNEL_GAMMA * kernel) + KERNEL_COEF, KERNEL_DEGREE); //(1.0 + kernel) * (1.0 + kernel);//
                }
                break;
            case 'r':
                // exp(-y|x-x'|^2)
                for (int i = 0; i < num_vectors; i++) {
                    kernel = 0.;
                    for (int j = 0; j < N_FEATURES_svr_image_features; j++) {
                        kernel += pow(svr.vectors[i][j] - features[j], 2);
                    }
                    kernels[i] = exp(-KERNEL_GAMMA * kernel);
                }
                break;
            case 's':
                // tanh(y<x,x'>+r)
                for (int i = 0; i < num_vectors; i++) {
                    kernel = 0.;
                    for (int j = 0; j < N_FEATURES_svr_image_features; j++) {
                        kernel += svr.vectors[i][j] * features[j];
                    }
                    kernels[i] = tanh((KERNEL_GAMMA * kernel) + KERNEL_COEF);
                }
                break;
        }

        //compute the product of kernel and coeff
        double predicted = 0.0;
        for (int i = 0; i < num_vectors; i++) {
            predicted = predicted + kernels[i] * svr.coefficients[i];
            if (i == num_vectors - 1) {
                predicted = predicted + svr.intercepts[0];
            }
        }

        return predicted;
    }


//female predict function definition
    double classification_helper::female_svr_image_features_predictor_v2_predict(std::map<std::string, AHIModelSVR> &svrModels,
                                                                                 double features[],
                                                                                 std::string param) {
        // Get ref to SVR
        AHIModelSVR svr;
        if (param == "chest") {
            svr = svrModels.at("female_chest_svr_image_features_v2");
        } else if (param == "waist") {
            svr = svrModels.at("female_waist_svr_image_features_v2");
        } else if (param == "hip") {
            svr = svrModels.at("female_hip_svr_image_features_v2");
        } else if (param == "inseam") {
            svr = svrModels.at("female_inseam_svr_image_features_v2");
        } else if (param == "chest_UWA_all") {
            svr = svrModels.at("female_chest_svr_image_features_v2_UWA_all");
        } else if (param == "waist_UWA_all") {
            svr = svrModels.at("female_waist_svr_image_features_v2_UWA_all");
        } else if (param == "hip_UWA_all") {
            svr = svrModels.at("female_hip_svr_image_features_v2_UWA_all");
        } else if (param == "inseam_UWA_all") {
            svr = svrModels.at("female_inseam_svr_image_features_v2_UWA_all");
        }

        size_t num_vectors = svr.vectors.size();

        //compute the mapped vectors wrt support vectors
        double kernels[num_vectors];
        double kernel;
        switch (KERNEL_TYPE) {
            case 'l':
                // <x,x'>
                for (int i = 0; i < num_vectors; i++) {
                    kernel = 0.;
                    for (int j = 0; j < N_FEATURES_svr_image_features; j++) {
                        kernel += svr.vectors[i][j] * features[j];
                    }
                    kernels[i] = kernel;
                }
                break;
            case 'p':
                // (y<x,x'>+r)^d
                for (int i = 0; i < num_vectors; i++) {
                    kernel = 0.;
                    for (int j = 0; j < N_FEATURES_svr_image_features; j++) {
                        kernel += svr.vectors[i][j] * features[j];
                    }
                    kernels[i] = pow((KERNEL_GAMMA * kernel) + KERNEL_COEF, KERNEL_DEGREE);//(1.0 + kernel) * (1.0 + kernel);//
                }
                break;
            case 'r':
                // exp(-y|x-x'|^2)
                for (int i = 0; i < num_vectors; i++) {
                    kernel = 0.;
                    for (int j = 0; j < N_FEATURES_svr_image_features; j++) {
                        kernel += pow(svr.vectors[i][j] - features[j], 2);
                    }
                    kernels[i] = exp(-KERNEL_GAMMA * kernel);
                }
                break;
            case 's':
                // tanh(y<x,x'>+r)
                for (int i = 0; i < num_vectors; i++) {
                    kernel = 0.;
                    for (int j = 0; j < N_FEATURES_svr_image_features; j++) {
                        kernel += svr.vectors[i][j] * features[j];
                    }
                    kernels[i] = tanh((KERNEL_GAMMA * kernel) + KERNEL_COEF);
                }
                break;
        }

        //compute the product of kernel and coeff
        double predicted = 0.0;
        for (int i = 0; i < num_vectors; i++) {
            predicted = predicted + kernels[i] * svr.coefficients[i];
            if (i == num_vectors - 1) {
                predicted = predicted + svr.intercepts[0];
            }
        }

        return predicted;
    }


#if 0 // this for a model for both male and female

    double classification_helper::svr_image_features_predictor_v3_predict(double features[], std::string param)
    {

        int num_vectors;

        if (param == "chest")
        {
            num_vectors = sizeof(chest_svr_image_features_v3::vectors) / (N_FEATURES_svr_image_features * 8);
            //std::cout <<"\n chest num_vectors = " << num_vectors;
        }
        if (param == "waist")
        {
            num_vectors = sizeof(waist_svr_image_features_v3::vectors) / (N_FEATURES_svr_image_features * 8);
            //std::cout <<"\n num_vectors = " << num_vectors;
        }
        if (param == "hip")
        {
            num_vectors = sizeof(hip_svr_image_features_v3::vectors) / (N_FEATURES_svr_image_features * 8);
            //std::cout <<"\n num_vectors = " << num_vectors;
        }
        if (param == "inseam")
        {
            num_vectors = sizeof(inseam_svr_image_features_v3::vectors) / (N_FEATURES_svr_image_features * 8);
            //std::cout <<"\n num_vectors = " << num_vectors;
        }

        if (param == "weight")
        {
            num_vectors = sizeof(weight_svr_image_features_v3::vectors) / (N_FEATURES_svr_image_features * 8);
            //std::cout <<"\n num_vectors = " << num_vectors;
        }

        if (param == "fat")
        {
            num_vectors = sizeof(fat_svr_image_features_v3::vectors) / (N_FEATURES_svr_image_features * 8);
            //std::cout <<"\n num_vectors = " << num_vectors;
        }

        //compute the mapped vectors wrt support vectors
        double kernels[num_vectors];
        double kernel;
        switch (KERNEL_TYPE)
        {
            case 'l':
                // <x,x'>
                for (int i = 0; i < num_vectors; i++)
                {
                    kernel = 0.;
                    for (int j = 0; j < N_FEATURES_svr_image_features; j++)
                    {
                        if (param == "waist")
                            kernel += waist_svr_image_features_v3::vectors[i][j] * features[j];
                        if (param == "chest")
                            kernel += chest_svr_image_features_v3::vectors[i][j] * features[j];
                        if (param == "hip")
                            kernel += hip_svr_image_features_v3::vectors[i][j] * features[j];
                        if (param == "inseam")
                            kernel += inseam_svr_image_features_v3::vectors[i][j] * features[j];
                        if (param == "inseam")
                            kernel += inseam_svr_image_features_v3::vectors[i][j] * features[j];
                    }
                    kernels[i] = kernel;

                }
                break;
            case 'p':
                // (y<x,x'>+r)^d
                for (int i = 0; i < num_vectors; i++)
                {
                    kernel = 0.;
                    for (int j = 0; j < N_FEATURES_svr_image_features; j++)
                    {
                        if (param == "waist")
                            kernel += waist_svr_image_features_v3::vectors[i][j] * features[j];
                        if (param == "chest")
                            kernel += chest_svr_image_features_v3::vectors[i][j] * features[j];
                        if (param == "hip")
                            kernel += hip_svr_image_features_v3::vectors[i][j] * features[j];
                        if (param == "inseam")
                            kernel += inseam_svr_image_features_v3::vectors[i][j] * features[j];
                        if (param == "weight")
                            kernel += weight_svr_image_features_v3::vectors[i][j] * features[j];
                        if (param == "fat")
                            kernel += fat_svr_image_features_v3::vectors[i][j] * features[j];

                    }
                    kernels[i] = pow((KERNEL_GAMMA * kernel) + KERNEL_COEF, KERNEL_DEGREE);// (1.0 + kernel) * (1.0 + kernel); //
                }
                break;
            case 'r':
                // exp(-y|x-x'|^2)
                for (int i = 0; i < num_vectors; i++)
                {
                    kernel = 0.;
                    for (int j = 0; j < N_FEATURES_svr_image_features; j++)
                    {
                        kernel += pow(waist_svr_image_features_v3::vectors[i][j] - features[j], 2);
                    }
                    kernels[i] = exp(-KERNEL_GAMMA * kernel);
                }
                break;
            case 's':
                // tanh(y<x,x'>+r)
                for (int i = 0; i < num_vectors; i++)
                {
                    kernel = 0.;
                    for (int j = 0; j < N_FEATURES_svr_image_features; j++)
                    {
                        kernel += waist_svr_image_features_v3::vectors[i][j] * features[j];
                    }
                    kernels[i] = tanh((KERNEL_GAMMA * kernel) + KERNEL_COEF);
                }
                break;
        }

        //compute the product of kernel and coeff
        double predicted = 0.0;
        for (int i = 0; i < num_vectors; i++)
        {
            if (param == "waist")
            {
                predicted = predicted + kernels[i] * waist_svr_image_features_v3::coefficients[i];

                if (i == num_vectors - 1)
                {
                    predicted = predicted + waist_svr_image_features_v3::intercepts[0];
                }
            }

            if (param == "chest")
            {
                predicted = predicted + kernels[i] * chest_svr_image_features_v3::coefficients[i];

                if (i == num_vectors - 1)
                {
                    predicted = predicted + chest_svr_image_features_v3::intercepts[0];
                }
            }

            if (param == "hip")
            {
                predicted = predicted + kernels[i] * hip_svr_image_features_v3::coefficients[i];

                if (i == num_vectors - 1)
                {
                    predicted = predicted + hip_svr_image_features_v3::intercepts[0];
                }
            }

            if (param == "inseam")
            {
                predicted = predicted + kernels[i] * inseam_svr_image_features_v3::coefficients[i];

                if (i == num_vectors - 1)
                {
                    predicted = predicted + inseam_svr_image_features_v3::intercepts[0];
                }
            }

            if (param == "weight")
            {
                predicted = predicted + kernels[i] * weight_svr_image_features_v3::coefficients[i];

                if (i == num_vectors - 1)
                {
                    predicted = predicted + weight_svr_image_features_v3::intercepts[0];
                }
            }

            if (param == "fat")
            {
                predicted = predicted + kernels[i] * fat_svr_image_features_v3::coefficients[i];

                if (i == num_vectors - 1)
                {
                    predicted = predicted + fat_svr_image_features_v3::intercepts[0];
                }
            }
        }

        return predicted;

    }

#endif

    double classification_helper::male_svr_image_features_predictor_v3_predict(std::map<std::string, AHIModelSVR> &svrModels, double features[],
                                                                               std::string param) {
        // Get ref to SVR
        AHIModelSVR svr;
        if (param == "chest") {
            svr = svrModels.at("male_chest_svr_image_features_v3");
        } else if (param == "waist") {
            svr = svrModels.at("male_waist_svr_image_features_v3");
        } else if (param == "hip") {
            svr = svrModels.at("male_hip_svr_image_features_v3");
        } else if (param == "inseam") {
            svr = svrModels.at("male_inseam_svr_image_features_v3");
        }

        size_t num_vectors = svr.vectors.size();

        //compute the mapped vectors wrt support vectors
        double kernels[num_vectors];
        double kernel;
        switch (KERNEL_TYPE) {
            case 'l':
                // <x,x'>
                for (int i = 0; i < num_vectors; i++) {
                    kernel = 0.;
                    for (int j = 0; j < N_FEATURES_svr_image_features; j++) {
                        kernel += svr.vectors[i][j] * features[j];
                    }
                    kernels[i] = kernel;
                }
                break;
            case 'p':
                // (y<x,x'>+r)^d
                for (int i = 0; i < num_vectors; i++) {
                    kernel = 0.;
                    for (int j = 0; j < N_FEATURES_svr_image_features; j++) {
                        kernel += svr.vectors[i][j] * features[j];
                    }
                    kernels[i] = pow((KERNEL_GAMMA * kernel) + KERNEL_COEF, KERNEL_DEGREE); //(1.0 + kernel) * (1.0 + kernel);//
                }
                break;
            case 'r':
                // exp(-y|x-x'|^2)
                for (int i = 0; i < num_vectors; i++) {
                    kernel = 0.;
                    for (int j = 0; j < N_FEATURES_svr_image_features; j++) {
                        kernel += pow(svr.vectors[i][j] - features[j], 2);
                    }
                    kernels[i] = exp(-KERNEL_GAMMA * kernel);
                }
                break;
            case 's':
                // tanh(y<x,x'>+r)
                for (int i = 0; i < num_vectors; i++) {
                    kernel = 0.;
                    for (int j = 0; j < N_FEATURES_svr_image_features; j++) {
                        kernel += svr.vectors[i][j] * features[j];
                    }
                    kernels[i] = tanh((KERNEL_GAMMA * kernel) + KERNEL_COEF);
                }
                break;
        }

        //compute the product of kernel and coeff
        double predicted = 0.0;
        for (int i = 0; i < num_vectors; i++) {
            predicted = predicted + kernels[i] * svr.coefficients[i];
            if (i == num_vectors - 1) {
                predicted = predicted + svr.intercepts[0];
            }
        }

        return predicted;
    }


//female predict function definition
    double classification_helper::female_svr_image_features_predictor_v3_predict(std::map<std::string, AHIModelSVR> &svrModels, double features[],
                                                                                 std::string param) {
        // Get ref to SVR
        AHIModelSVR svr;
        if (param == "chest") {
            svr = svrModels.at("female_chest_svr_image_features_v3");
        } else if (param == "waist") {
            svr = svrModels.at("female_waist_svr_image_features_v3");
        } else if (param == "hip") {
            svr = svrModels.at("female_hip_svr_image_features_v3");
        } else if (param == "inseam") {
            svr = svrModels.at("female_inseam_svr_image_features_v3");
        }

        size_t num_vectors = svr.vectors.size();

        //compute the mapped vectors wrt support vectors
        double kernels[num_vectors];
        double kernel;
        switch (KERNEL_TYPE) {
            case 'l':
                // <x,x'>
                for (int i = 0; i < num_vectors; i++) {
                    kernel = 0.;
                    for (int j = 0; j < N_FEATURES_svr_image_features; j++) {
                        kernel += svr.vectors[i][j] * features[j];
                    }
                    kernels[i] = kernel;
                }
                break;
            case 'p':
                // (y<x,x'>+r)^d
                for (int i = 0; i < num_vectors; i++) {
                    kernel = 0.;
                    for (int j = 0; j < N_FEATURES_svr_image_features; j++) {
                        kernel += svr.vectors[i][j] * features[j];
                    }
                    kernels[i] = pow((KERNEL_GAMMA * kernel) + KERNEL_COEF, KERNEL_DEGREE);//(1.0 + kernel) * (1.0 + kernel);//
                }
                break;
            case 'r':
                // exp(-y|x-x'|^2)
                for (int i = 0; i < num_vectors; i++) {
                    kernel = 0.;
                    for (int j = 0; j < N_FEATURES_svr_image_features; j++) {
                        kernel += pow(svr.vectors[i][j] - features[j], 2);
                    }
                    kernels[i] = exp(-KERNEL_GAMMA * kernel);
                }
                break;
            case 's':
                // tanh(y<x,x'>+r)
                for (int i = 0; i < num_vectors; i++) {
                    kernel = 0.;
                    for (int j = 0; j < N_FEATURES_svr_image_features; j++) {
                        kernel += svr.vectors[i][j] * features[j];
                    }
                    kernels[i] = tanh((KERNEL_GAMMA * kernel) + KERNEL_COEF);
                }
                break;
        }

        //compute the product of kernel and coeff
        double predicted = 0.0;
        for (int i = 0; i < num_vectors; i++) {
            predicted = predicted + kernels[i] * svr.coefficients[i];
            if (i == num_vectors - 1) {
                predicted = predicted + svr.intercepts[0];
            }
        }

        return predicted;
    }

    std::vector<double> classification_helper::classify(double height,
                                                        double weight,
                                                        const std::string &gender,
                                                        cv::Mat const &inp_front_silhoutte,
                                                        cv::Mat const &inp_side_silhoutte,
                                                        std::map<std::string, cv::Point2f> const &front_joints_vector,
                                                        std::map<std::string, cv::Point2f> const &side_joints_vector,
                                                        std::vector<double> &sil_features_for_DL,
                                                        std::map<std::string, std::pair<char *, std::size_t>> &svrModels,
                                                        std::vector<std::pair<std::string, std::vector<float>>> &svr_class_resultsRawPairs) {

        std::map<std::string, AHIModelSVR> decodedSVRs;
        for (auto &model: svrModels) {
            auto svr = ahiDecodeSvrFromBytes(model.second.first, model.second.second);
            decodedSVRs[model.first] = svr;
        }

        std::vector<double> svr_results;
        std::vector<double> dummy(126, 0);
        sil_features_for_DL = dummy;
        svr_class_resultsRawPairs.clear();
        try {
            if (inp_front_silhoutte.empty() || inp_side_silhoutte.empty()) {
                AHILog(ANDROID_LOG_ERROR, "\n One or both of the silhouettes in classify is/are empty/n");
                return svr_results;
            }

            int NoOfFrontChannels = inp_front_silhoutte.channels();
            int NoOfSideChannels = inp_side_silhoutte.channels();
            cv::Mat front_silhoutte = inp_front_silhoutte.clone();
            cv::Mat side_silhoutte = inp_side_silhoutte.clone();

            if (NoOfFrontChannels > 1) {
                cv::cvtColor(inp_front_silhoutte, front_silhoutte, cv::COLOR_BGRA2GRAY);
            }

            if (NoOfSideChannels > 1) {
                cv::cvtColor(inp_side_silhoutte, side_silhoutte, cv::COLOR_BGRA2GRAY);
            }
            if (cv::sum(inp_front_silhoutte).val[0] < 1000 || cv::sum(inp_side_silhoutte).val[0] < 1000) {
                return svr_results;
            }

            std::vector<float> chest, waist, hip, inseam, thigh, fat, weight_predic, ffm, andriod, gynoid, vat;
            // This is for SVR v1
            std::vector<double> sil_features_v1;
            sil_features_v1 = extract_image_features_v1(height, weight, gender, front_silhoutte, side_silhoutte, front_joints_vector,
                                                        side_joints_vector);
            double *image_features_v1 = &sil_features_v1[0];


            // This is for SVR v2 and v3
            std::vector<double> sil_features;

            sil_features = extract_image_features(height, weight, gender, front_silhoutte, side_silhoutte, front_joints_vector, side_joints_vector);
            double *image_features = &sil_features[0];

            sil_features_for_DL = sil_features;

            float chest_svr_v1, chest_svr_v2, chest_svr_v3, chest_svr_v2_UWA_all;
            float waist_svr_v1, waist_svr_v2, waist_svr_v3, waist_svr_v2_UWA_all;
            float hip_svr_v1, hip_svr_v2, hip_svr_v3, hip_svr_v2_UWA_all;
            float inseam_svr_v1, inseam_svr_v2, inseam_svr_v3, inseam_svr_v2_UWA_all, thigh_svr_v2_UWA_all;

            if ((int(gender.find("F")) > 0) || (int(gender.find("f")) > 0)) // female
            {
                chest_svr_v1 = female_svr_image_features_predictor_predict(decodedSVRs, image_features_v1, "chest");
                waist_svr_v1 = female_svr_image_features_predictor_predict(decodedSVRs, image_features_v1, "waist");
                hip_svr_v1 = female_svr_image_features_predictor_predict(decodedSVRs, image_features_v1, "hip");
                inseam_svr_v1 = female_svr_image_features_predictor_predict(decodedSVRs, image_features_v1, "inseam");
                if (image_features != NULL) {
                    chest_svr_v2 = female_svr_image_features_predictor_v2_predict(decodedSVRs, image_features, "chest");
                    waist_svr_v2 = female_svr_image_features_predictor_v2_predict(decodedSVRs, image_features, "waist");
                    hip_svr_v2 = female_svr_image_features_predictor_v2_predict(decodedSVRs, image_features, "hip");
                    inseam_svr_v2 = female_svr_image_features_predictor_v2_predict(decodedSVRs, image_features, "inseam");

                    chest_svr_v3 = female_svr_image_features_predictor_v3_predict(decodedSVRs, image_features, "chest");
                    waist_svr_v3 = female_svr_image_features_predictor_v3_predict(decodedSVRs, image_features, "waist");
                    hip_svr_v3 = female_svr_image_features_predictor_v3_predict(decodedSVRs, image_features, "hip");
                    inseam_svr_v3 = female_svr_image_features_predictor_v3_predict(decodedSVRs, image_features, "inseam");

                    chest_svr_v2_UWA_all = female_svr_image_features_predictor_v2_predict(decodedSVRs, image_features, "chest_UWA_all");
                    waist_svr_v2_UWA_all = female_svr_image_features_predictor_v2_predict(decodedSVRs, image_features, "waist_UWA_all");
                    hip_svr_v2_UWA_all = female_svr_image_features_predictor_v2_predict(decodedSVRs, image_features, "hip_UWA_all");
                    inseam_svr_v2_UWA_all = female_svr_image_features_predictor_v2_predict(decodedSVRs, image_features, "inseam_UWA_all");
                }
            } else { // male
                chest_svr_v1 = male_svr_image_features_predictor_predict(decodedSVRs, image_features_v1, "chest");
                waist_svr_v1 = male_svr_image_features_predictor_predict(decodedSVRs, image_features_v1, "waist");
                hip_svr_v1 = male_svr_image_features_predictor_predict(decodedSVRs, image_features_v1, "hip");
                inseam_svr_v1 = male_svr_image_features_predictor_predict(decodedSVRs, image_features_v1, "inseam");
                if (image_features != NULL) {
                    chest_svr_v2 = male_svr_image_features_predictor_v2_predict(decodedSVRs, image_features, "chest");
                    waist_svr_v2 = male_svr_image_features_predictor_v2_predict(decodedSVRs, image_features, "waist");
                    hip_svr_v2 = male_svr_image_features_predictor_v2_predict(decodedSVRs, image_features, "hip");
                    inseam_svr_v2 = male_svr_image_features_predictor_v2_predict(decodedSVRs, image_features, "inseam");

                    chest_svr_v3 = male_svr_image_features_predictor_v3_predict(decodedSVRs, image_features, "chest");
                    waist_svr_v3 = male_svr_image_features_predictor_v3_predict(decodedSVRs, image_features, "waist");
                    hip_svr_v3 = male_svr_image_features_predictor_v3_predict(decodedSVRs, image_features, "hip");
                    inseam_svr_v3 = male_svr_image_features_predictor_v3_predict(decodedSVRs, image_features, "inseam");

                    chest_svr_v2_UWA_all = male_svr_image_features_predictor_v2_predict(decodedSVRs, image_features, "chest_UWA_all");
                    waist_svr_v2_UWA_all = male_svr_image_features_predictor_v2_predict(decodedSVRs, image_features, "waist_UWA_all");
                    hip_svr_v2_UWA_all = male_svr_image_features_predictor_v2_predict(decodedSVRs, image_features, "hip_UWA_all");
                    inseam_svr_v2_UWA_all = male_svr_image_features_predictor_v2_predict(decodedSVRs, image_features, "inseam_UWA_all");
                }
            }

            // below push backs can be better but lets POC first
            chest.push_back(chest_svr_v1);
            chest.push_back(chest_svr_v2);
            chest.push_back(chest_svr_v3);
            chest.push_back(chest_svr_v2_UWA_all);

            svr_results.push_back(mean(chest));
            svr_class_resultsRawPairs.push_back({"ChestSVRCurrent", chest});
            waist.push_back(waist_svr_v1);
            waist.push_back(waist_svr_v2);
            waist.push_back(waist_svr_v3);
            waist.push_back(waist_svr_v2_UWA_all);

            svr_results.push_back(mean(waist));
            svr_class_resultsRawPairs.push_back({"WaistSVRCurrent", waist});

            hip.push_back(hip_svr_v1);
            hip.push_back(hip_svr_v2);
            hip.push_back(hip_svr_v3);
            hip.push_back(hip_svr_v2_UWA_all);

            svr_results.push_back(mean(hip));
            svr_class_resultsRawPairs.push_back({"HipSVRCurrent", hip});

            inseam.push_back(inseam_svr_v1);
            inseam.push_back(inseam_svr_v2);
            inseam.push_back(inseam_svr_v3);
            inseam.push_back(inseam_svr_v2_UWA_all);

            svr_results.push_back(mean(inseam));
            svr_class_resultsRawPairs.push_back({"InseamSVRCurrent", inseam});

            if (image_features != NULL) {
                thigh_svr_v2_UWA_all = svr_image_features_predictor_predict(decodedSVRs, image_features, "thigh_UWA_all");
            }

            thigh.push_back(thigh_svr_v2_UWA_all);

            svr_results.push_back(mean(thigh));
            svr_class_resultsRawPairs.push_back({"ThighSVRCurrent", thigh});

            //weight prediction
            std::vector<double> sil_features_for_weight_pred = sil_features;
            if (sil_features_for_weight_pred.size() > 1) {
                // remove the weight as we are predicting
                sil_features_for_weight_pred[1] = 0.0;
                double *image_features_for_weight_pred = &sil_features_for_weight_pred[0];
                double weight_predic_both_poly2 = svr_image_features_predictor_predict(decodedSVRs, image_features_for_weight_pred, "weight");
                weight_predic_both_poly2 = std::max(25.0,
                                                    std::min(weight_predic_both_poly2,
                                                             180.0)); // making sure weight isn't beyond human phyisical values
                double weight_predic_both_poly2_UWA_all = svr_image_features_predictor_predict(decodedSVRs, image_features_for_weight_pred,
                                                                                               "weight_UWA_all");
                weight_predic_both_poly2_UWA_all = std::max(25.0, std::min(weight_predic_both_poly2_UWA_all,
                                                                           180.0)); // making sure weight isn't beyond human phyisical values
                weight_predic_both_poly2_UWA_all = std::max(25.0, std::min(weight_predic_both_poly2_UWA_all,
                                                                           180.0)); // making sure weight isn't beyond human phyisical values
                weight_predic.push_back(weight_predic_both_poly2);
                weight_predic.push_back(weight_predic_both_poly2_UWA_all);
            }

            svr_results.push_back(mean(weight_predic));
            svr_class_resultsRawPairs.push_back({"WeightPredSVRCurrent", weight_predic});
            //Fat Free mass
            if (image_features != NULL) {
                double FFM_predic_both_poly2_UWA_all = svr_image_features_predictor_predict(decodedSVRs, image_features, "ffm_UWA_all");
                FFM_predic_both_poly2_UWA_all = std::max(FFM_predic_both_poly2_UWA_all, 0.55 * weight); // make sure
                ffm.push_back(FFM_predic_both_poly2_UWA_all);
            }

            svr_results.push_back(mean(ffm));
            svr_class_resultsRawPairs.push_back({"FFMkgSVRCurrent", ffm});
            //Fat
            if (image_features != NULL) {
                double fat_predic_both_poly2 = svr_image_features_predictor_predict(decodedSVRs, image_features, "fat");
                fat_predic_both_poly2 = std::max(3.0, std::min(fat_predic_both_poly2,
                                                               weight / 2.0 * 1.01)); //, making sure Fat isn't beyond human phyisical values
                double fat_predic_both_poly2_UWA_all = svr_image_features_predictor_predict(decodedSVRs, image_features, "fat_UWA_all");
                fat_predic_both_poly2_UWA_all = std::max(3.0, std::min(fat_predic_both_poly2_UWA_all,
                                                                       weight / 2.0 * 1.01)); //, making sure Fat isn't beyond human phyisical values
                fat.push_back(fat_predic_both_poly2);
                fat.push_back(fat_predic_both_poly2_UWA_all);
            }
            svr_results.push_back(mean(fat));
            svr_class_resultsRawPairs.push_back({"FatkgSVRCurrent", fat});
            //gynoid
            if (image_features != NULL) {
                double gynoid_predic_both_poly2_UWA_all = svr_image_features_predictor_predict(decodedSVRs, image_features, "gynoid_UWA_all");
                gynoid_predic_both_poly2_UWA_all = std::max(gynoid_predic_both_poly2_UWA_all, 0.6 / 100.0 * weight); // make sure
                gynoid_predic_both_poly2_UWA_all = std::min(gynoid_predic_both_poly2_UWA_all, 11. / 100. * weight); // make sure
                gynoid.push_back(gynoid_predic_both_poly2_UWA_all);
            }

            svr_results.push_back(mean(gynoid));
            svr_class_resultsRawPairs.push_back({"GynoidkgSVRCurrent", gynoid});
            //andriod
            if (image_features != NULL) {
                double andriod_predic_both_poly2_UWA_all = svr_image_features_predictor_predict(decodedSVRs, image_features, "andriod_UWA_all");
                andriod_predic_both_poly2_UWA_all = std::max(andriod_predic_both_poly2_UWA_all, 0.27 / 100.0 * weight); // make sure
                andriod_predic_both_poly2_UWA_all = std::min(andriod_predic_both_poly2_UWA_all, 6. / 100.0 * weight); // make sure
                andriod.push_back(andriod_predic_both_poly2_UWA_all);
            }

            svr_results.push_back(mean(andriod));
            svr_class_resultsRawPairs.push_back({"AndroidkgSVRCurrent", andriod});
            //vat
            if (image_features != NULL) {
                double visceral_predic_both_poly2_UWA_all = svr_image_features_predictor_predict(decodedSVRs, image_features, "visceral_UWA_all");
                visceral_predic_both_poly2_UWA_all = std::max(visceral_predic_both_poly2_UWA_all, 0.1 / 100.0 * weight); // make sure
                visceral_predic_both_poly2_UWA_all = std::min(visceral_predic_both_poly2_UWA_all, 3.9 / 100.0 * weight); // make sure
                vat.push_back(visceral_predic_both_poly2_UWA_all);
            }

            svr_results.push_back(mean(vat));
            svr_class_resultsRawPairs.push_back({"VATkgSVRCurrent", vat});

            return svr_results;
        }
        catch (...) {
            AHILog(ANDROID_LOG_ERROR, "Exception caught or an Error in classify");
            return svr_results;
        }

    }

}