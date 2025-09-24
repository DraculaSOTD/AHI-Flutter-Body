//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#ifndef AvatarGenPredMesh_hpp
#define AvatarGenPredMesh_hpp

namespace avatar_gen {

    class pred_mesh {
    public:
        BodyScanCommon::SexType m_gender;
    private:
        std::string pred_mesh_error_id;
        std::vector<float> mvn_all_values;
        std::vector<float> sigma_22_inverse_times_offsets;
        std::vector<float> previous_sigma_22_inverse_times_offsets;
    public:

        std::vector<float> initialize_parameters(const std::vector<float> &data);

        std::string run(std::vector<float> &data_in, const std::vector<float> &thetas_pose,
                        const std::vector<float> &thetas_feet, std::vector<float> &OutVertices);

        std::string runInv(std::vector<float> &data_in, const std::vector<float> &thetas_pose,
                           const std::vector<float> &thetas_feet, std::vector<float> &OutVertices);

    private:
        std::vector<std::vector<float> > set_sigma_22(const std::vector<int> &index);

        std::vector<float> set_conditioned_value_offsets(const std::vector<float> &current_data,
                                                         const std::vector<int> &index);

        std::vector<float> set_all_values(const std::vector<std::vector<float> > &sigma_22,
                                          const std::vector<float> &conditioned_values_by_index,
                                          const std::vector<float> &conditioned_value_offsets,
                                          const std::vector<int> &index);

        std::vector<float> update_all_values(const std::vector<std::vector<float> > &sigma_22,
                                             const std::vector<float> &conditioned_values_by_index,
                                             const std::vector<float> &conditioned_value_offsets,
                                             const std::vector<int> &index);

        std::vector<float>
        deform(const std::vector<float> &thetas_pose, const std::vector<float> &thetas_feet);

        // MATRIX FUNCS
        std::vector<std::vector<float> > MatrixInverse(const std::vector<std::vector<float> > &A);

        float MatrixDeterminant(const std::vector<std::vector<float> > &D, int order);

        std::vector<std::vector<float> >
        MatrixTranspose(const std::vector<std::vector<float> > &A, int order);

        std::vector<float>
        Matrix_times_vector(const std::vector<std::vector<float> > &A, const std::vector<float> &b);
    };
}

#endif /* AvatarGenPredMesh_hpp */
