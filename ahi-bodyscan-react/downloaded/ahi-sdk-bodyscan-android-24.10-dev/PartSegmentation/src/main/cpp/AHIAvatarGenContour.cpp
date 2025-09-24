//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#include "AHIAvatarGenContour.hpp"

#include "AHIAvatarGenSegmentAndAuto.hpp"

namespace ahi_avatar_gen {

// PUBLIC
    contour::contour(void) {
    }

    void
    contour::inspect(const cv::Mat &src, const cv::Mat &mask, BodyScanCommon::Profile view,
                     int x_offset, int y_offset,
                     std::string &ERROR_IDs_as_json) {
        cv::Mat image;
        ERROR_IDs_as_json = "";
        std::string ERROR_IDs;
        std::string error_id;
        std::string generic_error = "{\"GE\": \"1\"}";
        if (src.empty() || mask.empty() || src.rows != mask.rows || src.cols != mask.cols) {
            ERROR_IDs_as_json = generic_error;
            return;
        }
        segment_auto seg;
        try {
            std::string HSV_or_HS = "HSV";
            // mask the image with the contour mask
            threshold(mask, mask, 210, 255, cv::THRESH_BINARY);
            cv::Mat filled_mask = seg.fillHoles(mask, error_id);
            if (error_id.size() > 0) {
                ERROR_IDs = ERROR_IDs + ", " + error_id;
                ERROR_IDs_as_json = generic_error;
                return;
            }
            cvtColor(src.clone(), image, cv::COLOR_RGB2BGR);
            image = seg.mask_color_image(image, filled_mask, error_id);
            if (error_id.size() > 0) {
                ERROR_IDs = ERROR_IDs + ", " + error_id;
                ERROR_IDs_as_json = generic_error;
                return;
            }
            if (error_id.size() > 0) {
                ERROR_IDs = ERROR_IDs + ", " + error_id;
                ERROR_IDs_as_json = generic_error;
                return;
            }
            // detect face
            std::vector<cv::Rect> detected_face_rect;
            cv::Rect face_rect;
            cv::Rect roi;
            roi.x = 1;
            roi.y = 1;
            roi.height = (int) (float(image.rows) / 3.0);
            roi.width = image.cols - roi.x;
            std::string subject;
            if (view == BodyScanCommon::Profile::front) {
                subject = "frontal_face_haar_alt2";
            } else {
                subject = "side_face_haar";
            }
            // start face detection
            cv::Mat image_roi = image(roi);
            seg.detect(image_roi, subject, (int) (image.rows / 20), (int) image.cols / 2,
                       detected_face_rect, error_id);
            if (detected_face_rect.size() > 1) {
                error_id = "FM"; // more than one face
                ERROR_IDs = ERROR_IDs + ", " + error_id;// ("FM");
                ERROR_IDs_as_json = "{\"Face\": \"2\"}";
                return;
            } else if ((int) detected_face_rect.size() <
                       1) { // no face, flip the image and try again
                flip(image_roi, image_roi, 1);
                seg.detect(image_roi, subject, (int) (image.cols / 20), (int) image.cols / 2,
                           detected_face_rect, error_id);
                flip(image_roi, image_roi, 1);
                if (detected_face_rect.size() > 1) {
                    error_id = "FM"; // more than one face
                    ERROR_IDs = ERROR_IDs + ", " + error_id;// ("FM");
                    ERROR_IDs_as_json = "{\"Face\": \"2\"}";
                    return;
                } else if (detected_face_rect.size() < 1) {
                    if (subject == "side_face_haar") {
                        subject = "frontal_face_haar_alt2";
                    } else {
                        subject = "side_face_haar";
                    }
                    seg.detect(image_roi, subject, (int) (image.cols / 20), (int) image.cols / 2,
                               detected_face_rect, error_id);
                    if (detected_face_rect.size() > 1) {
                        error_id = "FM"; // more than one face
                        ERROR_IDs = ERROR_IDs + ", " + error_id;// ("FM");
                        ERROR_IDs_as_json = "{\"Face\": \"2\"}";
                        return;
                    } else if ((int) detected_face_rect.size() < 1) {
                        error_id = "FN";
                        ERROR_IDs = ERROR_IDs + ", " + error_id;// ("FN");
                        ERROR_IDs_as_json = "{\"Face\": \"0\"}";
                        return;
                    } else { // a single face
                        face_rect = detected_face_rect[0];
                    }
                } else { // if we reach here that means a face is detected
                    detected_face_rect[0].x =
                            image.cols - (detected_face_rect[0].x + detected_face_rect[0].width);
                    face_rect = detected_face_rect[0];
                }
            } else { // if we reach here that means a face is detected
                face_rect = detected_face_rect[0];
            }
            ERROR_IDs = ERROR_IDs + ", " + "FD1";
            int face_x_center = face_rect.x + (int) (face_rect.width / 2.);
            int face_y_center = face_rect.y + (int) (face_rect.height / 2.);
            cv::Rect face_rect_cropped;
            face_rect_cropped.x = face_x_center - face_rect.width / 4;
            face_rect_cropped.y = face_y_center - face_rect.height / 3;
            face_rect_cropped.width = face_rect.width / 2;
            face_rect_cropped.height = (int) (face_rect.height * (2. / 3.));
            // make the height and width even for MatchTemp
            face_rect_cropped.width = 2 * int(face_rect_cropped.width / 2);
            face_rect_cropped.height = 2 * int(face_rect_cropped.height / 2);
            cv::Mat FaceMask = image(face_rect_cropped);
            // mask contour points again unless the shifts/offsets are given
            std::vector<cv::Point2i> ContourPoints;
            seg.findNonZero_BugFree(mask, ContourPoints);
            if (ContourPoints.size() < 1) {
                error_id = "GE1";
                ERROR_IDs = ERROR_IDs + ", " + error_id;
                ERROR_IDs_as_json = generic_error;
                return;
            }
            int Xl, Yl, Xr, Yr, Xt, Yt, Xb, Yb, Xbl, Xbr; // left right top bottom etc.
            int LargeValue = 2 * std::max(mask.rows, mask.cols);
            Xl = 0;
            Yl = 0;
            Xr = LargeValue;
            Yr = 0;
            Xb = 0;
            Yb = 0;
            Xt = 0;
            Yt = LargeValue;
            Xb = 0;
            Yb = 0;
            Xbl = 0;
            Xbr = LargeValue;
            int Xc = 0; // center
            int Yc = 0; // center
            // ordered conexhull points
            for (int i = 0; i < (int) ContourPoints.size(); i++) {
                Xc = Xc + ContourPoints[i].x;
                Yc = Yc + ContourPoints[i].y;
                Xl = std::max(Xl, ContourPoints[i].x);
                if (Xl == ContourPoints[i].x) {
                    Yl = ContourPoints[i].y;
                }
                Xr = std::min(Xr, ContourPoints[i].x);
                if (Xr == ContourPoints[i].x) {
                    Yr = ContourPoints[i].y;
                }
                Yt = std::min(Yt, ContourPoints[i].y);
                if (Yt == ContourPoints[i].y) {
                    Xt = ContourPoints[i].x;
                }
                Yb = std::max(Yb, ContourPoints[i].y);
                if (Yb == ContourPoints[i].y) {
                    Xb = ContourPoints[i].x;
                }
                if (ContourPoints[i].y > (int) (0.9 * Yb)) {
                    Xbr = std::min(Xbr,
                                   ContourPoints[i].x); // right foot (front image), or right outmost point of side image
                    Xbl = std::max(Xbl,
                                   ContourPoints[i].x); // left foot (front image), or left outmost  point of the side image
                }
            }
            Xc = Xc / ContourPoints.size();
            Yc = Yc / ContourPoints.size();
            cv::Mat skin_final = seg.segment_skin_specific(FaceMask, image, HSV_or_HS, error_id);
            if (error_id.size() > 0) {
                ERROR_IDs = ERROR_IDs + ", " + error_id;
                ERROR_IDs_as_json = generic_error;
                return;
            }
            cv::Mat Temp_mask = cv::Mat::zeros(image.rows, image.cols, 0);
            Temp_mask = match_with_template(FaceMask, image, 0);
            bitwise_or(skin_final, Temp_mask, skin_final);
            threshold(skin_final, skin_final, 100, 255, cv::THRESH_BINARY);
            // accept/reject the face and check if the face (user) is too close or too far. then check BG of the entire image
            cv::Scalar SkinBlob;
            cv::Scalar MaskBlob;
            SkinBlob = sum(skin_final);
            MaskBlob = sum(filled_mask);
            if (float(SkinBlob.val[0]) > 0.98 * float(MaskBlob.val[0])) {
                ERROR_IDs = ERROR_IDs + ", " + "BG0";
            }
            // face is too far?
            if (view == BodyScanCommon::Profile::front) {
                // hands area
                // RH area
                roi.x = Xr;
                roi.width = 30;
                roi.y = Yr - 30;
                roi.height = 30;
                SkinBlob = sum(skin_final(roi));
                MaskBlob = sum(filled_mask(roi));
                // LH area
                roi.x = Xl - 30;
                roi.width = 30;
                roi.y = Yr - 30;
                roi.height = 30;
                SkinBlob = SkinBlob + sum(skin_final(roi));
                MaskBlob = MaskBlob + sum(filled_mask(roi));
                // feet area
                roi.x = Xbr;
                roi.width = Xbl - Xbr;
                roi.y = int(0.95 * Yb);
                roi.height = Yb - roi.y;
                SkinBlob = SkinBlob + sum(skin_final(roi));
                MaskBlob = MaskBlob + sum(filled_mask(roi));
            } else {
                // feet area
                roi.x = Xbr;
                roi.width = Xbl - Xbr;
                roi.y = int(0.95 * Yb);
                roi.height = Yb - roi.y;
                SkinBlob = sum(skin_final(roi));
                MaskBlob = sum(filled_mask(roi));
            }
            if (face_rect.height < 20 || face_rect.width < 20 ||
                float(SkinBlob.val[0]) < 0.02 * float(MaskBlob.val[0])) {
                ERROR_IDs = ERROR_IDs + ", " + "DP2"; // Face Too Far
            }
            // Face is too close?
            // not done yet
            if (view == BodyScanCommon::Profile::front) {
                float th_low = 0.05;
                float th_high = 0.95;
                // right arm
                roi.x = Xr;// max(1,Xr-10);
                roi.width = int(1.1 * face_rect.width);
                roi.y = std::max(1, Yr - int(1.5 * face_rect.height));
                roi.height = std::min(int(1.5 * face_rect.height), skin_final.rows - roi.y - 1);
                if (roi.x > 0 && roi.x + roi.width <= skin_final.cols && roi.y > 0 &&
                    roi.y + roi.height <= skin_final.rows) {
                    check_blob_size_new(skin_final(roi), filled_mask(roi), th_low, th_high, "RA",
                                        error_id);
                } else {
                    error_id = "GE1";
                    ERROR_IDs = ERROR_IDs + ", " + error_id;
                    ERROR_IDs_as_json = generic_error;
                    return;
                }
                ERROR_IDs = ERROR_IDs + ", " + error_id;
                // left arm
                roi.x = Xl - int(1.1 * face_rect.width);
                roi.width = int(1.1 * face_rect.width);
                roi.y = std::max(1, Yl - int(1.5 * face_rect.height));
                roi.height = std::min(int(1.5 * face_rect.height), skin_final.rows - roi.y - 1);
                if (roi.x > 0 && roi.x + roi.width <= skin_final.cols && roi.y > 0 &&
                    roi.y + roi.height <= skin_final.rows) {
                    check_blob_size_new(skin_final(roi), filled_mask(roi), th_low, th_high, "LA",
                                        error_id);
                } else {
                    error_id = "GE1";
                    ERROR_IDs = ERROR_IDs + ", " + error_id;
                    ERROR_IDs_as_json = generic_error;
                    return;
                }
                ERROR_IDs = ERROR_IDs + ", " + error_id;
                // right leg
                roi.x = Xbr;
                roi.width = Xc - Xbr;
                roi.y = Yb - 2 * face_rect.height;
                roi.height = 2 * face_rect.height;
                if (roi.x > 0 && roi.x + roi.width <= skin_final.cols && roi.y > 0 &&
                    roi.y + roi.height <= skin_final.rows) {
                    check_blob_size_new(skin_final(roi), filled_mask(roi), th_low, th_high, "RL",
                                        error_id);
                } else {
                    error_id = "GE1";
                    ERROR_IDs = ERROR_IDs + ", " + error_id;
                    ERROR_IDs_as_json = generic_error;
                    return;
                }
                ERROR_IDs = ERROR_IDs + ", " + error_id;
                // left leg
                roi.x = Xc;
                roi.width = Xbl - Xc;
                roi.y = Yb - 2 * face_rect.height;
                roi.height = 2 * face_rect.height;
                if (roi.x > 0 && roi.x + roi.width <= skin_final.cols && roi.y > 0 &&
                    roi.y + roi.height <= skin_final.rows) {
                    check_blob_size_new(skin_final(roi), filled_mask(roi), th_low, th_high, "LL",
                                        error_id);
                } else {
                    error_id = "GE1";
                    ERROR_IDs = ERROR_IDs + ", " + error_id;
                    ERROR_IDs_as_json = generic_error;
                    return;
                }
                ERROR_IDs = ERROR_IDs + ", " + error_id;
            } else if (view == BodyScanCommon::Profile::side) {
                float th_low = 0.03;
                float th_high = 0.95;
                // upper body
                roi.x = Xr; // max(1,Xr-10);
                roi.width = Xl - Xr;
                roi.y = face_rect.y + face_rect.height;
                roi.height = int((Yb - Yt) / 2.0 - face_rect.height / 2.0); // Yc - int(roi.y);
                if (roi.x > 0 && roi.x + roi.width <= skin_final.cols && roi.y > 0 &&
                    roi.y + roi.height <= skin_final.rows) {
                    check_blob_size_new(skin_final(roi), filled_mask(roi), th_low, th_high, "UB",
                                        error_id);
                } else {
                    error_id = "GE1";
                    ERROR_IDs = ERROR_IDs + ", " + error_id;
                    ERROR_IDs_as_json = generic_error;
                    return;
                }
                ERROR_IDs = ERROR_IDs + ", " + error_id;
                // lower body
                roi.x = Xr; // max(1,Xr-10);
                roi.width = Xl - Xr;
                roi.y = int(Yb - 2.5 * face_rect.height);
                roi.height = std::min(int(2.5 * face_rect.height), skin_final.rows - roi.y - 1);
                if (roi.x > 0 && roi.x + roi.width <= skin_final.cols && roi.y > 0 &&
                    roi.y + roi.height <= skin_final.rows) {
                    check_blob_size_new(skin_final(roi), filled_mask(roi), th_low, th_high, "LB",
                                        error_id);
                } else {
                    error_id = "GE1";
                    ERROR_IDs = ERROR_IDs + ", " + error_id;
                    ERROR_IDs_as_json = generic_error;
                    return;
                }
                ERROR_IDs = ERROR_IDs + ", " + error_id;
            } else {
                error_id = "GE1"; // neither Front nor Side (a call problem not a user problem)
                ERROR_IDs = ERROR_IDs + ", " + error_id;
                ERROR_IDs_as_json = generic_error;
                return;
            }
            ERROR_IDs_as_json = create_json(view, ERROR_IDs);
            return;
        } catch (int) {
            error_id = "GE1";
            ERROR_IDs = ERROR_IDs + ", " + error_id;
            ERROR_IDs_as_json = generic_error;
            return;
        }
    }

