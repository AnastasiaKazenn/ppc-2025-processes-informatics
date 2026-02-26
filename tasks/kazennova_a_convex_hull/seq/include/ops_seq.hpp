#pragma once

#include <vector>

#include "kazennova_a_convex_hull/common/include/common.hpp"
#include "task/include/task.hpp"

namespace kazennova_a_convex_hull {

class KazennovaAConvexHullSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit KazennovaAConvexHullSEQ(const InType &in);

  static double DistSq(const Point &a, const Point &b);
  static double Orientation(const Point &a, const Point &b, const Point &c);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;

  static std::vector<Point> FilterCollinearPoints(const Point &pivot, std::vector<Point> &points);
  static std::vector<Point> BuildHull(const Point &pivot, const std::vector<Point> &filtered);
};

}  // namespace kazennova_a_convex_hull
