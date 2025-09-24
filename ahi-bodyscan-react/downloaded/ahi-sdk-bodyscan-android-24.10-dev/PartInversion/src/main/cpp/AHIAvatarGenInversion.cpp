//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#include "AHIAvatarGenInversion.hpp"
#include "AHIAvatarGenPredMesh.hpp"
#include "AvatarGenCommon.hpp"

namespace avatar_gen {
    inversion::inversion(void) : m_rnd(time(0)) {
    }

    void inversion::wrap_vertices(std::vector<AHIAvatarGenVec3> &dest, std::vector<float> &src) {
        size_t nVerts = src.size();
        for (int i = 0; i < nVerts; i += 3) {
            dest.push_back(AHIAvatarGenVec3(&src.data()[i]));
        }
    }

    void inversion::wrap_faces(std::vector<AHIAvatarGenFace> &dest, std::vector<int> &src) {
        size_t nFaces = src.size();
        for (int i = 0; i < nFaces; i += 3) {
            dest.push_back(AHIAvatarGenFace(&src.data()[i]));
        }
    }

    void inversion::create_normals(std::vector<float> &Out, std::vector<AHIAvatarGenVec3> &Vertices,
                                   std::vector<AHIAvatarGenFace> &Faces) {
        // Fill out with empty values (one triplet per vert).
        Out.assign(Vertices.size() * 3, 0);

        // wrap output in a vector of AHIAvatarGenVec3...
        std::vector<AHIAvatarGenVec3> verts;

        size_t nVerts = Vertices.size();
        for (int i = 0; i < nVerts; i++) {
            verts.push_back(AHIAvatarGenVec3(&Out.data()[i * 3]));
        }

        size_t nFaces = Faces.size();
        AHIAvatarGenVec3 e1;
        AHIAvatarGenVec3 e2;
        AHIAvatarGenVec3 no;

        for (int i = 0; i < nFaces; i++) {
            int ia = Faces[i][0];
            int ib = Faces[i][1];
            int ic = Faces[i][2];

            e1.subtract(Vertices[ia], Vertices[ib]);
            e2.subtract(Vertices[ic], Vertices[ib]);
            no.cross(e1, e2);

            verts[ia].add(no);
            verts[ib].add(no);
            verts[ic].add(no);
        }

        nVerts = verts.size();
        for (int i = 0; i < nVerts; i++) {
            verts[i].normalize();
        }
    }

    bool
    inversion::invert(std::vector<float> &OutVertices, BodyScanCommon::SexType Gender, float H,
                      float W, float Chest,
                      float Waist, float Hip, float Inseam, float Fitness,
                      std::string &errorString) {
        const common *c = common::getInstance();
        pred_mesh pm(Gender);
        try {
            std::vector<float> arcL(3, -100);
            std::vector<float> Parameters(7, -100);
            Parameters[0] = H;
            Parameters[1] = W;
            Parameters[2] = Chest;
            Parameters[3] = Waist;
            Parameters[4] = Hip;
            Parameters[5] = Inseam;
            Parameters[6] = Fitness;
            std::vector<float> thetas_pose(4, 0.0); // arms and legs poses
            std::vector<float> thetas_feet(2, 0.0);

            std::string output_msg = pm.runInv(Parameters, thetas_pose, thetas_feet, OutVertices);

            if (output_msg == "Passed") {
                compute_part_laplacian_cot_weights(OutVertices, Gender,
                                                   c->getLaplacianRingsAsVectors(Gender),
                                                   c->getLaplacianRings(Gender), errorString);
                if (errorString.size() > 0) {
                    return false;
                }
                errorString = "";
                return true;
            } else {
                return false;
            }
        } catch (cv::Exception e) {
            errorString = "11";
            return false;
        }
    }

