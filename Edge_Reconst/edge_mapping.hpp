#ifndef EDGE_MAPPING_HPP
#define EDGE_MAPPING_HPP

#include <Eigen/Core>
#include <unordered_map>
#include <vector>
#include <Eigen/Dense>
#include <algorithm>
#include <numeric>
#include <memory>

#include "util.hpp"
#include "file_reader.hpp"
#include "definitions.h"

// Hash function for Eigen::Vector3d to use in unordered_map
struct HashEigenVector3d {
    std::size_t operator()(const Eigen::Vector3d& vec) const {
        std::size_t seed = 0;
        for (int i = 0; i < vec.size(); ++i) {
            seed ^= std::hash<double>()(vec[i]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};



struct FuzzyVector3dEqual {
    bool operator()(const Eigen::Vector3d& a, const Eigen::Vector3d& b) const {
        return (a - b).norm() < 1e-6;
    }
};



struct FuzzyVector3dPairEqual {
    bool operator()(const std::pair<Eigen::Vector3d, Eigen::Vector3d>& a,
                    const std::pair<Eigen::Vector3d, Eigen::Vector3d>& b) const {
        FuzzyVector3dEqual eq;
        return (eq(a.first, b.first) && eq(a.second, b.second)) ||
               (eq(a.first, b.second) && eq(a.second, b.first));
    }
};



/////////// For creating weighted map of 3D edges
struct HashEigenVector3dPair {
    std::size_t operator()(const std::pair<Eigen::Vector3d, Eigen::Vector3d>& p) const {
        HashEigenVector3d hash_fn;
        return hash_fn(p.first) ^ (hash_fn(p.second) << 1);
    }
};



class EdgeMapping {
public:
    struct SupportingEdgeData {  
        Eigen::Vector2d edge;
        Eigen::Vector3d edge_uncorrected;
        int image_number;
        int edge_idx;
        Eigen::Matrix3d rotation;
        Eigen::Vector3d translation;
        Eigen::Vector3d tangents_3D_world;

        bool operator<(const SupportingEdgeData& other) const {
            if (image_number != other.image_number) return image_number < other.image_number;
            if (edge.x() != other.edge.x()) return edge.x() < other.edge.x();
            return edge.y() < other.edge.y();
        }
    };

    //////////////////////////// mapping 2D edge to 3D ////////////////////////////
    struct Uncorrected2DEdgeMappingData {
        Eigen::Vector3d edge_3D;
        Eigen::Vector3d tangents_3D_world;
        Eigen::Vector2d supporting_edge;
    };


    struct Uncorrected2DEdgeKey {
        Eigen::Vector3d edge_uncorrected;
        int image_number;
        int edge_idx;
        Eigen::Matrix3d rotation;
        Eigen::Vector3d translation;

        bool operator==(const Uncorrected2DEdgeKey& other) const {
            return edge_uncorrected == other.edge_uncorrected &&
                image_number == other.image_number &&
                rotation.isApprox(other.rotation, 1e-8) &&
                translation.isApprox(other.translation, 1e-8);
        }
    };
    //////////////////////////// mapping 2D edge to 3D ////////////////////////////

    struct EdgeNode {
        Eigen::Vector3d location;  
        Eigen::Vector3d orientation; 
        std::vector<std::pair<int, EdgeNode*>> neighbors;
        bool has_orientation_fixed_in_connectivity_graph;
    };

    struct ConnectivityGraphNode {
        Eigen::Vector3d location;  
        Eigen::Vector3d orientation;      
        std::pair<int, Eigen::Vector3d> left_neighbor;  
        std::pair<int, Eigen::Vector3d> right_neighbor; 
    };

    struct Curve {
        std::vector<int> edge_indices;
    };


    using PointerNeighborMap = std::unordered_map<const Eigen::Vector3d*, std::vector<std::pair<const Eigen::Vector3d*, std::pair<Eigen::Vector3d, Eigen::Vector3d>>>>;
    using ConnectivityGraph = std::unordered_map<int, ConnectivityGraphNode>;

    
    // Custom hash function
    struct HashUncorrected2DEdgeKey {
        std::size_t operator()(const Uncorrected2DEdgeKey& key) const {
            std::size_t seed = 0;

            // Hash Eigen::Vector3d edge_uncorrected
            for (int i = 0; i < 3; ++i)
                seed ^= std::hash<double>()(key.edge_uncorrected[i]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

            seed ^= std::hash<int>()(key.image_number) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

            // Hash Eigen::Matrix3d rotation
            for (int i = 0; i < 9; ++i)
                seed ^= std::hash<double>()(key.rotation(i)) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

            // Hash translation
            for (int i = 0; i < 3; ++i)
                seed ^= std::hash<double>()(key.translation[i]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

            return seed;
        }
    };



    void add3DToSupportingEdgesMapping(const Eigen::Vector3d &edge_3D, 
                                       const Eigen::Vector3d &tangents_3D_world,
                                       const Eigen::Vector2d &supporting_edge, 
                                       const Eigen::Vector3d &supporting_edge_uncorrected, 
                                       int image_number,
                                       int edge_idx,
                                       const Eigen::Matrix3d &rotation,
                                       const Eigen::Vector3d &translation);

    void add3DToFrameMapping(const Eigen::Vector3d& edge_3D, 
                             const Eigen::Vector2d& supporting_edge, 
                             int frame);

    void printFirst10Edges();
    // void write_edge_linking_to_file();
    using EdgeNodeList = std::vector<std::unique_ptr<EdgeNode>>;


    std::vector<std::vector<SupportingEdgeData>> findMergable2DEdgeGroups(const std::vector<Eigen::Matrix3d> all_R,const std::vector<Eigen::Vector3d> all_T, const Eigen::Matrix3d K, const int Num_Of_Total_Imgs);
    
    std::unordered_map<Eigen::Vector3d, std::vector<SupportingEdgeData>, HashEigenVector3d> edge_3D_to_supporting_edges;
    std::unordered_map<Uncorrected2DEdgeKey, std::vector<Uncorrected2DEdgeMappingData>, HashUncorrected2DEdgeKey> map_Uncorrected2DEdge_To_SupportingData();
    std::unordered_map<std::pair<Eigen::Vector3d, Eigen::Vector3d>, int, HashEigenVector3dPair, FuzzyVector3dPairEqual>
    build3DEdgeWeightedGraph(const std::unordered_map<Uncorrected2DEdgeKey, std::vector<Uncorrected2DEdgeMappingData>, HashUncorrected2DEdgeKey>& uncorrected_map, 
                            const std::vector<Eigen::MatrixXd> All_Edgels, std::vector<EdgeCurvelet> all_curvelets,
                            const std::vector<Eigen::Matrix3d> All_R, const std::vector<Eigen::Vector3d> All_T);
    //EdgeNodeList createEdgeNodesFromEdges(const std::vector<Eigen::Vector3d>& locations, const std::unordered_map<std::pair<Eigen::Vector3d, Eigen::Vector3d>, int, HashEigenVector3dPair, FuzzyVector3dPairEqual>& pruned_graph);
    //EdgeNodeList createEdgeNodesFromFiles(const std::string& points_file, const std::string& tangents_file, const std::string& connections_file);

    std::unordered_map<std::pair<Eigen::Vector3d, Eigen::Vector3d>, int, 
                       HashEigenVector3dPair, FuzzyVector3dPairEqual>
    pruneEdgeGraph_by_3DProximityAndOrientation(std::unordered_map<std::pair<Eigen::Vector3d, Eigen::Vector3d>, int, 
                           HashEigenVector3dPair, FuzzyVector3dPairEqual>& graph );

    EdgeNodeList buildEdgeNodeGraph(const std::unordered_map<std::pair<Eigen::Vector3d, Eigen::Vector3d>, int,
                                HashEigenVector3dPair, FuzzyVector3dPairEqual>& pruned_graph);

    void align3DEdgesUsingEdgeNodes(EdgeNodeList& edge_nodes, int iterations, double step_size_force, double step_size_torque);

    ConnectivityGraph createConnectivityGraph(EdgeNodeList& edge_nodes);

    void writeConnectivityGraphToFile(const ConnectivityGraph& graph, const std::string& file_name);
    std::vector<Curve> buildCurvesFromConnectivityGraph(const ConnectivityGraph& connectivity_graph);
    void writeCurvesToFile(const std::vector<Curve>& curves, const ConnectivityGraph& connectivity_graph, const std::string& file_name, bool b_write_curve_info);
    
    std::unordered_map<std::pair<Eigen::Vector3d, Eigen::Vector3d>, int, HashEigenVector3dPair, FuzzyVector3dPairEqual> pruneEdgeGraphbyProjections(
                                                                                                                                                    std::unordered_map<std::pair<Eigen::Vector3d, Eigen::Vector3d>, int, 
                                                                                                                                                    HashEigenVector3dPair, FuzzyVector3dPairEqual>& graph,
                                                                                                                                                    const std::vector<Eigen::Matrix3d> All_R,
                                                                                                                                                    const std::vector<Eigen::Vector3d> All_T,
                                                                                                                                                    const Eigen::Matrix3d K,
                                                                                                                                                    const int Num_Of_Total_Imgs
                                                                                                                                                    );
    std::vector<EdgeCurvelet> read_curvelets(int thresh_EDG = 1);
    std::vector<Eigen::MatrixXd> read_edgels(int thresh_EDG = 1);
private:
    std::shared_ptr<MultiviewGeometryUtil::multiview_geometry_util> util = nullptr;
    std::shared_ptr<file_reader> file_reader_ptr = nullptr;
    void write_edge_graph( std::unordered_map<std::pair<Eigen::Vector3d, Eigen::Vector3d>, int, HashEigenVector3dPair, FuzzyVector3dPairEqual>& graph, std::string file_name );

};

#endif  // EDGE_MAPPING_HPP