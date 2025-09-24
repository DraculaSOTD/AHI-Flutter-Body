//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#include "AHIAvatarGenSegmentationJointsHelper.hpp"
#include "AHILogging.hpp"

namespace ahi_avatar_gen {

// PUBLIC

    joints_helper::joints_helper(void) {
        // nothing to construct
    }

    cv::Mat joints_helper::segment_dl_grabcut_with_contourmask(const cv::Mat &orig_image,
                                                               BodyScanCommon::Profile type,
                                                               const cv::Mat &net_mask,
                                                               const std::vector<cv::Point> &joints,
                                                               cv::Mat contour_mask) {
        // with DL+grabCut
        // This to deal with the contour that comes with image and limits extra blobs/noise in seg.,
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(contour_mask.clone(), contours, cv::RETR_EXTERNAL,
                         cv::CHAIN_APPROX_SIMPLE);
        for (int i = 0; i < (int) contours.size(); i++) {
            cv::drawContours(contour_mask, contours, i, cv::Scalar(255), cv::FILLED);
        }
        cv::Mat dilated_contour_mask = contour_mask.clone();
        cv::Mat dilate_element_cont = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(31, 31));
        cv::dilate(contour_mask, dilated_contour_mask, dilate_element_cont);

        cv::Mat mask;
        mask.create(net_mask.size(), CV_8UC1);
        mask.setTo(cv::Scalar::all(cv::GC_BGD)); // Set "background" as default guess
        cv::Mat dilate_element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(21, 21));
        cv::Mat dilated_net_mask = cv::Mat::zeros(mask.size(), mask.type());
        cv::dilate(net_mask, dilated_net_mask, dilate_element);
        mask.setTo(cv::GC_PR_FGD, (dilated_net_mask > 0) &
                                  dilated_contour_mask); // Relax this to "probably Foregrounds"
        cv::Mat eroded_net_mask = cv::Mat::zeros(mask.size(), mask.type());
        cv::Mat erode_element = getStructuringElement(cv::MORPH_RECT, cv::Size(11, 11));
        cv::erode(net_mask, eroded_net_mask, erode_element);
        mask.setTo(cv::GC_FGD, (eroded_net_mask > 0) & contour_mask); // Set pixels to "foreground"
        // image borders are BKGD
        cv::line(mask, cv::Point(0, 0), cv::Point(mask.cols, 0), cv::Scalar(cv::GC_BGD), 30);
        cv::line(mask, cv::Point(0, 0), cv::Point(0, mask.rows), cv::Scalar(cv::GC_BGD), 30);
        cv::line(mask, cv::Point(mask.cols, mask.rows), cv::Point(mask.cols, 0),
                 cv::Scalar(cv::GC_BGD),
                 30);
        cv::line(mask, cv::Point(mask.cols, mask.rows), cv::Point(0, mask.rows),
                 cv::Scalar(cv::GC_BGD),
                 30);
        cv::Mat inp_image = orig_image.clone();
        const cv::Mat *grabcut_image;
        grabcut_image = &inp_image;
        cv::Mat bgdModel, fgdModel;
        cv::grabCut(*grabcut_image, mask, cv::Rect(), bgdModel, fgdModel, 3, cv::GC_INIT_WITH_MASK);
        mask = (mask == cv::GC_FGD) | (mask == cv::GC_PR_FGD);
        return mask;
    }

    cv::Mat
    joints_helper::segment_using_net_joints_and_grabcut_and_contourmask(const cv::Mat &orig_image,
                                                                        BodyScanCommon::Profile type,
                                                                        const cv::Mat &net_mask,
                                                                        const std::vector<cv::Point> &joints,
                                                                        cv::Mat contour_mask) {
        int radius = 2;
        int thickness = -1;
        try {
            // This is common for front and side. It was in the very early codes, but we brought it back
            // This to deal with the contour that comes with image and limits extra blobs/noise in seg.,
            std::vector<std::vector<cv::Point>> contours;
            cv::findContours(contour_mask.clone(), contours, cv::RETR_EXTERNAL,
                             cv::CHAIN_APPROX_SIMPLE);
            for (int i = 0; i < (int) contours.size(); i++) {
                cv::drawContours(contour_mask, contours, i, cv::Scalar(255), cv::FILLED);
            }
            cv::Mat dilated_contour_mask = contour_mask.clone();
            cv::Mat dilate_element_cont = cv::getStructuringElement(cv::MORPH_ELLIPSE,
                                                                    cv::Size(31, 31));
            cv::dilate(contour_mask, dilated_contour_mask, dilate_element_cont);

            // caution you may think it can be done if=n a loop but it is not.
            bool isJointsOrderCorrect =
                    (joints[0].y < joints[1].y) && (joints[1].y < joints[2].y) &&
                    (joints[2].y < joints[3].y) && (joints[3].y < joints[4].y) &&
                    (joints[1].y < joints[5].y) && (joints[5].y < joints[6].y) &&
                    (joints[6].y < joints[7].y) && (joints[8].y < joints[9].y) &&
                    (joints[9].y < joints[10].y) && (joints[11].y < joints[12].y) &&
                    (joints[12].y < joints[13].y);
            if (!isJointsOrderCorrect) {
                return segment_dl_grabcut_with_contourmask(orig_image, type, net_mask, joints,
                                                           contour_mask); // use default
            }
            for (int j = 0; j < joints.size(); j++) {
                if ((joints[j].x < 1) || (joints[j].x > orig_image.cols) || (joints[j].y < 1) ||
                    (joints[j].x > orig_image.rows)) {
                    return segment_dl_grabcut_with_contourmask(orig_image, type, net_mask, joints,
                                                               contour_mask); // use default
                }
            }
            cv::RNG &rng = cv::theRNG();
            rng.state = cv::getTickCount();
            // getting face from joints
            cv::Rect face_rect;
            face_rect.height = std::max(50.,
                                        (0.7 - 0.05 * (type == BodyScanCommon::Profile::side ? 1.0
                                                                                             : 0.0)) *
                                        (joints[1].y - joints[0].y));
            face_rect.width = face_rect.height;
            face_rect.x = std::max(joints[0].x - 20., joints[0].x - face_rect.height / 2.0 -
                                                      face_rect.height / 4.0 *
                                                      (type == BodyScanCommon::Profile::side ? 1.0
                                                                                             : 0.0));
            face_rect.y = 1.1 * joints[0].y * (type == BodyScanCommon::Profile::front ? 1.0 : 0.0) +
                          1.01 * joints[0].y * (type == BodyScanCommon::Profile::side ? 1.0 : 0.0);
            std::vector<cv::Point> connectivity = get_bone_connectivity();
            std::string error_id;
            // get a thick skeleton using the joints and a an ROI mask
            cv::Mat SkelBinMask = cv::Mat::zeros(orig_image.size(), 0);
            // This is to get 100% FGD features
            SkelBinMask = get_skel_mask_from_joints(SkelBinMask, joints, type, error_id);
            cv::Mat dilate_element_skel = cv::getStructuringElement(cv::MORPH_RECT,
                                                                    cv::Size(45, 45)); // 55,55
            cv::Mat dilated_SkelBinMask = cv::Mat::zeros(SkelBinMask.size(), SkelBinMask.type());
            cv::Mat dummy = dilated_SkelBinMask;
            if (type == BodyScanCommon::Profile::front) { // don't dilate the FG hand regions
                cv::Rect skel_rect;
                skel_rect.x = 0.9 * joints[2].x; // SkelBinMask.cols/3;
                skel_rect.width = 1.1 * joints[5].x - 0.9 * joints[2].x; // SkelBinMask.cols/3;
                skel_rect.y = 30;
                skel_rect.height = SkelBinMask.rows - 30; // skel_rect.height - 80;//60
                (SkelBinMask(skel_rect)).copyTo(dummy(skel_rect));
                cv::dilate(dummy, dilated_SkelBinMask, dilate_element_skel);
            } else {
                dilate(SkelBinMask, dilated_SkelBinMask, dilate_element_skel);
            }
            dummy.release();
            // net heatmap shall not be outside the likely position of the user PR_FGD
            cv::Mat dilated_net_mask = net_mask.clone();
            cv::Mat dilate_element = getStructuringElement(cv::MORPH_RECT, cv::Size(21, 21));
            cv::dilate(net_mask.clone(), dilated_net_mask, dilate_element);
            cv::Mat mask, erroded_net_mask;
            // BG features
            // set all BG first then overwrite FG and PR_FG later
            mask.create(net_mask.size(), CV_8UC1);
            mask.setTo(cv::Scalar::all(cv::GC_BGD));
            if (type == BodyScanCommon::Profile::side) {
                bool use_skel = true;
                bool clean_unmatched = false;
                if (!use_skel) {
                    SkelBinMask = cv::Mat::zeros(SkelBinMask.size(), SkelBinMask.type());
                }
                if (clean_unmatched) {
                    int L = 7;
                    cv::Mat errod_element = getStructuringElement(cv::MORPH_RECT, cv::Size(L, L));
                    erode(net_mask.clone(), erroded_net_mask, errod_element);
                    cv::Mat erroded_net_mask_Image;
                    orig_image.copyTo(erroded_net_mask_Image, erroded_net_mask);
                    erroded_net_mask = erroded_net_mask &
                                       match_images(erroded_net_mask_Image, SkelBinMask.clone(),
                                                    0.99);
                    cv::Mat dilated_net_mask_Image;
                    dilated_net_mask =
                            (dilated_net_mask | dilated_SkelBinMask) & dilated_contour_mask;
                    orig_image.copyTo(dilated_net_mask_Image, dilated_net_mask);
                    dilated_net_mask = dilated_net_mask &
                                       match_images(dilated_net_mask_Image, SkelBinMask.clone(),
                                                    0.99);
                } else {
                    int L = 11;
                    cv::Mat errod_element = getStructuringElement(cv::MORPH_RECT, cv::Size(L, L));
                    erode(net_mask.clone(), erroded_net_mask, errod_element);
                }
                mask.setTo(cv::GC_PR_FGD,
                           ((dilated_net_mask | (dilated_SkelBinMask) > 0)) & dilated_contour_mask);
                // The eroded net mask is a FG
                cv::Mat FrGdFeatureImage = (erroded_net_mask | SkelBinMask) & contour_mask;
                mask.setTo(cv::GC_FGD, FrGdFeatureImage > 0);
            } else { // front
                cv::Mat errod_element = getStructuringElement(cv::MORPH_RECT,
                                                              cv::Size(11, 11)); // 51,51
                erode(net_mask.clone(), erroded_net_mask, errod_element);
                cv::Mat FrGdFeatureImage = (dilated_net_mask | dilated_SkelBinMask) &
                                           dilated_contour_mask; // JointsContBinMask > 0;
                mask.setTo(cv::GC_PR_FGD, FrGdFeatureImage > 0);
                // The eroded net mask is a FG
                FrGdFeatureImage = (erroded_net_mask | SkelBinMask) & contour_mask;
                mask.setTo(cv::GC_FGD, FrGdFeatureImage > 0);
                // under arms is a BKGD, must be written here so arm itself will be included
                cv::Point P1 = joints[2];
                cv::Point P2 =
                        (0.85 * joints[2] + 1.2 * joints[3] + 0.95 * joints[8]) /
                        3; // underarm area
                cv::Point P3 = (joints[4] + joints[3] + joints[8]) / 3; // underarm area
                cv::line(mask, 0.4 * P1 + 0.6 * cv::Point(mask.cols, 1), cv::Point(mask.cols, 1),
                         cv::Scalar(cv::GC_BGD), 20); // just in case
                cv::line(mask, P2, P3, cv::Scalar(cv::GC_BGD), 1);
                P3 = (0.9 * joints[4] + 0.9 * joints[3] + 1.2 * joints[8]) / 3;
                cv::line(mask, P2, P3, cv::Scalar(cv::GC_BGD), 2);
                cv::line(mask, P3, cv::Point(1, 1280), cv::Scalar(cv::GC_BGD), 20);
                P1 = joints[5];
                P2 = (0.85 * joints[5] + 1.2 * joints[6] + 0.95 * joints[11]) / 3;
                P3 = (joints[7] + joints[6] + joints[11]) / 3;
                cv::line(mask, 0.4 * P1 + 0.6 * cv::Point(1, 1), cv::Point(1, 1),
                         cv::Scalar(cv::GC_BGD), 20); // just in case
                cv::line(mask, P2, P3, cv::Scalar(cv::GC_BGD), 1);
                P3 = (0.9 * joints[7] + 0.9 * joints[6] + 1.2 * joints[11]) / 3;
                cv::line(mask, P2, P3, cv::Scalar(cv::GC_BGD), 2);
                cv::line(mask, P3, cv::Point(720, 1280), cv::Scalar(cv::GC_BGD), 20);
                // between Knees and legs are a bkg
                float xknee_mid = 0.5 * (joints[9].x + joints[12].x);
                if (std::abs(xknee_mid - joints[0].x) >
                    100) { // in case POJO have wrongly got knee joints
                    xknee_mid = joints[0].x;
                }
                float d = 0.25 * (joints[10].y + joints[13].y -
                                  (joints[9].y + joints[12].y)); // 1/2 AVG LEG LENGTH
                float yknee_mid = (joints[9].y + joints[12].y) - d / 2; // above mid knee
                float x1 = 3;
                float x2 = 50;
                float y1 = yknee_mid - 0.7 * d;
                float y2 = mask.rows;
                for (int y = yknee_mid - 0.7 * d; y < mask.rows; y++) {
                    int dx = (y - y1) * (x2 - x1) / (y2 - y1) + x1;
                    for (int x = -dx; x < dx; x++) {
                        cv::circle(mask, cv::Point(xknee_mid + x, y), radius,
                                   cv::Scalar(cv::GC_BGD),
                                   thickness);
                    }
                }
                // mid hip to the knee
                P1 = (joints[8] + joints[11]) / 2.;
                P2 = (joints[9] + joints[12]) / 2.;
                P1 = (0.3 * P1 + 0.7 * P2);
                cv::line(mask, P1, P2, cv::Scalar(cv::GC_PR_BGD), 2);
                // remove more shadows
                int dist = 0.7 * (joints[1].y - joints[0].y);
                // around to arms
                P1 = joints[2] + cv::Point(-dist, 0);
                P2 = joints[3] + cv::Point(-dist, 0);
                P3 = joints[4] + cv::Point(-dist, 0);
                cv::line(mask, P1, P2, cv::Scalar(cv::GC_BGD), 4);
                cv::line(mask, P2, P3, cv::Scalar(cv::GC_BGD), 4);
                P2 = joints[3] + cv::Point(0, dist);
                P3 = joints[4] + cv::Point(dist, 0);
                cv::line(mask, P2, P3, cv::Scalar(cv::GC_BGD), 4);
                P1 = joints[5] + cv::Point(dist, 0);
                P2 = joints[6] + cv::Point(dist, 0);
                P3 = joints[7] + cv::Point(dist, 0);
                cv::line(mask, P1, P2, cv::Scalar(cv::GC_BGD), 4);
                cv::line(mask, P2, P3, cv::Scalar(cv::GC_BGD), 4);
                P2 = joints[6] + cv::Point(0, dist);
                P3 = joints[7] + cv::Point(-dist, 0);
                cv::line(mask, P2, P3, cv::Scalar(cv::GC_BGD), 4);
                P1 = joints[9] + cv::Point(-dist, 0);
                P2 = joints[10] + cv::Point(-dist, 0);
                cv::line(mask, P1, P2, cv::Scalar(cv::GC_BGD), 8);
                P1 = joints[12] + cv::Point(dist, 0);
                P2 = joints[13] + cv::Point(dist, 0);
                cv::line(mask, P1, P2, cv::Scalar(cv::GC_BGD), 8);
                // FG from joints (and over write under arms etc. BG too with 100 FG coming from skel)
                mask.setTo(cv::GC_FGD, SkelBinMask > 0);
            }
            if (type == BodyScanCommon::Profile::side) {
                // enforce BGD around knee joints
                float Jx = (joints[9].x + joints[12].x) / 2;
                float Jy = (joints[9].y + joints[12].y) / 2;
                for (int x = 0; x < 2 * face_rect.width; x++) {
                    for (int y = -(0.5 * face_rect.height); y < (0.5 * face_rect.height); y++) {
                        cv::circle(mask, cv::Point(Jx + face_rect.width + x, Jy + y), radius,
                                   cv::Scalar(cv::GC_BGD), thickness);
                        cv::circle(mask, cv::Point(Jx - face_rect.width - x, Jy + y), radius,
                                   cv::Scalar(cv::GC_BGD), thickness);
                        cv::line(mask, cv::Point(0, mask.rows / 2),
                                 cv::Point(Jx - face_rect.width - x, Jy + y),
                                 cv::Scalar(cv::GC_BGD),
                                 10);
                        cv::line(mask, cv::Point(0, mask.rows),
                                 cv::Point(Jx - face_rect.width - x, Jy + y),
                                 cv::Scalar(cv::GC_BGD),
                                 10);
                        cv::line(mask, cv::Point(mask.cols, mask.rows / 2),
                                 cv::Point(Jx + face_rect.width + x, Jy + y),
                                 cv::Scalar(cv::GC_BGD),
                                 10);
                        cv::line(mask, cv::Point(mask.cols, mask.rows),
                                 cv::Point(Jx + face_rect.width + x, Jy + y),
                                 cv::Scalar(cv::GC_BGD),
                                 10);
                    }
                }
            }
            // around the face is a BKGD
            for (int x = 0; x < mask.cols; x++) {
                for (int y = 0; y < joints[1].y; y++) {
                    if (y < 0.8 * joints[0].y) {
                        cv::circle(mask, cv::Point(x, y), radius, cv::Scalar(cv::GC_BGD),
                                   thickness);
                    }
                    if (type == BodyScanCommon::Profile::front) {
                        if (x < face_rect.x - 0.8 * face_rect.width ||
                            x > face_rect.x + 1.8 * face_rect.width) {
                            cv::circle(mask, cv::Point(x, y), radius, cv::Scalar(cv::GC_BGD),
                                       thickness);
                        }
                    } else {
                        if (x < (face_rect.x - 1.1 * face_rect.width) ||
                            x > (face_rect.x + 2.1 * face_rect.width)) {
                            cv::circle(mask, cv::Point(x, y), radius, cv::Scalar(cv::GC_BGD),
                                       thickness);
                        }
                    }
                }
            }
            // image borders are BKGD
            cv::line(mask, cv::Point(0, 0), cv::Point(mask.cols, 0), cv::Scalar(cv::GC_BGD), 20);
            cv::line(mask, cv::Point(0, 0), cv::Point(0, mask.rows), cv::Scalar(cv::GC_BGD), 20);
            cv::line(mask, cv::Point(mask.cols, mask.rows), cv::Point(mask.cols, 0),
                     cv::Scalar(cv::GC_BGD), 20);
            cv::line(mask, cv::Point(mask.cols, mask.rows), cv::Point(0, mask.rows),
                     cv::Scalar(cv::GC_BGD), 20);
            // below feet and above head is BKGD
            for (int x = 0; x < mask.cols; x++) {
                for (int y = 0; y < mask.rows; y++) {
                    if ((y > (std::max(joints[10].y, joints[13].y) + face_rect.height)) |
                        (y < (joints[0].y - 30))) {
                        cv::circle(mask, cv::Point(x, y), radius, cv::Scalar(cv::GC_BGD),
                                   thickness);
                    }
                }
            }
            mask.setTo(cv::GC_FGD, SkelBinMask > 0); // reinforce again
            // Actual start of feeding and calling grabcut
            cv::Mat inp_image = orig_image.clone();
            const cv::Mat *grabcut_image;
            grabcut_image = &inp_image;
            cv::Mat bgdModel, fgdModel;
            int N_iterations = 3;
            cv::grabCut(*grabcut_image, mask, cv::Rect(), bgdModel, fgdModel, N_iterations,
                        cv::GC_INIT_WITH_MASK);
            mask = (mask == 1) | (mask == 3); // FG or probably FG
            return mask; // &  (JointsContBinMask);
        } catch (cv::Exception &e) {
            AHILog(ANDROID_LOG_ERROR, "Exception error is : %s", e.what());
            return segment_dl_grabcut_with_contourmask(orig_image, type, net_mask, joints,
                                                       contour_mask); // use default
        }
    }

    cv::Mat joints_helper::segment_using_net_joints_and_grabcut(const cv::Mat &orig_image,
                                                                BodyScanCommon::Profile type,
                                                                const cv::Mat &net_mask,
                                                                const std::vector<cv::Point> &joints) {
        int radius = 2;
        int thickness = -1;
        try {
            // caution you may think it can be done if=n a loop but it is not.
            bool isJointsOrderCorrect =
                    (joints[0].y < joints[1].y) && (joints[1].y < joints[2].y) &&
                    (joints[2].y < joints[3].y) && (joints[3].y < joints[4].y) &&
                    (joints[1].y < joints[5].y) && (joints[5].y < joints[6].y) &&
                    (joints[6].y < joints[7].y) && (joints[8].y < joints[9].y) &&
                    (joints[9].y < joints[10].y) && (joints[11].y < joints[12].y) &&
                    (joints[12].y < joints[13].y);
            if (!isJointsOrderCorrect) {
                return segment_dl_grabcut(orig_image, type, net_mask, joints); // use default
            }
            for (int j = 0; j < joints.size(); j++) {
                if ((joints[j].x < 1) || (joints[j].x > orig_image.cols) || (joints[j].y < 1) ||
                    (joints[j].x > orig_image.rows)) {
                    return segment_dl_grabcut(orig_image, type, net_mask, joints); // use default
                }
            }
            cv::RNG &rng = cv::theRNG();
            rng.state = cv::getTickCount();
            // getting face from joints
            cv::Rect face_rect;
            face_rect.height = std::max(50.,
                                        (0.7 - 0.05 * (type == BodyScanCommon::Profile::side ? 1.0
                                                                                             : 0.0)) *
                                        (joints[1].y - joints[0].y));
            face_rect.width = face_rect.height;
            face_rect.x = std::max(joints[0].x - 20., joints[0].x - face_rect.height / 2.0 -
                                                      face_rect.height / 4.0 *
                                                      (type == BodyScanCommon::Profile::side ? 1.0
                                                                                             : 0.0));
            face_rect.y = 1.1 * joints[0].y * (type == BodyScanCommon::Profile::front ? 1.0 : 0.0) +
                          1.01 * joints[0].y * (type == BodyScanCommon::Profile::side ? 1.0 : 0.0);
            std::vector<cv::Point> connectivity = get_bone_connectivity();
            std::string error_id;
            // get a thick skeleton using the joints and a an ROI mask
            cv::Mat SkelBinMask = cv::Mat::zeros(orig_image.size(), 0);
            // This is to get 100% FGD feautres
            SkelBinMask = get_skel_mask_from_joints(SkelBinMask, joints, type, error_id);
            cv::Mat dilate_element_skel = cv::getStructuringElement(cv::MORPH_RECT,
                                                                    cv::Size(45, 45)); // 55,55
            cv::Mat dilated_SkelBinMask = cv::Mat::zeros(SkelBinMask.size(), SkelBinMask.type());
            cv::Mat dummy = dilated_SkelBinMask;
            if (type == BodyScanCommon::Profile::front) { // don't dilate the FG hand regions
                cv::Rect skel_rect;
                skel_rect.x = 0.9 * joints[2].x; // SkelBinMask.cols/3;
                skel_rect.width = 1.1 * joints[5].x - 0.9 * joints[2].x; // SkelBinMask.cols/3;
                skel_rect.y = 30;
                skel_rect.height = SkelBinMask.rows - 30; // skel_rect.height - 80;//60
                (SkelBinMask(skel_rect)).copyTo(dummy(skel_rect));
                cv::dilate(dummy, dilated_SkelBinMask, dilate_element_skel);
            } else {
                dilate(SkelBinMask, dilated_SkelBinMask, dilate_element_skel);
            }
            dummy.release();
            // net heatmap shall not be outside the likely position of the user PR_FGD
            cv::Mat dilated_net_mask = net_mask.clone();
            cv::Mat dilate_element = getStructuringElement(cv::MORPH_RECT, cv::Size(11, 11));
            cv::dilate(net_mask.clone(), dilated_net_mask, dilate_element);
            cv::Mat mask, erroded_net_mask;
            // BG features
            // set all BG first then overwrite FG and PR_FG later
            mask.create(net_mask.size(), CV_8UC1);
            mask.setTo(cv::Scalar::all(cv::GC_BGD));
            if (type == BodyScanCommon::Profile::side) {
                bool use_skel = true;
                bool clean_unmatched = false;
                if (!use_skel) {
                    SkelBinMask = cv::Mat::zeros(SkelBinMask.size(), SkelBinMask.type());
                }
                if (clean_unmatched) {
                    int L = 7;
                    cv::Mat errod_element = getStructuringElement(cv::MORPH_RECT, cv::Size(L, L));
                    erode(net_mask.clone(), erroded_net_mask, errod_element);
                    cv::Mat erroded_net_mask_Image;
                    orig_image.copyTo(erroded_net_mask_Image, erroded_net_mask);
                    erroded_net_mask = erroded_net_mask &
                                       match_images(erroded_net_mask_Image, SkelBinMask.clone(),
                                                    0.99);
                    cv::Mat dilated_net_mask_Image;
                    dilated_net_mask = dilated_net_mask | dilated_SkelBinMask;
                    orig_image.copyTo(dilated_net_mask_Image, dilated_net_mask);
                    dilated_net_mask = dilated_net_mask &
                                       match_images(dilated_net_mask_Image, SkelBinMask.clone(),
                                                    0.99);
                } else {
                    int L = 11;
                    cv::Mat errod_element = getStructuringElement(cv::MORPH_RECT, cv::Size(L, L));
                    erode(net_mask.clone(), erroded_net_mask, errod_element);
                }
                mask.setTo(cv::GC_PR_FGD, (dilated_net_mask | dilated_SkelBinMask) > 0);
                // The eroded net mask is a FG
                cv::Mat FrGdFeatureImage = (erroded_net_mask | SkelBinMask);
                mask.setTo(cv::GC_FGD, FrGdFeatureImage > 0);
            } else { // front
                cv::Mat errod_element = getStructuringElement(cv::MORPH_RECT,
                                                              cv::Size(11, 11)); // 51,51
                erode(net_mask.clone(), erroded_net_mask, errod_element);
                cv::Mat FrGdFeatureImage =
                        dilated_net_mask | dilated_SkelBinMask; // JointsContBinMask > 0;
                mask.setTo(cv::GC_PR_FGD, FrGdFeatureImage > 0);
                // The eroded net mask is a FG
                FrGdFeatureImage = (erroded_net_mask | SkelBinMask);
                mask.setTo(cv::GC_FGD, FrGdFeatureImage > 0);
                // under arms is a BKGD, must be written here so arm itself will be included
                cv::Point P1 = joints[2];
                cv::Point P2 =
                        (0.85 * joints[2] + 1.2 * joints[3] + 0.95 * joints[8]) /
                        3; // underarm area
                cv::Point P3 = (joints[4] + joints[3] + joints[8]) / 3; // underarm area
                cv::line(mask, 0.4 * P1 + 0.6 * cv::Point(mask.cols, 1), cv::Point(mask.cols, 1),
                         cv::Scalar(cv::GC_BGD), 20); // just in case
                cv::line(mask, P2, P3, cv::Scalar(cv::GC_BGD), 1);
                P3 = (0.9 * joints[4] + 0.9 * joints[3] + 1.2 * joints[8]) / 3;
                cv::line(mask, P2, P3, cv::Scalar(cv::GC_BGD), 2);
                cv::line(mask, P3, cv::Point(1, 1280), cv::Scalar(cv::GC_BGD), 20);
                P1 = joints[5];
                P2 = (0.85 * joints[5] + 1.2 * joints[6] + 0.95 * joints[11]) / 3;
                P3 = (joints[7] + joints[6] + joints[11]) / 3;
                cv::line(mask, 0.4 * P1 + 0.6 * cv::Point(1, 1), cv::Point(1, 1),
                         cv::Scalar(cv::GC_BGD), 20); // just in case
                cv::line(mask, P2, P3, cv::Scalar(cv::GC_BGD), 1);
                P3 = (0.9 * joints[7] + 0.9 * joints[6] + 1.2 * joints[11]) / 3;
                cv::line(mask, P2, P3, cv::Scalar(cv::GC_BGD), 2);
                cv::line(mask, P3, cv::Point(720, 1280), cv::Scalar(cv::GC_BGD), 20);
                // between Knees and legs are a bkg
                float xknee_mid = 0.5 * (joints[9].x + joints[12].x);
                if (std::abs(xknee_mid - joints[0].x) >
                    100) { // in case POJO have wrongly got knee joints
                    xknee_mid = joints[0].x;
                }
                float d = 0.25 * (joints[10].y + joints[13].y -
                                  (joints[9].y + joints[12].y)); // 1/2 AVG LEG LENGTH
                float yknee_mid = (joints[9].y + joints[12].y) - d / 2; // above mid knee
                float x1 = 3;
                float x2 = 50;
                float y1 = yknee_mid - 0.7 * d;
                float y2 = mask.rows;
                for (int y = yknee_mid - 0.7 * d; y < mask.rows; y++) {
                    int dx = (y - y1) * (x2 - x1) / (y2 - y1) + x1;
                    for (int x = -dx; x < dx; x++) {
                        cv::circle(mask, cv::Point(xknee_mid + x, y), radius,
                                   cv::Scalar(cv::GC_BGD),
                                   thickness);
                    }
                }
                // mid hip to the knee9
                P1 = (joints[8] + joints[11]) / 2.;
                P2 = (joints[9] + joints[12]) / 2.;
                P1 = (0.3 * P1 + 0.7 * P2);
                cv::line(mask, P1, P2, cv::Scalar(cv::GC_PR_BGD), 2);
                // remove more shadows
                int dist = 0.7 * (joints[1].y - joints[0].y);
                // around to arms
                P1 = joints[2] + cv::Point(-dist, 0);
                P2 = joints[3] + cv::Point(-dist, 0);
                P3 = joints[4] + cv::Point(-dist, 0);
                cv::line(mask, P1, P2, cv::Scalar(cv::GC_BGD), 4);
                cv::line(mask, P2, P3, cv::Scalar(cv::GC_BGD), 4);
                P2 = joints[3] + cv::Point(0, dist);
                P3 = joints[4] + cv::Point(dist, 0);
                cv::line(mask, P2, P3, cv::Scalar(cv::GC_BGD), 4);
                P1 = joints[5] + cv::Point(dist, 0);
                P2 = joints[6] + cv::Point(dist, 0);
                P3 = joints[7] + cv::Point(dist, 0);
                cv::line(mask, P1, P2, cv::Scalar(cv::GC_BGD), 4);
                cv::line(mask, P2, P3, cv::Scalar(cv::GC_BGD), 4);
                P2 = joints[6] + cv::Point(0, dist);
                P3 = joints[7] + cv::Point(-dist, 0);
                cv::line(mask, P2, P3, cv::Scalar(cv::GC_BGD), 4);
                P1 = joints[9] + cv::Point(-dist, 0);
                P2 = joints[10] + cv::Point(-dist, 0);
                cv::line(mask, P1, P2, cv::Scalar(cv::GC_BGD), 8);
                P1 = joints[12] + cv::Point(dist, 0);
                P2 = joints[13] + cv::Point(dist, 0);
                cv::line(mask, P1, P2, cv::Scalar(cv::GC_BGD), 8);
                // FG from joints (and over write under arms etc. BG too with 100 FG coming from skel)
                mask.setTo(cv::GC_FGD, SkelBinMask > 0);
            }
            if (type == BodyScanCommon::Profile::side) {
                //enforce BGD around knee joints
                float Jx = (joints[9].x + joints[12].x) / 2;
                float Jy = (joints[9].y + joints[12].y) / 2;
                for (int x = 0; x < 2 * face_rect.width; x++) {
                    for (int y = -(0.5 * face_rect.height); y < (0.5 * face_rect.height); y++) {
                        cv::circle(mask, cv::Point(Jx + face_rect.width + x, Jy + y), radius,
                                   cv::Scalar(cv::GC_BGD), thickness);
                        cv::circle(mask, cv::Point(Jx - face_rect.width - x, Jy + y), radius,
                                   cv::Scalar(cv::GC_BGD), thickness);
                        cv::line(mask, cv::Point(0, mask.rows / 2),
                                 cv::Point(Jx - face_rect.width - x, Jy + y),
                                 cv::Scalar(cv::GC_BGD),
                                 10);
                        cv::line(mask, cv::Point(0, mask.rows),
                                 cv::Point(Jx - face_rect.width - x, Jy + y),
                                 cv::Scalar(cv::GC_BGD),
                                 10);
                        cv::line(mask, cv::Point(mask.cols, mask.rows / 2),
                                 cv::Point(Jx + face_rect.width + x, Jy + y),
                                 cv::Scalar(cv::GC_BGD),
                                 10);
                        cv::line(mask, cv::Point(mask.cols, mask.rows),
                                 cv::Point(Jx + face_rect.width + x, Jy + y),
                                 cv::Scalar(cv::GC_BGD),
                                 10);
                    }
                }
            }
            // around the face is a BKGD
            for (int x = 0; x < mask.cols; x++) {
                for (int y = 0; y < joints[1].y; y++) {
                    if (y < 0.8 * joints[0].y) {
                        cv::circle(mask, cv::Point(x, y), radius, cv::Scalar(cv::GC_BGD),
                                   thickness);
                    }
                    if (type == BodyScanCommon::Profile::front) {
                        if (x < face_rect.x - 0.8 * face_rect.width ||
                            x > face_rect.x + 1.8 * face_rect.width) {
                            cv::circle(mask, cv::Point(x, y), radius, cv::Scalar(cv::GC_BGD),
                                       thickness);
                        }
                    } else {
                        if (x < (face_rect.x - 1.1 * face_rect.width) ||
                            x > (face_rect.x + 2.1 * face_rect.width)) {
                            cv::circle(mask, cv::Point(x, y), radius, cv::Scalar(cv::GC_BGD),
                                       thickness);
                        }
                    }
                }
            }
            // image borders are BKGD
            cv::line(mask, cv::Point(0, 0), cv::Point(mask.cols, 0), cv::Scalar(cv::GC_BGD), 20);
            cv::line(mask, cv::Point(0, 0), cv::Point(0, mask.rows), cv::Scalar(cv::GC_BGD), 20);
            cv::line(mask, cv::Point(mask.cols, mask.rows), cv::Point(mask.cols, 0),
                     cv::Scalar(cv::GC_BGD), 20);
            cv::line(mask, cv::Point(mask.cols, mask.rows), cv::Point(0, mask.rows),
                     cv::Scalar(cv::GC_BGD), 20);
            // below feet and above head is BKGD
            for (int x = 0; x < mask.cols; x++) {
                for (int y = 0; y < mask.rows; y++) {
                    if ((y > (std::max(joints[10].y, joints[13].y) + face_rect.height)) |
                        (y < (joints[0].y - 30))) {
                        cv::circle(mask, cv::Point(x, y), radius, cv::Scalar(cv::GC_BGD),
                                   thickness);
                    }
                }
            }
            mask.setTo(cv::GC_FGD, SkelBinMask > 0); // reinforce again
            // Actual start of feeding and calling grabcut
            cv::Mat inp_image = orig_image.clone();
            const cv::Mat *grabcut_image;
            grabcut_image = &inp_image;
            cv::Mat bgdModel, fgdModel;
            int N_iterations = 3;
            cv::grabCut(*grabcut_image, mask, cv::Rect(), bgdModel, fgdModel, N_iterations,
                        cv::GC_INIT_WITH_MASK);
            mask = (mask == 1) | (mask == 3); // FG or probably FG
            return mask; // &  (JointsContBinMask);
        } catch (cv::Exception &e) {
            AHILog(ANDROID_LOG_ERROR, "Exception error is : %s", e.what());
            return segment_dl_grabcut(orig_image, type, net_mask, joints); // use default
        }
    }

