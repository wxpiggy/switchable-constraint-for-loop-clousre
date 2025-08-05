#include <vector>
#include <fstream>
#include <iostream>
#include <Eigen/Core>
#include "error_term.h"

Sophus::SE3d ReadVertex(std::ifstream *fin) {
  double x, y, z, qx, qy, qz, qw;
  *fin >> x >> y >> z >> qx >> qy >> qz >> qw;
  Sophus::SE3d
      pose(Eigen::Quaterniond(qw, qx, qy, qz), Eigen::Vector3d(x, y, z));
  return pose;
}

Sophus::SE3d ReadEdge(std::ifstream *fin) {
  double x, y, z, qx, qy, qz, qw;
  *fin >> x >> y >> z >> qx >> qy >> qz >> qw;
  Sophus::SE3d
      pose(Eigen::Quaterniond(qw, qx, qy, qz), Eigen::Vector3d(x, y, z));

  double information;
  for (int i = 0; i < 21; i++){
     *fin >> information ;
     information = information / 10;
  }
   
  return pose;
}

int main(int argc, char **argv) {
  typedef Eigen::aligned_allocator<Sophus::SE3d> sophus_allocator;
  std::vector<Sophus::SE3d, sophus_allocator> vertices;
  std::vector<std::pair<std::pair<int, int>, Sophus::SE3d>, sophus_allocator>
      edges;

  std::ifstream fin("../test.g2o");
  if (!fin.is_open()) {
    std::cerr << "Failed to open file ../test.g2o" << std::endl;
    return -1;
  }

  std::string data_type;
  while (fin.good()) {
    fin >> data_type;
    if (data_type == "VERTEX_SE3:QUAT") {
      int id;
      fin >> id;
      vertices.emplace_back(ReadVertex(&fin));
    } else if (data_type == "EDGE_SE3:QUAT") {
      int i, j;
      fin >> i >> j;
      edges.emplace_back(std::pair<int, int>(i, j), ReadEdge(&fin));
    }
    fin >> std::ws;
  }
  {
    std::ofstream fout_loops("loop_closures.txt");
    if (!fout_loops.is_open()) {
      std::cerr << "Failed to open loop_closures.txt for writing" << std::endl;
      return -1;
    }

    for (auto &e : edges) {
      int i = e.first.first;
      int j = e.first.second;
      // 非连续顶点边即回环边
      if (i + 1 != j) {
        fout_loops << i << " " << j << "\n";
      }
    }
  }
  // 先保存优化前轨迹
  {
    std::ofstream fout_before("poses_before.txt");
    for (auto &pose : vertices) {
      Eigen::Vector3d t = pose.translation();
      Eigen::Quaterniond q = pose.unit_quaternion();
      fout_before << t.x() << " " << t.y() << " " << t.z() << " "
                  << q.x() << " " << q.y() << " " << q.z() << " " << q.w() << "\n";
    }
  }

  // 统计回环边数量
  int num_loop_closures = 0;
  for (auto &e : edges) {
    int i = e.first.first;
    int j = e.first.second;
    if (i + 1 != j) {
      num_loop_closures++;
    }
  }

  std::vector<double> v_s(num_loop_closures, 1.0);
  std::vector<double> v_gamma(num_loop_closures, 1.0);

  ceres::Problem problem;

  int loop_idx = 0;

  for (auto &e : edges) {
    int i = e.first.first;
    int j = e.first.second;

    if (i >= vertices.size() || j >= vertices.size()) {
      std::cerr << "Vertex index out of range: " << i << ", " << j << std::endl;
      continue;
    }

    auto &pose_i = vertices.at(i);
    auto &pose_j = vertices.at(j);
    auto &edge = e.second;

    if (i + 1 == j) {
      ceres::CostFunction *cost_function = OdometryFunctor::Creat(edge);
      problem.AddResidualBlock(cost_function, new ceres::HuberLoss(1.0), pose_i.data(), pose_j.data());
    } else {
      ceres::CostFunction *cost_function = LoopClosureFunctor::Creat(edge);
      problem.AddResidualBlock(cost_function,
                               new ceres::HuberLoss(1.0),
                               pose_i.data(),
                               pose_j.data(),
                               &v_s[loop_idx]);

      ceres::CostFunction *cost_function1 = PriorFunctor::Create(v_gamma[loop_idx]);
      problem.AddResidualBlock(cost_function1, NULL, &v_s[loop_idx]);

      problem.SetParameterLowerBound(&v_s[loop_idx], 0, 0.0);
      problem.SetParameterUpperBound(&v_s[loop_idx], 0, 1.0);

      loop_idx++;
    }
  }

  for (auto &vertex : vertices) {
    problem.SetParameterization(vertex.data(), new LocalParameterizationSE3);
  }

  problem.SetParameterBlockConstant(vertices.at(0).data());

  ceres::Solver::Options options;
  options.minimizer_progress_to_stdout = true;
  options.max_num_iterations = 5;
  ceres::Solver::Summary summary;
  ceres::Solve(options, &problem, &summary);
  std::cout << summary.FullReport() << std::endl;

  // 保存优化后轨迹
  {
    std::ofstream fout_after("poses_after.txt");
    for (auto &pose : vertices) {
      Eigen::Vector3d t = pose.translation();
      Eigen::Quaterniond q = pose.unit_quaternion();
      fout_after << t.x() << " " << t.y() << " " << t.z() << " "
                 << q.x() << " " << q.y() << " " << q.z() << " " << q.w() << "\n";
    }
  }

  // 输出优化后的回环开关变量
  std::cout << "Optimized loop closure switches (s):" << std::endl;
  for (size_t i = 0; i < v_s.size(); i++) {
    std::cout << "s[" << i << "] = " << v_s[i] << std::endl;
  }

  return 0;
}