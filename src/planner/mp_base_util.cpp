#include <motion_primitive_library/planner/mp_base_util.h>

using namespace MPL;

MPBaseUtil::MPBaseUtil() {
  planner_verbose_ = false;
}

bool MPBaseUtil::initialized() {
  return !(ss_ptr_ == nullptr);
}

void MPBaseUtil::reset() {
  ss_ptr_ = nullptr;
}

void MPBaseUtil::setLPAstar(bool use_lpastar) {
  use_lpastar_ =  use_lpastar;
  if(use_lpastar_)
    printf("[MPBaseUtil] use Lifelong Planning A*\n");
  else
    printf("[MPBaseUtil] use normal A*\n");
}

void MPBaseUtil::setEpsilon(decimal_t eps) {
  epsilon_ = eps;
  if(planner_verbose_)
    printf("[MPBaseUtil] set epsilon: %f\n", epsilon_);
}

void MPBaseUtil::setMaxNum(int num) {
  max_num_ = num;
  if(planner_verbose_)
    printf("[MPBaseUtil] set max num: %d\n", max_num_);
}

void MPBaseUtil::setDt(decimal_t dt) {
  ENV_->set_dt(dt);
  if(planner_verbose_)
    printf("[MPBaseUtil] set dt: %f\n", dt);
}

void MPBaseUtil::setW(decimal_t w) {
  ENV_->set_w(w);
  if(planner_verbose_)
    printf("[MPBaseUtil] set w: %f\n", w);
}

void MPBaseUtil::setAlpha(int alpha) {
  ENV_->set_alpha(alpha);
  if(planner_verbose_)
    printf("[MPBaseUtil] set alpha: %d\n", alpha);
}

void MPBaseUtil::setU(int n, bool use_3d) {
  ENV_->set_discretization(n, use_3d);
}

void MPBaseUtil::setU(const vec_Vec3f& U) {
  ENV_->set_U(U);
}

void MPBaseUtil::setVmax(decimal_t v_max) {
  ENV_->set_v_max(v_max);
  if(planner_verbose_)
    printf("[MPBaseUtil] set v_max: %f\n", v_max);
}

void MPBaseUtil::setAmax(decimal_t a_max) {
  ENV_->set_a_max(a_max);
  if(planner_verbose_)
    printf("[MPBaseUtil] set a_max: %f\n", a_max);
}

void MPBaseUtil::setJmax(decimal_t j_max) {
  ENV_->set_j_max(j_max);
  if(planner_verbose_)
    printf("[MPBaseUtil] set j_max: %f\n", j_max);
}

void MPBaseUtil::setUmax(decimal_t u_max) {
  ENV_->set_u_max(u_max);
  if(planner_verbose_)
    printf("[MPBaseUtil] set u_max: %f\n", u_max);
}

void MPBaseUtil::setTmax(decimal_t t) {
  //ENV_->set_t_max(t);
  max_t_ = t;
  if(planner_verbose_)
    printf("[MPBaseUtil] set max time: %f\n", t);
}

void MPBaseUtil::setPriorTrajectory(const Trajectory& traj) {
  ENV_->set_prior_trajectory(traj);
  if(planner_verbose_)
    printf("[MPBaseUtil] set prior trajectory\n");
}

void MPBaseUtil::setTol(decimal_t tol_dis, decimal_t tol_vel, decimal_t tol_acc) {
  ENV_->set_tol_dis(tol_dis);
  ENV_->set_tol_vel(tol_vel);
  ENV_->set_tol_acc(tol_acc);
  if(planner_verbose_) {
    printf("[MPBaseUtil] set tol_dis: %f\n", tol_dis);
    printf("[MPBaseUtil] set tol_vel: %f\n", tol_vel);
    printf("[MPBaseUtil] set tol_acc: %f\n", tol_acc);
  }
}

std::vector<Primitive> MPBaseUtil::getValidPrimitives() const { 
  std::vector<Primitive> prs;
  for(const auto& it: ss_ptr_->hm_) {
   if(it.second && !it.second->pred_hashkey.empty()) {
      for(unsigned int i = 0; i < it.second->pred_hashkey.size(); i++) {
        Key key = it.second->pred_hashkey[i];
        //if(!ss_ptr_->hm_[key] || std::isinf(it.second->pred_action_cost[i])) 
        if(std::isinf(it.second->pred_action_cost[i])) 
          continue;
        Primitive pr;
        ENV_->forward_action( ss_ptr_->hm_[key]->coord, it.second->pred_action_id[i], pr );
        prs.push_back(pr);
      }
    }
  }

  if(planner_verbose_)
    printf("number of states in hm: %zu, number of valid prs: %zu\n", 
        ss_ptr_->hm_.size(), prs.size());
 
  return prs;
}

std::vector<Primitive> MPBaseUtil::getAllPrimitives() const { 
  std::vector<Primitive> prs;
  for(const auto& it: ss_ptr_->hm_) {
    if(it.second && !it.second->pred_hashkey.empty()) {
      for(unsigned int i = 0; i < it.second->pred_hashkey.size(); i++) {
        Key key = it.second->pred_hashkey[i];
        Primitive pr;
        ENV_->forward_action( ss_ptr_->hm_[key]->coord, it.second->pred_action_id[i], pr );
        prs.push_back(pr);
      }
    }
  }


  if(planner_verbose_) 
    printf("number of states in hm: %zu, number of prs: %zu\n", 
        ss_ptr_->hm_.size(), prs.size());

  return prs;
}

