//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#ifndef AHIAvatarGenContour_hpp
#define AHIAvatarGenContour_hpp

#include <opencv2/imgproc/imgproc.hpp>

#include "Common.hpp"

namespace ahi_avatar_gen {
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
        void inspect(const cv::Mat &src, const cv::Mat &mask, BodyScanCommon::Profile view, int x_offset, int y_offset, std::string &ERROR_IDs_as_json);
    private:
        cv::Mat match_with_template(const cv::Mat &templ, const cv::Mat &img, int match_method);
        void check_blob_size_new(const cv::Mat &part_bin_image, const cv::Mat &mask, float min_th, float max_th, const std::string &part_name, std::string &error_id);
        std::string create_json(BodyScanCommon::Profile view, const std::string &ERROR_IDs);
    };
}
#endif /* AHIAvatarGenContour_hpp */
