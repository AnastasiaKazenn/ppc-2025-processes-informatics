#include "kazenova_a_vec_change_sign/mpi/include/ops_mpi.hpp"

#include <mpi.h>

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

bool KazenovaAVecChangeSignMPI::PreProcessingImpl() {
  int initialized;
  MPI_Initialized(&initialized);
  if (!initialized) {
    MPI_Init(nullptr, nullptr);
  }
  return true;
}

bool KazenovaAVecChangeSignMPI::RunImpl() {
  const auto &input_vec = GetInput();
  int world_size, world_rank;

  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  int total_size = input_vec.size();
  int local_size = total_size / world_size;
  int remainder = total_size % world_size;
  int start = world_rank * local_size + std::min(world_rank, remainder);
  int end = start + local_size + (world_rank < remainder ? 1 : 0);

  if (world_rank == world_size - 1) {
    end = total_size;
  }

  int local_count = 0;
  for (int i = std::max(start, 1); i < end; i++) {
    if (i == start && start > 0) {
      if ((input_vec[i] > 0 && input_vec[i - 1] < 0) ||
          (input_vec[i] < 0 && input_vec[i - 1] > 0)) {
        local_count++;
      }
    }

    if (i > start) {
      if ((input_vec[i] > 0 && input_vec[i - 1] < 0) ||
          (input_vec[i] < 0 && input_vec[i - 1] > 0)) {
        local_count++;
      }
    }
  }

  int global_count = 0;
  MPI_Reduce(&local_count, &global_count, 1, MPI_INT, MPI_SUM, 0,
             MPI_COMM_WORLD);

  if (world_rank == 0) {
    GetOutput() = global_count;
  } else {
    GetOutput() = 0;
  }

  MPI_Barrier(MPI_COMM_WORLD);
  return true;
}

bool KazenovaAVecChangeSignMPI::PostProcessingImpl() { return true; }

} // namespace kazenova_a_vec_change_sign