#pragma once

#include <vector>

#include "kazennova_a_convex_hull/common/include/common.hpp"
#include "task/include/task.hpp"

namespace kazennova_a_convex_hull {

class KazennovaAConvexHullMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit KazennovaAConvexHullMPI(const InType &in);

  static double DistSq(const Point &a, const Point &b);
  static double Orientation(const Point &a, const Point &b, const Point &c);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;

  void DistributePoints();
  static std::vector<Point> ComputeLocalHull(const std::vector<Point> &points);
  static std::vector<Point> FilterCollinearPoints(const Point &pivot, std::vector<Point> &points);
  static std::vector<Point> BuildHull(const Point &pivot, const std::vector<Point> &filtered);
  std::vector<Point> GatherLocalHulls();

  std::vector<Point> local_points_;
};

}  // namespace kazennova_a_convex_hull
