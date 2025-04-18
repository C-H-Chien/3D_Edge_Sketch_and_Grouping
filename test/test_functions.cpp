#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/SVD>

#include "../Edge_Reconst/definitions.h"
#include "../Edge_Reconst/util.hpp"

Eigen::Matrix3d getSkewSymmetric(Eigen::Vector3d T) {
    Eigen::Matrix3d T_x = (Eigen::Matrix3d() << 0.,  -T(2),   T(1), T(2),  0.,  -T(0), -T(1),  T(0),   0.).finished();
    return T_x;
}

Eigen::Matrix3d getRodriguesRotationMatrix(Eigen::Vector3d v1, Eigen::Vector3d v2) {

    //> make sure that the input vectors are unit-vectors
    v1 /= v1.norm();
    v2 /= v2.norm();

    Eigen::Vector3d v1_cross_v2 = v1.cross(v2);
    double s = v1_cross_v2.norm();
    double c = v1.dot(v2);
    double coeff = 1.0 / (1.0 + c);
    Eigen::Matrix3d v_x = getSkewSymmetric(v1_cross_v2);
    Eigen::Matrix3d R = Eigen::Matrix3d::Identity() + v_x + coeff * v_x * v_x;
    return R;
}

void Compute_3D_Tangents(
    const Eigen::MatrixXd& pt_edge_view1,
    const Eigen::MatrixXd& pt_edge_view2,
    const Eigen::Matrix3d& K1,
    const Eigen::Matrix3d& K2,
    const Eigen::Matrix3d& R21,
    const Eigen::Vector3d& T21,
    Eigen::MatrixXd& tangents_3D)
{
    tangents_3D.resize(1, 3);

    Eigen::Vector3d e1  = {1,0,0};
    Eigen::Vector3d e3  = {0,0,1};

    // Normalize edge points
    Eigen::Vector3d Gamma1 = K1.inverse() * Eigen::Vector3d(pt_edge_view1(0), pt_edge_view1(1), 1.0);
    Eigen::Vector3d Gamma2 = K2.inverse() * Eigen::Vector3d(pt_edge_view2(0), pt_edge_view2(1), 1.0);

    Eigen::Vector3d tgt1(cos(pt_edge_view1(2)), sin(pt_edge_view1(2)), 0.0);
    Eigen::Vector3d tgt2(cos(pt_edge_view2(2)), sin(pt_edge_view2(2)), 0.0);
    Eigen::Vector3d tgt1_meters = K1.inverse() * tgt1;
    Eigen::Vector3d tgt2_meters = K2.inverse() * tgt2;

    double rho1 = (double(e1.transpose() * T21) - double(e3.transpose() * T21) * double(e1.transpose() *Gamma2))/(double(e3.transpose() * R21 * Gamma1)* double(e1.transpose() * Gamma2) - double(e1.transpose() * R21 * Gamma1));

    Eigen::Vector3d n1 = tgt1_meters.cross(Gamma1);
    Eigen::Vector3d n2 = R21.transpose() * tgt2_meters.cross(Gamma2);

    Eigen::Vector3d T3D = n1.cross(n2) / (n1.cross(n2) ).norm();
    tangents_3D = T3D;
}

