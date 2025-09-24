//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#include "ahiModelsZoo.hpp"

std::vector<std::string> ahiModelsZoo::getSvrModelList(std::string measCatagory) {
    std::vector<std::string> result;
    if (measCatagory.find("shape") != std::string::npos) {
        result.push_back("andriod_svr_image_features_UWA_all");
        result.push_back("chest_svr_image_features");
        result.push_back("waist_svr_image_features");
        result.push_back("fat_svr_image_features");
        result.push_back("fat_svr_image_features_UWA_all");
        result.push_back("female_chest_svr_image_features");
        result.push_back("female_chest_svr_image_features_v2");
        result.push_back("female_chest_svr_image_features_v2_UWA_all");
        result.push_back("female_chest_svr_image_features_v3");
        result.push_back("female_hip_svr_image_features");
        result.push_back("female_hip_svr_image_features_v2");
        result.push_back("female_hip_svr_image_features_v2_UWA_all");
        result.push_back("female_hip_svr_image_features_v3");
        result.push_back("female_inseam_svr_image_features");
        result.push_back("female_inseam_svr_image_features_v2");
        result.push_back("female_inseam_svr_image_features_v2_UWA_all");
        result.push_back("female_inseam_svr_image_features_v3");
        result.push_back("female_waist_svr_image_features");
        result.push_back("female_waist_svr_image_features_v2");
        result.push_back("female_waist_svr_image_features_v2_UWA_all");
        result.push_back("female_waist_svr_image_features_v3");
        result.push_back("hip_svr_image_features");
        result.push_back("inseam_svr_image_features");
        result.push_back("male_chest_svr_image_features");
        result.push_back("male_chest_svr_image_features_v2");
        result.push_back("male_chest_svr_image_features_v2_UWA_all");
        result.push_back("male_chest_svr_image_features_v3");
        result.push_back("male_hip_svr_image_features");
        result.push_back("male_hip_svr_image_features_v2");
        result.push_back("male_hip_svr_image_features_v2_UWA_all");
        result.push_back("male_hip_svr_image_features_v3");
        result.push_back("male_inseam_svr_image_features");
        result.push_back("male_inseam_svr_image_features_v2");
        result.push_back("male_inseam_svr_image_features_v2_UWA_all");
        result.push_back("male_inseam_svr_image_features_v3");
        result.push_back("male_waist_svr_image_features");
        result.push_back("male_waist_svr_image_features_v2");
        result.push_back("male_waist_svr_image_features_v2_UWA_all");
        result.push_back("male_waist_svr_image_features_v3");
        result.push_back("thigh_svr_image_features_UWA_all");
    }
    if (measCatagory.find("composition") != std::string::npos) {
        result.push_back("FFM_svr_image_features_UWA_all");
        result.push_back("gynoid_svr_image_features_UWA_all");
        result.push_back("visceral_svr_image_features_UWA_all");
        result.push_back("weight_svr_image_features");
        result.push_back("weight_svr_image_features_UWA_all");
    }
    return result;
}
