#include "kazenova_a_vec_change_sign/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <numeric>
#include <vector>

#include "kazenova_a_vec_change_sign/common/include/common.hpp"
#include "util/include/util.hpp"

namespace kazenova_a_vec_change_sign {

KazenovaAVecChangeSignMPI::KazenovaAVecChangeSignMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool KazenovaAVecChangeSignMPI::ValidationImpl() {
  return (!GetInput().empty()) && (GetOutput() == 0);
}

bool KazenovaAVecChangeSignMPI::PreProcessingImpl() { return true; }

bool KazenovaAVecChangeSignMPI::RunImpl() {
  const auto &input_vec = GetInput();
  int world_size, world_rank;

  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  int total_size = static_cast<int>(input_vec.size());

  if (total_size < 10000) {
    if (world_rank == 0) {
      int count = 0;
      for (int i = 1; i < total_size; i++) {
        bool prev_pos = (input_vec[i - 1] >= 0);
        bool curr_pos = (input_vec[i] >= 0);
        if (prev_pos != curr_pos)
          count++;
      }
      GetOutput() = count;
    }
    MPI_Bcast(&GetOutput(), 1, MPI_INT, 0, MPI_COMM_WORLD);
    return true;
  }

  int chunk_size = total_size / world_size;
  int remainder = total_size % world_size;

  int start_idx = world_rank * chunk_size + std::min(world_rank, remainder);
  int end_idx = start_idx + chunk_size + (world_rank < remainder ? 1 : 0);

  int local_count = 0;
  for (int i = start_idx + 1; i < end_idx; i++) {
    bool prev_pos = (input_vec[i - 1] >= 0);
    bool curr_pos = (input_vec[i] >= 0);
    if (prev_pos != curr_pos) {
      local_count++;
    }
  }

  int boundary_check = 0;
  if (world_rank < world_size - 1) {

    int last_idx = end_idx - 1;
    int first_next_idx = end_idx;

    if (last_idx >= 0 && first_next_idx < total_size) {
      bool last_pos = (input_vec[last_idx] >= 0);
      bool first_pos = (input_vec[first_next_idx] >= 0);
      if (last_pos != first_pos) {
        boundary_check = 1;
      }
    }
  }

  int total_inner = 0;
  MPI_Reduce(&local_count, &total_inner, 1, MPI_INT, MPI_SUM, 0,
             MPI_COMM_WORLD);

  int total_boundary = 0;
  MPI_Reduce(&boundary_check, &total_boundary, 1, MPI_INT, MPI_SUM, 0,
             MPI_COMM_WORLD);

  int result = 0;
  if (world_rank == 0) {
    result = total_inner + total_boundary;
  }

  MPI_Bcast(&result, 1, MPI_INT, 0, MPI_COMM_WORLD);
  GetOutput() = result;

  return true;
}

bool KazenovaAVecChangeSignMPI::PostProcessingImpl() { return true; }

} // namespace kazenova_a_vec_change_sign