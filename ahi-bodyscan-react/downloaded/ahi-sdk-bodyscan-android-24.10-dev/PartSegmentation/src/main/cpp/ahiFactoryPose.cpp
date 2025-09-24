//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#include "ahiFactoryPose.hpp"

#include <iostream>

#include "ahiFactoryTensor.hpp"
#include "Logging.hpp"

std::string to_lowerStr(std::string str) {
    std::for_each(str.begin(), str.end(), [](char &c) {
        c = ::tolower(c);
    });
    return str;
}

void ahiFactoryPose::getFactorTensorInstant() {
    ahiFactoryTensor poseFT;
    isPoseInit = true;
}

void ahiFactoryPose::initPose() {
    getFactorTensorInstant();
}

bool ahiFactoryPose::mlkitPose(ahiPoseInfo &poseInfoPredictions) {
    int lData = (int) mlkitPoseData.size() / 3;
    if (lData < 1) {
        return false;
    }
    try {
        for (int i = 0; i < lData; i++) {
            int index = i * 3;
            float X = mlkitPoseData[index];
            float Y = mlkitPoseData[index + 1];
            float confidence = mlkitPoseData[index + 2];
            cv::Point joint(X, Y);
            if (i == 0) {
                poseInfoPredictions.CentroidHeadTop = joint;
                poseInfoPredictions.CentroidHeadTopConfidence = confidence;
                poseInfoPredictions.headFound = confidence > 0.1;
            }
            if (i == 1) {
                poseInfoPredictions.CentroidNeck = joint;
                poseInfoPredictions.CentroidNeckConfidence = confidence;
                poseInfoPredictions.headFound = poseInfoPredictions.headFound && confidence > 0.1;
            }
            if (i == 2) {
                poseInfoPredictions.CentroidRightShoulder = joint;
                poseInfoPredictions.CentroidRightShoulderConfidence = confidence;
            }
            if (i == 3) {
                poseInfoPredictions.CentroidRightElbow = joint;
                poseInfoPredictions.CentroidRightElbowConfidence = confidence;
            }
            if (i == 4) {
                poseInfoPredictions.CentroidRightHand = joint;
                poseInfoPredictions.CentroidRightHandConfidence = confidence;
            }
            if (i == 5) {
                poseInfoPredictions.CentroidLeftShoulder = joint;
                poseInfoPredictions.CentroidLeftShoulderConfidence = confidence;
            }
            if (i == 6) {
                poseInfoPredictions.CentroidLeftElbow = joint;
                poseInfoPredictions.CentroidLeftElbowConfidence = confidence;
            }
            if (i == 7) {
                poseInfoPredictions.CentroidLeftHand = joint;
                poseInfoPredictions.CentroidLeftHandConfidence = confidence;
            }
            if (i == 8) {
                poseInfoPredictions.CentroidRightHip = joint;
                poseInfoPredictions.CentroidRightHipConfidence = confidence;
            }
            if (i == 9) {
                poseInfoPredictions.CentroidRightKnee = joint;
                poseInfoPredictions.CentroidRightKneeConfidence = confidence;
            }
            if (i == 10) {
                poseInfoPredictions.CentroidRightAnkle = joint;
                poseInfoPredictions.CentroidRightAnkleConfidence = confidence;
            }
            if (i == 11) {
                poseInfoPredictions.CentroidLeftHip = joint;
                poseInfoPredictions.CentroidLeftHipConfidence = confidence;
            }
            if (i == 12) {
                poseInfoPredictions.CentroidLeftKnee = joint;
                poseInfoPredictions.CentroidLeftKneeConfidence = confidence;
            }
            if (i == 13) {
                poseInfoPredictions.CentroidLeftAnkle = joint;
                poseInfoPredictions.CentroidLeftAnkleConfidence = confidence;
            }
        }
        return true;
    }
    catch (...) {
        poseInfoPredictions.GE = true;
        poseInfoPredictions.ErrorMsg = "Errors in tranfering mlkit results to cpp";
        LOG_GUARD(
                std::cout << "\nErrors occured in ahiFactoryPose::mlkitPose, mlkit pose has errors"
                          << std::endl);
        return false;
    }
    return false;// TODO
}

