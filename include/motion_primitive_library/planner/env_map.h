/**
 * @file env_map.h
 * @biref environment for planning in voxel map
 */

#ifndef ENV_MP_H
#define ENV_MP_H
#include <motion_primitive_library/planner/env_base.h>
#include <motion_primitive_library/primitive/primitive.h>
#include <motion_primitive_library/collision_checking/voxel_map_util.h>
#include <unordered_map>

namespace MPL {

  /**
   * @brief Voxel map environment
   */
  class env_map : public env_base
  {
    ///Lookup table for voxels
    using lookUpTable = std::unordered_map<Key, vec_Vec3i>;


    public:
    lookUpTable collision_checking_table_;

    std::shared_ptr<VoxelMapUtil> map_util_;

    ///Constructor with map util as input
    env_map(std::shared_ptr<VoxelMapUtil> map_util)
      : map_util_(map_util)
    {}

    ///Check if a point is in free space
    bool is_free(const Vec3f& pt) const {
      return map_util_->isFree(map_util_->floatToInt(pt));
    }

    /**
     * @brief Check if the primitive is in free space
     *
     * Sample points along the primitive, and check each point for collision; the number of sampling is calculated based on the maximum velocity and resolution of the map.
     */
    bool is_free(const Primitive& pr) const {
      double max_v = std::max(std::max(pr.max_vel(0), pr.max_vel(1)), pr.max_vel(2));
      int n = std::ceil(max_v * pr.t() / map_util_->getRes());
      std::vector<Waypoint> pts = pr.sample(n);
      for(const auto& pt: pts) {
        Vec3i pn = map_util_->floatToInt(pt.pos);
        if(map_util_->isOccupied(pn) || map_util_->isOutSide(pn))
          return false;
      }

      return true;
    }

    /**
     * @brief Get successor
     *
     * @param curr The node to expand
     * @param succ The array stores valid successors
     * @param succ_idx The array stores successors' Key
     * @param succ_cost The array stores cost along valid edges
     * @param action_idx The array stores corresponding idx of control for each successor
     *
     * When goal is outside, extra step is needed for finding optimal trajectory.
     * Only return the primitive satisfies valid dynamic constriants (include the one hits obstacles).
     */
    void get_succ( const Waypoint& curr, 
        std::vector<Waypoint>& succ,
        std::vector<Key>& succ_idx,
        std::vector<double>& succ_cost,
        std::vector<int>& action_idx) const
    {
      succ.clear();
      succ_idx.clear();
      succ_cost.clear();
      action_idx.clear();

      expanded_nodes_.push_back(curr.pos);

      const Vec3i pn = map_util_->floatToInt(curr.pos);
      if(map_util_->isOutSide(pn))
        return;

      for(unsigned int i = 0; i < U_.size(); i++) {
        Primitive pr(curr, U_[i], dt_);
        Waypoint tn = pr.evaluate(dt_);
        if(tn == curr) 
          continue;
        if(pr.valid_vel(v_max_) && pr.valid_acc(a_max_) && pr.valid_jrk(j_max_)) {
          tn.use_pos = curr.use_pos;
          tn.use_vel = curr.use_vel;
          tn.use_acc = curr.use_acc;
          tn.use_jrk = curr.use_jrk;

          succ.push_back(tn);
          succ_idx.push_back(state_to_idx(tn));
          //double cost = is_free(pr) ? 0.01 * pr.J(0) + pr.J(wi_) + w_*dt_: std::numeric_limits<double>::infinity();
          double cost = is_free(pr) ? pr.J(wi_) + w_*dt_: std::numeric_limits<double>::infinity();
          succ_cost.push_back(cost);
          action_idx.push_back(i);
        }
      }

      /*
         if ((goal_outside_ && map_util_->isOutSide(pn)) ||
         (t_max_ > 0 && curr.t >= t_max_ && !succ.empty())) {
         succ.push_back(goal_node_);
         succ_idx.push_back(state_to_idx(goal_node_));
         succ_cost.push_back(get_heur(curr));
         action_idx.push_back(-1); // -1 indicates directly connection to the goal 
         printf("connect to the goal, curr t: %f!\n", curr.t);
      //curr.print();
      }
      */


    }


  };
}


#endif
