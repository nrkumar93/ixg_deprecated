#ifndef INSAT_ACTION_HPP
#define INSAT_ACTION_HPP

#include <common/insat/InsatState.hpp>
#include <common/Action.hpp>

namespace ps
{

    class InsatAction : public Action
    {

    public:
        // Action(){};
        InsatAction(const std::string& type, ParamsType params = ParamsType(), bool is_expensive = true)
                : Action(type, params, is_expensive){};
        virtual ~InsatAction(){};

        virtual TrajType optimize(const StateVarsType& s1, const StateVarsType& s2, int thread_id=0) const =0;
        virtual TrajType warmOptimize(const TrajType& t1, const TrajType& t2, int thread_id=0) const =0;
        virtual TrajType warmOptimize(const TrajType& t, int thread_id=0) const =0;
        virtual TrajType optimize(const TrajType& incoming_traj,
                                  const StateVarsType &s1,
                                  const StateVarsType &s2,
                                  int thread_id=0) = 0;
      virtual TrajType optimize(const TrajType& incoming_traj,
                                const std::vector<StateVarsType> &ancestors,
                                const StateVarsType& successor,
                                int thread_id=0) = 0;
      virtual TrajType optimize(const std::vector<StateVarsType> &ancestors,
                                const StateVarsType& successor,
                                int thread_id=0) = 0;
      virtual TrajType optimize(const std::vector<int> &gcs_nodes, int thread_id=0) = 0;
      virtual double lowerboundCost(const std::vector<int> &gcs_nodes, int thread_id=0) = 0;
      virtual MatDf sampleTrajectory(const GCSTraj::TrajInstanceType &traj, double dt) const = 0;
      virtual double getCost(const TrajType& traj, int thread_id=0) const =0;
      virtual bool isFeasible(MatDf& traj, int thread_id) const =0;
      virtual std::vector<double> SampleFeasibleState(int thread_id){};
      virtual std::unordered_map<int, std::vector<int>> getAdjacencyList() = 0;

    };

}

#endif
