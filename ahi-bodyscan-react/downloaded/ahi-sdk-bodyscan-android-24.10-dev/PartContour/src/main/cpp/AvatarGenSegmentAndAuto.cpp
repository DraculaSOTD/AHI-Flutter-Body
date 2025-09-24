//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#include "AvatarGenSegmentAndAuto.hpp"
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>

namespace avatar_gen {
    segment_auto::segment_auto() {
    }

    void segment_auto::reset() {
        if (!mask.empty()) {
            mask.setTo(cv::Scalar::all(cv::GC_BGD));
        }
        bgdPxls.clear();
        fgdPxls.clear();
        prBgdPxls.clear();
        prFgdPxls.clear();
        ContPxls.clear();

        bgdPxls = std::vector<cv::Point>();
        fgdPxls = std::vector<cv::Point>();
        prBgdPxls = std::vector<cv::Point>();
        prFgdPxls = std::vector<cv::Point>();
        ContPxls = std::vector<cv::Point>();

        isInitialized = false;
        rectState = NOT_SET;
        lblsState = NOT_SET;
        prLblsState = NOT_SET;
        iterCount = 0;
    }

    void segment_auto::setImageAndWinName(const cv::Mat &_image, const std::string &_winName) {
        try {
            if (_image.empty()) {
                return;
            }
            image = _image;
            winName = _winName;
            mask.create(image.size(), CV_8UC1);
            reset();
        } catch (cv::Exception &e) {
            reset();
            return;
        }
    }

    void segment_auto::showSetImage(const std::string &view) {
        try {
            if (image.empty()) {
                return;
            }
            cv::Mat res;
            cv::Mat binMask;
            if (!isInitialized) {
                image.copyTo(res);
            } else {
                getBinMaskNew(binMask);
                image.copyTo(res, binMask);
            }
        } catch (cv::Exception &e) {
            return;
        }
    }

    void segment_auto::setRectInMask() {
        try {
            assert(!mask.empty());
            mask.setTo(cv::GC_BGD);
            rect.x = std::max(1, rect.x);
            rect.y = std::max(1, rect.y);
            rect.width = std::min(rect.width, image.cols - rect.x);
            rect.height = std::min(rect.height, image.rows - rect.y);
            (mask(rect)).setTo(cv::Scalar(cv::GC_PR_FGD));
        } catch (cv::Exception &e) {
            return;
        }
    }

    void segment_auto::setLblsInMask(int flags, cv::Point p, bool isPr) {
        try {
            uchar bvalue, fvalue;
            if (!isPr) {
                bvalue = cv::GC_BGD;
                fvalue = cv::GC_FGD;
            } else {
                bvalue = cv::GC_PR_BGD;
                fvalue = cv::GC_PR_FGD;
            }
            if (flags & BGD_KEY) {
                circle(mask, p, radius, bvalue, thickness);
            }
            if (flags & FGD_KEY) {
                circle(mask, p, radius, fvalue, thickness);
            }
        } catch (cv::Exception &e) {
            return;
        }
    }


    void segment_auto::getBinMaskNew(cv::Mat &binMask) {
        try {
            if (mask.empty() || mask.type() != CV_8UC1) {
                return;
            }
            if (binMask.empty() || binMask.rows != mask.rows || binMask.cols != mask.cols) {
                binMask.create(mask.size(), CV_8UC1);
            }
            binMask = mask & 1;
        } catch (cv::Exception &e) {
            return;
        }
    }

    cv::Mat segment_auto::segment_skin_specific(const cv::Mat &base_image, const cv::Mat &src_image,
                                                const std::string &HSV_or_HS,
                                                std::string &error_id) {
        try {
            cv::Mat skin_backproj = segment_skin_specific_options(base_image, src_image, HSV_or_HS,
                                                                  error_id);
            return skin_backproj;
        } catch (int) {
            error_id = "{\"GE\": \"1\"}";
            return (0. * src_image.clone());
        }
    }

    cv::Mat
    segment_auto::mask_color_image(const cv::Mat &src, const cv::Mat &bin_mask,
                                   std::string &error_id) {
        if (bin_mask.empty() || !(bin_mask.rows == src.rows) || !(bin_mask.cols == src.cols)) {
            return src;
        }
        try {
            cv::Mat image = src.clone();
            if (image.channels() == 1) {
                bitwise_and(bin_mask, image, image);
            }
            if (image.channels() == 3) {
                cv::Mat channels[3];
                // splitting
                cv::split(image, channels);
                bitwise_and(channels[0], bin_mask, channels[0]);
                bitwise_and(channels[1], bin_mask, channels[1]);
                bitwise_and(channels[2], bin_mask, channels[2]);
                // merging
                merge(channels, 3, image);
            }
            return image;
        } catch (cv::Exception &e) {
            error_id = "{\"GE\": \"1\"}";
            return src;
        }
    }

