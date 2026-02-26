#include "kazennova_a_convex_hull/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <cstddef>
#include <vector>

#include "kazennova_a_convex_hull/common/include/common.hpp"

namespace kazennova_a_convex_hull {

double KazennovaAConvexHullMPI::DistSq(const Point &a, const Point &b) {
  double dx = a.x - b.x;
  double dy = a.y - b.y;
  return (dx * dx) + (dy * dy);
}

double KazennovaAConvexHullMPI::Orientation(const Point &a, const Point &b, const Point &c) {
  return ((b.x - a.x) * (c.y - b.y)) - ((b.y - a.y) * (c.x - b.x));
}

class PolarAngleComparator {
 private:
  const Point *pivot_;

 public:
  explicit PolarAngleComparator(const Point &p) : pivot_(&p) {}

  bool operator()(const Point &a, const Point &b) const {
    double orient = KazennovaAConvexHullMPI::Orientation(*pivot_, a, b);
    if (orient == 0.0) {
      return KazennovaAConvexHullMPI::DistSq(*pivot_, a) < KazennovaAConvexHullMPI::DistSq(*pivot_, b);
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
  int world_size = 0;
  int world_rank = 0;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  int total_points = static_cast<int>(all_points.size());

  if (world_rank == 0) {
    for (int i = 1; i < world_size; ++i) {
      MPI_Send(&total_points, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
    }

    int base_size = total_points / world_size;
    int remainder = total_points % world_size;

    int start = 0;
    for (int i = 0; i < world_size; ++i) {
      int count = base_size + (i < remainder ? 1 : 0);

      if (i == 0) {
        local_points_.assign(all_points.begin(), all_points.begin() + count);
      } else {
        int bytes_to_send = count * static_cast<int>(sizeof(Point));
        MPI_Send(all_points.data() + start, bytes_to_send, MPI_BYTE, i, 1, MPI_COMM_WORLD);
      }
      start += count;
    }
  } else {
    MPI_Recv(&total_points, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    int base_size = total_points / world_size;
    int remainder = total_points % world_size;
    int count = base_size + (world_rank < remainder ? 1 : 0);

    local_points_.resize(count);
    int bytes_to_recv = count * static_cast<int>(sizeof(Point));
    MPI_Recv(local_points_.data(), bytes_to_recv, MPI_BYTE, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }
}

std::vector<Point> KazennovaAConvexHullMPI::FilterCollinearPoints(const Point &pivot, std::vector<Point> &points) {
  std::vector<Point> filtered;
  if (points.empty()) {
    return filtered;
  }

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
  return filtered;
}

std::vector<Point> KazennovaAConvexHullMPI::BuildHull(const Point &pivot, const std::vector<Point> &filtered) {
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

std::vector<Point> KazennovaAConvexHullMPI::ComputeLocalHull(const std::vector<Point> &points) {
  if (points.size() <= 3) {
    return points;
  }

  auto local_points = points;

  size_t pivot_idx = FindMinIndex(local_points);
  Point pivot = local_points[pivot_idx];
  local_points.erase(local_points.begin() + static_cast<ptrdiff_t>(pivot_idx));

  auto polar_comp = [&pivot](const Point &a, const Point &b) { return PolarAngle(pivot, a, b); };

  if (!local_points.empty()) {
    SortQuick(local_points, 0, static_cast<int>(local_points.size()) - 1, polar_comp);
  }

  auto filtered = FilterCollinearPoints(pivot, local_points);
  return BuildHull(pivot, filtered);
}

std::vector<Point> KazennovaAConvexHullMPI::GatherLocalHulls() {
  int world_size = 0;
  int world_rank = 0;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  std::vector<Point> local_hull = ComputeLocalHull(local_points_);
  int local_size = static_cast<int>(local_hull.size());

  std::vector<Point> all_hull_points;

  if (world_rank == 0) {
    std::vector<int> sizes(world_size);
    MPI_Gather(&local_size, 1, MPI_INT, sizes.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

    std::vector<int> displs(world_size, 0);
    int total_size = sizes[0];
    for (int i = 1; i < world_size; ++i) {
      displs[i] = displs[i - 1] + sizes[i - 1];
      total_size += sizes[i];
    }

    all_hull_points.resize(total_size);

    CopyElements(local_hull, all_hull_points, 0);

    for (int i = 1; i < world_size; ++i) {
      int bytes_to_recv = sizes[i] * static_cast<int>(sizeof(Point));
      MPI_Recv(all_hull_points.data() + displs[i], bytes_to_recv, MPI_BYTE, i, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
  } else {
    MPI_Gather(&local_size, 1, MPI_INT, nullptr, 0, MPI_INT, 0, MPI_COMM_WORLD);
    int bytes_to_send = local_size * static_cast<int>(sizeof(Point));
    MPI_Send(local_hull.data(), bytes_to_send, MPI_BYTE, 0, 2, MPI_COMM_WORLD);
  }

  return all_hull_points;
}

bool KazennovaAConvexHullMPI::RunImpl() {
  DistributePoints();

  std::vector<Point> all_hull_points = GatherLocalHulls();

  int world_rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  if (world_rank == 0) {
    if (all_hull_points.size() <= 3) {
      GetOutput() = all_hull_points;
      return true;
    }

    GetOutput() = ComputeLocalHull(all_hull_points);
  }

  MPI_Barrier(MPI_COMM_WORLD);
  return true;
}

bool KazennovaAConvexHullMPI::PostProcessingImpl() {
  int world_rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  if (world_rank == 0) {
    return !GetOutput().empty();
  }
  return true;
}

}  // namespace kazennova_a_convex_hull