    std::vector<std::string>
    inversion::invert(
            BodyScanCommon::SexType Gender,
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
            const char delim
    ) {
        const common *c = common::getInstance(Gender, cvModelsMale, cvModelsFemale);
        std::vector<std::string> mesh;
        pred_mesh pm(Gender);
        try {
            std::vector<float> arcL(3, -100);
            std::vector<float> Parameters(7, -100);
            Parameters[0] = H;
            Parameters[1] = W;
            Parameters[2] = Chest;
            Parameters[3] = Waist;
            Parameters[4] = Hip;
            Parameters[5] = Inseam;
            Parameters[6] = Fitness;
            std::vector<float> thetas_pose(4, 0.0); // arms and legs poses
            std::vector<float> thetas_feet(2, 0.0);
            std::vector<float> OutVertices;
            std::string output_msg = pm.runInv(Parameters, thetas_pose, thetas_feet, OutVertices);
            if (output_msg == "Passed") {
                compute_part_laplacian_cot_weights(OutVertices, Gender,
                                                   c->getLaplacianRingsAsVectors(Gender),
                                                   c->getLaplacianRings(Gender), errorString);
                if (errorString.size() > 0) {
                    errorString = "11";
                    return mesh;
                }
                for (int i = 0; i < (int) OutVertices.size(); i += 3) {
                    std::ostringstream tmpV;
                    tmpV << "v " << OutVertices[i] << " " << OutVertices[i + 1] << " "
                         << OutVertices[i + 2] << delim;
                    mesh.push_back(tmpV.str());
                }
                for (int i = 0; i < c->getFacesInv(Gender).size(); i += 3) {
                    std::ostringstream tmpF;
                    tmpF << "f " << c->getFacesInv(Gender)[i] + 1 << " "
                         << c->getFacesInv(Gender)[i + 1] + 1 << " "
                         << c->getFacesInv(Gender)[i + 2] + 1 << delim;
                    mesh.push_back(tmpF.str());
                }
                errorString = "";
                return mesh;
            } else {
                errorString = "11";
                return mesh;
            }
        } catch (cv::Exception e) {
            errorString = "11";
            return mesh;
        }
    }

    std::string inversion::average(BodyScanCommon::SexType Gender, float H, float W,
                                   const std::vector<float> &Chests,
                                   const std::vector<float> &Waists, const std::vector<float> &Hips,
                                   const std::vector<float> &Inseams,
                                   const std::vector<float> &Fits,
                                   const std::vector<float> &Thighs) {
        std::ostringstream jsonStr;
        jsonStr << "{ \n";
        std::string genderStr = Gender == BodyScanCommon::male ? "M" : "F";
        jsonStr << "\t\"gender\" : \"" << genderStr << "\",\n";
        jsonStr << "\t\"height\":" << H << ",\n";
        jsonStr << "\t\"weight\":" << W << ",\n";
        jsonStr << "\t\"chest\":" << vector_mean(Chests) << ",\n";
        jsonStr << "\t\"waist\":" << vector_mean(Waists) << ",\n";
        jsonStr << "\t\"hip\":" << vector_mean(Hips) << ",\n";
        jsonStr << "\t\"inseam\":" << vector_mean(Inseams) << ",\n";
        jsonStr << "\t\"fitness\":" << vector_mean(Fits) << ",\n";
        jsonStr << "\t\"thigh\":" << vector_mean(Thighs) << ",\n";
        jsonStr << "\t\"error_id\" : \"0\"";
        jsonStr << "\n}";
        return jsonStr.str();
    }