std::vector<Waypoint> MPBaseUtil::getWs() const {
  return ws_; 
}

Trajectory MPBaseUtil::getTraj() const {
  return traj_;
}

vec_Vec3f MPBaseUtil::getOpenSet() const {
  vec_Vec3f ps;
  for(const auto& it: ss_ptr_->pq_)
    ps.push_back(it.second->coord.pos);
  return ps;
}

vec_Vec3f MPBaseUtil::getCloseSet() const {
  vec_Vec3f ps;
  for(const auto& it: ss_ptr_->hm_) {
    if(it.second && it.second->iterationclosed)
      ps.push_back(it.second->coord.pos);
  }
  return ps;
}

vec_Vec3f MPBaseUtil::getNullSet() const {
  vec_Vec3f ps;
  for(const auto& it: ss_ptr_->hm_) {
    if(it.second && !it.second->iterationopened)
      ps.push_back(it.second->coord.pos);
  }
  return ps;
}

vec_Vec3f MPBaseUtil::getExpandedNodes() const {
  return ENV_->expanded_nodes_;
}

int MPBaseUtil::getExpandedNum() const {
  return ss_ptr_->expand_iteration_;
}

void MPBaseUtil::getSubStateSpace(int time_step) {
  ss_ptr_->getSubStateSpace(time_step);
}

void MPBaseUtil::checkValidation() {
  ss_ptr_->checkValidation(ss_ptr_->hm_);
}

bool MPBaseUtil::plan(const Waypoint &start, const Waypoint &goal) {
  if(planner_verbose_) {
    printf("start pos: [%f, %f, %f], vel: [%f, %f, %f], acc: [%f, %f, %f]\n",
        start.pos(0), start.pos(1), start.pos(2),
        start.vel(0), start.vel(1), start.vel(2),
        start.acc(0), start.acc(1), start.acc(2));
    printf("goal pos: [%f, %f, %f], vel: [%f, %f, %f], acc: [%f, %f, %f]\n",
        goal.pos(0), goal.pos(1), goal.pos(2),
        goal.vel(0), goal.vel(1), goal.vel(2),
        goal.acc(0), goal.acc(1), goal.acc(2));
  }

  if(!ENV_->is_free(start.pos)) {
    printf(ANSI_COLOR_RED "[MPPlanner] start is not free!" ANSI_COLOR_RESET "\n");
    return false;
  }

  if(start.use_pos && start.use_vel && start.use_acc && start.use_jrk) {
    ENV_->set_wi(3);
    if(planner_verbose_)
      printf("[MPBaseUtil] set effort in snap\n");
  }
  else if(start.use_pos && start.use_vel && start.use_acc && !start.use_jrk) {
    ENV_->set_wi(2);
    if(planner_verbose_)
      printf("[MPBaseUtil] set effort in jrk\n");
  }
  else if(start.use_pos && start.use_vel && !start.use_acc && !start.use_jrk) {
    ENV_->set_wi(1);
    if(planner_verbose_)
      printf("[MPBaseUtil] set effort in acc\n");
  }
  else if(start.use_pos && !start.use_vel && !start.use_acc && !start.use_jrk) {
    ENV_->set_wi(0);
    if(planner_verbose_)
      printf("[MPBaseUtil] set effort in vel\n");
  }
  else {
    if(planner_verbose_) {
      printf(ANSI_COLOR_RED "[MPBaseUtil] fail to set effort, pos/vel/acc/jrk --> %d/%d/%d/%d\n" ANSI_COLOR_RESET,
          start.use_pos, start.use_vel, start.use_acc, start.use_jrk);
    }
    return false;
  }


  std::unique_ptr<MPL::GraphSearch> planner_ptr(new MPL::GraphSearch(planner_verbose_));

  // If use A*, reset the state space 
  if(!use_lpastar_) 
    ss_ptr_.reset(new MPL::StateSpace(epsilon_));
  else {
    // If use LPA*, reset the state space only at the initial planning
    if(!initialized()) {
      if(planner_verbose_)
        printf(ANSI_COLOR_CYAN "[MPPlanner] reset planner state space!" ANSI_COLOR_RESET "\n");
      ss_ptr_.reset(new MPL::StateSpace(epsilon_));
    }
  }

  ENV_->set_goal(goal);

  ENV_->expanded_nodes_.clear();

  ss_ptr_->dt_ = ENV_->get_dt();
  if(use_lpastar_)
    planner_ptr->LPAstar(start, ENV_->state_to_idx(start), ENV_, ss_ptr_, traj_, max_num_, max_t_);
  else
    planner_ptr->Astar(start, ENV_->state_to_idx(start), ENV_, ss_ptr_, traj_, max_num_, max_t_);

  if (traj_.segs.empty()) {
    if(planner_verbose_)
      printf(ANSI_COLOR_RED "[MPPlanner] Cannot find a traj!" ANSI_COLOR_RESET "\n");
    return false;
  }

  ws_.clear();
  ws_.push_back(start);
  double time = 0;
  std::vector<decimal_t> dts = traj_.getSegsT();
  for (const auto &t : dts) {
    time += t;
    Waypoint waypoint;
    traj_.evaluate(time, waypoint);
    waypoint.use_pos = start.use_pos;
    waypoint.use_vel = start.use_vel;
    waypoint.use_acc = start.use_acc;
    waypoint.use_jrk = start.use_jrk;
    ws_.push_back(waypoint);
  }

  return true;
}
