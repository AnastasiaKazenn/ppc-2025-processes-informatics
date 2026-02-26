#pragma once

#include <cstddef>
#include <stack>
#include <string>
#include <tuple>
#include <utility>
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

// Вспомогательные функции
inline double DistSq(const Point &a, const Point &b) {
  double dx = a.x - b.x;
  double dy = a.y - b.y;
  return (dx * dx) + (dy * dy);
}

inline double Orientation(const Point &a, const Point &b, const Point &c) {
  return ((b.x - a.x) * (c.y - b.y)) - ((b.y - a.y) * (c.x - b.x));
}

inline bool PolarAngle(const Point &pivot, const Point &a, const Point &b) {
  double orient = Orientation(pivot, a, b);
  if (orient == 0.0) {
    return DistSq(pivot, a) < DistSq(pivot, b);
  }
  return orient > 0;
}

// Итеративная быстрая сортировка (без рекурсии)
template <typename T, typename Compare>
int Partition(std::vector<T> &a, int low, int high, Compare comp) {
  T pivot = a[high];
  int i = low - 1;

  for (int j = low; j <= high - 1; j++) {
    if (comp(a[j], pivot)) {
      i++;
      std::swap(a[i], a[j]);
    }
  }
  std::swap(a[i + 1], a[high]);
  return i + 1;
}

template <typename T, typename Compare>
void SortQuick(std::vector<T> &a, int low, int high, Compare comp) {
  if (low >= high) {
    return;
  }

  std::stack<std::pair<int, int>> stack;
  stack.emplace(low, high);

  while (!stack.empty()) {
    auto [l, h] = stack.top();
    stack.pop();

    if (l < h) {
      int pi = Partition(a, l, h, comp);
      if (pi - l > h - pi) {
        stack.emplace(l, pi - 1);
        stack.emplace(pi + 1, h);
      } else {
        stack.emplace(pi + 1, h);
        stack.emplace(l, pi - 1);
      }
    }
  }
}

// Поиск минимального элемента
template <typename T>
size_t FindMinIndex(const std::vector<T> &a) {
  size_t min_idx = 0;
  for (size_t i = 1; i < a.size(); ++i) {
    if (a[i] < a[min_idx]) {
      min_idx = i;
    }
  }
  return min_idx;
}

// Копирование элементов
template <typename T>
void CopyElements(const std::vector<T> &src, std::vector<T> &dst, size_t dst_start) {
  for (size_t i = 0; i < src.size(); ++i) {
    dst[dst_start + i] = src[i];
  }
}

using InType = std::vector<Point>;
using OutType = std::vector<Point>;
using TestType = std::tuple<int, std::string>;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace kazennova_a_convex_hull