    std::string inversion::average(BodyScanCommon::SexType Gender, float H, float W,
                                   const std::vector<float> &Chests,
                                   const std::vector<float> &Waists, const std::vector<float> &Hips,
                                   const std::vector<float> &Inseams,
                                   const std::vector<float> &Fits,
                                   const std::vector<float> &Thighs,
                                   const std::vector<float> &PercentBodyFat) {
        std::ostringstream jsonStr;
        jsonStr << "{ \n";
        std::string genderStr = Gender == BodyScanCommon::male ? "M" : "F";
        jsonStr << "\t\"gender\" : \"" << genderStr << "\",\n";
        jsonStr << "\t\"height\":" << H << ",\n";
        jsonStr << "\t\"weight\":" << W << ",\n";
        jsonStr << "\t\"chest\":" << vector_mean(Chests) << ",\n";
        jsonStr << "\t\"waist\":" << vector_mean(Waists) << ",\n";
        jsonStr << "\t\"hip\":" << vector_mean(Hips) << ",\n";
        jsonStr << "\t\"inseam\":" << vector_mean(Inseams) << ",\n";
        jsonStr << "\t\"fitness\":" << vector_mean(Fits) << ",\n";
        jsonStr << "\t\"thigh\":" << vector_mean(Thighs) << ",\n";
        jsonStr << "\t\"PercentBodyFat\":" << vector_mean(PercentBodyFat) << ",\n";
        jsonStr << "\t\"error_id\" : \"0\"";
        jsonStr << "\n}";
        return jsonStr.str();
    }

// PRIVATE
    std::vector<float>
    inversion::ArcLength(const std::vector<float> &Vi, float theta_RA, float theta_RL,
                         std::string &error_id) {
        std::vector<float> arcL(3, -100);
        if (Vi.size() < BodyScanCommon::N_VERTS_INV_3) {
            error_id = "Size of xyz Verts is not 3*Nverts";
            return arcL;
        }
        try {
            arcL.clear();
            const float(&Vxyz)[BodyScanCommon::N_VERTS_INV][3] = *reinterpret_cast<const float (*)[BodyScanCommon::N_VERTS_INV][3]>(&(Vi[0]));
            std::vector<double> X;
            std::vector<double> Y;
            std::vector<double> Z;
            std::vector<int> idx_pca;
            cv::Scalar color;
            // Mat drawing;
            std::vector<std::vector<cv::Point>> contours(1, std::vector<cv::Point>(1));
            std::vector<cv::Vec4i> hierarchy;

            const common *c = common::getInstance();

            idx_pca = c->getInvRightThigh();
            idx_pca.insert(idx_pca.end(), c->getInvRightCalf().begin(), c->getInvRightCalf().end());

            for (int i = 0; i < 3; i++) {
                const std::vector<int> idx =
                        i == 0 ? c->getInvRightCalf() : i == 1 ? c->getInvRightThigh()
                                                               : c->getInvRightUpperArm();
                // 3D
                cv::Mat data_pts = cv::Mat((int) idx_pca.size(), 3, CV_64FC1); // for 3D PCA
                for (int n = 0; n < idx_pca.size(); n++) {
                    int j = idx_pca[n] - 1; // matlb starts with one
                    data_pts.at<float>(n, 0) = Vxyz[j][0];
                    data_pts.at<float>(n, 1) = Vxyz[j][1];
                    data_pts.at<float>(n, 2) = Vxyz[j][2];
                }
                // Perform PCA analysis
                cv::PCA pca_analysis(data_pts, cv::Mat(), cv::PCA::DATA_AS_ROW);
                // Store the eigenvalues and eigenvectors
                std::vector<cv::Point3d> eigen_vecs(3);
                std::vector<double> eigen_val(3);
                for (int k = 0; k < 3; k++) {
                    eigen_vecs[k] = cv::Point3d(pca_analysis.eigenvectors.at<double>(k, 0),
                                                pca_analysis.eigenvectors.at<double>(k, 1),
                                                pca_analysis.eigenvectors.at<double>(k, 2));
                    eigen_val[k] = pca_analysis.eigenvalues.at<double>(k, 0);
                }
                double min_d = 10000., max_d = -10000.;
                for (int n = 0; n < idx.size(); n++) {
                    int j = idx[n] - 1;// matlab starts with one hence -1
                    X.push_back(eigen_vecs[0].x * (double) (Vxyz[j][0]) +
                                eigen_vecs[0].y * (double) (Vxyz[j][1]) +
                                eigen_vecs[0].z * (double) (Vxyz[j][2]));
                    Y.push_back(eigen_vecs[1].x * (double) (Vxyz[j][0]) +
                                eigen_vecs[1].y * (double) (Vxyz[j][1]) +
                                eigen_vecs[1].z * (double) (Vxyz[j][2]));
                    Z.push_back(eigen_vecs[2].x * (double) (Vxyz[j][0]) +
                                eigen_vecs[2].y * (double) (Vxyz[j][1]) +
                                eigen_vecs[2].z * (double) (Vxyz[j][2]));
                    min_d = MIN(min_d, Y[n]);
                    min_d = MIN(min_d, Z[n]);
                    max_d = MAX(max_d, Y[n]);
                    max_d = MAX(max_d, Z[n]);
                }
                data_pts.release();
                eigen_vecs.clear();
                eigen_val.clear();
                for (int n = 0; n < (int) Y.size(); n++) {
                    float yi = (Y[n] - min_d) / (max_d - min_d) * (480. - 20.) +
                               20.; // 20 pixels on borders for safety
                    float zi = (Z[n] - min_d) / (max_d - min_d) * (480. - 20.) +
                               20.; // 20 pixels on borders for safety
                    contours[0].push_back(cv::Point(int(yi), int(zi)));
                }
                std::vector<std::vector<cv::Point>> hull(contours.size());
                convexHull(cv::Mat(contours[0]), hull[0], false);
                float scale = (max_d - min_d) / (480. - 20.);
                if (contours.size() > 0) {
                    arcL.push_back(100 * scale * arcLength(hull[0], true));
                } else {
                    arcL.push_back(-100);
                }
                X.clear();
                Y.clear();
                Z.clear();
                contours[0].clear();
                hull[0].clear();
                error_id = "Passed";
            }
            if (arcL.size() < 3) {
                arcL.clear();
                arcL.push_back(-100);
                arcL.push_back(-100);
                arcL.push_back(-100);
                error_id = "Arc length size is less than 3";
                return arcL;
            } else if (arcL[1] <= 0 || arcL[1] > 100) {
                arcL.clear();
                arcL.push_back(-100);
                arcL.push_back(-100);
                arcL.push_back(-100);
                error_id = "arcL[1] is <= 0 or > 100 cm";
                return arcL;
            }
            if (error_id == "Passed") {
                error_id.clear();
            }
            return arcL;
        } catch (cv::Exception &e) {
            arcL.clear();
            arcL.push_back(-100);
            arcL.push_back(-100);
            arcL.push_back(-100);
            error_id = "Exception error in ArcLength" + std::string(e.what());
            return arcL;
        }
    }

