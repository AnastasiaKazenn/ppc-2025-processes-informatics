#include "kazennova_a_convex_hull/seq/include/ops_seq.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

namespace kazennova_a_convex_hull {

// Вспомогательные функции
double KazennovaAConvexHullSEQ::DistSq(const Point &a, const Point &b) {
  double dx = a.x - b.x;
  double dy = a.y - b.y;
  return dx * dx + dy * dy;
}

double KazennovaAConvexHullSEQ::Orientation(const Point &a, const Point &b, const Point &c) {
  return (b.x - a.x) * (c.y - b.y) - (b.y - a.y) * (c.x - b.x);
}

// Компаратор для сортировки по полярному углу
class PolarAngleComparator {
 private:
  const Point &pivot;

 public:
  explicit PolarAngleComparator(const Point &p) : pivot(p) {}

  bool operator()(const Point &a, const Point &b) const {
    double orient = KazennovaAConvexHullSEQ::Orientation(pivot, a, b);
    if (orient == 0.0) {
      return KazennovaAConvexHullSEQ::DistSq(pivot, a) < KazennovaAConvexHullSEQ::DistSq(pivot, b);
    }
    return orient > 0.0;
  }
};

KazennovaAConvexHullSEQ::KazennovaAConvexHullSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = OutType();
}

bool KazennovaAConvexHullSEQ::ValidationImpl() {
  return !GetInput().empty();
}

bool KazennovaAConvexHullSEQ::PreProcessingImpl() {
  GetOutput().clear();
  return true;
}

bool KazennovaAConvexHullSEQ::RunImpl() {
  auto points = GetInput();  // копируем для изменений

  // Случаи с малым числом точек
  if (points.size() <= 3) {
    GetOutput() = points;
    return true;
  }

  // 1. Находим самую нижнюю-левую точку (pivot)
  auto pivot_it = std::min_element(points.begin(), points.end());
  Point pivot = *pivot_it;
  points.erase(pivot_it);

  // 2. Сортируем по полярному углу относительно pivot
  PolarAngleComparator comp(pivot);
  std::sort(points.begin(), points.end(), comp);

  // 3. Фильтруем коллинеарные точки (оставляем самую дальнюю)
  std::vector<Point> filtered;
  if (!points.empty()) {
    filtered.push_back(points[0]);
    for (size_t i = 1; i < points.size(); ++i) {
      while (i < points.size() && Orientation(pivot, filtered.back(), points[i]) == 0.0) {
        if (DistSq(pivot, points[i]) > DistSq(pivot, filtered.back())) {
          filtered.back() = points[i];
        }
        ++i;
      }
      if (i < points.size()) {
        filtered.push_back(points[i]);
      }
    }
  }

  std::vector<Point> hull;
  hull.push_back(pivot);

  if (filtered.empty()) {
    GetOutput() = hull;
    return true;
  }

  hull.push_back(filtered[0]);

  for (size_t i = 1; i < filtered.size(); ++i) {
    while (hull.size() >= 2) {
      Point last = hull.back();
      Point second_last = hull[hull.size() - 2];
      double orient = Orientation(second_last, last, filtered[i]);

      if (orient > 0.0) {
        break;
      }
      hull.pop_back();
    }
    hull.push_back(filtered[i]);
  }

  GetOutput() = hull;
  return true;
}

bool KazennovaAConvexHullSEQ::PostProcessingImpl() {
  return !GetOutput().empty();
}

}  // namespace kazennova_a_convex_hull
