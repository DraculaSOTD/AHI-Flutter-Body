//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#ifndef AHI_INVERSION_H
#define AHI_INVERSION_H

#include <opencv2/imgproc/imgproc.hpp>
#include "AHIAvatarGenVec3.hpp"
#include "AHIAvatarGenFace.hpp"
#include "Common.hpp"
#include <map>

namespace avatar_gen {

    class inversion {
    private:
        cv::RNG m_rnd;
    public:
        inversion(void);

        bool
        invert(std::vector<float> &OutVertices, BodyScanCommon::SexType Gender, float H, float W,
               float Chest,
               float Waist, float Hip, float Inseam, float Fitness, std::string &errorString);

        std::vector<std::string>
        invert(BodyScanCommon::SexType Gender,
               float H,
               float W,
               float Chest,
               float Waist,
               float Hip,
               float Inseam,
               float Fitness,
               std::string &errorString,
               std::map<std::string, std::pair<char *, std::size_t>> &cvModelsMale,
               std::map<std::string, std::pair<char *, std::size_t>> &cvModelsFemale,
               const char delim = '\n'
        );

        void wrap_vertices(std::vector<AHIAvatarGenVec3> &dest, std::vector<float> &src);

        void wrap_faces(std::vector<AHIAvatarGenFace> &dest, std::vector<int> &src);

        void create_normals(std::vector<float> &Out, std::vector<AHIAvatarGenVec3> &Vertices,
                            std::vector<AHIAvatarGenFace> &Faces);

        std::string
        average(BodyScanCommon::SexType Gender, float H, float W, const std::vector<float> &Chests,
                const std::vector<float> &Waists, const std::vector<float> &Hips,
                const std::vector<float> &Inseams, const std::vector<float> &Fits,
                const std::vector<float> &Thighs);

        std::string
        average(BodyScanCommon::SexType Gender, float H, float W, const std::vector<float> &Chests,
                const std::vector<float> &Waists, const std::vector<float> &Hips,
                const std::vector<float> &Inseams, const std::vector<float> &Fits,
                const std::vector<float> &Thighs,
                const std::vector<float> &PercentBodyFat);

        float vector_mean(const std::vector<float> &V);

    private:
        std::vector<float> ArcLength(const std::vector<float> &Vi, float theta_RA, float theta_RL,
                                     std::string &error_id);

        void compute_part_laplacian_cot_weights(std::vector<float> &OutVertices,
                                                BodyScanCommon::SexType gender,
                                                const std::vector<int> &rings_as_vector,
                                                const std::vector<int> &num_of_points_per_ring,
                                                std::string &error_id);

        void gen_points_for_ransac(const std::vector<float> &ChestsIn,
                                   const std::vector<float> &WaistsIn,
                                   const std::vector<float> &HipsIn,
                                   const std::vector<float> &InseamsIn,
                                   const std::vector<float> &FitsIn, int resample_ntimes,
                                   std::vector<cv::Point2f> &src_points,
                                   std::vector<cv::Point2f> &dst_points,
                                   std::vector<bool> &ChestIdx, std::vector<bool> &WaistIdx,
                                   std::vector<bool> &HipIdx, std::vector<bool> &InseamIdx,
                                   std::vector<bool> &FitIdx);
    };
}

#endif /* AHI_INVERSION_H */
