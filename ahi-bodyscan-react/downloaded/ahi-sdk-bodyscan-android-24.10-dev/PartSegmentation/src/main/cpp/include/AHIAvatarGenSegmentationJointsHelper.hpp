//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#ifndef AHIAvatarGenSegmentationJointsHelper_hpp
#define AHIAvatarGenSegmentationJointsHelper_hpp

#include <opencv2/imgproc/imgproc.hpp>

#include "Common.hpp"

namespace ahi_avatar_gen {

    class joints_helper {
    private:
        cv::Mat Hist_and_Backproj(const cv::Mat &hsv, const cv::Mat &mask);

        cv::Mat
        match_images(const cv::Mat &src_image, const cv::Mat &templ_base_mask, double thrld);

        std::vector<cv::Point> get_bone_connectivity(void);

        std::vector<std::string> get_joint_names(void);

        void fix_joint_x_positions(std::vector<cv::Point> &joints, int m, int n);

        cv::Mat get_skel_mask_from_joints(const cv::Mat &sbinMask, std::vector<cv::Point> joints,
                                          BodyScanCommon::Profile type, std::string &error_id);

        cv::Mat segment_dl_grabcut(const cv::Mat &orig_image, BodyScanCommon::Profile type,
                                   const cv::Mat &net_mask,
                                   const std::vector<cv::Point> &joints);

        cv::Mat
        segment_dl_grabcut_with_contourmask(const cv::Mat &orig_image, BodyScanCommon::Profile type,
                                            const cv::Mat &net_mask,
                                            const std::vector<cv::Point> &joints,
                                            cv::Mat contour_mask);

    public:
        joints_helper(void);

        cv::Mat
        segment_using_net_joints_and_grabcut(const cv::Mat &orig_image, BodyScanCommon::Profile type,
                                             const cv::Mat &net_mask,
                                             const std::vector<cv::Point> &joints);

        cv::Mat draw_image_with_joints(cv::Mat inp_image, std::vector<cv::Point> joints);

        cv::Mat segment_using_net_joints_and_grabcut_and_contourmask(const cv::Mat &orig_image,
                                                                     BodyScanCommon::Profile type,
                                                                     const cv::Mat &net_mask,
                                                                     const std::vector<cv::Point> &joints,
                                                                     cv::Mat contour_mask);
    };
}

#endif /* AHIAvatarGenSegmentationJointsHelper_hpp */
