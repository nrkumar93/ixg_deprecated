//
// Created by gaussian on 2/17/23.
//

#ifndef INSATTYPES_HPP
#define INSATTYPES_HPP

// Drake
#include <drake/planning/trajectory_optimization/kinematic_trajectory_optimization.h>
#include <drake/solvers/solve.h>
#include <drake/common/trajectories/composite_trajectory.h>
#include <drake/common/copyable_unique_ptr.h>


#include <common/EigenTypes.h>

namespace ps
{

    struct InsatParams
    {
        InsatParams(int lowD_dims, int fullD_dims_, int aux_dims) : lowD_dims_(lowD_dims),
                                                                    fullD_dims_(fullD_dims_),
                                                                    aux_dims_(aux_dims) {}

        int lowD_dims_ = 6;
        int fullD_dims_ = 12;
        int aux_dims_ = 6;
    };

    struct DummyTraj
    {
        MatDf disc_traj_;

        inline bool isValid() {return disc_traj_.size()>=2;}
    };

    struct BSplineTraj
    {
        typedef drake::solvers::MathematicalProgramResult OptResultType;
        typedef drake::trajectories::BsplineTrajectory<double> TrajInstanceType;

        inline long size() const {return disc_traj_.size();}
        inline bool isValid() const {return result_.is_success();}

        OptResultType result_;
        TrajInstanceType traj_;
        MatDf disc_traj_;

        std::string story_;
    };

    struct GCSTraj
    {
      typedef drake::solvers::MathematicalProgramResult OptResultType;
      typedef drake::trajectories::CompositeTrajectory<double> TrajInstanceType;

      GCSTraj() : traj_(std::vector< drake::copyable_unique_ptr< drake::trajectories::Trajectory<double>>>()) {}
      GCSTraj(TrajInstanceType traj, OptResultType result)
        : result_(result),
          traj_(traj) {}

      inline long size() const {return disc_traj_.size();}
      inline bool isValid() const {return result_.is_success();}

      OptResultType result_;
      TrajInstanceType traj_;
      MatDf disc_traj_;

      std::string story_;
    };

}


#endif //INSATTYPES_HPP
