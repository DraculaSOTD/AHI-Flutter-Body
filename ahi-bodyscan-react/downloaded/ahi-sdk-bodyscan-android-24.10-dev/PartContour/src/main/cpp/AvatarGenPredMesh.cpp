//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#include "AvatarGenPredMesh.hpp"
#include "AvatarGenCommon.hpp"

namespace avatar_gen {
    pred_mesh::pred_mesh(SexType g) : m_gender(g) {
        const common *c = common::getInstance();
        mvn_all_values = c->getMvnMu(m_gender);
    }

    std::vector<float> pred_mesh::initialize_parameters(const std::vector<float> &data) {
        const common *c = common::getInstance();
        try {
            std::vector<int> index;
            std::vector<std::vector<float> > sigma_22;
            float delta;
            float current_data_of_var_num_idx;
            std::vector<float> conditioned_values_by_index(7);
            std::vector<float> conditioned_value_offsets;

            mvn_all_values = c->getMvnMu(m_gender);

            float p = 5.0; // dividing the prediction into 5 iterations, can increase/reduce if you like
            bool isNeg = false;
            for (int i = 0; i < (int) data.size(); i++) {
                if (data[i] <= 0) {
                    isNeg = true;
                    break;
                }
            }
            if (isNeg) // if(any_of(data.begin(), data.end(), [](int i){return i<0;})) // check if any parameter is -ve (e.g. hip isn't known so -100 was already so the code will predict the hip circumference)
            {
                for (int idx = 0; idx < 7; idx++) {
                    if (data[idx] >= c->getRanges(m_gender)[idx][0] &&
                        data[idx] <= c->getRanges(m_gender)[idx][1]) {
                        index.push_back(idx);

                        sigma_22 = set_sigma_22(index);
                        if ((int) pred_mesh_error_id.size() > 0) {
                            return data;
                        }

                        conditioned_values_by_index[idx] = mvn_all_values[idx];
                        conditioned_value_offsets = set_conditioned_value_offsets(
                                conditioned_values_by_index, index);
                        if ((int) pred_mesh_error_id.size() > 0) {
                            return data;
                        }

                        mvn_all_values = set_all_values(sigma_22, conditioned_values_by_index,
                                                        conditioned_value_offsets, index);
                        if ((int) pred_mesh_error_id.size() > 0) {
                            return data;
                        }
                        current_data_of_var_num_idx = mvn_all_values[idx];
                        delta = (data[idx] - current_data_of_var_num_idx) / p;

                        for (int pidx = 0; pidx < (int) p; pidx++) {
                            conditioned_values_by_index[idx] =
                                    (pidx + 1) * delta + current_data_of_var_num_idx;
                            conditioned_value_offsets = set_conditioned_value_offsets(
                                    conditioned_values_by_index, index);
                            if ((int) pred_mesh_error_id.size() > 0) {
                                return data;
                            }
                            mvn_all_values = update_all_values(sigma_22,
                                                               conditioned_values_by_index,
                                                               conditioned_value_offsets, index);
                            if ((int) pred_mesh_error_id.size() > 0) {
                                return data;
                            }
                        }
                    }
                }
                return mvn_all_values;
            } else {
                mvn_all_values = data;
                return data;
            }
        } catch (...) {
            return data;
        }
    }

