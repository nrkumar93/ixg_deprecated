#pragma once
#ifndef INSATxGCS_OPT_HPP
#define INSATxGCS_OPT_HPP

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include <chrono>

#include <drake/common/trajectories/bezier_curve.h>
#include <drake/common/trajectories/composite_trajectory.h>
#include <drake/common/symbolic/decompose.h>
#include <drake/solvers/constraint.h>
#include <drake/common/pointer_cast.h>
#include <drake/geometry/optimization/hpolyhedron.h>
#include <drake/geometry/optimization/point.h>
#include <drake/geometry/optimization/convex_set.h>
#include <drake/geometry/optimization/graph_of_convex_sets.h>
//#include <drake/planning/trajectory_optimization/gcs_trajectory_optimization.h>
#include <drake/geometry/optimization/cartesian_product.h>
#include <drake/solvers/solve.h>
#include <drake/common/trajectories/trajectory.h>
#include <drake/solvers/mosek_solver.h>

#include <common/insat/InsatTypes.hpp>

namespace ps {

  typedef drake::geometry::optimization::GraphOfConvexSets GCS;
  typedef drake::geometry::optimization::GraphOfConvexSets::Vertex GCSVertex;
  typedef drake::geometry::optimization::GraphOfConvexSets::Edge GCSEdge;
  typedef drake::geometry::optimization::GraphOfConvexSets::VertexId VertexId;
  typedef drake::geometry::optimization::GraphOfConvexSets::EdgeId EdgeId;
  typedef drake::geometry::optimization::HPolyhedron HPolyhedron;

  typedef drake::solvers::Binding<drake::solvers::Cost> CostBinding;
  typedef drake::solvers::Binding<drake::solvers::Constraint> ConstraintBinding;
  typedef std::shared_ptr<drake::solvers::Cost> CostPtr;
  typedef std::shared_ptr<drake::solvers::Constraint> ConstraintPtr;

  using VectorXb = Eigen::Matrix<bool, 1, Eigen::Dynamic>;

  const double kInf = std::numeric_limits<double>::infinity();

  struct GCSOptResult {
    drake::trajectories::CompositeTrajectory<double> traj_;
    drake::solvers::MathematicalProgramResult result_;
    drake::VectorX<drake::symbolic::Variable> vars_;
    std::vector<drake::solvers::Binding<drake::solvers::Cost>> costs_;
    std::vector< drake::solvers::Binding<drake::solvers::Constraint>> constraints_;
  };

  class GCSOpt {

  public:

    /// Sets up the GCS regions and edges
    GCSOpt(const std::vector<HPolyhedron>& regions,
           const std::vector<std::pair<int, int>>& edges_between_regions,
           int order, double h_min, double h_max,
           double path_length_weight, double time_weight,
           Eigen::VectorXd& vel_lb, Eigen::VectorXd& vel_ub,
           bool verbose=false);

    GCSOpt (const GCSOpt &)=default;
    GCSOpt & 	operator= (const GCSOpt &)=default;
    GCSOpt (GCSOpt &&)=default;
    GCSOpt & 	operator= (GCSOpt &&)=default;

    void FormulateAndSetCostsAndConstraints();

    VertexId AddStart(Eigen::VectorXd& start);

    VertexId AddGoal(Eigen::VectorXd& goal);

    double LowerboundSolve(const std::vector<int>& path_ids);

    std::pair<drake::trajectories::CompositeTrajectory<double>,
            drake::solvers::MathematicalProgramResult> Solve(std::vector<VertexId>& path_vids,
                                                             std::vector<EdgeId>& path_eids);

    std::pair<drake::trajectories::CompositeTrajectory<double>,
            drake::solvers::MathematicalProgramResult> Solve(std::vector<VertexId>& path_vids,
                                                             std::vector<EdgeId>& path_eids,
                                                             Eigen::VectorXd& initial_guess);

    std::pair<drake::trajectories::CompositeTrajectory<double>,
            drake::solvers::MathematicalProgramResult> Solve(std::vector<VertexId>& path_vids);

    std::pair<drake::trajectories::CompositeTrajectory<double>,
            drake::solvers::MathematicalProgramResult> Solve(std::vector<VertexId>& path_vids,
                                                             Eigen::VectorXd& initial_guess);

    std::pair<drake::trajectories::CompositeTrajectory<double>,
          drake::solvers::MathematicalProgramResult> Solve(std::vector<int>& path_ids);

    double CalculateCost(std::pair<drake::trajectories::CompositeTrajectory<double>,
            drake::solvers::MathematicalProgramResult>& soln);

    const std::shared_ptr<drake::geometry::optimization::GraphOfConvexSets> GetGCS() const {
      return gcs_;
    }

    const std::unordered_map<int64_t, GCSVertex*> GetVertexIdToVertexMap() const {
      return vertex_id_to_vertex_;
    }

    void CleanUp();

