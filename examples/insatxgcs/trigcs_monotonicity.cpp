//  * Copyright (c) 2023, Ramkumar Natarajan
//  * All rights reserved.
//  *
//  * Redistribution and use in source and binary forms, with or without
//  * modification, are permitted provided that the following conditions are met:
//  *
//  *     * Redistributions of source code must retain the above copyright
//  *       notice, this list of conditions and the following disclaimer.
//  *     * Redistributions in binary form must reproduce the above copyright
//  *       notice, this list of conditions and the following disclaimer in the
//  *       documentation and/or other materials provided with the distribution.
//  *     * Neither the name of the Carnegie Mellon University nor the names of its
//  *       contributors may be used to endorse or promote products derived from
//  *       this software without specific prior written permission.
//  *
//  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//  * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
//  * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//  * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//  * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  * POSSIBILITY OF SUCH DAMAGE.
//

/*!
 * \file trigcs_monotonicity.cpp 
 * \author Ram Natarajan (rnataraj@cs.cmu.edu)
 * \date 10/24/23
*/

#include <iostream>
#include <memory>
#include <cstdlib>

#include <common/insatxgcs/utils.hpp>
#include <drake/planning/trajectory_optimization/gcs_trajectory_optimization.h>
#include <iomanip>
#include <drake/solvers/mosek_solver.h>

using drake::geometry::optimization::GraphOfConvexSets;
using drake::geometry::optimization::HPolyhedron;
using utils::operator<<;

typedef drake::geometry::optimization::GraphOfConvexSets GCS;
typedef drake::geometry::optimization::GraphOfConvexSets::Vertex GCSVertex;
typedef drake::geometry::optimization::GraphOfConvexSets::Edge GCSEdge;
typedef drake::geometry::optimization::GraphOfConvexSets::VertexId VertexId;
typedef drake::geometry::optimization::GraphOfConvexSets::EdgeId EdgeId;
typedef drake::geometry::optimization::HPolyhedron HPolyhedron;


std::vector<VertexId> pathToVertexId(std::vector<int64_t>& path, const std::vector<GraphOfConvexSets::Vertex*>& vertices) {
  std::vector<VertexId> solve_vids;
  for (auto vid : path) {
    for (auto& v : vertices) {
      if (v->id().get_value() == vid) {
        solve_vids.push_back(v->id());
        break;
      }
    }
  }
  return solve_vids;
}

std::vector<EdgeId > pathToEdgeId(std::vector<int64_t>& path, const std::vector<GraphOfConvexSets::Edge*>& edges) {
  std::vector<EdgeId> solve_eids;
  for (int i=0; i<path.size()-1; ++i) {
    int64_t uid = path[i];
    int64_t vid = path[i+1];
    for (auto& e : edges) {
      if (e->u().id().get_value() == uid && e->v().id().get_value() == vid) {
        solve_eids.push_back(e->id());
        break;
      }
    }
  }
  return solve_eids;
}

