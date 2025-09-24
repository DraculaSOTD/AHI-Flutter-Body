//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#ifndef SEGMENT_SILHOUTTE_AUTO_H
#define SEGMENT_SILHOUTTE_AUTO_H

#include <opencv2/highgui/highgui.hpp>

namespace avatar_gen {
    typedef enum gc_application_state_t {
        NOT_SET = 0,
        IN_PROCESS,
        SET
    } gc_application_state_t;

    class segment_auto {
    private:
        // Attributes
        cv::Rect front_face_rect;
        cv::Rect side_face_rect;
        bool with_skel = true;
        std::string prob_inside_contour = "FG";
        cv::Mat seg_FG_BW_mask;
        // Attributes
        std::string winName;
        cv::Mat image;
        cv::Mat mask;
        cv::Mat bgdModel, fgdModel;
        uchar rectState, lblsState, prLblsState;
        bool isInitialized;
        cv::Rect rect;
        std::vector<cv::Point> fgdPxls, bgdPxls, prFgdPxls, prBgdPxls, ContPxls, trusted_fgdPxls;
        int iterCount;
    public:
        // Methods
        segment_auto(void);

        cv::Mat segment_skin_specific(const cv::Mat &base_image, const cv::Mat &src_image,
                                      const std::string &HSV_or_HS, std::string &error_id);

        cv::Rect getFrontFaceRect(void);

        cv::Rect getSideFaceRect(void);

        // Static Methods
        static void findNonZero_BugFree(const cv::Mat &m, std::vector<cv::Point2i> &locations);

        static cv::Mat fillHoles(const cv::Mat &src, std::string &error_id);

        static cv::Mat
        mask_color_image(const cv::Mat &src, const cv::Mat &bin_mask, std::string &error_id);

        // GC
        // Constants
        static const int BGD_KEY = cv::EVENT_FLAG_CTRLKEY;
        static const int FGD_KEY = cv::EVENT_FLAG_SHIFTKEY;
        static const int radius = 2;
        static const int thickness = -1;

        // Methods
        void reset(void);

        void setImageAndWinName(const cv::Mat &_image, const std::string &_winName);

        void showSetImage(const std::string &view);

        int getIterCount(void) const { return iterCount; }

    private:
        void setRectInMask(void);

        void setLblsInMask(int flags, cv::Point p, bool isPr);

        void getBinMaskNew(cv::Mat &binMask);

        void est_image_skel_and_coutours(cv::Mat &img, cv::Mat &imContours, float scale_factor,
                                         std::string view, int phase_number);

        cv::Rect boundingboxNew(cv::Mat image);

        cv::Mat segment_skin_specific_options(const cv::Mat base_image, cv::Mat src_image,
                                              std::string HSV_or_HS, std::string &error_id);
    };
}

#endif /* SEGMENT_SILHOUTTE_AUTO_H */
