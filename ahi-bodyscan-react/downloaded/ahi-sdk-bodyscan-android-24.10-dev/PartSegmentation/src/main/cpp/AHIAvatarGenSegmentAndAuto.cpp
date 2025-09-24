//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#include "AHIAvatarGenSegmentAndAuto.hpp"

#include <opencv2/objdetect/objdetect.hpp>

#include "AHIAvatarGenHaarcascade_frontalface_alt2.hpp"
#include "AHIAvatarGenHaarcascade_profileface.hpp"

namespace ahi_avatar_gen {

// Methods
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

    void segment_auto::ExtractFeatures(const cv::Mat &ContBinMask_orig, const std::string &view,
                                       int phase_number, std::string &error_id) {
        try {
            trusted_fgdPxls.clear();
            if (ContBinMask_orig.empty()) {
                return;
            }
            cv::Mat ContBinMask = ContBinMask_orig.clone();
            cv::Mat dilate_element = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(21,
                                                                                           21));//(MORPH_CROSS, cv::Size(21, 21));
            dilate(ContBinMask, ContBinMask, dilate_element);
            cv::Mat im_orig;
            image.copyTo(im_orig);
            im_orig = segment_auto::mask_color_image(im_orig, ContBinMask_orig, error_id);
            cv::Mat skin_final = cv::Mat::zeros(im_orig.rows, im_orig.cols, 0);
            cv::Mat backproj = cv::Mat::zeros(im_orig.rows, im_orig.cols, 0);
            cv::Mat Templ_match = cv::Mat::zeros(im_orig.rows, im_orig.cols, 0);
            cv::Mat connected_graph = cv::Mat::zeros(im_orig.rows, im_orig.cols, 0);
            cv::Rect face_rect;
            cv::Rect face_rect_cropped;
            cv::Mat FaceMask = get_FaceMask(im_orig, view, face_rect, error_id);
            if (!(error_id.size() > 0 || FaceMask.cols < 10 || FaceMask.rows < 10)) {
                int face_x_center = face_rect.x + (int) (face_rect.width / 2.);
                int face_y_center = face_rect.y + (int) (face_rect.height / 2.);
                if (view == "Front") {
                    face_rect_cropped.x = face_x_center - face_rect.width / 4;
                    face_rect_cropped.width = face_rect.width / 2;
                    face_rect_cropped.y = face_y_center - face_rect.height / 3;
                    face_rect_cropped.height = (int) (face_rect.height * (2. / 3.));
                } else {
                    face_rect_cropped.x = face_x_center - face_rect.width / 5;
                    face_rect_cropped.width = face_rect.width / 5;
                    face_rect_cropped.y = face_y_center;
                    face_rect_cropped.height = (int) (face_rect.height * (1. / 3.));
                }
                // make the width and height even values
                face_rect_cropped.width = 2 * int(face_rect_cropped.width / 2);
                face_rect_cropped.height = 2 * int(face_rect_cropped.height / 2);
                cv::Mat element = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(31, 31));
                cv::Mat im_orig_masked = cv::Mat::zeros(im_orig.rows, im_orig.cols, im_orig.type());
                cv::Mat ContBinMask_backproj = cv::Mat::zeros(im_orig.rows, im_orig.cols, 0);
                erode(ContBinMask_orig, ContBinMask_backproj, element);
                im_orig_masked = mask_color_image(im_orig, ContBinMask_backproj, error_id);
                bitwise_or(backproj, Templ_match, skin_final);
                threshold(skin_final, skin_final, 100, 255, cv::THRESH_BINARY);
            } else {
                error_id = "ProbNoFace";
                return;
            }
            int flags;
            // Rect
            int offset = 1;
            rect.x = offset;
            rect.y = offset;
            rect.width = ContBinMask.cols - 2 * offset;
            rect.height = ContBinMask.rows - 2 * offset;
            // set rectangle where the global FG is within
            flags = 1;
            rectState = SET;
            setRectInMask();
            showSetImage(view);
            assert(bgdPxls.empty() && fgdPxls.empty() && prBgdPxls.empty() && prFgdPxls.empty());
            // BG features
            cv::Mat BkGdFeatureImage = cv::Mat::zeros(ContBinMask.rows, ContBinMask.cols,
                                                      ContBinMask.type());
            (BkGdFeatureImage(rect)).setTo(cv::Scalar(255));
            BkGdFeatureImage = BkGdFeatureImage - ContBinMask;
            BkGdFeatureImage =
                    BkGdFeatureImage > 1; // just in case of rounding, values are either 0 or 255
            std::vector<cv::Point2i> BGlocations;
            segment_auto::findNonZero_BugFree(BkGdFeatureImage, BGlocations);
            flags = BGD_KEY; // BackGround
            if ((int) BGlocations.size() > 0) {
                for (int j = 0; j < (int) BGlocations.size(); j++) {
                    setLblsInMask(BGD_KEY, cv::Point(BGlocations[j].x, BGlocations[j].y), false);
                }
            }
            std::vector<cv::Point2i> FGlocations;
            flags = FGD_KEY; // ForeGround
            cv::Mat FrGdFeatureImage = ContBinMask > 0; // ContBinMask_orig
            // The inside the dilated contour is likely a FG unless .... testing
            FGlocations.clear();
            segment_auto::findNonZero_BugFree(FrGdFeatureImage, FGlocations);
            if (!(prob_inside_contour == "None")) {
                for (int k = 0; k < (int) FGlocations.size(); k++) {
                    if (prob_inside_contour == "FG") {
                        setLblsInMask(FGD_KEY, cv::Point(FGlocations[k].x, FGlocations[k].y), true);
                    } else if (prob_inside_contour == "BG") {
                        setLblsInMask(BGD_KEY, cv::Point(FGlocations[k].x, FGlocations[k].y), true);
                    }
                }
            }
            FrGdFeatureImage = ContBinMask_orig > 0;
            FGlocations.clear();
            cv::Mat imContours = cv::Mat::zeros(im_orig.rows, im_orig.cols, 0);
            if (view == "Side") {
                if (with_skel) {
                    // side skel is a FG
                    est_image_skel_and_coutours(FrGdFeatureImage, imContours, 0.45, view,
                                                phase_number);
                    FGlocations.clear();
                    segment_auto::findNonZero_BugFree(FrGdFeatureImage, FGlocations);
                    if ((int) FGlocations.size() > 0) {
                        for (int k = 0; k < (int) FGlocations.size(); k++) {
                            setLblsInMask(FGD_KEY, cv::Point(FGlocations[k].x, FGlocations[k].y),
                                          false);
                        }
                    }
                }
            } else { // Front
                if (with_skel) {
                    // not all front skel is a FG
                    est_image_skel_and_coutours(FrGdFeatureImage, imContours, 0.2, view,
                                                phase_number);
                    int dy = 0; // this is to fix the y bias (e.g. under arm curvatures or similar)
                    FGlocations.clear();
                    segment_auto::findNonZero_BugFree(FrGdFeatureImage, FGlocations);
                    if ((int) FGlocations.size() > 0) {
                        for (int k = 0; k < (int) FGlocations.size(); k++) {
                            setLblsInMask(FGD_KEY,
                                          cv::Point(FGlocations[k].x, FGlocations[k].y - dy),
                                          false);
                        }
                    }
                    if (phase_number > 1) {
                        trusted_fgdPxls = FGlocations;
                    }
                }
                FGlocations.clear();
                segment_auto::findNonZero_BugFree(connected_graph, FGlocations);
                if ((int) FGlocations.size() > 0) {
                    for (int k = 0; k < (int) FGlocations.size(); k++) {
                        setLblsInMask(FGD_KEY, cv::Point(FGlocations[k].x, FGlocations[k].y),
                                      false);
                    }
                }
            }
            prLblsState = SET;
            lblsState = SET;
            showSetImage(view);
        } catch (cv::Exception &e) {
            error_id = "ProbNoFace";
            return;
        }
    }

    int segment_auto::nextIter(std::string &error_id) {
        try {
            if (isInitialized) {
                for (int j = 0; j < (int) trusted_fgdPxls.size(); j++) {
                    setLblsInMask(FGD_KEY, cv::Point(trusted_fgdPxls[j].x, trusted_fgdPxls[j].y),
                                  false);
                }
                grabCut(image, mask, rect, bgdModel, fgdModel, 1);
            } else {
                if (rectState != SET) {
                    return iterCount;
                }
                if (lblsState == SET || prLblsState == SET) {
                    try {
                        grabCut(image, mask, rect, bgdModel, fgdModel, 1, cv::GC_INIT_WITH_MASK);
                    } catch (cv::Exception &e) {
                        grabCut(image, mask, rect, bgdModel, fgdModel, 1, cv::GC_INIT_WITH_RECT);
                    }
                } else {
                    grabCut(image, mask, rect, bgdModel, fgdModel, 1, cv::GC_INIT_WITH_RECT);
                }
                isInitialized = true;
            }
            iterCount++;
            bgdPxls.clear();
            fgdPxls.clear();
            prBgdPxls.clear();
            prFgdPxls.clear();
            seg_FG_BW_mask = (mask == 1) | (mask == 3);
            return iterCount;
        } catch (cv::Exception &e) {
            error_id = "ProbNoFace";
            return 0;
        }
    }

    std::string
    segment_auto::segment_silhouette_auto(const cv::Mat &src_image, const std::string &view,
                                          int No_of_iterations, cv::Mat &ContBinMask,
                                          int phase_number) {
        cv::RNG rng = cv::theRNG();
        rng.state = 0xffffffff;
        std::string error_id;
        if (src_image.empty() || ContBinMask.empty()) {
            error_id = "6";
            return "6";
        }
        if (phase_number == 1) {
            ContBinMask = fillHoles(ContBinMask, error_id);
        }
        if ((int) error_id.size() > 0) {
            return error_id;
        }
        seg_FG_BW_mask = ContBinMask.clone(); // added to remove skin pixels from the background
        cv::Mat image = src_image.clone();
        setImageAndWinName(image, "");
        showSetImage(view);
        ExtractFeatures(ContBinMask, view, phase_number, error_id);
        if (error_id == "ProbNoFace") {
            cv::Mat ele = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(15, 15));
            erode(ContBinMask, ContBinMask, ele);
            return "Passed";
        } else if ((int) error_id.size() > 0) {
            return error_id;
        }
        for (int it = 0; it < No_of_iterations; it++) {
            int iterCount = getIterCount();
            int newIterCount = nextIter(error_id);
            if (newIterCount > iterCount) {
                showSetImage(view);
            } else {
                return ("Error: Features needed to segment the user must be determined first (at least the border)");
            }
        }
        ContBinMask = seg_FG_BW_mask.clone();
        if (ContBinMask.channels() == 3) {
            inRange(ContBinMask, cv::Scalar(1, 1, 1), cv::Scalar(255, 255, 255), ContBinMask);
        } else {
            inRange(ContBinMask, cv::Scalar(1), cv::Scalar(255), ContBinMask);
        }
        if ((sum(ContBinMask)).val[0] < 255 * (300 * 100)) {
            return ("Error: Silhouette is very small blob");
        }
        int erosion_size = 3;
        cv::Mat element = cv::getStructuringElement(cv::MORPH_ELLIPSE,
                                                    cv::Size(2 * erosion_size + 1,
                                                             2 * erosion_size + 1));
        erode(ContBinMask, ContBinMask, element);
        showSetImage(view);
        reset();
        return ("Passed");
    }

    void
    segment_auto::detect(const cv::Mat &src, const std::string &subject, int min_size, int max_size,
                         std::vector<cv::Rect> &rect, std::string &error_id) {
        try {
            cv::Mat image = src.clone();
            rect.clear();
            rect.resize(0);
            // declaration
            cv::CascadeClassifier cascadcalssifier;
            cv::FileStorage fs;
            if (subject ==
                "frontal_face_haar_alt2") { // could also use view instead of  subject but I made general as there are eyes detectors etc.
                fs.open(front_face_features::data,
                        cv::FileStorage::READ | cv::FileStorage::FORMAT_XML |
                        cv::FileStorage::MEMORY);
            } else {
                fs.open(side_face_features::data,
                        cv::FileStorage::READ | cv::FileStorage::FORMAT_XML |
                        cv::FileStorage::MEMORY);
            }
            if (!(cascadcalssifier.read(fs.getFirstTopLevelNode()))) {
                error_id = "GE1";
                rect.clear();
                return;
            }
            cascadcalssifier.detectMultiScale(image, rect, 1.1, 3, 0, cv::Size(min_size, min_size),
                                              cv::Size(max_size, max_size));
            if ((int) rect.size() == 2) {
                cv::Rect temp;
                if (rect[0].y < rect[1].y)
                    temp = rect[0];
                else
                    temp = rect[1];
                rect.clear();
                rect.push_back(temp);
            }
        } catch (cv::Exception &e) {
            error_id = "GE1";
            rect.clear();
            return;
        }
    }

    cv::Mat
    segment_auto::get_FaceMask(const cv::Mat &src, const std::string &view, cv::Rect face_rect,
                               std::string &error_id) {
        try {
            error_id.clear();
            // detect face
            cv::Mat image = src.clone();
            std::vector<cv::Rect> detected_face_rect;
            cv::Rect roi;
            roi.x = 1;
            roi.y = 1;
            roi.height = (int) (float(image.rows) / 3.0);
            roi.width = image.cols - roi.x;
            cv::Mat image_roi = image.clone();
            std::string subject;
            if (view == "Front") {
                subject = "frontal_face_haar_alt2";
            } else {
                subject = "side_face_haar";
            }
            detect(image_roi, subject, (int) (image.cols / 20), (int) image.cols / 2,
                   detected_face_rect, error_id);
            if (detected_face_rect.size() > 1) {
                error_id = "FM"; // more than one face
                return cv::Mat::zeros(1, 1, 0);
            } else if ((int) detected_face_rect.size() <
                       1) { // no face, flip the image and try again
                flip(image_roi, image_roi, 1);
                detect(image_roi, subject, (int) (image.cols / 20), (int) image.cols / 2,
                       detected_face_rect, error_id);
                flip(image_roi, image_roi, 1);
                if ((int) detected_face_rect.size() > 1) {
                    error_id = "FM";
                    return cv::Mat::zeros(1, 1, 0);
                } else if ((int) detected_face_rect.size() < 1) {
                    // try again with the front/side features
                    if (subject == "side_face_haar") {
                        subject = "frontal_face_haar_alt2";
                    } else {
                        subject = "side_face_haar";
                    }
                    detect(image_roi, subject, (int) (image.cols / 20), (int) image.cols / 2,
                           detected_face_rect, error_id);
                    if (detected_face_rect.size() > 1) {
                        error_id = "FM"; // more than one face
                        return cv::Mat::zeros(1, 1, 0);
                    } else if ((int) detected_face_rect.size() < 1) {
                        error_id = "FN";
                        return cv::Mat::zeros(1, 1, 0);
                    } else { // a single face
                        face_rect = detected_face_rect[0];
                    }
                } else { // a single face
                    detected_face_rect[0].x =
                            image.cols - (detected_face_rect[0].x + detected_face_rect[0].width);
                    face_rect = detected_face_rect[0];
                }
            } else { // if we reach here that means a face is detected
                face_rect = detected_face_rect[0];
            }
            int face_x_center = face_rect.x + (int) (face_rect.width / 2.);
            int face_y_center = face_rect.y + (int) (face_rect.height / 2.);
            cv::Rect face_rect_cropped;
            if (view == "Front") {
                front_face_rect = face_rect;
                face_rect_cropped.x = face_x_center - face_rect.width / 4;
                face_rect_cropped.width = face_rect.width / 2;
                face_rect_cropped.y = face_y_center;
                face_rect_cropped.height = (int) (face_rect.height * (1. / 3.));
            } else {
                side_face_rect = face_rect;
                face_rect_cropped.x = face_x_center - face_rect.width / 5;
                face_rect_cropped.width = face_rect.width / 5;
                face_rect_cropped.y = face_y_center;
                face_rect_cropped.height = (int) (face_rect.height * (1. / 3.));
            }
            // make the width and height even value
            face_rect_cropped.width = 2 * int(face_rect_cropped.width / 2);
            face_rect_cropped.height = 2 * int(face_rect_cropped.height / 2);
            return image(face_rect_cropped);
        } catch (cv::Exception &e) {
            error_id = "11";
            return cv::Mat::zeros(1, 1, 0);
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

    cv::Mat segment_auto::mask_color_image(const cv::Mat &src, const cv::Mat &bin_mask,
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
            } else { // phase #2
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
            std::vector<std::vector<cv::Point> > contours;
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
                float h_range[] = {3, 20}; // all/any colors
                float s_range[] = {20, 250}; // all/any colors
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