    //// TEMPORARY
    std::vector<drake::geometry::optimization::GraphOfConvexSets::Vertex*> GetVertices() const {
      return vertices_;
    }

    std::vector<drake::geometry::optimization::GraphOfConvexSets::Edge*> GetEdges() const {
      return edges_;
    }

    void RewriteForConvexSolver();
    void RewriteForConvexSolver(drake::solvers::MathematicalProgram* prog);

  protected:

    virtual void setupVars();
    /// Preprocess regions to add the time scaling set and create vertices
    virtual void preprocess(const drake::geometry::optimization::ConvexSets& regions,
                            const std::vector<std::pair<int, int>>& edges_between_regions);
    void addCosts(const GCSVertex* v);
    void addConstraints(const GCSVertex* v);
    void addConstraints(const GCSEdge* e);
    void setupCostsAndConstraints();
    virtual void formulateTimeCost();
    void formulatePathLengthCost();
    void formulatePathContinuityConstraint();
    virtual void formulateVelocityConstraint();
    void formulateCostsAndConstraints();

    bool verbose_;

    /// Basics
    int order_;
    int num_positions_;
    double h_min_;
    double h_max_;
    Eigen::VectorXd vel_lb_;
    Eigen::VectorXd vel_ub_;
    std::vector<HPolyhedron> hpoly_regions_;
    std::vector<std::pair<int, int>> edges_bw_regions_;
    std::shared_ptr<drake::geometry::optimization::GraphOfConvexSets> gcs_;

    /// Terminals
    GCSVertex* start_vtx_;
    GCSVertex* goal_vtx_;
    std::vector<GCSVertex*>::iterator start_vit_;
    std::vector<GCSVertex*>::iterator goal_vit_;
    std::vector<GCSEdge*>::iterator start_eit_;
    std::vector<GCSEdge*>::iterator goal_eit_;

    /// Variables
    drake::VectorX<drake::symbolic::Variable> u_h_;
    drake::VectorX<drake::symbolic::Variable> u_vars_;
    drake::trajectories::BezierCurve<drake::symbolic::Expression> u_r_trajectory_;

    drake::VectorX<drake::symbolic::Variable> v_h_;
    drake::VectorX<drake::symbolic::Variable> v_vars_;
    drake::trajectories::BezierCurve<drake::symbolic::Expression> v_r_trajectory_;

    /// Vertices and edges for optimization (for matrix operations over control points)
    std::vector<drake::geometry::optimization::GraphOfConvexSets::Vertex*> vertices_;
    std::vector<drake::geometry::optimization::GraphOfConvexSets::Edge*> edges_;

    /// Costs
    std::shared_ptr<drake::solvers::Cost> time_cost_;
//    std::vector<std::pair<std::shared_ptr<drake::solvers::Cost>, VectorXb>>
//        path_length_cost_;
    std::vector<std::shared_ptr<drake::solvers::Cost>> path_length_cost_;

    /// Constraints
    std::pair<std::shared_ptr<drake::solvers::Constraint>, VectorXb> path_continuity_constraint_;
    std::vector<std::pair<std::shared_ptr<drake::solvers::Constraint>, VectorXb>>
            velocity_constraint_;
    std::pair<std::shared_ptr<drake::solvers::Constraint>, VectorXb> start_point_constraint_;

    /// Lowerbound solver
    std::shared_ptr<GCSOpt> lb_solver_;

    /// Maps
    /// Dict for vertex id to vertex
    std::unordered_map<int64_t, HPolyhedron> vertex_id_to_regions_;
    /// Dict for vertex id to vertex
    std::unordered_map<int64_t, drake::geometry::optimization::GraphOfConvexSets::Vertex*> vertex_id_to_vertex_;
    /// Dict for vertex id to cost binding
    std::unordered_map<int64_t, std::vector<CostBinding>> vertex_id_to_cost_binding_;
    /// Dict for vertex id to constraint binding
    std::unordered_map<int64_t, std::vector<ConstraintBinding>> vertex_id_to_constraint_binding_;
    /// Dict for edge id to edge
    std::unordered_map<int64_t, drake::geometry::optimization::GraphOfConvexSets::Edge*> edge_id_to_edge_;
    /// Dict for edge id to cost binding
    std::unordered_map<int64_t, std::vector<CostBinding>> edge_id_to_cost_binding_;
    /// Dict for edge id to constraint binding
    std::unordered_map<int64_t, std::vector<ConstraintBinding>> edge_id_to_constraint_binding_;
    /// Tracking slack variables
    std::vector<drake::VectorX<drake::symbolic::Variable>> slack_vars_;

    /// Flags for enabling/disabling costs and constraints
    bool enable_time_cost_;
    bool enable_path_length_cost_;
    bool enable_path_velocity_constraint_;
    /// weights
    double time_weight_;
    Eigen::MatrixXd path_length_weight_;

  };
}

#endif //INSATxGCS_OPT_HPP