    cv::Mat
    contour::match_with_template(const cv::Mat &templIn, const cv::Mat &imgIn, int match_method) {
        cv::Mat matched = cv::Mat::zeros(imgIn.rows, imgIn.cols, 0);
        try {
            cv::Mat templ = templIn.clone();
            cv::Mat img = imgIn.clone();
            cvtColor(templ, templ, cv::COLOR_BGR2HSV);
            cvtColor(img, img, cv::COLOR_BGR2HSV);
            for (int i = 0; i < templ.rows; i++) {
                for (int j = 0; j < templ.cols; j++) {
                    templ.at<cv::Vec3b>(i, j)[2] = 0;
                }
            }
            for (int i = 0; i < img.rows; i++) {
                for (int j = 0; j < img.cols; j++) {
                    img.at<cv::Vec3b>(i, j)[2] = 0;
                }
            }
            // Create the result matrix
            int result_cols = img.cols - templ.cols + 1;
            int result_rows = img.rows - templ.rows + 1;
            cv::Mat result;
            result.create(result_rows, result_cols, CV_32FC1);
            // Do the Matching and Normalize
            matchTemplate(img, templ, result, match_method);
            normalize(result, result, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());
            // Localizing the best match with minMaxLoc
            double minValD;
            double maxValD;
            cv::Point minLoc;
            cv::Point maxLoc;
            minMaxLoc(result, &minValD, &maxValD, &minLoc, &maxLoc, cv::Mat());
            cv::Mat bin_result = result < 0.3 * maxValD;
            cv::Rect roi;
            roi.x = templ.cols / 2;
            roi.y = templ.rows / 2;
            roi.width = result.cols;
            roi.height = result.rows;
            bin_result.copyTo(matched(roi));
            return matched;
        } catch (int) {
            return matched;
        }
    }