    void inversion::compute_part_laplacian_cot_weights(std::vector<float> &OutVertices,
                                                       BodyScanCommon::SexType gender,
                                                       const std::vector<int> &rings_as_vector,
                                                       const std::vector<int> &num_of_points_per_ring,
                                                       std::string &error_id) {
        const common *c = common::getInstance();
        try {
            float (&verts)[BodyScanCommon::N_VERTS_INV][3] = *reinterpret_cast<float (*)[BodyScanCommon::N_VERTS_INV][3]>(&(c->getVertsInv(
                    gender)[0]));
            // first we find the valid number of vertices and rings for a given bound (bounds)
            int L_ring_total = 0;
            float bound, th;
            if (gender == BodyScanCommon::SexType::male) {
                bound = 1.57;
                th = OutVertices[3 * 3552 + 1];// over ear point; needed at the end
            } else {
                bound = 1.48;
                th = OutVertices[3 * 3552 + 1];// over ear point;
            }
            int N_rows = 0;
            for (int i = 0; i < BodyScanCommon::N_VERTS_INV; i++) {
                std::vector<int> ring;
                ring.reserve(L_ring_total + num_of_points_per_ring[i]);
                for (int r = L_ring_total; r < (L_ring_total + num_of_points_per_ring[i]); r++) {
                    ring.push_back(rings_as_vector[r]);
                }
                L_ring_total = L_ring_total + num_of_points_per_ring[i];

                int Lring = (int) ring.size() - 1;
                bool N_rows_plus_one = false;
                for (int ii = 0; ii < Lring; ii++) {
                    int j = ring[ii] - 1;
                    int k = ring[ii + 1] - 1;
                    if (verts[i][1] < bound || verts[j][1] < bound || verts[k][1] < bound) {
                        continue;
                    }
                    N_rows_plus_one = true; // true if at least i had one j and links
                }
                if (N_rows_plus_one) {
                    N_rows = N_rows + 1;
                }
            }
            // now we get W for the vertices defined as per the bound above
            cv::Mat W = cv::Mat::zeros(N_rows, BodyScanCommon::N_VERTS_INV, CV_32F);
            L_ring_total = 0;
            int row_counter = 0;
            for (int i = 0; i < BodyScanCommon::N_VERTS_INV; i++) {
                std::vector<int> ring;// = rings_as_vector(L_ring_total: L_ring_total + num_of_points_per_ring(i)-1);
                ring.reserve(L_ring_total + num_of_points_per_ring[i]);

                for (int r = L_ring_total; r < (L_ring_total + num_of_points_per_ring[i]); r++) {
                    ring.push_back(rings_as_vector[r]);
                }

                L_ring_total = L_ring_total + num_of_points_per_ring[i];

                int Lring = (int) ring.size() - 1;

                bool row_counter_plus_one = false;
                for (int ii = 0; ii < Lring; ii++) {
                    int j = ring[ii] - 1;
                    int k = ring[ii + 1] - 1;

                    if (verts[i][1] < bound || verts[j][1] < bound || verts[k][1] < bound) {
                        continue;
                    }
                    row_counter_plus_one = true;

                    float vi[3];
                    vi[0] = verts[i][0];
                    vi[1] = verts[i][1];
                    vi[2] = verts[i][2];
                    float vj[3];
                    vj[0] = verts[j][0];
                    vj[1] = verts[j][1];
                    vj[2] = verts[j][2];
                    float vk[3];
                    vk[0] = verts[k][0];
                    vk[1] = verts[k][1];
                    vk[2] = verts[k][2];

                    float u[3], v[3];
                    // u = vk-vi; v = vk-vj;
                    u[0] = vk[0] - vi[0];
                    u[1] = vk[1] - vi[1];
                    u[2] = vk[2] - vi[2];
                    v[0] = vk[0] - vj[0];
                    v[1] = vk[1] - vj[1];
                    v[2] = vk[2] - vj[2];

                    float cot1, dot, cross[3]; // = dot(u,v)/norm(cross(u,v));

                    dot = u[0] * v[0] + u[1] * v[1] + u[2] * v[2];

                    cross[0] = u[1] * v[2] - u[2] * v[1];
                    cross[1] = u[2] * v[0] - u[0] * v[2];
                    cross[2] = u[0] * v[1] - u[1] * v[0];

                    cot1 = dot /
                           sqrt(cross[0] * cross[0] + cross[1] * cross[1] + cross[2] * cross[2]);
                    W.at<float>(row_counter, j) =
                            W.at<float>(row_counter, j) + cot1;// W(i,j) = W(i,j) + cot1;
                    // u = vj-vi; v = vj-vk;
                    u[0] = vj[0] - vi[0];
                    u[1] = vj[1] - vi[1];
                    u[2] = vj[2] - vi[2];
                    v[0] = vj[0] - vk[0];
                    v[1] = vj[1] - vk[1];
                    v[2] = vj[2] - vk[2];

                    dot = u[0] * v[0] + u[1] * v[1] + u[2] * v[2];

                    cross[0] = u[1] * v[2] - u[2] * v[1];
                    cross[1] = u[2] * v[0] - u[0] * v[2];
                    cross[2] = u[0] * v[1] - u[1] * v[0];

                    float cot2 =
                            dot /
                            sqrt(cross[0] * cross[0] + cross[1] * cross[1] + cross[2] * cross[2]);

                    W.at<float>(row_counter, k) =
                            W.at<float>(row_counter, k) + cot2;// W(i,k) = W(i,k) + cot2;
                }
                if (row_counter_plus_one) {
                    row_counter = row_counter + 1;
                }
            }
            // now we keep non-zero values (cols) of W and the correspoinding vertices
            cv::Mat W_rows_summed;
            reduce(W, W_rows_summed, 0, cv::REDUCE_SUM, CV_32F);

            int N_of_nonzero_cols = countNonZero(W_rows_summed);

            cv::Mat W_part(N_rows, N_of_nonzero_cols, CV_32F);
            cv::Mat V_part = cv::Mat(N_rows, 3, CV_32F);
            std::vector<int> V_idx;

            for (int row = 0; row < N_rows; row++) {
                int col_counter = 0;
                float tmp_sum = 0.0;
                for (int col = 0; col < BodyScanCommon::N_VERTS_INV; col++) {
                    if (W_rows_summed.at<float>(col) != 0) {
                        if (row ==
                            0) {// we need to do it just once (could be in  a seprate loop but thought of having it here)
                            V_idx.push_back(
                                    col);// don't confuse col here also refers to the row of the V matrix (W V  = delta)
                            V_part.at<float>(col_counter, 0) = verts[col][0];
                            V_part.at<float>(col_counter, 1) = verts[col][1];
                            V_part.at<float>(col_counter, 2) = verts[col][2];
                        }
                        W_part.at<float>(row, col_counter) = W.at<float>(row, col);
                        tmp_sum = tmp_sum + W_part.at<float>(row, col_counter);
                        col_counter = col_counter + 1;
                    }
                }
                tmp_sum = std::abs(tmp_sum);
                if (tmp_sum > 10000) {
                    for (int col = 0;
                         col < N_rows; col++) {// no of cols = no of raws (must equal actually)
                        W_part.at<float>(row, col) = W.at<float>(row, col) * 10000.0 / tmp_sum;
                    }
                }
            }
            cv::Mat W_part_col_summed;
            reduce(W, W_part_col_summed, 1, cv::REDUCE_SUM, CV_32F);
            for (int i = 0; i < N_rows; i++) {
                W_part.at<float>(i, i) = W_part.at<float>(i, i) - W_part_col_summed.at<float>(i);
            }
            cv::Mat delta_part = W_part * V_part;
            // if we use all vertex indices in V_part
            std::vector<int> anchor_idx = V_idx;
            // anchors
            int N_rows_reflected = 0;
            for (int idx = 0; idx < (int) V_idx.size(); idx++) {
                int j = 3 * V_idx[idx];
                if (OutVertices[j + 1] < th || OutVertices[j + 2] > 0) {
                    N_rows_reflected = N_rows_reflected + 1;
                }
            }
            cv::Mat WH = cv::Mat::zeros(N_rows_reflected, (int) V_idx.size(), CV_32F);
            cv::Mat V_anchors = cv::Mat::zeros(N_rows_reflected, 3, CV_32F);
            int reflected_counter = 0;
            for (int idx = 0; idx < (int) V_idx.size(); idx++) {
                int j = 3 * V_idx[idx];
                if (OutVertices[j + 1] < th || OutVertices[j + 2] > 0) {
                    WH.at<float>(reflected_counter, idx) = 1.0;
                    V_anchors.at<float>(reflected_counter, 0) = float(OutVertices[j]);
                    V_anchors.at<float>(reflected_counter, 1) = float(OutVertices[j + 1]);
                    V_anchors.at<float>(reflected_counter, 2) = float(OutVertices[j + 2]);
                    reflected_counter = reflected_counter + 1;
                }
            }
            vconcat(W_part, WH, W_part);
            WH.release();
            vconcat(delta_part, V_anchors, delta_part);
            V_anchors.release();
            // slow here
            cv::Mat t = W_part.t();
            delta_part = t * delta_part;
            W_part = (t * W_part);
            // and slow(ish) here...
            cv::Mat VL_part = W_part.inv() * delta_part;
            W_part.release();
            delta_part.release();
            for (int idx = 0; idx < (int) V_idx.size(); idx++) {
                int k = 3 * V_idx[idx];
                OutVertices[k] = VL_part.at<float>(idx, 0);
                OutVertices[k + 1] = VL_part.at<float>(idx, 1);
                OutVertices[k + 2] = VL_part.at<float>(idx, 2);
            }
            error_id.clear();// if it reaches here it is then passed;
        } catch (cv::Exception e) {
            error_id = "Failed in Laplace: " + std::string(e.what());
        }
    }

