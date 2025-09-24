//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#include "AvatarGenContour.hpp"
#include "AvatarGenPredMesh.hpp"
#include "AvatarGenSegmentAndAuto.hpp"
#include "AvatarGenCommon.hpp"
#include "AHILogging.hpp"

namespace avatar_gen {

// PUBLIC
    contour::contour(void) {
    }

    std::vector<cv::Point> contour::predict(
            int image_Height, int image_Width,
            float Height, float Weight,
            BodyScanCommon::SexType gender, BodyScanCommon::Profile view,
            cv::Mat &ContourAsImage, float theta_phone, std::string &error_id,
            int onColor, int offColor, int onLen, int offLen, int lineWidth) {
        if (error_id.size() > 0) {
            std::vector<cv::Point> temp;
            return temp;
        }

        try {
            cv::Mat estimated_silhoutte = get_silhouttes_from_avatar(image_Height, image_Width,
                                                                     Height,
                                                                     Weight, gender, view,
                                                                     error_id);

            cv::Mat imContours = cv::Mat::zeros(image_Height, image_Width, CV_8U);
            cv::Mat ele0 = getStructuringElement(cv::MORPH_CROSS, cv::Size(29, 29));
            dilate(estimated_silhoutte.clone(), imContours, ele0);

            GaussianBlur(imContours, imContours, cv::Size(29, 29), 11, 11, cv::BORDER_REPLICATE);

            Canny(imContours < 0.01, imContours, 0, 30, 5);
            cv::Mat ele = getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3));

            dilate(imContours, imContours, ele);

            std::vector<cv::Point2i> ContourPoints;
            std::vector<std::vector<cv::Point>> contours;
            findContours(imContours, contours, cv::RETR_TREE, cv::CHAIN_APPROX_NONE,
                         cv::Point2f(0, 0));

            if (contours.empty()) {
                throw std::runtime_error("findContours returned an empty vector");
            }

            ContourPoints = contours[0];

            if (std::abs(theta_phone) < 2.0) {
                // here we get the contour as a variable of theta (highly accurate approximation is used for theta <= 5 degrees as cos(theta) ~= theta)
                int y_feet = 0; // any small value
                int y_head = 2 * imContours.rows;// any very large value

                for (int i = 0; i < ContourPoints.size(); i++) {
                    y_feet = std::max(y_feet, ContourPoints[i].y);
                    y_head = std::min(y_head, ContourPoints[i].y);
                }

                // below is an indirect implementation of the above as I don't prefer to re-visit the contour generation again ( computations etc.)
                theta_phone = -theta_phone;
                float y_feet_wrt_theta = (y_feet -
                                          0.2 * theta_phone * (y_feet - y_head) / Height * 200.0 /
                                          Height);
                float y_head_wrt_theta = (y_head +
                                          5.0 * theta_phone * (y_feet - y_head) / Height * 200.0 /
                                          Height);
                y_head_wrt_theta = std::min(y_head_wrt_theta, (float) (imContours.rows -
                                                                       30.0f)); // making sure we won't go beyond image

                y_head_wrt_theta =
                        y_head_wrt_theta +
                        (-3.3498 * Height + 632.1638); // shift due to person height

                y_head_wrt_theta = std::max(y_head_wrt_theta, 30.0f);
                y_feet_wrt_theta = std::min(y_feet_wrt_theta, (float) (imContours.rows - 30.0f));
                float ay =
                        (y_feet_wrt_theta - y_head_wrt_theta) / (float(y_feet - y_head) + 1.0e-20);
                float by = y_feet_wrt_theta - ay * y_feet;
                // flush or reset imContour
                imContours = cv::Mat::zeros(imContours.rows, imContours.cols, CV_8U);

                int lineLen = 0;
                bool bInLine = true;
                for (int i = 0; i < ContourPoints.size(); i++) {
                    int yn = (int) (ay * ContourPoints[i].y + by);
                    yn = std::max(30, yn);
                    yn = std::min(yn, imContours.rows - 30);
                    ContourPoints[i].y = yn;
                    lineLen++;
                    if (onLen > 0 && offLen > 0) {
                        if (bInLine) {
                            if (lineLen > onLen) {
                                lineLen = 0;
                                bInLine = !bInLine;
                            }
                        } else {
                            if (lineLen > offLen) {
                                lineLen = 0;
                                bInLine = !bInLine;
                            }
                        }
                    }
                    if (bInLine) {
                        imContours.at<uchar>(
                                cv::Point(ContourPoints[i].x, ContourPoints[i].y)) = onColor;
                    } else {
                        imContours.at<uchar>(
                                cv::Point(ContourPoints[i].x, ContourPoints[i].y)) = offColor;
                    }
                }
            }
            ele = getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(lineWidth, lineWidth));
            dilate(imContours, imContours, ele);
            findContours(imContours.clone(), contours, cv::RETR_TREE, cv::CHAIN_APPROX_NONE,
                         cv::Point(0, 0));
            if (contours.empty()) {
                throw std::runtime_error("findContours returned an empty vector");
            }
            ContourPoints = contours[0];
            ContourAsImage = imContours;
            return ContourPoints;
        }
        catch (...) {
            error_id = "{\"GE\": \"1\"}";
            auto expPtr = std::current_exception();
            try {
                if (expPtr) {
                    std::rethrow_exception(expPtr);
                }
            }
            catch (const std::exception &e) {
                AHILog(ANDROID_LOG_ERROR, "\nException caught in contour::predict: %s\n\", e.what()");
            }
            std::vector<cv::Point> temp;
            return temp;
        }
    }