    std::string pred_mesh::run(std::vector<float> &data_in, const std::vector<float> &thetas_pose,
                               const std::vector<float> &thetas_feet,
                               std::vector<float> &OutVertices) {
        pred_mesh_error_id.clear();
        const common *c = common::getInstance();
        try {
            std::vector<float> data(7, -100);
            for (int k = 0; k < 7; k++) {
                data[k] = data_in[k];
            }
            std::vector<int> index;
            std::vector<std::vector<float> > sigma_22;
            float delta;
            float current_data_of_var_num_idx;
            std::vector<float> conditioned_values_by_index(7);
            std::vector<float> conditioned_value_offsets;
            OutVertices.clear();
            OutVertices.resize((int) c->getAvgVerts(m_gender).size());

            mvn_all_values = c->getMvnMu(m_gender);
            float p = 5.0; // dividing the prediction into 5 iterations, can increase/reduce if you like
            bool isNeg = false;
            for (int i = 0; i < (int) data.size(); i++) {
                if (data[i] <= 0) {
                    isNeg = true;
                    break;
                }
            }
            if (isNeg) { // if(any_of(data.begin(), data.end(), [](int i){return i<0;})) // check if any parameter is -ve (e.g. hip isn't known so -100 was already so the code will predict the hip circumference)
                for (int idx = 0; idx < 7; idx++) {
                    if (data[idx] >= c->getRanges(m_gender)[idx][0] &&
                        data[idx] <= c->getRanges(m_gender)[idx][1]) { // data[idx] > 0 &&
                        index.push_back(idx);

                        sigma_22 = set_sigma_22(index);
                        if ((int) pred_mesh_error_id.size() > 0) {
                            return (pred_mesh_error_id);
                        }
                        conditioned_values_by_index[idx] = mvn_all_values[idx];
                        conditioned_value_offsets = set_conditioned_value_offsets(
                                conditioned_values_by_index, index);
                        if ((int) pred_mesh_error_id.size() > 0) {
                            return (pred_mesh_error_id);
                        }
                        mvn_all_values = set_all_values(sigma_22, conditioned_values_by_index,
                                                        conditioned_value_offsets, index);
                        if ((int) pred_mesh_error_id.size() > 0) {
                            return (pred_mesh_error_id);
                        }
                        current_data_of_var_num_idx = mvn_all_values[idx];
                        delta = (data[idx] - current_data_of_var_num_idx) / p;
                        for (int pidx = 0; pidx < (int) p; pidx++) {
                            conditioned_values_by_index[idx] =
                                    (pidx + 1) * delta + current_data_of_var_num_idx;
                            conditioned_value_offsets = set_conditioned_value_offsets(
                                    conditioned_values_by_index, index);
                            if ((int) pred_mesh_error_id.size() > 0) {
                                return (pred_mesh_error_id);
                            }
                            mvn_all_values = update_all_values(sigma_22,
                                                               conditioned_values_by_index,
                                                               conditioned_value_offsets, index);
                            if ((int) pred_mesh_error_id.size() > 0) {
                                return (pred_mesh_error_id);
                            }
                        }
                    }
                }
                data = mvn_all_values; // % predicted data
            } else {
                mvn_all_values = data;
            }
            // mvn_all_values - mvn_mu, c is the parameters' index we are dealing with
            std::vector<float> shape_coefficients(7);
            for (int idx = 0; idx < 7; idx++) {
                shape_coefficients[idx] = mvn_all_values[idx] - c->getMvnMu(m_gender)[idx];
            }

            std::vector<float> svb((int) c->getAvgVerts(m_gender).size());
            svb = Matrix_times_vector(c->getSv(m_gender), shape_coefficients);
            if ((int) pred_mesh_error_id.size() > 0) {
                return (pred_mesh_error_id);
            }
            if ((std::abs(thetas_pose[0]) + std::abs(thetas_pose[1]) + std::abs(thetas_pose[2]) +
                 std::abs(thetas_pose[3]) + std::abs(thetas_feet[0]) + std::abs(thetas_feet[1])) >
                0.01) { // 08/03
                OutVertices = deform(thetas_pose, thetas_feet);
            } else {
                OutVertices = c->getAvgVerts(m_gender);
            }
            if ((int) pred_mesh_error_id.size() > 0) {
                return (pred_mesh_error_id);
            }
            for (int i = 0; i < (int) c->getAvgVerts(m_gender).size(); i++) {
                OutVertices[i] = OutVertices[i] + svb[i];
            }
            sigma_22_inverse_times_offsets = std::vector<float>();
            previous_sigma_22_inverse_times_offsets = std::vector<float>();
            mvn_all_values = std::vector<float>();
            for (int k = 0; k < 7; k++) {
                data_in[k] = data[k];
            }
            return ("Passed");
        } catch (cv::Exception &e) {
            sigma_22_inverse_times_offsets = std::vector<float>();
            previous_sigma_22_inverse_times_offsets = std::vector<float>();
            mvn_all_values = std::vector<float>();
            std::string error_id = "11";
            return (error_id);
        }
    }