int main() {
  setenv("MOSEKLM_LICENSE_FILE", "/home/gaussian/Documents/softwares/mosektoolslinux64x86/mosek.lic", true);
  auto lic = drake::solvers::MosekSolver::AcquireLicense();


  std::vector<HPolyhedron> regions = utils::DeserializeRegions("/home/gaussian/cmu_ri_phd/phd_research/temp_INSATxGCS/INSATxGCS-Planner/src/data/maze.csv");
  auto edges_bw_regions = utils::DeserializeEdges("/home/gaussian/cmu_ri_phd/phd_research/temp_INSATxGCS/INSATxGCS-Planner/src/data/maze_edges.csv");

  int num_positions = 2;
  int order = 1;
  double h_min = 1e-3;
  double h_max = 1;
  double path_len_weight = 1;
  double time_weight = 0;
  bool verbose = false;

  std::vector<double> costs;

  int64_t vid_on_path[] = {1, 51, 52, 2, 3, 4, 54, 104, 154, 204, 203, 202, 252, 302, 301, 351, 401, 451, 501, 551, 552, 553, 554, 504, 505, 506, 556, 557, 507, 457, 407, 406, 456, 455, 405, 355, 356, 306, 305, 304, 303, 253, 254, 255, 205, 206, 256, 257, 207, 157, 156, 155, 105, 106, 56, 57, 7, 8, 9, 59, 58, 108, 109, 110, 111, 112, 62, 61, 11, 12, 13, 63, 64, 14, 15, 16, 66, 116, 115, 114, 164, 163, 162, 161, 211, 210, 260, 259, 209, 208, 258, 308, 358, 408, 409, 410, 411, 461, 462, 512, 513, 463, 464, 414, 415, 416, 466, 516, 517, 518, 468, 467, 417, 418, 419, 369, 319, 320, 270, 271, 321, 322, 372, 422, 472, 471, 470, 520, 570, 571, 572, 622, 623, 624, 574, 524, 525, 575, 576, 626, 676, 726, 776, 777, 827, 877, 878, 828, 829, 830, 831, 881, 882, 932, 982, 1032, 1033, 1083, 1133, 1134, 1135, 1185, 1184, 1183, 1233, 1283, 1333, 1332, 1382, 1383, 1433, 1483, 1533, 1583, 1584, 1534, 1535, 1485, 1435, 1385, 1386, 1436, 1486, 1487, 1537, 1587, 1586, 1585, 1635, 1685, 1735, 1785, 1835, 1885, 1935, 1936, 1937, 1987, 2037, 2087, 2086, 2136, 2186, 2185, 2235, 2285, 2286, 2287, 2237, 2238, 2239, 2289, 2290, 2340, 2341, 2342, 2343, 2344, 2294, 2244, 2245, 2195, 2196, 2197, 2247, 2246, 2296, 2346, 2396, 2446, 2496, 2497, 2447, 2448, 2449, 2499, 2500};
  int sz = static_cast<int>(sizeof(vid_on_path) / sizeof(int64_t ));

//  for (int i=4; i<sz-1; ++i) {
  for (int i=4; i<5; ++i) {
//  for (int i=29; i<34; ++i) {
    drake::planning::trajectory_optimization::GcsTrajectoryOptimization::Subgraph* main_graph = nullptr;

    drake::geometry::optimization::ConvexSets regions_cs;
    for (int j=1; j<i; ++j) {
      auto cs = MakeConvexSets(regions[vid_on_path[j]-1]);
      regions_cs.push_back(cs[0]);
    }

    std::vector<std::pair<int, int>> edges_cs;
    for (int j=0; j<i-2; ++j) {
      edges_cs.emplace_back(j, j+1);
    }

    auto opt = drake::planning::trajectory_optimization::GcsTrajectoryOptimization(num_positions);
    opt.AddPathLengthCost(path_len_weight);
    opt.AddTimeCost(0);
    main_graph = &opt.AddRegions(regions_cs, edges_cs, order, h_min, h_max);

    auto cs_st = MakeConvexSets(regions[vid_on_path[0]-1]);
    auto cs_go = MakeConvexSets(regions[vid_on_path[i]-1]);
//    std::vector<std::pair<int, int>> e_st(1, {i-2, i-1});
//    std::vector<std::pair<int, int>> e_go(1, {i-1, i});

    drake::planning::trajectory_optimization::GcsTrajectoryOptimization::Subgraph* start_convex_set = nullptr;
    drake::planning::trajectory_optimization::GcsTrajectoryOptimization::Subgraph* goal_convex_set = nullptr;
//    opt.AddRegions(cs_st, e_st, order, h_min, h_max);
//    opt.AddRegions(cs_go, e_go, order, h_min, h_max);
    start_convex_set = &opt.AddRegions(cs_st, order, h_min, h_max);
    goal_convex_set = &opt.AddRegions(cs_go, order, h_min, h_max);

    opt.AddEdges(*start_convex_set, *main_graph);
    opt.AddEdges(*main_graph, *goal_convex_set);

    auto soln = opt.SolvePath(*start_convex_set, *goal_convex_set);

    if (soln.second.is_success()) {

      std::cout << (cs_st[0]->PointInSet(soln.first.value(soln.first.start_time()))? "Start in start set": "Start NOT in start set") << std::endl;
      std::cout << (cs_go[0]->PointInSet(soln.first.value(soln.first.end_time()))? "Goal in goal set": "Goal NOT in goal set") << std::endl;
      std::cout << "Start cell: " << vid_on_path[0] << " Start set center: " << regions[vid_on_path[0]-1].ChebyshevCenter().transpose() << std::endl;
      std::cout << "Goal cell: " << vid_on_path[i] << " Goal set center: " << regions[vid_on_path[i]-1].ChebyshevCenter().transpose() << std::endl;

      double path_length = 0;
      auto final_trajectory = soln.first;
      auto p0 = final_trajectory.value(0);
      double dt = final_trajectory.end_time()/100;
//      std::cout << "end_time: " << final_trajectory.end_time() << std::endl;
      for (double t=0; t<final_trajectory.end_time(); t+=dt) {
        path_length += (final_trajectory.value(t) - p0).norm();
        p0 = final_trajectory.value(t);
//        std::cout << final_trajectory.value(t).transpose() << std::endl;
        std::cout << final_trajectory.value(t)(0) << " " << final_trajectory.value(t)(1) << std::endl;
//        std::cout << t << " " << final_trajectory.value(t)(0) << " " << final_trajectory.value(t)(1) << std::endl;
      }

      std::cout << "(" << vid_on_path[edges_cs[0].first] << ", " << vid_on_path[edges_cs[0].second] << "), ";
      for (auto e : edges_cs) {
        std::cout << "(" << vid_on_path[e.first+1] << ", " << vid_on_path[e.second+1] << "), ";
      }
      std::cout << "(" << vid_on_path[edges_cs.back().first+2] << ", " << vid_on_path[edges_cs.back().second+2] << "), ";
      std::cout << std::endl;
      costs.push_back(path_length);
    } else {
      std::cerr << "Solution NOT found..." << std::endl;
      std::runtime_error("Solution NOT found...");
    }
    std::cout << "-1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 " << std::endl;
//    std::cout << i << " convergence status: " << soln.second.get_solution_result() << std::endl;
  }

  int i = 0;
  for (auto c : costs) {
    std::cout << i << "\t" << c << "\n";
    ++i;
  }
  std::cout << std::endl;

  return 0;
}