// PRIVATE
    cv::Mat joints_helper::Hist_and_Backproj(const cv::Mat &hsv, const cv::Mat &mask) {
        cv::Mat hist;
        int h_bins = 30;
        int s_bins = 32;
        int histSize[] = {h_bins, s_bins};
        float h_range[] = {0, 180};
        float s_range[] = {0, 256};
        const float *ranges[] = {h_range, s_range};
        int channels[] = {0, 1};
        // Get the Histogram and normalize it
        cv::calcHist(&hsv, 1, channels, mask, hist, 2, histSize, ranges, true, false);
        cv::normalize(hist, hist, 0, 255, cv::NORM_MINMAX, -1, cv::Mat());
        // Get Backprojection
        cv::Mat backproj;
        cv::calcBackProject(&hsv, 1, channels, hist, backproj, ranges, 1, true);
        return backproj;
    }

    cv::Mat joints_helper::match_images(const cv::Mat &src_image, const cv::Mat &templ_base_mask,
                                        double thrld) {
        std::vector<cv::Point2i> Points;
        cv::findNonZero(templ_base_mask, Points);
        cv::Rect ROI = cv::boundingRect(Points);
        int offset = 30;
        ROI.x = ROI.x + offset;
        ROI.width = ROI.width - 2 * offset;
        ROI.y = ROI.y - offset;
        ROI.height = ROI.height - offset;
        cv::Mat base_mask = cv::Mat::zeros(templ_base_mask.size(), templ_base_mask.type());
        templ_base_mask(ROI).copyTo(base_mask(ROI));
        cv::Mat src = src_image.clone();
        cv::Mat base;
        src_image.copyTo(base, base_mask);
        cv::Mat backproj, src_hsv, base_hsv, base_hist, mask;
        GaussianBlur(src, src, cv::Size(5, 5), 1.5, 1.5); // remove noise
        GaussianBlur(base, base, cv::Size(5, 5), 1.5, 1.5); // remove noise
        cvtColor(base.clone(), base_hsv, cv::COLOR_BGR2HSV);
        cvtColor(src.clone(), src_hsv, cv::COLOR_BGR2HSV);
        cv::Mat base_hsv_channels[3];
        cv::split(base_hsv, base_hsv_channels);
        cv::Point minLoc, maxLoc;
        double minVal, maxVal;
        float smin = 2 - thrld;
        float smax = thrld;
        cv::minMaxLoc(base_hsv_channels[0], &minVal, &maxVal, &minLoc, &maxLoc);
        float h_ranges[] = {smin * float(minVal), smax * float(maxVal)};
        cv::minMaxLoc(base_hsv_channels[1], &minVal, &maxVal, &minLoc, &maxLoc);
        float s_ranges[] = {smin * float(minVal), smax * float(maxVal)};
        cv::minMaxLoc(base_hsv_channels[2], &minVal, &maxVal, &minLoc, &maxLoc);
        float v_ranges[] = {smin * float(minVal), smax * float(maxVal)};
        const float *ranges[] = {h_ranges, s_ranges, v_ranges};
        int h_bins = 32;
        int s_bins = 32;
        int v_bins = 32;
        int histSize[] = {h_bins, s_bins, v_bins};
        int channels[] = {0, 1, 2};
        calcHist(&src_hsv, 1, channels, base_mask, base_hist, 3, histSize, ranges, true, false);
        // if we have a separate base image (not a portion or masked part the src image) then:
        bool use_HS = false;
        bool use_HSV = true;
        if (use_HS) {
            backproj = Hist_and_Backproj(src_hsv, base_mask);
        } else if (use_HSV) {
            cv::calcBackProject(&src_hsv, 1, channels, base_hist, backproj, ranges, 1.0, true);
        } else {
            backproj = cv::Mat();
        }
        cv::inRange(backproj, cv::Scalar(1, 1, 1), cv::Scalar(255, 255, 255), backproj);
        cv::Mat base_image;
        src_image.copyTo(base_image, base_mask);
        cv::Mat matched;
        src_image.copyTo(matched, backproj);
        return backproj;
    }

    std::vector<cv::Point> joints_helper::get_bone_connectivity(void) {
        std::vector<cv::Point> connectivity;
        connectivity.push_back(cv::Point(0, 1)); //0
        connectivity.push_back(cv::Point(1, 2)); //1
        connectivity.push_back(cv::Point(1, 8)); //2
        connectivity.push_back(cv::Point(1, 11)); //3
        connectivity.push_back(cv::Point(2, 3)); //4
        connectivity.push_back(cv::Point(2, 11)); //5
        connectivity.push_back(cv::Point(3, 4)); //6
        connectivity.push_back(cv::Point(1, 5)); //7
        connectivity.push_back(cv::Point(5, 6)); //8
        connectivity.push_back(cv::Point(5, 8)); //9
        connectivity.push_back(cv::Point(6, 7)); //10
        connectivity.push_back(cv::Point(2, 8)); //11
        connectivity.push_back(cv::Point(8, 9)); //12
        connectivity.push_back(cv::Point(9, 10)); //13
        connectivity.push_back(cv::Point(5, 11)); //14
        connectivity.push_back(cv::Point(11, 12)); //15
        connectivity.push_back(cv::Point(12, 13)); //16
        connectivity.push_back(cv::Point(8, 11)); //17
        return connectivity;
    }

    std::vector<std::string> joints_helper::get_joint_names(void) {
        std::vector<std::string> joint_names;
        joint_names.push_back("Head"); // 0
        joint_names.push_back("Neck"); // 1
        joint_names.push_back("RightShoulder"); // 2
        joint_names.push_back("RightElbow"); // 3
        joint_names.push_back("RightWrist"); // 4
        joint_names.push_back("LeftShoulder"); // 5
        joint_names.push_back("LeftElbow"); // 6
        joint_names.push_back("LeftWrist"); // 7
        joint_names.push_back("RightHip"); // 8
        joint_names.push_back("RightKnee"); // 9
        joint_names.push_back("RightAnkle"); // 10
        joint_names.push_back("LeftHip"); // 11
        joint_names.push_back("LeftKnee"); // 12
        joint_names.push_back("LeftAnkle"); // 13
        joint_names.push_back("BackGround"); // 14
        return joint_names;
    }

    void joints_helper::fix_joint_x_positions(std::vector<cv::Point> &joints, int m, int n) {
        cv::Point Pm, Pn;
        // n supposed to have .x larger than m
        Pm = joints[m];
        Pn = joints[n];
        if (Pm.x < 10) {
            Pm.x = joints[0].x -
                   100; // 100 is just a quick fix as the joints (POJO) didn't detect the joint
        }
        if (Pn.x < 10) {
            Pn.x = joints[0].x +
                   100; // 100 is just a quick fix as the joints (POJO) didn't detect the joint
        }
        joints[m] = Pm;
        joints[n] = Pn;
        // fix x position atm
        if (joints[m].x > joints[0].x) {
            Pm.x = 2 * joints[0].x - joints[n].x;
        }
        // fix x position atm
        if (joints[n].x < joints[0].x) {
            Pn.x = 2 * joints[0].x - joints[m].x;
        }
        joints[m] = Pm;
        joints[n] = Pn;
    }

    cv::Mat
    joints_helper::get_skel_mask_from_joints(const cv::Mat &sbinMask, std::vector<cv::Point> joints,
                                             BodyScanCommon::Profile type, std::string &error_id) {
        try {
            cv::Mat SkelBinMask = sbinMask.clone();
            if (SkelBinMask.channels() == 3 && !SkelBinMask.empty()) {
                cv::cvtColor(SkelBinMask, SkelBinMask, cv::COLOR_BGR2GRAY);
            }
            std::vector<cv::Point> connectivity = get_bone_connectivity();
            // tune head a bit down
            joints[0].y = (0.75 * joints[0].y + 0.25 * joints[1].y);
            // neck a bit down
            joints[1].y = (0.5 * joints[1].y + 0.25 * joints[2].y + 0.25 * joints[5].y);
            cv::line(SkelBinMask, joints[0], joints[1], cv::Scalar(255), 20, 8);
            // before shoulder tune up
            cv::circle(SkelBinMask, cv::Point((int) joints[2].x, (int) joints[2].y), 8,
                       cv::Scalar(255),
                       -1);
            cv::circle(SkelBinMask, cv::Point((int) joints[5].x, (int) joints[5].y), 8,
                       cv::Scalar(255),
                       -1);
            cv::line(SkelBinMask, joints[2], joints[3], cv::Scalar(255), 8, 6);
            cv::line(SkelBinMask, joints[5], joints[6], cv::Scalar(255), 8, 6);
            cv::line(SkelBinMask, joints[8], joints[9], cv::Scalar(255), 30, 6);
            cv::line(SkelBinMask, joints[11], joints[12], cv::Scalar(255), 30, 6);
            // tune shoulders
            joints[2].x = (0.6 * joints[2].x +
                           0.4 * joints[1].x);
            joints[2].y = (0.5 * joints[2].y + 0.5 * joints[1].y);
            joints[5].x = (0.6 * joints[5].x +
                           0.4 * joints[1].x);
            joints[5].y = (0.5 * joints[5].y + 0.5 * joints[1].y);
            // tune hips
            joints[8].x = (0.9 * joints[8].x + 0.1 * joints[1].x);
            joints[11].x = (0.9 * joints[11].x + 0.1 * joints[1].x);
            if (type == BodyScanCommon::Profile::side) {
                joints[9].x = joints[9].x - 15;
                joints[12].x = joints[12].x - 15;
            }
            int h = std::max(10, joints[1].y - joints[0].y);
            int line_width = 20;
            line_width = 16;
            if (type == BodyScanCommon::Profile::side) {
                line_width = std::max(16, h / 8);
            }
            for (int bone = 0; bone < (int) connectivity.size(); bone++) {
                line(SkelBinMask, cv::Point((int) joints[connectivity[bone].x].x,
                                            (int) joints[connectivity[bone].x].y),
                     cv::Point((int) joints[connectivity[bone].y].x,
                               (int) joints[connectivity[bone].y].y), cv::Scalar(255),
                     0.8 * line_width,
                     8);
            }
            if (type == BodyScanCommon::Profile::front) {
                line(SkelBinMask, cv::Point((int) joints[4].x, (int) joints[4].y),
                     cv::Point((int) joints[4].x - 10, (int) joints[4].y + 10), cv::Scalar(255),
                     h / 8,
                     8);
                line(SkelBinMask, cv::Point((int) joints[4].x, (int) joints[4].y),
                     cv::Point((int) joints[4].x - 10, (int) joints[4].y + 10), cv::Scalar(255),
                     h / 8,
                     8);
                line(SkelBinMask, cv::Point((int) joints[10].x, (int) joints[10].y),
                     cv::Point((int) joints[10].x - 15, (int) joints[10].y + 60), cv::Scalar(255),
                     h / 10, 8);
                line(SkelBinMask, cv::Point((int) joints[13].x, (int) joints[13].y),
                     cv::Point((int) joints[13].x + 15, (int) joints[13].y + 60), cv::Scalar(255),
                     h / 10, 8);
                circle(SkelBinMask, cv::Point((int) joints[8].x, (int) joints[8].y), h / 6,
                       cv::Scalar(255), -1);
                circle(SkelBinMask, cv::Point((int) joints[11].x, (int) joints[11].y), h / 6,
                       cv::Scalar(255), -1);
            }
            if (type == BodyScanCommon::Profile::side) {
                cv::line(SkelBinMask, joints[1], 0.5 * (joints[8] + joints[11]), cv::Scalar(255),
                         20,
                         8);
                int face_height = joints[1].y - joints[0].y;
                cv::Point P1 = cv::Point(joints[1].x, joints[1].y + 40); // just below neck
                cv::Point P2 = (0.35 * joints[1] +
                                0.65 * (joints[8] + joints[11]) /
                                2.); // upper center (trunk center)
                int tx = face_height / 2.7;
                P2.x = P2.x + tx;
                cv::line(SkelBinMask, P1, P2, cv::Scalar(255), 4, 8);
                cv::line(SkelBinMask, joints[8], P2, cv::Scalar(255), 4, 8);
                P2.x = P2.x - tx - tx / 2.;
                cv::line(SkelBinMask, cv::Point((int) joints[2].x, (int) joints[2].y),
                         cv::Point((int) joints[8].x, (int) joints[8].y), cv::Scalar(255), h / 4,
                         8);
                cv::line(SkelBinMask, cv::Point((int) joints[5].x, (int) joints[5].y),
                         cv::Point((int) joints[11].x, (int) joints[11].y), cv::Scalar(255), h / 4,
                         8);
                cv::circle(SkelBinMask, cv::Point((int) joints[8].x, (int) joints[8].y), h / 4,
                           cv::Scalar(255), -1);
                cv::circle(SkelBinMask, cv::Point((int) joints[11].x, (int) joints[11].y), h / 4,
                           cv::Scalar(255), -1);
                cv::line(SkelBinMask, cv::Point((int) joints[10].x, (int) joints[10].y),
                         cv::Point((int) joints[10].x + 15, (int) joints[10].y + 5),
                         cv::Scalar(255),
                         h / 16, 8);
                cv::line(SkelBinMask, cv::Point((int) joints[13].x, (int) joints[13].y),
                         cv::Point((int) joints[13].x + 15, (int) joints[13].y + 5),
                         cv::Scalar(255),
                         h / 16, 8);
            }
            // thigh area
            cv::line(SkelBinMask, cv::Point((int) joints[8].x, (int) joints[8].y),
                     cv::Point((int) joints[9].x, (int) joints[9].y), cv::Scalar(255), line_width,
                     8);
            cv::line(SkelBinMask, cv::Point((int) joints[11].x, (int) joints[11].y),
                     cv::Point((int) joints[12].x, (int) joints[12].y), cv::Scalar(255), line_width,
                     8);
            // filling the mask
            std::vector<std::vector<cv::Point>> contours;
            cv::findContours(SkelBinMask.clone(), contours, cv::RETR_EXTERNAL,
                             cv::CHAIN_APPROX_SIMPLE);
            if ((int) contours.size() < 1) {
                error_id = "11";
                return cv::Mat();
            }
            for (int i = 0; i < (int) contours.size(); i++) {
                cv::drawContours(SkelBinMask, contours, i, cv::Scalar(255), cv::FILLED);
            }
            return SkelBinMask;
        } catch (cv::Exception &e) {
            error_id = e.what();
            return sbinMask;
        }
    }

    cv::Mat
    joints_helper::segment_dl_grabcut(const cv::Mat &orig_image, BodyScanCommon::Profile type,
                                      const cv::Mat &net_mask,
                                      const std::vector<cv::Point> &joints) {
        // with DL+grabCut
        cv::Mat mask;
        mask.create(net_mask.size(), CV_8UC1);
        mask.setTo(cv::Scalar::all(cv::GC_BGD)); // Set "background" as default guess
        cv::Mat dilate_element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(11, 11));
        cv::Mat dilated_net_mask = cv::Mat::zeros(mask.size(), mask.type());
        cv::dilate(net_mask, dilated_net_mask, dilate_element);
        mask.setTo(cv::GC_PR_FGD, dilated_net_mask > 0); // Relax this to "probably Foregrounds"
        cv::Mat eroded_net_mask = cv::Mat::zeros(mask.size(), mask.type());
        cv::Mat erode_element = getStructuringElement(cv::MORPH_RECT, cv::Size(11, 11));
        cv::erode(net_mask, eroded_net_mask, erode_element);
        mask.setTo(cv::GC_FGD, eroded_net_mask > 0); // Set pixels to "foreground"
        // image borders are BKGD
        cv::line(mask, cv::Point(0, 0), cv::Point(mask.cols, 0), cv::Scalar(cv::GC_BGD), 30);
        cv::line(mask, cv::Point(0, 0), cv::Point(0, mask.rows), cv::Scalar(cv::GC_BGD), 30);
        cv::line(mask, cv::Point(mask.cols, mask.rows), cv::Point(mask.cols, 0),
                 cv::Scalar(cv::GC_BGD),
                 30);
        cv::line(mask, cv::Point(mask.cols, mask.rows), cv::Point(0, mask.rows),
                 cv::Scalar(cv::GC_BGD),
                 30);
        cv::Mat inp_image = orig_image.clone();
        const cv::Mat *grabcut_image;
        grabcut_image = &inp_image;
        cv::Mat bgdModel, fgdModel;
        cv::grabCut(*grabcut_image, mask, cv::Rect(), bgdModel, fgdModel, 3, cv::GC_INIT_WITH_MASK);
        mask = (mask == cv::GC_FGD) | (mask == cv::GC_PR_FGD);
        return mask;
    }

    cv::Mat
    joints_helper::draw_image_with_joints(cv::Mat inp_image, std::vector<cv::Point> joints) {
        int bone_links[14][2] = {{0,  1},
                                 {1,  2},
                                 {2,  3},
                                 {3,  4},
                                 {1,  5},
                                 {5,  6},
                                 {6,  7},
                                 {2,  8},
                                 {8,  9},
                                 {9,  10},
                                 {5,  11},
                                 {11, 12},
                                 {12, 13},
                                 {8,  11}};
        for (int j = 0; j < (int) joints.size(); j++) {
            std::vector<std::string> part_names = get_joint_names();
            cv::circle(inp_image, cv::Point(joints[j].x, joints[j].y), 10 * inp_image.rows / 1860,
                       cv::Scalar(255, 0, 0), -1);
            cv::putText(inp_image, part_names[j], cv::Point(1.02 * joints[j].x, 1.02 * joints[j].y),
                        cv::FONT_HERSHEY_COMPLEX_SMALL, 1.5 * inp_image.rows / 1860,
                        cv::Scalar(255, 155, 15), 1, 0);
            std::stringstream ss;
            ss << j;
            cv::putText(inp_image, ss.str(), cv::Point(0.98 * joints[j].x, 0.98 * joints[j].y),
                        cv::FONT_HERSHEY_COMPLEX_SMALL, 4 * inp_image.rows / 1860,
                        cv::Scalar(255, 0, 0), 1, 0);
            cv::line(inp_image, cv::Point(joints[bone_links[j][0]].x, joints[bone_links[j][0]].y),
                     cv::Point(joints[bone_links[j][1]].x, joints[bone_links[j][1]].y),
                     cv::Scalar(0, 0, 255), 6);
            cv::circle(inp_image, cv::Point(joints[j].x, joints[j].y), 10 * inp_image.rows / 1860,
                       cv::Scalar(255, 0, 0), -1);
        }
        return inp_image;
    }
}