    std::string
    pred_mesh::runInv(std::vector<float> &data_in, const std::vector<float> &thetas_pose,
                      const std::vector<float> &thetas_feet,
                      std::vector<float> &OutVertices) {
        pred_mesh_error_id.clear();
        const common *c = common::getInstance();
        try {
            std::vector<float> data(7, -100);
            for (int k = 0; k < 7; k++) {
                data[k] = data_in[k];
            }
            std::vector<int> index;
            std::vector<std::vector<float> > sigma_22;
            float delta;
            float current_data_of_var_num_idx;
            std::vector<float> conditioned_values_by_index(7);
            std::vector<float> conditioned_value_offsets;
            OutVertices.clear();
            OutVertices.resize((int) c->getVertsInv(m_gender).size());
            mvn_all_values = c->getMvnMu(m_gender);
            float p = 5.0; // dividing the prediction into 5 iterations, can increase/reduce if you like
            bool isNeg = false;
            for (int i = 0; i < (int) data.size(); i++) {
                if (data[i] <= 0) {
                    isNeg = true;
                    break;
                }
            }
            if (isNeg) { // if(any_of(data.begin(), data.end(), [](int i){return i<0;})) // check if any parameter is -ve (e.g. hip isn't known so -100 was already so the code will predict the hip circumference)
                for (int idx = 0; idx < 7; idx++) {
                    if (data[idx] >= c->getRanges(m_gender)[idx][0] &&
                        data[idx] <= c->getRanges(m_gender)[idx][1]) {
                        index.push_back(idx);
                        sigma_22 = set_sigma_22(index);
                        if ((int) pred_mesh_error_id.size() > 0) {
                            return (pred_mesh_error_id);
                        }
                        conditioned_values_by_index[idx] = mvn_all_values[idx];
                        conditioned_value_offsets = set_conditioned_value_offsets(
                                conditioned_values_by_index, index);
                        if ((int) pred_mesh_error_id.size() > 0) {
                            return (pred_mesh_error_id);
                        }
                        mvn_all_values = set_all_values(sigma_22, conditioned_values_by_index,
                                                        conditioned_value_offsets, index);
                        if ((int) pred_mesh_error_id.size() > 0) {
                            return (pred_mesh_error_id);
                        }
                        current_data_of_var_num_idx = mvn_all_values[idx];
                        delta = (data[idx] - current_data_of_var_num_idx) / p;
                        for (int pidx = 0; pidx < (int) p; pidx++) {
                            conditioned_values_by_index[idx] =
                                    (pidx + 1) * delta + current_data_of_var_num_idx;
                            conditioned_value_offsets = set_conditioned_value_offsets(
                                    conditioned_values_by_index, index);
                            if ((int) pred_mesh_error_id.size() > 0) {
                                return (pred_mesh_error_id);
                            }
                            mvn_all_values = update_all_values(sigma_22,
                                                               conditioned_values_by_index,
                                                               conditioned_value_offsets, index);
                            if ((int) pred_mesh_error_id.size() > 0) {
                                return (pred_mesh_error_id);
                            }
                        }
                    }
                }
                data = mvn_all_values; // % predicted data
            } else {
                mvn_all_values = data;
            }
            // mvn_all_values - mvn_mu, c is the parameters' index we are dealing with
            std::vector<float> shape_coefficients(7);
            for (int idx = 0; idx < 7; idx++) {
                shape_coefficients[idx] = mvn_all_values[idx] - c->getMvnMu(m_gender)[idx];
            }
            std::vector<float> svb((int) c->getVertsInv(m_gender).size());
            svb = Matrix_times_vector(c->getSvInv(m_gender), shape_coefficients);
            if ((int) pred_mesh_error_id.size() > 0) {
                return (pred_mesh_error_id);
            }
            if ((std::abs(thetas_pose[0]) + std::abs(thetas_pose[1]) + std::abs(thetas_pose[2]) +
                 std::abs(thetas_pose[3]) + std::abs(thetas_feet[0]) + std::abs(thetas_feet[1])) >
                0.01) { // 08/03
                OutVertices = deform(thetas_pose, thetas_feet);
            } else {
                OutVertices = c->getVertsInv(m_gender);
            }
            if ((int) pred_mesh_error_id.size() > 0) {
                return (pred_mesh_error_id);
            }
            for (int i = 0; i < (int) c->getVertsInv(m_gender).size(); i++) {
                OutVertices[i] = OutVertices[i] + svb[i];
            }
            sigma_22_inverse_times_offsets = std::vector<float>();
            previous_sigma_22_inverse_times_offsets = std::vector<float>();
            mvn_all_values = std::vector<float>();
            for (int k = 0; k < 7; k++) {
                data_in[k] = data[k];
            }
            return ("Passed");
        } catch (cv::Exception &e) {
            sigma_22_inverse_times_offsets = std::vector<float>();
            previous_sigma_22_inverse_times_offsets = std::vector<float>();
            mvn_all_values = std::vector<float>();
            std::string error_id = "11";
            return (error_id);
        }
    }

// PRIVATE
    std::vector<std::vector<float> > pred_mesh::set_sigma_22(const std::vector<int> &index) {
        int n = (int) index.size();
        const common *c = common::getInstance();
        std::vector<std::vector<float> > sigma_22(n, std::vector<float>(n, 0.0));
        try {
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < n; j++) {
                    sigma_22[i][j] = c->getCov(m_gender)[index[i]][index[j]];
                }
            }
            return sigma_22;
        } catch (int) {
            pred_mesh_error_id = "11";
            return (sigma_22);
        }
    }

    std::vector<float>
    pred_mesh::set_conditioned_value_offsets(const std::vector<float> &current_data,
                                             const std::vector<int> &index) {
        const common *c = common::getInstance();
        int n = (int) index.size();
        std::vector<float> conditioned_value_offsets(n);
        try {
            for (int i = 0; i < n; i++) {
                conditioned_value_offsets[i] =
                        current_data[index[i]] - c->getMvnMu(m_gender)[index[i]];
            }
            return conditioned_value_offsets;
        } catch (int) {
            pred_mesh_error_id = "11";
            return conditioned_value_offsets;
        }
    }

    std::vector<float> pred_mesh::set_all_values(const std::vector<std::vector<float> > &sigma_22,
                                                 const std::vector<float> &conditioned_values_by_index,
                                                 const std::vector<float> &conditioned_value_offsets,
                                                 const std::vector<int> &index) {
        const common *c = common::getInstance();
        try {
            sigma_22_inverse_times_offsets.resize((int) index.size());
            previous_sigma_22_inverse_times_offsets.resize((int) index.size());

            std::vector<float> e(7);

            mvn_all_values = c->getMvnMu(m_gender);
            sigma_22_inverse_times_offsets = Matrix_times_vector(MatrixInverse(sigma_22),
                                                                 conditioned_value_offsets);
            for (int b = 0; b < (int) index.size(); b++) {
                int f = index[b];
                for (int k = 0; k < 7; k++) {
                    e[k] = c->getCov(m_gender)[f][k];
                }
                float c = sigma_22_inverse_times_offsets[b];
                for (int a = 0; a < 7; a++) {
                    mvn_all_values[a] += c * e[a];
                }
                mvn_all_values[f] = conditioned_values_by_index[f];
            }
            for (int j = 0; j < (int) index.size(); j++) {
                previous_sigma_22_inverse_times_offsets[j] = sigma_22_inverse_times_offsets[j];
            }
            return mvn_all_values;
        } catch (int) {
            pred_mesh_error_id = "11";
            return mvn_all_values;
        }
    }

    std::vector<float>
    pred_mesh::update_all_values(const std::vector<std::vector<float> > &sigma_22,
                                 const std::vector<float> &conditioned_values_by_index,
                                 const std::vector<float> &conditioned_value_offsets,
                                 const std::vector<int> &index) {
        const common *c = common::getInstance();
        try {
            std::vector<float> e(7);
            sigma_22_inverse_times_offsets = Matrix_times_vector(MatrixInverse(sigma_22),
                                                                 conditioned_value_offsets);
            for (int i = 0; i < (int) index.size(); i++) {
                int g = index[i];
                for (int k = 0; k < 7; k++) {
                    e[k] = c->getCov(m_gender)[g][k];
                }
                float b =
                        sigma_22_inverse_times_offsets[i] -
                        previous_sigma_22_inverse_times_offsets[i];
                for (int f = 0; f < 7; f++) {
                    mvn_all_values[f] += b * e[f];
                }
            }
            for (int i = 0; i < (int) index.size(); i++) {
                mvn_all_values[index[i]] = conditioned_values_by_index[index[i]];
            }
            for (int j = 0; j < (int) index.size(); j++) {
                previous_sigma_22_inverse_times_offsets[j] = sigma_22_inverse_times_offsets[j];
            }
            return mvn_all_values;
        } catch (int) {
            pred_mesh_error_id = "11";
            return mvn_all_values;
        }
    }

    std::vector<float>
    pred_mesh::deform(const std::vector<float> &thetas_pose,
                      const std::vector<float> &thetas_feet) {
        const common *c = common::getInstance();
        int L = (int) c->getAvgVerts(m_gender).size();
        std::vector<float> deformed_mesh(L, 0);
        std::vector<float> avg_vertices_initial_deform(L, 0);
        std::vector<float> Pr(3, 0);

        float theta;
        try {
            for (int b = 0; b < 17; b++) {// 17 bones made of  18 SkV skel verts
                Pr[0] = 0.0;
                Pr[1] = 0.0;
                Pr[2] = 0.0;
                theta = 0.0;
                if (b == 6) {// rf
                    Pr[0] = c->getSkV(m_gender)[6][0];
                    Pr[1] = c->getSkV(m_gender)[6][1];
                    Pr[2] = c->getSkV(m_gender)[6][2];
                    theta = thetas_feet[0]; // thetas_rf;
                } else if (b == 10) {// lf
                    Pr[0] = c->getSkV(m_gender)[10][0];
                    Pr[1] = c->getSkV(m_gender)[10][1];
                    Pr[2] = c->getSkV(m_gender)[10][2];
                    theta = thetas_feet[1]; // thetas_lf;
                }
                for (int i = 0; i < L; i = i + 3) {
                    avg_vertices_initial_deform[i] = avg_vertices_initial_deform[i] +
                                                     c->getBonW(m_gender)[(int) i / 3][b] * (Pr[0] +
                                                                                             cos(theta) *
                                                                                             (c->getAvgVerts(
                                                                                                     m_gender)[i] -
                                                                                              Pr[0]) +
                                                                                             sin(theta) *
                                                                                             (c->getAvgVerts(
                                                                                                     m_gender)[
                                                                                                      i +
                                                                                                      2] -
                                                                                              Pr[2]));
                    avg_vertices_initial_deform[i + 1] = avg_vertices_initial_deform[i + 1] +
                                                         c->getBonW(m_gender)[(int) i / 3][b] *
                                                         c->getAvgVerts(m_gender)[i + 1];
                    avg_vertices_initial_deform[i + 2] = avg_vertices_initial_deform[i + 2] +
                                                         c->getBonW(m_gender)[(int) i / 3][b] *
                                                         (Pr[2] -
                                                          sin(theta) *
                                                          (c->getAvgVerts(
                                                                  m_gender)[i] -
                                                           Pr[0]) +
                                                          cos(theta) *
                                                          (c->getAvgVerts(
                                                                  m_gender)[
                                                                   i +
                                                                   2] -
                                                           Pr[2]));
                }
            }
            for (int b = 0; b < 17; b++) {// 17 bones made of  18 SkV skel verts
                Pr[0] = 0.0;
                Pr[1] = 0.0;
                Pr[2] = 0.0;
                theta = 0.0;
                if (b == 12 || b == 13) {// ra
                    Pr[0] = c->getSkV(m_gender)[12][0];
                    Pr[1] = c->getSkV(m_gender)[12][1];
                    Pr[2] = c->getSkV(m_gender)[12][2];
                    theta = thetas_pose[0];
                } else if (b == 15 || b == 16) {// la
                    Pr[0] = c->getSkV(m_gender)[15][0];
                    Pr[1] = c->getSkV(m_gender)[15][1];
                    Pr[2] = c->getSkV(m_gender)[15][2];
                    theta = thetas_pose[1];
                } else if (b == 4 || b == 5 || b == 6) {// rl
                    Pr[0] = c->getSkV(m_gender)[4][0];
                    Pr[1] = c->getSkV(m_gender)[4][1];
                    Pr[2] = c->getSkV(m_gender)[4][2];
                    theta = thetas_pose[2];
                } else if (b == 8 || b == 9 || b == 10) {// ll
                    Pr[0] = c->getSkV(m_gender)[8][0];
                    Pr[1] = c->getSkV(m_gender)[8][1];
                    Pr[2] = c->getSkV(m_gender)[8][2];
                    theta = thetas_pose[3];
                }
                for (int i = 0; i < L; i = i + 3) {
                    deformed_mesh[i] = deformed_mesh[i] + c->getBonW(m_gender)[(int) i / 3][b] *
                                                          (Pr[0] + cos(theta) *
                                                                   (avg_vertices_initial_deform[i] -
                                                                    Pr[0]) - sin(theta) *
                                                                             (avg_vertices_initial_deform[
                                                                                      i + 1] -
                                                                              Pr[1]));
                    deformed_mesh[i + 1] =
                            deformed_mesh[i + 1] + c->getBonW(m_gender)[(int) i / 3][b] *
                                                   (Pr[1] + sin(theta) *
                                                            (avg_vertices_initial_deform[i] -
                                                             Pr[0]) + cos(theta) *
                                                                      (avg_vertices_initial_deform[
                                                                               i + 1] -
                                                                       Pr[1]));
                    deformed_mesh[i + 2] =
                            deformed_mesh[i + 2] + c->getBonW(m_gender)[(int) i / 3][b] *
                                                   avg_vertices_initial_deform[i + 2];
                }
            }
            return deformed_mesh;
        } catch (int) {
            pred_mesh_error_id = "11";
            return deformed_mesh;
        }
    }