bool ahiFactoryPose::ahiPoseLight(ahiPoseInfo &poseInfoPredictions) {
    // declare the constant for the heatmap
    int const numChanel = 15;
    int const numRow = 96; // we can get this from the model output as well
    int const numCol = 96;
    // now we use ML to get the pose/joints, in this case this is a pose_light heatmap model
    std::vector<std::string> OutputNames = poseFT.mOutputNames;
    ahiTensorOutputMap outputs;
    bool predPass = poseFT.invokeMIMO(poseFT.mInputs, outputs);
    // iterate through the outputs
    // in this case I'm use pose_light so one vector output is expected, but can be generalized
    cv::Mat outputBlob;
    for (auto outIter = outputs.begin(); outIter != outputs.end(); outIter++) {
        std::string currModelOutNodeName = outIter->first;
        std::cout << "Current output node name :" << currModelOutNodeName << "\n";
        outputBlob = outIter->second._mat;
        break;// index 0 is are the heatmaps
    }
    int paddingOffset = 0;
    float xScale = 720.0 / 96.0;
    float yScale = 1280.0 / 96.0;
    if (isPaddedForResize) {
        xScale = 1280.0 / 96.0;
        paddingOffset = 280;
    }
    if (predPass) {
        // perform the post processing of the heatMap
        std::vector<float> heatMapEachSum(14, -1);
        std::vector<float> heatMapEachRadius(14, -1);
        for (int channel = 1; channel < numChanel; channel++) {
            cv::Mat heatMapEach = cv::Mat::zeros(numRow, numCol, CV_32F);
            for (int i = 0; i < numRow; i++) {
                for (int j = 0; j < numCol; j++) {
                    auto featureValue = (float) (outputBlob.at<cv::Vec4f>(0, i, j)[channel - 1]);
                    if (featureValue > 1.0) { featureValue = 1.0; }
                    if (featureValue < 0.1) // 0.1 was for iOS based
                    { featureValue = 0.0; }
                    heatMapEach.at<float>(cv::Point(j, i)) = featureValue;
                }
            }
            cv::Point min_loc, max_loc;
            double min_val, max_val;
            cv::minMaxLoc(heatMapEach, &min_val, &max_val, &min_loc, &max_loc);
            // refine the heatmap based on the contours
            heatMapEach = 255. * heatMapEach / (max_val + 1.0e-10);
            heatMapEachSum[channel - 1] = cv::sum(heatMapEach)[0];
            // TODO: try to do moments as float and convert after.
            heatMapEach.convertTo(heatMapEach, CV_8U);
            cv::blur(heatMapEach, heatMapEach, cv::Size(3, 3)); // you may remove/keep but it helps
            float cx = -1;
            float cy = -1;
            cv::Moments Mom;
            float cx_mom = -1;
            float cy_mom = -1;
            float cxMinMax = -1;
            float cyMinMax = -1;
            heatMapEachRadius[channel - 1] = 0;
            if (max_val > 0) {
                std::vector<std::vector<cv::Point> > Blobcontours;
                cv::findContours(heatMapEach.clone(), Blobcontours, cv::RETR_EXTERNAL,
                                 cv::CHAIN_APPROX_SIMPLE);
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
                // Head and ankle heatmaps is expected to have  the same or a wider width when cropped, but we can have
                // a general method for all other joints if needed
                float radius = 1;
                cv::Point2f center;
                // 4 or a small value means too close image anyway
                if (Blobcontours[biggest_blob_idx].size() > 4) {
                    cv::minEnclosingCircle(Blobcontours[biggest_blob_idx], center, radius);
                    cx = center.x;
                    cy = center.y;
                } else {
                    cxMinMax = max_loc.x;
                    cyMinMax = max_loc.y;
                    Mom = moments(heatMapEach, 0);
                    cx_mom = Mom.m10 / (Mom.m00 + 1e-10);
                    cy_mom = Mom.m01 / (Mom.m00 + 1e-10);
                    cx = (cxMinMax + cx_mom) / 2;
                    cy = (cyMinMax + cy_mom) / 2;
                }
                heatMapEachRadius[channel - 1] = radius;
            }
            // map jointCentroid to actual image size;
            float cx_ = cx * xScale - paddingOffset;
            float cy_ = cy * yScale;
            cv::Point jointCentroid;
            jointCentroid.x = (int) cx_;
            jointCentroid.y = (int) cy_;
            if (channel == 1) {
                poseInfoPredictions.CentroidHeadTop = jointCentroid;
                poseInfoPredictions.headFound = (jointCentroid.x > 0 && jointCentroid.y > 0) &&
                                                (poseInfoPredictions.numOfDetectedFaces == 1);
            }
            if (channel == 2) { poseInfoPredictions.CentroidNeck = jointCentroid; }
            if (channel == 3) { poseInfoPredictions.CentroidRightShoulder = jointCentroid; }
            if (channel == 6) { poseInfoPredictions.CentroidLeftShoulder = jointCentroid; }
            if (channel == 4) { poseInfoPredictions.CentroidRightElbow = jointCentroid; }
            if (channel == 7) { poseInfoPredictions.CentroidLeftElbow = jointCentroid; }
            if (channel == 5) {
                poseInfoPredictions.CentroidRightHand = jointCentroid;
                poseInfoPredictions.rightHandFound = (jointCentroid.x > 0 && jointCentroid.y > 0);
            }
            if (channel == 8) {
                poseInfoPredictions.CentroidLeftHand = jointCentroid;
                poseInfoPredictions.leftHandFound = (jointCentroid.x > 0 && jointCentroid.y > 0);
            }
            if (channel == 9) { poseInfoPredictions.CentroidRightHip = jointCentroid; }
            if (channel == 12) { poseInfoPredictions.CentroidLeftHip = jointCentroid; }
            if (channel == 10) { poseInfoPredictions.CentroidRightKnee = jointCentroid; }
            if (channel == 13) { poseInfoPredictions.CentroidLeftKnee = jointCentroid; }
            if (channel == 11) {
                poseInfoPredictions.CentroidRightAnkle = jointCentroid;
                poseInfoPredictions.rightLegFound = (jointCentroid.x > 0 && jointCentroid.y > 0);
            }
            if (channel == 14) {
                poseInfoPredictions.CentroidLeftAnkle = jointCentroid;
                poseInfoPredictions.leftLegFound = (jointCentroid.x > 0 && jointCentroid.y > 0);
            }
        }
        // Another fix attempt from the heatmap size itself. This is similar confidence scoring w.r.t others
        // This seems working fine to adjust head, ankles and even wrists/hands
        int counter = 0;
        float heatmapAvg = 0;
        float heatmapAvgRadious = 0;
        for (int ch = 0; ch < 14; ch++) {
            if (heatMapEachSum[ch] > 0) {
                counter = counter + 1;
                heatmapAvg = heatmapAvg + heatMapEachSum[ch];
                heatmapAvgRadious = heatmapAvgRadious + heatMapEachRadius[ch];
            }
        }
        heatmapAvg = heatmapAvg / counter;
        heatmapAvgRadious = xScale * heatmapAvgRadious / counter;
        float ratioHead = std::min(1.0, heatMapEachSum[0] / (1.0e-10 + heatmapAvg));
        float ratioRightAnkle = std::min(1.0, heatMapEachSum[10] / (1.0e-10 + heatmapAvg));
        float ratioLeftAnkle = std::min(1.0, heatMapEachSum[13] / (1.0e-10 + heatmapAvg));
        // if any of the above ratios is < 0.6, it simply means the joints is partly visible or not there at all

        // Now headtop correction
        if (ratioHead > 0 && poseInfoPredictions.CentroidHeadTop.y < 60) {
            poseInfoPredictions.CentroidHeadTop.y = poseInfoPredictions.CentroidHeadTop.y -
                                                    2.0 * (1. - ratioHead) * heatmapAvgRadious;
        }
        // Now RightAnkle correction
        float ScaleRadiusAnkleX = 1.0;
        if (poseInfoPredictions.view == "side") {
            ScaleRadiusAnkleX = 0;
        }
        if (ratioRightAnkle > 0 && poseInfoPredictions.CentroidRightAnkle.y > (1280 - 60)) {
            poseInfoPredictions.CentroidRightAnkle = poseInfoPredictions.CentroidRightAnkle +
                                                     2.0 * (1. - ratioRightAnkle) * (cv::Point(
                                                             -heatmapAvgRadious / 4 *
                                                             ScaleRadiusAnkleX, heatmapAvgRadious));
        }
        // Now LeftAnkle correction
        if (ratioLeftAnkle > 0 && poseInfoPredictions.CentroidLeftAnkle.y > (1280 - 60)) {
            poseInfoPredictions.CentroidLeftAnkle = poseInfoPredictions.CentroidLeftAnkle +
                                                    2.0 * (1. - ratioLeftAnkle) * (cv::Point(
                                                            heatmapAvgRadious / 4 *
                                                            ScaleRadiusAnkleX, heatmapAvgRadious));
        }
        // neck fix(up)
        if (poseInfoPredictions.headFound) {
            std::cout << "\n neck fix(up) using head" << "\n";
            poseInfoPredictions.CentroidNeck.y = 0.85 * poseInfoPredictions.CentroidNeck.y +
                                                 0.15 * poseInfoPredictions.CentroidHeadTop.y;
        } else {
            std::cout << "\n neck fix(up) using shoulders" << "\n";
            poseInfoPredictions.CentroidNeck.y = 1.4 * poseInfoPredictions.CentroidNeck.y - 0.4 *
                                                                                            (poseInfoPredictions.CentroidRightShoulder.y +
                                                                                             poseInfoPredictions.CentroidLeftShoulder.y) /
                                                                                            2.0;
        }
        // Approx confidence
        float confidence_threshold = 0.7499;
        for (int channel = 1; channel < 15; channel++) {
            int j = channel - 1;
            float confidence = std::min(1.0, heatMapEachSum[j] / (1.0e-10 + heatmapAvg));
            float score = float(confidence < confidence_threshold);
            if (channel == 1) {
                poseInfoPredictions.CentroidHeadTopConfidence = confidence;
            }
            if (channel == 2) {
                poseInfoPredictions.CentroidNeckConfidence = confidence;
            }
            if (channel == 3) {
                poseInfoPredictions.CentroidRightShoulderConfidence = confidence;
            }
            if (channel == 6) {
                poseInfoPredictions.CentroidLeftShoulderConfidence = confidence;
            }
            if (channel == 4) {
                poseInfoPredictions.CentroidRightElbowConfidence = confidence;
            }
            if (channel == 7) {
                poseInfoPredictions.CentroidLeftElbowConfidence = confidence;
            }
            if (channel == 5) {
                poseInfoPredictions.CentroidRightHandConfidence = confidence;
            }
            if (channel == 8) {
                poseInfoPredictions.CentroidLeftHandConfidence = confidence;
            }
            if (channel == 9) {
                poseInfoPredictions.CentroidRightHipConfidence = confidence;
            }
            if (channel == 12) {
                poseInfoPredictions.CentroidLeftHipConfidence = confidence;
            }
            if (channel == 10) {
                poseInfoPredictions.CentroidRightKneeConfidence = confidence;
            }
            if (channel == 13) {
                poseInfoPredictions.CentroidLeftKneeConfidence = confidence;
            }
            if (channel == 11) {
                poseInfoPredictions.CentroidRightAnkleConfidence = confidence;
            }
            if (channel == 14) {
                poseInfoPredictions.CentroidLeftAnkleConfidence = confidence;
            }
        }
        // fix/check for people with face mask
        if (poseInfoPredictions.CentroidHeadTopConfidence >= confidence_threshold) {
            if (poseInfoPredictions.CentroidRightShoulderConfidence >= confidence_threshold &&
                poseInfoPredictions.CentroidLeftShoulderConfidence >= confidence_threshold) {
                cv::Point meanShoulder = (poseInfoPredictions.CentroidRightShoulder +
                                          poseInfoPredictions.CentroidLeftShoulder) / 2;
                cv::Point predNeckFromHeadShoulder =
                        0.5 * poseInfoPredictions.CentroidHeadTop + 0.5 * meanShoulder;
                if ((abs(poseInfoPredictions.CentroidNeck.x - predNeckFromHeadShoulder.x) +
                     abs(poseInfoPredictions.CentroidNeck.y - predNeckFromHeadShoulder.y)) > 40) {
                    poseInfoPredictions.CentroidNeck = predNeckFromHeadShoulder;
                }
                if (abs(poseInfoPredictions.CentroidNeck.x - meanShoulder.x) > 60) {
                    poseInfoPredictions.CentroidNeck.x = meanShoulder.x;
                }
            } else if (
                    poseInfoPredictions.CentroidRightShoulderConfidence >= confidence_threshold &&
                    poseInfoPredictions.CentroidLeftShoulderConfidence < confidence_threshold) {
                cv::Point predNeckFromHeadShoulder = (poseInfoPredictions.CentroidHeadTop +
                                                      poseInfoPredictions.CentroidRightShoulder) /
                                                     2;
                if ((abs(poseInfoPredictions.CentroidNeck.x - predNeckFromHeadShoulder.x) +
                     abs(poseInfoPredictions.CentroidNeck.y - predNeckFromHeadShoulder.y)) > 40) {
                    poseInfoPredictions.CentroidNeck = predNeckFromHeadShoulder;
                }
            } else if (poseInfoPredictions.CentroidRightShoulderConfidence < confidence_threshold &&
                       poseInfoPredictions.CentroidLeftShoulderConfidence >= confidence_threshold) {
                cv::Point predNeckFromHeadShoulder = (poseInfoPredictions.CentroidHeadTop +
                                                      poseInfoPredictions.CentroidLeftShoulder) / 2;
                if ((abs(poseInfoPredictions.CentroidNeck.x - predNeckFromHeadShoulder.x) +
                     abs(poseInfoPredictions.CentroidNeck.y - predNeckFromHeadShoulder.y)) > 40) {
                    poseInfoPredictions.CentroidNeck = predNeckFromHeadShoulder;
                }
            }
        }
        return true;
    } else {
        LOG_GUARD(std::cout << "predictFromHeatMapOpt() failed to run model" << std::endl)
        return false;
    }
}

