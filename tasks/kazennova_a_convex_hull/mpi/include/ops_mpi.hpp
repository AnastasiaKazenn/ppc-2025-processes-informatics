#pragma once

#include "kazennova_a_convex_hull/common/include/common.hpp"
#include "task/include/task.hpp"

namespace kazennova_a_convex_hull {

class KazennovaAConvexHullMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit KazennovaAConvexHullMPI(const InType &in);

  // Вспомогательные функции - public для доступа из компаратора
  static double DistSq(const Point& a, const Point& b);
  static double Orientation(const Point& a, const Point& b, const Point& c);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
  
  // MPI-специфичные функции
  void DistributePoints();
  std::vector<Point> ComputeLocalHull(const std::vector<Point>& local_points);
  std::vector<Point> GatherLocalHulls();
  
  std::vector<Point> local_points_;
};

}  // namespace kazennova_a_convex_hull