    cv::Rect segment_auto::boundingboxNew(cv::Mat image) {
        cv::Rect roi;
        if (image.empty()) {
            cv::Rect roi;
            roi.x = 1;
            roi.width = 0;
            roi.y = 1;
            roi.height = 0;
            return roi;
        }
        try {
            cv::Mat GrayScaleImage;
            if (image.channels() == 1) {
                GrayScaleImage = image.clone();
            } else if (image.channels() == 3) {
                cv::cvtColor(image, GrayScaleImage, cv::COLOR_BGR2GRAY);
            }
            std::vector<cv::Point2i> Points;
            findNonZero_BugFree(GrayScaleImage, Points);
            roi = boundingRect(Points);
            return roi;
        } catch (cv::Exception &e) {
            roi.x = 1;
            roi.width = image.cols - 1;
            roi.y = 1;
            roi.height = image.rows - 1;
            return roi;
        }
    }

    void
    segment_auto::est_image_skel_and_coutours(cv::Mat &img, cv::Mat &imContours, float scale_factor,
                                              std::string view, int phase_number) {
        try {
            cv::Mat channels[3];
            channels[0] = img.clone();
            GaussianBlur(img.clone(), imContours, cv::Size(21, 21), 3, 3, cv::BORDER_REPLICATE);
            Canny(imContours < 0.01, imContours, 0, 30, 5);
            cv::Mat ele = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
            dilate(imContours, imContours, ele);
            channels[1] = imContours;
            erode(img > 0, img, ele);
            // below is from my matlab skeleton codes
            img.convertTo(img, CV_32F);
            int L = 11;
            float sigma = 3.;
            float th = 0.95;
            threshold(img.clone(), img, 0.0, 1.0, cv::THRESH_BINARY);
            GaussianBlur(img.clone(), img, cv::Size(L, L), sigma, sigma, cv::BORDER_REPLICATE);
            threshold(img.clone(), img, 0.8, 1.0, cv::THRESH_BINARY);
            cv::Mat temp = img.clone();

            float scale = 1.0;
            float delta = 0.0;
            int ddepth = img.depth();

            // Generate grad_x and grad_y
            cv::Mat grad_x(img.size(), img.type(), cv::Scalar(0));
            cv::Mat grad_y(img.size(), img.type(), cv::Scalar(0));

            float imBlobsize = float(countNonZero(img));
            bool done;
            do {
                GaussianBlur(img.clone(), img, cv::Size(L, L), sigma, sigma);
                // Gradient X
                Sobel(img.clone(), grad_x, ddepth, 1, 0, 3, scale, delta, cv::BORDER_REPLICATE);
                // Gradient Y
                Sobel(img.clone(), grad_y, ddepth, 0, 1, 3, scale, delta, cv::BORDER_REPLICATE);
                // Total Gradient (approximate)
                img = grad_x.mul(grad_x) + grad_y.mul(grad_y);
                sqrt(img.clone(), img);
                threshold(img.clone(), img, th, 1.0, cv::THRESH_BINARY);
                temp = temp - img;
                threshold(temp.clone(), img, 0.0, 1.0, cv::THRESH_BINARY);
                done = (countNonZero(img) <= phase_number * scale_factor * imBlobsize);
            } while (!done);
            img = img > 0;
            cv::Rect skel_rect = boundingboxNew(img.clone());
            cv::Mat skel = cv::Mat::zeros(img.rows, img.cols, img.type());
            if (phase_number == 1) {
                if (view == "Side") {
                    skel_rect.x = skel_rect.x + 30;
                    skel_rect.y = skel_rect.y + 30;
                    skel_rect.width = skel_rect.width - 60;
                    skel_rect.height = 0.8 * skel_rect.height;
                    (img(skel_rect)).copyTo(skel(skel_rect));
                } else { // (view == "Front")
                    skel_rect.x = skel_rect.x + skel_rect.width / 5;
                    skel_rect.width = skel_rect.width - skel_rect.width * (2.0 / 5.0);
                    skel_rect.y = skel_rect.y + 40;
                    skel_rect.height = 0.75 * skel_rect.height;

                    (img(skel_rect)).copyTo(skel(skel_rect));

                    cv::Rect roi;
                    cv::Mat ZEROS = cv::Mat::zeros(img.rows, img.cols, img.type());
                    roi.x = int(img.cols / 2 - 25);
                    roi.y = int(img.rows / 2);
                    roi.width = 50;
                    roi.height = img.rows - roi.y - 1;
                    (ZEROS(roi)).copyTo(skel(roi));
                }
            } else {// phase #2
                skel_rect.x = skel_rect.x + skel_rect.width / 20;
                skel_rect.width = skel_rect.width - skel_rect.width * (2.0 / 20.0);
                skel_rect.y = skel_rect.y + 0.05 * skel_rect.height;
                skel_rect.height = 0.9 * skel_rect.height;

                (img(skel_rect)).copyTo(skel(skel_rect));

                cv::Rect roi;
                cv::Mat ZEROS = cv::Mat::zeros(img.rows, img.cols, img.type());
                roi.x = int(img.cols / 2 - 25);
                roi.y = int(img.rows / 2);
                roi.width = 50;
                roi.height = img.rows - roi.y - 1;
                (ZEROS(roi)).copyTo(skel(roi));
            }
            img = skel;
            return;
        } catch (cv::Exception &e) {
            return;
        }
    }