bool ahiFactoryPose::ahiMoveNetPose(ahiPoseInfo &poseInfoPredictions) {
    try {
        if (!isPoseInit) {
            initPose();
        }
        // now we use ML to get the pose/joints, in this case this is a movenet type model
        std::vector<std::string> OutputNames = poseFT.mOutputNames;
        ahiTensorOutputMap outputs;
        poseFT.invokeMIMO(poseFT.mInputs, outputs);
        // iterate through the outputs
        std::vector<float> predictedResults; // in this case I'm use movenet so one vector output is expected
        for (auto outIter = outputs.begin(); outIter != outputs.end(); outIter++) {
            std::string currModelOutNodeName = outIter->first;
            std::cout << "Current output node name :" << currModelOutNodeName << "\n";
            cv::Mat currOutResult = outIter->second._mat;
            predictedResults.assign(currOutResult.begin<float>(), currOutResult.end<float>());
        }
        std::string all_joint_names[17] = {
                "nose",// 0,
                "left_eye",// 1,
                "right_eye",// 2,
                "left_ear",// 3,
                "right_ear",// 4,
                "left_shoulder",// 5,
                "right_shoulder",// 6,
                "left_elbow",// 7,
                "right_elbow",// 8,
                "left_wrist",// 9,
                "right_wrist",// 10,
                "left_hip",// 11,
                "right_hip",// 12,
                "left_knee",// 13,
                "right_knee",// 14,
                "left_ankle",// 15,
                "right_ankle"// 16
        };
        int nOutputElement = predictedResults.size();
        cv::Point NoseP, LeyeP, ReyeP, LearP, RearP, RshP, LshP;
        float mean_confidence = 0;
        int counter = 0;
        cv::Scalar COLOR(0, 255, 0);
        // filter those joint with small score
        poseInfoPredictions.headFound = bool(poseInfoPredictions.numOfDetectedFaces == 1);
        poseInfoPredictions.leftHandFound = false;
        poseInfoPredictions.rightHandFound = false;
        poseInfoPredictions.leftLegFound = false;
        poseInfoPredictions.rightLegFound = false;
        float confidence_threshold = 0.3; // based on MoveNet
        for (int idx = 0; idx < nOutputElement / 3; idx++) {
            float rawYr = predictedResults[3 * idx];
            float rawXr = predictedResults[3 * idx + 1];
            float confidence = predictedResults[3 * idx + 2];
            if (confidence > 0.1) {
                mean_confidence = mean_confidence + confidence;
                counter++;
            }
            int x, y;
            if (isPaddedForResize) {
                // originalImageWidth and originalImageHeight
                x = originalImageWidth * rawXr - (1280 - 720) / 2.;
                y = originalImageHeight * rawYr;
            } else {
                x = originalImageWidth * rawXr;
                y = originalImageHeight * rawYr;
            }
            cv::Point P(x, y);
            cv::circle(origImageMat, P, 10, COLOR, -1);
            cv::putText(origImageMat, all_joint_names[idx], P, 0, 1, COLOR);
            cv::Point jointCentroid;
            jointCentroid.x = x;
            jointCentroid.y = y;
            if (idx == 0) {
                NoseP = cv::Point(x, y);
                poseInfoPredictions.headFound =
                        poseInfoPredictions.headFound || (confidence > confidence_threshold);
            }
            if (idx == 1) {
                LeyeP = cv::Point(x, y);
                poseInfoPredictions.headFound =
                        poseInfoPredictions.headFound || (confidence > confidence_threshold);
            }
            if (idx == 2) {
                ReyeP = cv::Point(x, y);
                poseInfoPredictions.headFound =
                        poseInfoPredictions.headFound || (confidence > confidence_threshold);
            }
            if (idx == 3) {
                LearP = cv::Point(x, y);
                poseInfoPredictions.headFound =
                        poseInfoPredictions.headFound || (confidence > confidence_threshold);
            }
            if (idx == 4) {
                RearP = cv::Point(x, y);
                poseInfoPredictions.headFound =
                        poseInfoPredictions.headFound || (confidence > confidence_threshold);
            }
            if (idx == 5) {
                poseInfoPredictions.CentroidLeftShoulder = jointCentroid;
                poseInfoPredictions.CentroidLeftShoulderConfidence = confidence;
                LshP = cv::Point(x, y);
            }
            if (idx == 6) {
                poseInfoPredictions.CentroidRightShoulder = jointCentroid;
                poseInfoPredictions.CentroidRightShoulderConfidence = confidence;
                RshP = cv::Point(x, y);

            }
            if (idx == 7) {
                poseInfoPredictions.CentroidLeftElbow = jointCentroid;
                poseInfoPredictions.CentroidLeftElbowConfidence = confidence;
            }
            if (idx == 8) {
                poseInfoPredictions.CentroidRightElbow = jointCentroid;
                poseInfoPredictions.CentroidRightElbowConfidence = confidence;
            }
            if (idx == 9) {
                poseInfoPredictions.CentroidLeftHand = jointCentroid;
                poseInfoPredictions.CentroidLeftHandConfidence = confidence;
                poseInfoPredictions.leftHandFound = confidence > confidence_threshold;
            }
            if (idx == 10) {
                poseInfoPredictions.CentroidRightHand = jointCentroid;
                poseInfoPredictions.CentroidRightHandConfidence = confidence;
                poseInfoPredictions.rightHandFound = confidence > confidence_threshold;
            }
            if (idx == 11) {
                poseInfoPredictions.CentroidLeftHip = jointCentroid;
                poseInfoPredictions.CentroidLeftHipConfidence = confidence;
            }
            if (idx == 12) {
                poseInfoPredictions.CentroidRightHip = jointCentroid;
                poseInfoPredictions.CentroidRightHipConfidence = confidence;
            }
            if (idx == 13) {
                poseInfoPredictions.CentroidLeftKnee = jointCentroid;
                poseInfoPredictions.CentroidLeftKneeConfidence = confidence;
            }
            if (idx == 14) {
                poseInfoPredictions.CentroidRightKnee = jointCentroid;
                poseInfoPredictions.CentroidRightKneeConfidence = confidence;
            }
            if (idx == 15) {
                poseInfoPredictions.CentroidLeftAnkle = jointCentroid;
                poseInfoPredictions.CentroidLeftAnkleConfidence = confidence;
                poseInfoPredictions.leftLegFound = confidence > confidence_threshold;
            }
            if (idx == 16) {
                poseInfoPredictions.CentroidRightAnkle = jointCentroid;
                poseInfoPredictions.CentroidRightAnkleConfidence = confidence;
                poseInfoPredictions.rightLegFound = confidence > confidence_threshold;
            }
        }
        mean_confidence = mean_confidence / counter; // future work
        // chin/neck
        cv::Point Neck, headTop;
        if (poseInfoPredictions.view == "front") // need to revist
        {
            Neck = 0.4 * (0.5 * (RshP + LshP)) + 0.6 * NoseP;
            headTop.x = 0.5 * (RshP.x + LshP.x);
            headTop.y = NoseP.y - 1.25 * (Neck.y - NoseP.y);
        } else {
            Neck = 0.4 * (0.5 * (RshP + LshP)) + 0.6 * NoseP; // nose or one of the ears
            headTop.x = 0.5 * (RshP.x + LshP.x);
            headTop.y = NoseP.y - 1.25 * (Neck.y - NoseP.y);
        }
        cv::circle(origImageMat, Neck, 10, cv::Scalar(0, 0, 255), -1);
        cv::circle(origImageMat, headTop, 10, cv::Scalar(255, 0, 0), -1);
        poseInfoPredictions.CentroidNeck = Neck;
        poseInfoPredictions.CentroidNeckConfidence = 0.6 * float((Neck.x > 0) &&
                                                                 (Neck.x < originalImageWidth) &&
                                                                 (Neck.y > 0) &&
                                                                 (Neck.y < originalImageHeight));
        poseInfoPredictions.CentroidHeadTop = headTop;
        poseInfoPredictions.CentroidHeadTopConfidence = 0.6 * float((headTop.x > 0) &&
                                                                    (headTop.x <
                                                                     originalImageWidth) &&
                                                                    (headTop.y > 0) &&
                                                                    (headTop.y <
                                                                     originalImageHeight));

        poseInfoPredictions.FaceConfidence = poseInfoPredictions.FaceConfidence ||
                                             (0.5 * (poseInfoPredictions.CentroidHeadTopConfidence +
                                                     poseInfoPredictions.CentroidNeckConfidence)) *
                                             float(poseInfoPredictions.headFound);
        return true;
    }
    catch (cv::Exception &CVer) {
        return false;
    }
}

