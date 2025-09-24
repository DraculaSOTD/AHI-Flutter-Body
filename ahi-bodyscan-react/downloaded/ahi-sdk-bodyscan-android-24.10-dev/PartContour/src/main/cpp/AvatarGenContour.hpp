//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#ifndef AvatarGenContour_hpp
#define AvatarGenContour_hpp

#include <vector>
#include <opencv2/core/types.hpp>
#include "Common.hpp"

namespace avatar_gen {
    class contour {
    public:
        contour(void);

        /**
         * Generates a contour based on the supplied parameters.
         * @param image_Height - Image height to generate.
         * @param image_Width - Image width to generate.
         * @param Height - User height.
         * @param Weight - User weight.
         * @param gender - User gender.
         * @param view - Front or Side view.
         * @param ContourAsImage - Resulting contour.
         * @param theta_phone
         * @param error_id - Result string on error.
         * @param onColor - First line color (when drawing dashes).
         * @param offColor - Second line color (when drawing dashes).
         * @param onLen - Number of pixels for fist line length(s).
         * @param offLen - Number of pixels for second line length(s).
         * @param lineWidth - Width of line to draw.
         * @return
         */
        static std::vector<cv::Point> predict(
                int image_Height, int image_Width,
                float Height, float Weight,
                BodyScanCommon::SexType gender, BodyScanCommon::Profile view,
                cv::Mat &ContourAsImage, float theta_phone, std::string &error_id,
                int onColor, int offColor, int onLen, int offLen, int lineWidth);

    private:
        static cv::Mat
        get_silhouttes_from_avatar(int image_Height, int image_Width, float Height, float Weight,
                                   BodyScanCommon::SexType gender, BodyScanCommon::Profile view, std::string &error_id);

        cv::Mat match_with_template(const cv::Mat &templ, const cv::Mat &img, int match_method);

        void check_blob_size_new(const cv::Mat &part_bin_image, const cv::Mat &mask, float min_th,
                                 float max_th, const std::string &part_name, std::string &error_id);

        std::string create_json(BodyScanCommon::Profile view, const std::string &ERROR_IDs);
    };
}

#endif /* AvatarGenContour_hpp */