    cv::Mat segment_auto::fillHoles(const cv::Mat &src, std::string &error_id) {
        try {
            // better to use this instead of old fillHoles functions as openCV floodfill has issues
            cv::Mat filled = src.clone();
            if (filled.channels() == 3) {
                cv::cvtColor(filled, filled, cv::COLOR_BGR2GRAY);
            }
            std::vector<std::vector<cv::Point>> contours;
            findContours(filled.clone(), contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
            if ((int) contours.size() < 1) {
                error_id = "{\"GE\": \"1\"}";
                return src.clone();
            }
            for (int i = 0; i < (int) contours.size(); i++) {
                drawContours(filled, contours, i, cv::Scalar(255), cv::FILLED);
            }
            return filled;
        } catch (cv::Exception &e) {
            error_id = "{\"GE\": \"1\"}";
            return src.clone();
        }
    }

    void segment_auto::findNonZero_BugFree(const cv::Mat &m, std::vector<cv::Point2i> &locations) {
        int count = countNonZero(m);
        if (count > 0) {
            findNonZero(m, locations);
        } else {
            locations.clear();
        }
    }

    cv::Rect segment_auto::getFrontFaceRect(void) {
        return front_face_rect;
    }

    cv::Rect segment_auto::getSideFaceRect(void) {
        return side_face_rect;
    }

    cv::Mat segment_auto::segment_skin_specific_options(const cv::Mat base_image, cv::Mat src_image,
                                                        std::string HSV_or_HS,
                                                        std::string &error_id) {
        try {
            cv::Mat src = src_image.clone();
            cv::Mat base = base_image.clone();
            cv::Mat hsv, hist, mask;
            GaussianBlur(src, src, cv::Size(7, 7), 1.5, 1.5); // remove noise
            GaussianBlur(base, base, cv::Size(7, 7), 1.5, 1.5); // remove noise
            cvtColor(base, hsv, cv::COLOR_BGR2HSV);
            cv::Mat backproj;

            if (HSV_or_HS == "HSV") {
                float h_ranges[] = {2, 25};
                float s_ranges[] = {10, 250};
                float v_ranges[] = {40, 245};
                const float *ranges[] = {h_ranges, s_ranges, v_ranges};
                int h_bins = 20;
                int s_bins = 16;
                int v_bins = 16;
                int histSize[] = {h_bins, s_bins, v_bins};
                int channels[] = {0, 1, 2};
                calcHist(&hsv, 1, channels, mask, hist, 3, histSize, ranges, true, false);
                // Get Backprojection
                cvtColor(src, hsv, cv::COLOR_BGR2HSV);
                calcBackProject(&hsv, 1, channels, hist, backproj, ranges, 1.0, true);
            } else {
                int h_bins = 30;
                int s_bins = 32;
                int histSize[] = {h_bins, s_bins};
                float h_range[] = {3, 20};// all/any colors
                float s_range[] = {20, 250};// all/any colors
                const float *ranges[] = {h_range, s_range};
                int channels[] = {0, 1};

                // Get the Histogram and normalize it
                cvtColor(base_image, hsv, cv::COLOR_BGR2HSV);
                calcHist(&hsv, 1, channels, mask, hist, 2, histSize, ranges, true, false);
                normalize(hist, hist, 0, 255, cv::NORM_MINMAX, -1, cv::Mat());

                // Get Backprojection
                cvtColor(src, hsv, cv::COLOR_BGR2HSV);
                calcBackProject(&hsv, 1, channels, hist, backproj, ranges, 1.0, true);
            }
            inRange(backproj, cv::Scalar(1, 1, 1), cv::Scalar(255, 255, 255), backproj);
            return backproj;
        } catch (int) {
            error_id = "{\"GE\": \"1\"}";
            return (0. * src_image.clone());
        }
    }
}