bool
ahiFactoryPose::loadTensorFlowPoseModelFromBufferOrFile(const char *buffer, std::size_t buffer_size,
                                                        std::string modelFileName) {
    if (!isPoseInit) {
        initPose();
    }
    poseFT.modelFileName = modelFileName;
    if (to_lowerStr(modelFileName).find("mlkit") != std::string::npos) {
        return true;
    }
    if (buffer_size > 10)  // model as buffer from Java or elsewhere
    {
        poseFT.mModel = tflite::FlatBufferModel::BuildFromBuffer(buffer,
                                                                 buffer_size);
        bool saveSucess = poseFT.saveModelBufferOnDevice(modelFileName, buffer, buffer_size);
    } else if (modelFileName.size() > 0 && buffer_size < 10) {
        // here I can use the list of pose tflite models and load them. We need the full path of the model here
        poseFT.mModel = tflite::FlatBufferModel::BuildFromFile(
                modelFileName.c_str());// VerifyAndBuildFromFile
        // if above doesn't pass then we can try to download it here, save to cache/data folder and update the class modelFilename (full path)
    } else if (modelFileName.size() < 0 && buffer_size < 10) // no buffer or file name
    {
        return false;
    }
    poseFT.buildOptimalInterpreter();
    if (poseFT.mModel != nullptr) {
        poseFT.GetModelInpOutNames();
    }
    return poseFT.mModel != nullptr;
}