// PRIVATE
    cv::Mat
    contour::get_silhouttes_from_avatar(int image_Height, int image_Width, float Height,
                                        float Weight,
                                        BodyScanCommon::SexType gender,
                                        BodyScanCommon::Profile view, std::string &error_id) {
        if (error_id.size() > 0) {
            return cv::Mat::zeros(1, 1, 0);
        }
        try {
            std::vector<float> data(7, -100); // fill all with -100
            data[0] = Height;
            data[1] = Weight;
            cv::Mat generated_silhoutte = cv::Mat::zeros(image_Height, image_Width, CV_8U);

            if (data[0] < BodyScanCommon::MIN_H || data[0] > BodyScanCommon::MAX_H) {
                return generated_silhoutte;
            }
            if (data[1] < BodyScanCommon::MIN_W || data[1] > BodyScanCommon::MAX_W) {
                return generated_silhoutte;
            }

            std::vector<float> thetas_pose(4, 0.0); // arms and legs poses
            if (view == BodyScanCommon::Profile::side) {
                thetas_pose[0] = M_PI / 10;
                thetas_pose[1] = -M_PI / 10;
                thetas_pose[2] = M_PI / 40;
                thetas_pose[3] = -M_PI / 40;
            }

            const common *c = common::getInstance();
            pred_mesh pm(gender);
            segment_auto seg;

            std::vector<float> thetas_feet(2, 0.0);
            std::vector<float> V(N_VERTS_3);
            const std::vector<int> F = c->getFaces(gender);
            data[6] = 1.02;
            error_id = pm.run(data, thetas_pose, thetas_feet, V);

            if (error_id.size() > 0 && !(error_id == "Passed")) {
                return cv::Mat::zeros(1, 1, 0);
            }

            data[1] = 1.1 * data[1];
            data[2] = 1.1 * data[2];
            data[3] = 1.1 * data[3];
            data[4] = 1.1 * data[4];
            error_id = pm.run(data, thetas_pose, thetas_feet, V);

            if (!(error_id == "Passed")) {
                return cv::Mat::zeros(1, 1, 0);
            }
            error_id.clear();

            std::vector<float> thetas_cam(3, 0.0);
            thetas_cam[0] = M_PI;
            thetas_cam[1] = 0;
            thetas_cam[2] = 0.0;

            float ux = float(generated_silhoutte.cols) / 2.0 - 0.5;
            float uy = float(generated_silhoutte.rows) / 2.0 - 0.5;
            float fc = -1;
            float Tx = -1;
            float Ty = -1;
            float Tz = -1;
            float cx, sx, cy, sy, cz, sz;

            cx = cos(thetas_cam[0]);
            sx = sin(thetas_cam[0]);
            cy = cos(thetas_cam[1]);
            sy = sin(thetas_cam[1]);
            cz = cos(thetas_cam[2]);
            sz = sin(thetas_cam[2]);

            float min_w = 1.e6;
            float max_w = -1.e6;
            float min_h = 1.e6;
            float max_h = -1.e6;
            float min_z = 1.e6;
            float max_z = -1.e6;

            for (int i = 0; i < (V.size() / 3); i++) {
                if (view == Profile::side) {
                    float temp = V[3 * i];
                    V[3 * i] = V[3 * i + 2];
                    V[3 * i + 2] = temp;
                }
                min_w = std::min(min_w, V[3 * i]);
                max_w = std::max(max_w, V[3 * i]);
                min_h = std::min(min_h, V[3 * i + 1]);
                max_h = std::max(max_h, V[3 * i + 1]);
                min_z = std::min(min_z, V[3 * i + 2]);
                max_z = std::max(max_z, V[3 * i + 2]);
            }

            float a = 0.3;
            float mid_h = (a * min_h + (1.0 - a) * max_h);
            float sh = (20. / 100. + max_h - min_h) / (max_h - min_h);

            if (Tx < 0) {
                Tx = (max_w + min_w) / 2 * 1000.; // testing/centering
            }
            if (Ty < 0) {
                if (view == Profile::side) {
                    Ty = 0.95 * (max_h + min_h) / 2 * 1000;
                } else {
                    Ty = 0.95 * (max_h + min_h) / 2 *
                         1000;// testing/centering
                }
            }
            if (Tz < 0) {
                Tz = 3 * Ty;
            }
            float alpha = 0.4;
            bool pass = false;
            do {
                fc = -1;
                alpha = alpha + 0.005;
                if (fc < 0) {
                    if (Ty == 0) {
                        fc = 0.9 * ux / (Ty + 1.e-6) * Tz;
                    } else {
                        if (view == Profile::side) {
                            fc = 0.85 * (alpha * ux + (1.0 - alpha) * uy) / Ty * Tz;
                        } else {
                            fc = 0.9 * (alpha * ux + (1.0 - alpha) * uy) / Ty * Tz;
                        }
                    }
                }
                generated_silhoutte = cv::Mat::zeros(generated_silhoutte.rows,
                                                     generated_silhoutte.cols,
                                                     generated_silhoutte.type());
                int ximage, yimage, i, idx;
                int ximage_prev = 0;
                int yimage_prev = 0;
                int idx_prev;
                int xi_min = 2 * generated_silhoutte.cols;
                int xi_max = -1;
                int yi_min = 2 * generated_silhoutte.rows;
                int yi_max = -1;
                float (&Vxyz)[N_VERTS][3] = *reinterpret_cast<float (*)[N_VERTS][3]>(&(V[0]));
                for (int n = 0; n < ((int) (F.size() / 3)); n++) {
                    i = 3 * n;
                    idx_prev = -100; // any -ve number
                    for (int m = 0; m < 3; m++) {
                        idx = i + m;
                        float Xo = Vxyz[F[idx]][0];
                        float Yo = Vxyz[F[idx]][1];
                        Yo = sh * (Yo - mid_h) + mid_h;
                        float Zo = Vxyz[F[idx]][2];
                        if (view == Profile::side) {
                            if (fabs(Zo) > 0.7 * max_z)
                                continue;
                        }
                        float X, Y, Z;
                        X = cy * cz * Xo - cy * sz * Yo + sy * Zo;
                        Y = (cx * sz + cz * sx * sy) * Xo + (cx * cz - sx * sy * sz) * Yo -
                            cy * sx * Zo;
                        Z = (sx * sz - cx * cz * sy) * Xo + (cz * sx + cx * sy * sz) * Yo +
                            cx * cy * Zo;
                        if (std::abs(Tz + Z * 1000) > 0.001) {
                            float Scale = fc / (Tz + Z * 1000.);
                            ximage = (int) (Scale * (1000. * X + Tx) + ux);
                            yimage = (int) (Scale * (1000. * Y + Ty) + uy);
                            if (ximage >= 0 && yimage >= 0 && ximage_prev >= 0 &&
                                yimage_prev >= 0 &&
                                ximage < generated_silhoutte.cols &&
                                yimage < generated_silhoutte.rows) {
                                xi_min = std::min(xi_min, ximage);
                                xi_max = std::max(xi_max, ximage);
                                yi_min = std::min(yi_min, yimage);
                                yi_max = std::max(yi_max, yimage);
                                generated_silhoutte.at<uchar>(cv::Point(ximage, yimage)) = 255;
                                if ((idx - idx_prev) == 1) {
                                    line(generated_silhoutte, cv::Point(ximage, yimage),
                                         cv::Point(ximage_prev, yimage_prev), 255, 1);
                                }
                                idx_prev = idx;
                                ximage_prev = ximage;
                                yimage_prev = yimage;
                            }
                        }
                    }
                }
                pass = (xi_min > 30) && (xi_max < (generated_silhoutte.cols - 30)) &&
                       (yi_min > 30) &&
                       (yi_max < (generated_silhoutte.rows - 30));
            } while (!pass);
            generated_silhoutte = seg.fillHoles(generated_silhoutte, error_id);
            AHILog(ANDROID_LOG_DEBUG, "\n fc Tx Ty Tz %f \t %f \t %f \t %f \n", fc, Tx, Ty, Tz);
            return generated_silhoutte;
        } catch (cv::Exception &e) {
            error_id = "{\"GE\": \"1\"}"; // new error should be injected here
            return cv::Mat::zeros(1, 1, 0);
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
                error_id = part_name + "1";
            }
        } catch (int) {
            error_id = "GE1";
            return;
        }
    }

    std::string contour::create_json(Profile view, const std::string &ERROR_IDs) {
        int pos;
        int L = (int) ERROR_IDs.size();
        std::string json = "{ \n";
        std::string viewStr = view == Profile::front ? "Front" : "Side";
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
            json = json + "\"BG\":\"" + ERROR_IDs[pos + 2] + "\", \n";
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