int main(int argc, char **argv) {

    //> [TEST] 3D tangents
    // Given Rotation and Translation Matrices
    MultiviewGeometryUtil::multiview_geometry_util util;
    Eigen::Matrix3d R1, R2;
    Eigen::Vector3d T1, T2;

    R1 << 0.229714, 0.973258, -1.29728e-07,
            -0.220415, 0.0520239, 0.974018,
            -0.947971, 0.223746, -0.226472;
    T1 << -0.601486, -0.402814, -3.93395;
    R2 << 0.159322, -0.987227, -3.64782e-08,
            -0.0530916, -0.00856814, 0.998553,
            0.985798, 0.159092, 0.0537785;
    T2 << 0.413953, -0.468447, -3.95085;

    // Camera Intrinsic Matrix (example, adjust if needed)
    Eigen::Matrix3d K_;
    K_ << 2584.93250981950, 0, 249.771375872214,
            0, 2584.79186060577, 278.312679379194,
            0, 0, 1;

    // Point coordinates from two images
    std::vector<Eigen::Vector2d> points_img1 = {
        {520.590800, 428.498217}, {520.581695, 427.998471}, {520.573102, 427.498616},
        {520.564490, 426.998731}, {520.555647, 426.498837}, {520.645733, 430.996517},
        {520.634568, 430.496660}, {520.623192, 429.996899}, {520.611786, 429.497305},
        {520.600795, 428.997801}};
    std::vector<Eigen::Vector2d> points_img2 = {
        {240.855540, 429.000920}, {240.849805, 428.001160}, {240.839928, 427.001388},
        {240.839928, 427.001388}, {240.834057, 426.501440}, {240.885916, 432.001500},
        {240.872839, 431.001501}, {240.866802, 430.501425}, {240.861729, 430.001200},
        {240.855540, 429.000920}};

    // Relative Rotation and Translation
    Eigen::Matrix3d R21 = R2 * R1.transpose();
    Eigen::Vector3d T21 = T2 - R21 * T1;

    for (size_t i = 0; i < points_img1.size(); ++i) {
        // Prepare the corresponding points
        Eigen::Vector2d pt1 = points_img1[i];
        Eigen::Vector2d pt2 = points_img2[i];
        Eigen::Vector3d pt1_h(pt1(0), pt1(1), 1.0);
        Eigen::Vector3d pt2_h(pt2(0), pt2(1), 1.0);

        // Triangulate the 3D point
        std::vector<Eigen::Vector2d> pts_test = {pt1, pt2};
        std::vector<Eigen::Matrix3d> Rs_test = {R21};
        std::vector<Eigen::Vector3d> Ts_test = {T21};

        Eigen::Vector3d edge_pt_3D = util.linearTriangulation(2, pts_test, Rs_test, Ts_test, K_);
        Eigen::Vector3d edge_pt_3D_world = util.transformToWorldCoordinates(edge_pt_3D, R1, T1);

        // Compute the 3D tangent
        Eigen::MatrixXd tangent_3D;
        Compute_3D_Tangents(pt1_h, pt2_h, K_, K_, R21, T21, tangent_3D);
        Eigen::Vector3d tangent_3D_world = R1.transpose() * tangent_3D;

        // Print results
        std::cout << "Point " << i + 1 << ": (" << edge_pt_3D_world(0) << ", " << edge_pt_3D_world(1) << ", " << edge_pt_3D_world(2) << ")" << std::endl;
        std::cout << "Tangent " << i + 1 << ": (" << tangent_3D_world(0) << ", " << tangent_3D_world(1) << ", " << tangent_3D_world(2) << ")" << std::endl;
    }

    std::cout << "----------------------------------------------------------------------" << std::endl;

    //> [TEST] Verify the correctness of the aligning two unit vectors in 3D by Rodrigue's formula
    Eigen::Vector3d v1(0.700390490817757, 0.114407726497277, 0.704531072763853);
    Eigen::Vector3d v2(0.714979548359578, 0.362563357150639, 0.597789308602281);
    Eigen::Matrix3d R_align_v1_to_v2 = getRodriguesRotationMatrix(v1, v2);
    Eigen::Vector3d aligned_v2 = R_align_v1_to_v2 * v1;
    std::cout << aligned_v2 << std::endl;
    Eigen::Vector3d eulerAnglesXYZ = R_align_v1_to_v2.eulerAngles(0, 1, 2);
    std::cout << "Euler angles:" << std::endl << eulerAnglesXYZ << std::endl;

    std::cout << "----------------------------------------------------------------------" << std::endl;

    //> [TEST] Verify the correctness of projecting a 3D unit tangent vector (in world coordinate) to a 2D image
    Eigen::Vector3d Tangent_3D_w(-0.604940217207650, 0.119991911797586, 0.787178045112998);
    Eigen::Matrix3d Rot;
    Rot <<  0.283307133314224,  0.599568603826573,  0.748501541427090, \
           -0.799349013318374, -0.283603601454719,  0.529726488056670, \
            0.529885103697223, -0.748389261378974,  0.398917648559720;

    Eigen::Vector3d point_location(313.128094185699, 221.221179621611, 1.0);
    Eigen::Matrix3d K;
    K << 2584.93250981950,	0,	249.771375872214, \
         0,	2584.79186060577,	278.312679379194, \
         0,	0,	1;
    
    Eigen::Vector3d point_in_meters = K.inverse() * point_location;
    Eigen::Vector3d Tangent_3D_c = Rot * Tangent_3D_w;
    Eigen::Vector3d tangent_2D   = Tangent_3D_c - Tangent_3D_c(2) * point_in_meters;
    tangent_2D.normalize();
    std::cout << "projected tangent is (" << tangent_2D(0) << ", " << tangent_2D(1) << ", " << tangent_2D(2) << ")" << std::endl;

    return 0;
}