bool ahiFactoryPose::feedInputBufferImageToCppToPose(const void *data, cv::Mat mat) {
    try {
        if (!isPoseInit) {
            return false;
        }
        origImageMat = mat.clone();
        originalImageHeight = mat.rows;
        originalImageWidth = mat.cols;
        originalImageNumOfChannels = mat.channels();
        if (to_lowerStr(modelFileName).find("mlkit") != std::string::npos) {
            return true;
        }
        std::vector<std::string> InputNames = poseFT.mInputNames;
        ahiTensorInputMap mInputs;
        if (InputNames.empty()) {
            return false;
        }
        int pWidth = poseFT.getInputDim(0, 1);
        int pHeight = poseFT.getInputDim(0, 2);
        cv::Size targetSize(pWidth, pHeight); // TF model input image size
        int top, bottom, left, right;
        bool toBGR;
        bool doPadding;
        bool toF32;
        if (to_lowerStr(poseFT.modelFileName).find("movenet") != std::string::npos) {
            toBGR = false;
            doPadding = false;
            toF32 = false;
            isPaddedForResize = doPadding;
            poseFT.isPaddedForResize = doPadding;
            cv::Mat matPose = poseFT.processImageWorWoutPadding(mat, targetSize, top, bottom, left,
                                                                right,
                                                                toBGR,
                                                                doPadding, toF32);
            poseFT.addInput(InputNames[0],
                            ahiTensorInput(matPose, true, false, targetSize, {1.0f}, {0.}));
            return true;
        } else if (to_lowerStr(poseFT.modelFileName).find("light") != std::string::npos) {
            toBGR = true;
            doPadding = false;
            toF32 = true;
            isPaddedForResize = doPadding;
            poseFT.isPaddedForResize = doPadding;
            cv::Mat mat_BGR = mat.clone();
            cv::cvtColor(mat, mat_BGR, cv::COLOR_RGB2BGR);
            cv::Mat matPose = poseFT.processImageWorWoutPadding(mat_BGR, targetSize, top, bottom,
                                                                left, right,
                                                                toBGR, doPadding, toF32);
            poseFT.addInput(InputNames[0],
                            ahiTensorInput(matPose, true, false, targetSize, {1.0f}, {0.}));
            return true;
        }
        return false;
    }
    catch (std::exception e) {
        return false;
    }
    return false;
}

