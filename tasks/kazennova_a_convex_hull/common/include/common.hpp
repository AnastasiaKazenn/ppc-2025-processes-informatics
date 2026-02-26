#pragma once

#include <string>
#include <tuple>
#include <vector>

#include "task/include/task.hpp"

namespace kazennova_a_convex_hull {

struct Point {
  double x, y;

  Point() : x(0.0), y(0.0) {}
  Point(double x_coord, double y_coord) : x(x_coord), y(y_coord) {}

  bool operator==(const Point &other) const {
    return x == other.x && y == other.y;
  }

  bool operator<(const Point &other) const {
    return (y < other.y) || (y == other.y && x < other.x);
  }
};

using InType = std::vector<Point>;
using OutType = std::vector<Point>;
using TestType = std::tuple<int, std::string>;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace kazennova_a_convex_hull
