#include "kazennova_a_convex_hull/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace kazennova_a_convex_hull {

// Вспомогательные функции
double KazennovaAConvexHullMPI::DistSq(const Point &a, const Point &b) {
  double dx = a.x - b.x;
  double dy = a.y - b.y;
  return dx * dx + dy * dy;
}

double KazennovaAConvexHullMPI::Orientation(const Point &a, const Point &b, const Point &c) {
  return (b.x - a.x) * (c.y - b.y) - (b.y - a.y) * (c.x - b.x);
}

// Компаратор для сортировки по полярному углу
class PolarAngleComparator {
 private:
  const Point &pivot;

 public:
  explicit PolarAngleComparator(const Point &p) : pivot(p) {}

  bool operator()(const Point &a, const Point &b) const {
    double orient = KazennovaAConvexHullMPI::Orientation(pivot, a, b);
    if (orient == 0.0) {
      return KazennovaAConvexHullMPI::DistSq(pivot, a) < KazennovaAConvexHullMPI::DistSq(pivot, b);
    }
    return orient > 0.0;
  }
};

KazennovaAConvexHullMPI::KazennovaAConvexHullMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = OutType();
}

bool KazennovaAConvexHullMPI::ValidationImpl() {
  return !GetInput().empty();
}

bool KazennovaAConvexHullMPI::PreProcessingImpl() {
  GetOutput().clear();
  local_points_.clear();
  return true;
}

void KazennovaAConvexHullMPI::DistributePoints() {
  const auto &all_points = GetInput();
  int world_size, world_rank;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  int total_points = static_cast<int>(all_points.size());

  if (world_rank == 0) {
    // Рассылаем количество точек всем процессам
    for (int i = 1; i < world_size; ++i) {
      MPI_Send(&total_points, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
    }

    // Распределяем точки поровну
    int base_size = total_points / world_size;
    int remainder = total_points % world_size;

    int start = 0;
    for (int i = 0; i < world_size; ++i) {
      int count = base_size + (i < remainder ? 1 : 0);

      if (i == 0) {
        // Себе оставляем первую порцию
        local_points_.assign(all_points.begin(), all_points.begin() + count);
      } else {
        // Отправляем остальным
        MPI_Send(all_points.data() + start, count * sizeof(Point), MPI_BYTE, i, 1, MPI_COMM_WORLD);
      }
      start += count;
    }
  } else {
    // Получаем количество точек от процесса 0
    MPI_Recv(&total_points, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // Вычисляем сколько точек нам достанется
    int base_size = total_points / world_size;
    int remainder = total_points % world_size;
    int count = base_size + (world_rank < remainder ? 1 : 0);

    // Получаем точки
    local_points_.resize(count);
    MPI_Recv(local_points_.data(), count * sizeof(Point), MPI_BYTE, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }
}

std::vector<Point> KazennovaAConvexHullMPI::ComputeLocalHull(const std::vector<Point> &points) {
  if (points.size() <= 3) {
    return points;
  }

  auto local_points = points;  // копируем для изменений

  // Находим самую нижнюю-левую точку (pivot)
  auto pivot_it = std::min_element(local_points.begin(), local_points.end());
  Point pivot = *pivot_it;
  local_points.erase(pivot_it);

  // Сортируем по полярному углу
  PolarAngleComparator comp(pivot);
  std::sort(local_points.begin(), local_points.end(), comp);

  // Фильтруем коллинеарные точки
  std::vector<Point> filtered;
  if (!local_points.empty()) {
    filtered.push_back(local_points[0]);
    for (size_t i = 1; i < local_points.size(); ++i) {
      while (i < local_points.size() && Orientation(pivot, filtered.back(), local_points[i]) == 0.0) {
        if (DistSq(pivot, local_points[i]) > DistSq(pivot, filtered.back())) {
          filtered.back() = local_points[i];
        }
        ++i;
      }
      if (i < local_points.size()) {
        filtered.push_back(local_points[i]);
      }
    }
  }

  // Строим выпуклую оболочку
  std::vector<Point> hull;
  hull.push_back(pivot);

  if (filtered.empty()) {
    return hull;
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

  return hull;
}

std::vector<Point> KazennovaAConvexHullMPI::GatherLocalHulls() {
  int world_size, world_rank;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  // Сначала вычисляем локальную оболочку
  std::vector<Point> local_hull = ComputeLocalHull(local_points_);
  int local_size = static_cast<int>(local_hull.size());

  std::vector<Point> all_hull_points;

  if (world_rank == 0) {
    // Собираем размеры от всех процессов
    std::vector<int> sizes(world_size);
    MPI_Gather(&local_size, 1, MPI_INT, sizes.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Вычисляем смещения
    std::vector<int> displs(world_size, 0);
    int total_size = sizes[0];
    for (int i = 1; i < world_size; ++i) {
      displs[i] = displs[i - 1] + sizes[i - 1];
      total_size += sizes[i];
    }

    // Подготавливаем буфер для всех точек
    all_hull_points.resize(total_size);

    // Копируем свою локальную оболочку
    std::copy(local_hull.begin(), local_hull.end(), all_hull_points.begin());

    // Собираем данные от всех процессов
    for (int i = 1; i < world_size; ++i) {
      MPI_Recv(all_hull_points.data() + displs[i], sizes[i] * sizeof(Point), MPI_BYTE, i, 2, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);
    }
  } else {
    // Отправляем размер и данные процессу 0
    MPI_Gather(&local_size, 1, MPI_INT, nullptr, 0, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Send(local_hull.data(), local_size * sizeof(Point), MPI_BYTE, 0, 2, MPI_COMM_WORLD);
  }

  return all_hull_points;
}

bool KazennovaAConvexHullMPI::RunImpl() {
  // Распределяем точки между процессами
  DistributePoints();

  // Собираем все локальные оболочки на процессе 0
  std::vector<Point> all_hull_points = GatherLocalHulls();

  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  if (world_rank == 0) {
    // Если точек мало, просто возвращаем их
    if (all_hull_points.size() <= 3) {
      GetOutput() = all_hull_points;
      return true;
    }

    // Строим финальную оболочку из всех точек локальных оболочек
    GetOutput() = ComputeLocalHull(all_hull_points);
  }

  MPI_Barrier(MPI_COMM_WORLD);
  return true;
}

bool KazennovaAConvexHullMPI::PostProcessingImpl() {
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  if (world_rank == 0) {
    return !GetOutput().empty();
  }
  return true;
}

}  // namespace kazennova_a_convex_hull