bool ahiFactoryPose::getPoseInfoOutputs(cv::Mat image, ahiPoseInfo &poseInfoPredictions) {
    // lets do the pose now
    ahiFactoryPose FP;
    bool ismlKitUsed =
            to_lowerStr(poseInfoPredictions.poseUsed).find("mlkit") != std::string::npos ||
            to_lowerStr(modelFileName).find("mlkit") != std::string::npos ||
            to_lowerStr(poseFT.modelFileName).find("mlkit") != std::string::npos;
    bool poseSuccess = false;
    if (ismlKitUsed) {
        poseSuccess = mlkitPose(poseInfoPredictions);
        poseInfoPredictions.poseUsed = "mlkit";
        return poseSuccess;
    }
    if (!FP.isPoseInit) {
        FP.initPose();
    }
    if(origImageMat.empty() || !image.empty()){
        feedInputBufferImageToCppToPose(nullptr, image);
    }
    if (poseFT.modelFileName.size() < 1 && poseInfoPredictions.poseUsed.size() > 1) {
        poseFT.modelFileName = poseInfoPredictions.poseUsed;
    }
    if (poseFT.modelFileName.size() > 1 && poseInfoPredictions.poseUsed.size() < 1) {
        poseInfoPredictions.poseUsed = poseFT.modelFileName;
    }
    if (!isPoseInit) {
        poseInfoPredictions.GE = true;
        poseInfoPredictions.ErrorMsg = "please initialize the TF model and feed the input image";
        return false;
    }
    if (to_lowerStr(poseFT.modelFileName).find("movenet") != std::string::npos) {
        // now we use ML to get the pose/joints, in this case this is a movenet type model
        poseSuccess = ahiMoveNetPose(poseInfoPredictions);
    }
    if (to_lowerStr(poseFT.modelFileName).find("light") != std::string::npos) {
        poseSuccess = ahiPoseLight(poseInfoPredictions);
    }
    return poseSuccess;
}