    void
    contour::check_blob_size_new(const cv::Mat &part_bin_image, const cv::Mat &mask, float min_th,
                                 float max_th, const std::string &part_name,
                                 std::string &error_id) {
        try {
            error_id.clear();
            threshold(mask, mask, 100, 255, cv::THRESH_BINARY);
            threshold(mask, mask, 100, 255, cv::THRESH_BINARY);
            cv::Scalar SkinBlob = sum(part_bin_image);
            cv::Scalar MaskBlob = sum(mask);
            float S = (float) SkinBlob.val[0];
            float M = (float) MaskBlob.val[0];
            if (S < min_th / 10.0 * M) {
                error_id = part_name + "0";
                return;
            } else if (S > min_th / 10.0 * M && S < min_th * M) {
                error_id = part_name + "3";
                return;
            } else if (S > max_th * M) {
                error_id = part_name + "2";
                return;
            } else {
                error_id = part_name + "1"; // detected;
            }
        } catch (int) {
            error_id = "GE1";
            return;
        }
    }

    std::string contour::create_json(BodyScanCommon::Profile view, const std::string &ERROR_IDs) {
        int pos;
        int L = (int) ERROR_IDs.size();
        std::string json = "{ \n";
        std::string viewStr = view == BodyScanCommon::Profile::front ? "Front" : "Side";
        json = json + "\"Image\":\"" + viewStr + "\", \n";
        // Generic Error
        pos = (int) ERROR_IDs.find("GE1");
        if (pos >= 0 && pos < L) {
            json = json + "\"GE\":\"" + ERROR_IDs[pos + 2] + "\", \n";
        } else {
            json = json + "\"GE\":\"" + "0" + "\", \n";
        }
        // Face
        pos = (int) ERROR_IDs.find("FD");
        if (pos >= 0 && pos < L) {
            json = json + "\"Face\":\"" + ERROR_IDs[pos + 2] + "\", \n";
        }
        // LA
        pos = (int) ERROR_IDs.find("LA");
        if (pos >= 0 && pos < L) {
            json = json + "\"LA\":\"" + ERROR_IDs[pos + 2] + "\", \n";
        }
        // RA
        pos = (int) ERROR_IDs.find("RA");
        if (pos >= 0 && pos < L) {
            json = json + "\"RA\":\"" + ERROR_IDs[pos + 2] + "\", \n";
        }
        // LL
        pos = (int) ERROR_IDs.find("LL");
        if (pos >= 0 && pos < L) {
            json = json + "\"LL\":\"" + ERROR_IDs[pos + 2] + "\", \n";
        }
        // RL
        pos = (int) ERROR_IDs.find("RL");
        if (pos >= 0 && pos < L) {
            json = json + "\"RL\":\"" + ERROR_IDs[pos + 2] + "\", \n";
        }
        // UB
        pos = (int) ERROR_IDs.find("UB");
        if (pos >= 0 && pos < L) {
            json = json + "\"UB\":\"" + ERROR_IDs[pos + 2] + "\", \n";
        }
        pos = (int) ERROR_IDs.find("LB");
        if (pos >= 0 && pos < L) {
            json = json + "\"LB\":\"" + ERROR_IDs[pos + 2] + "\", \n";
        }
        pos = (int) ERROR_IDs.find("BG0");
        if (pos >= 0 && pos < L) {
        } else {
            json = json + "\"BG\":\"" + "1" + "\", \n";
        }
        int pos1 = (int) ERROR_IDs.find("DP0");
        int pos2 = (int) ERROR_IDs.find("DP2");
        if (pos1 >= 0 && pos1 < L) {
            json = json + "\"DP\":\"" + "0" + "\", \n";
        } else if (pos2 >= 0 && pos2 < L) {
            json = json + "\"DP\":\"" + "2" + "\", \n";
        } else {
            json = json + "\"DP\":\"" + "1" + "\", \n";
        }
        json = json + "\"Unused\":\"" + "NULL" + "\"";
        json = json + "\n }";
        return json;
    }
}