    void inversion::gen_points_for_ransac(const std::vector<float> &ChestsIn,
                                          const std::vector<float> &WaistsIn,
                                          const std::vector<float> &HipsIn,
                                          const std::vector<float> &InseamsIn,
                                          const std::vector<float> &FitsIn, int resample_ntimes,
                                          std::vector<cv::Point2f> &src_points,
                                          std::vector<cv::Point2f> &dst_points,
                                          std::vector<bool> &ChestIdx, std::vector<bool> &WaistIdx,
                                          std::vector<bool> &HipIdx, std::vector<bool> &InseamIdx,
                                          std::vector<bool> &FitIdx) {
        src_points.clear();
        dst_points.clear();
        int Lx = (int) ChestsIn.size();
        int idx1, idx2;
        for (int n = 0; n < resample_ntimes; n++) {
            // Chest-Waist
            idx1 = int(m_rnd.uniform(0, Lx - 1));
            do {
                idx2 = int(m_rnd.uniform(0, Lx - 1));
            } while (idx1 == idx2);
            src_points.push_back(cv::Point2f(ChestsIn[idx1], WaistsIn[idx2]));
            idx1 = int(m_rnd.uniform(0, Lx - 1));
            do {
                idx2 = int(m_rnd.uniform(0, Lx - 1));
            } while (idx1 == idx2);
            dst_points.push_back(cv::Point2f(ChestsIn[idx1], WaistsIn[idx2]));
            ChestIdx.push_back(true);
            WaistIdx.push_back(true);
            HipIdx.push_back(false);
            InseamIdx.push_back(false);
            FitIdx.push_back(false);
            // Chest-Hip
            idx1 = int(m_rnd.uniform(0, Lx - 1));
            do {
                idx2 = int(m_rnd.uniform(0, Lx - 1));
            } while (idx1 == idx2);
            src_points.push_back(cv::Point2f(ChestsIn[idx1], HipsIn[idx2]));
            idx1 = int(m_rnd.uniform(0, Lx - 1));
            do {
                idx2 = int(m_rnd.uniform(0, Lx - 1));
            } while (idx1 == idx2);
            dst_points.push_back(cv::Point2f(ChestsIn[idx1], HipsIn[idx2]));
            ChestIdx.push_back(true);
            WaistIdx.push_back(false);
            HipIdx.push_back(true);
            InseamIdx.push_back(false);
            FitIdx.push_back(false);
            // Chest-Inseam
            idx1 = int(m_rnd.uniform(0, Lx - 1));
            do {
                idx2 = int(m_rnd.uniform(0, Lx - 1));
            } while (idx1 == idx2);
            src_points.push_back(cv::Point2f(ChestsIn[idx1], InseamsIn[idx2]));
            idx1 = int(m_rnd.uniform(0, Lx - 1));
            do {
                idx2 = int(m_rnd.uniform(0, Lx - 1));
            } while (idx1 == idx2);
            dst_points.push_back(cv::Point2f(ChestsIn[idx1], InseamsIn[idx2]));
            ChestIdx.push_back(true);
            WaistIdx.push_back(false);
            HipIdx.push_back(false);
            InseamIdx.push_back(true);
            FitIdx.push_back(false);
            // Chest-Fitness
            idx1 = int(m_rnd.uniform(0, Lx - 1));
            do {
                idx2 = int(m_rnd.uniform(0, Lx - 1));
            } while (idx1 == idx2);
            src_points.push_back(cv::Point2f(ChestsIn[idx1], FitsIn[idx2]));
            idx1 = int(m_rnd.uniform(0, Lx - 1));
            do {
                idx2 = int(m_rnd.uniform(0, Lx - 1));
            } while (idx1 == idx2);
            dst_points.push_back(cv::Point2f(ChestsIn[idx1], FitsIn[idx2]));
            ChestIdx.push_back(true);
            WaistIdx.push_back(false);
            HipIdx.push_back(false);
            InseamIdx.push_back(false);
            FitIdx.push_back(true);
            // Waist-Hip
            idx1 = int(m_rnd.uniform(0, Lx - 1));
            do {
                idx2 = int(m_rnd.uniform(0, Lx - 1));
            } while (idx1 == idx2);
            src_points.push_back(cv::Point2f(WaistsIn[idx1], HipsIn[idx2]));
            idx1 = int(m_rnd.uniform(0, Lx - 1));
            do {
                idx2 = int(m_rnd.uniform(0, Lx - 1));
            } while (idx1 == idx2);
            dst_points.push_back(cv::Point2f(WaistsIn[idx1], HipsIn[idx2]));
            ChestIdx.push_back(false);
            WaistIdx.push_back(true);
            HipIdx.push_back(true);
            InseamIdx.push_back(false);
            FitIdx.push_back(false);
            // Waist-Inseam
            idx1 = int(m_rnd.uniform(0, Lx - 1));
            do {
                idx2 = int(m_rnd.uniform(0, Lx - 1));
            } while (idx1 == idx2);
            src_points.push_back(cv::Point2f(WaistsIn[idx1], InseamsIn[idx2]));
            idx1 = int(m_rnd.uniform(0, Lx - 1));
            do {
                idx2 = int(m_rnd.uniform(0, Lx - 1));
            } while (idx1 == idx2);
            dst_points.push_back(cv::Point2f(WaistsIn[idx1], InseamsIn[idx2]));
            ChestIdx.push_back(false);
            WaistIdx.push_back(true);
            HipIdx.push_back(false);
            InseamIdx.push_back(true);
            FitIdx.push_back(false);
            // Waist-Fitness
            idx1 = int(m_rnd.uniform(0, Lx - 1));
            do {
                idx2 = int(m_rnd.uniform(0, Lx - 1));
            } while (idx1 == idx2);
            src_points.push_back(cv::Point2f(WaistsIn[idx1], FitsIn[idx2]));
            idx1 = int(m_rnd.uniform(0, Lx - 1));
            do {
                idx2 = int(m_rnd.uniform(0, Lx - 1));
            } while (idx1 == idx2);
            dst_points.push_back(cv::Point2f(WaistsIn[idx1], FitsIn[idx2]));
            ChestIdx.push_back(false);
            WaistIdx.push_back(true);
            HipIdx.push_back(false);
            InseamIdx.push_back(false);
            FitIdx.push_back(true);
            // Hip-Inseam
            idx1 = int(m_rnd.uniform(0, Lx - 1));
            do {
                idx2 = int(m_rnd.uniform(0, Lx - 1));
            } while (idx1 == idx2);
            src_points.push_back(cv::Point2f(HipsIn[idx1], InseamsIn[idx2]));
            idx1 = int(m_rnd.uniform(0, Lx - 1));
            do {
                idx2 = int(m_rnd.uniform(0, Lx - 1));
            } while (idx1 == idx2);
            dst_points.push_back(cv::Point2f(HipsIn[idx1], InseamsIn[idx2]));
            ChestIdx.push_back(false);
            WaistIdx.push_back(false);
            HipIdx.push_back(true);
            InseamIdx.push_back(true);
            FitIdx.push_back(false);
            // Hip-Fitness
            idx1 = int(m_rnd.uniform(0, Lx - 1));
            do {
                idx2 = int(m_rnd.uniform(0, Lx - 1));
            } while (idx1 == idx2);
            src_points.push_back(cv::Point2f(HipsIn[idx1], FitsIn[idx2]));
            idx1 = int(m_rnd.uniform(0, Lx - 1));
            do {
                idx2 = int(m_rnd.uniform(0, Lx - 1));
            } while (idx1 == idx2);
            dst_points.push_back(cv::Point2f(HipsIn[idx1], FitsIn[idx2]));
            ChestIdx.push_back(false);
            WaistIdx.push_back(false);
            HipIdx.push_back(true);
            InseamIdx.push_back(false);
            FitIdx.push_back(true);
            // Inseam-Fitness
            idx1 = int(m_rnd.uniform(0, Lx - 1));
            do {
                idx2 = int(m_rnd.uniform(0, Lx - 1));
            } while (idx1 == idx2);
            src_points.push_back(cv::Point2f(InseamsIn[idx1], FitsIn[idx2]));
            idx1 = int(m_rnd.uniform(0, Lx - 1));
            do {
                idx2 = int(m_rnd.uniform(0, Lx - 1));
            } while (idx1 == idx2);
            dst_points.push_back(cv::Point2f(InseamsIn[idx1], FitsIn[idx2]));
            ChestIdx.push_back(false);
            WaistIdx.push_back(false);
            HipIdx.push_back(false);
            InseamIdx.push_back(true);
            FitIdx.push_back(true);
        }
    }

    float inversion::vector_mean(const std::vector<float> &V) {
        float mu = 0.0;
        size_t L = V.size();
        if (L > 0) {
            for (size_t n = 0; n < L; n++) {
                mu = mu + V[n];
            }
            mu = mu / (float) L;
        }
        return mu;
    }
}