// MATRIX FN
    std::vector<std::vector<float> >
    pred_mesh::MatrixInverse(const std::vector<std::vector<float> > &A) {
        try {
            int order = (int) A.size();
            std::vector<std::vector<float> > B(order, std::vector<float>(order, 0.0));
            std::vector<std::vector<float> > fac(order, std::vector<float>(order, 0.0));
            std::vector<std::vector<float> > facT(order, std::vector<float>(order, 0.0));
            std::vector<std::vector<float> > Ainv(order, std::vector<float>(order, 0.0));
            if (order == 1) {
                Ainv[0][0] = 1.0 / (A[0][0] + 1.0e-6);
                return Ainv;
            }
            float d;
            int p, q, m, n, i, j;
            for (q = 0; q < order; q++) {
                for (p = 0; p < order; p++) {
                    m = 0;
                    n = 0;
                    for (i = 0; i < order; i++) {
                        for (j = 0; j < order; j++) {
                            if (i != q && j != p) {
                                B[m][n] = A[i][j];
                                if (n < (order - 2)) {
                                    n++;
                                } else {
                                    n = 0;
                                    m++;
                                }
                            }
                        }
                    }
                    fac[q][p] = pow(-1.0, q + p) * MatrixDeterminant(B, order - 1);
                }
            }
            facT = MatrixTranspose(fac, order);
            d = MatrixDeterminant(A, order);
            for (i = 0; i < order; i++) {
                for (j = 0; j < order; j++) {
                    Ainv[i][j] = facT[i][j] / (d + 1.0e-10);
                }
            }
            return Ainv;
        } catch (int) {
            pred_mesh_error_id = "11";
            return (A);
        }
    }

    float pred_mesh::MatrixDeterminant(const std::vector<std::vector<float> > &D, int order) {
        try {
            float s = 1.0, det = 0.0;
            std::vector<std::vector<float> > E(order, std::vector<float>(order, 0.0));
            int i, j, m, n, c;
            if (order == 1) {
                return (D[0][0]);
            } else {
                det = 0;
                for (c = 0; c < order; c++) {
                    m = 0;
                    n = 0;
                    for (i = 0; i < order; i++) {
                        for (j = 0; j < order; j++) {
                            E[i][j] = 0;
                            if (i != 0 && j != c) {
                                E[m][n] = D[i][j];
                                if (n < (order - 2)) {
                                    n++;
                                } else {
                                    n = 0;
                                    m++;
                                }
                            }
                        }
                    }
                    det = det + s * (D[0][c] * MatrixDeterminant(E, order - 1));
                    s = -1 * s;
                }
            }
            return (det);
        } catch (int) {
            pred_mesh_error_id = "11";
            return (1.0);
        }
    }

    std::vector<std::vector<float> >
    pred_mesh::MatrixTranspose(const std::vector<std::vector<float> > &A, int order) {
        try {
            int i, j;
            std::vector<std::vector<float> > Atrans(order, std::vector<float>(order, 0.0));
            for (i = 0; i < order; i++) {
                for (j = 0; j < order; j++) {
                    Atrans[i][j] = A[j][i];
                }
            }
            return Atrans;
        } catch (int) {
            pred_mesh_error_id = "11";
            return (A);
        }
    }

    std::vector<float> pred_mesh::Matrix_times_vector(const std::vector<std::vector<float> > &A,
                                                      const std::vector<float> &b) {
        std::vector<float> c((int) A.size());
        try {
            for (int row = 0; row < (int) A.size(); row++) {
                c[row] = 0.0;
                for (int col = 0; col < (int) b.size(); col++) {
                    c[row] += A[row][col] * b[col];
                }
            }
            return c;
        } catch (int) {
            pred_mesh_error_id = "11";
            return (c);
        }
    }
}