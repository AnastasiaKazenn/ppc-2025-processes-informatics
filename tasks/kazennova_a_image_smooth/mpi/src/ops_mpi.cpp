#include "kazennova_a_image_smooth/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

#include "kazennova_a_image_smooth/common/include/common.hpp"

namespace kazennova_a_image_smooth {

const float kernel[3][3] = {
    {1.0F / 16, 2.0F / 16, 1.0F / 16}, {2.0F / 16, 4.0F / 16, 2.0F / 16}, {1.0F / 16, 2.0F / 16, 1.0F / 16}};

KazennovaAImageSmoothMPI::KazennovaAImageSmoothMPI(const InType &in) : strip_height_(0), strip_offset_(0) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = in;
}

bool KazennovaAImageSmoothMPI::ValidationImpl() {
  const auto &in = GetInput();
  return in.width > 0 && in.height > 0 && !in.data.empty() && (in.channels == 1 || in.channels == 3);
}

bool KazennovaAImageSmoothMPI::PreProcessingImpl() {
  return true;
}

void KazennovaAImageSmoothMPI::DistributeImage() {
  const auto &in = GetInput();
  int world_size = 0;
  int world_rank = 0;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  int rows_per_proc = in.height / world_size;
  int remainder = in.height % world_size;

  strip_height_ = rows_per_proc + (world_rank < remainder ? 1 : 0);
  strip_offset_ = (world_rank * rows_per_proc) + std::min(world_rank, remainder);

  int halo_strip_height = strip_height_ + 2;
  int row_size = in.width * in.channels;
  local_strip_.resize(static_cast<size_t>(halo_strip_height) * static_cast<size_t>(row_size));

  std::fill(local_strip_.begin(), local_strip_.end(), 0);

  for (int y = 0; y < strip_height_; ++y) {
    int global_y = strip_offset_ + y;
    int src_offset = global_y * row_size;
    int dst_offset = (y + 1) * row_size;

    std::copy(in.data.begin() + src_offset, in.data.begin() + src_offset + row_size, local_strip_.begin() + dst_offset);
  }
}

uint8_t KazennovaAImageSmoothMPI::ApplyKernelToPixel(int local_y, int x, int c) {
  float sum = 0.0F;
  const auto &in = GetInput();
  int row_size = in.width * in.channels;
  int local_height = strip_height_ + 2;

  for (int ky = -1; ky <= 1; ++ky) {
    for (int kx = -1; kx <= 1; ++kx) {
      int nx = std::clamp(x + kx, 0, in.width - 1);
      int ny_local = std::clamp(local_y + ky, 0, local_height - 1);

      int idx = (ny_local * row_size) + (nx * in.channels) + c;
      sum += local_strip_[idx] * kernel[ky + 1][kx + 1];
    }
  }

  return static_cast<uint8_t>(std::round(sum));
}

void KazennovaAImageSmoothMPI::ApplyKernelToStrip() {
  auto &out = GetOutput();
  const auto &in = GetInput();

  for (int y = 0; y < strip_height_; ++y) {
    int global_y = strip_offset_ + y;
    int local_y = y + 1;

    for (int x = 0; x < in.width; ++x) {
      for (int c = 0; c < in.channels; ++c) {
        // Добавлены скобки для ясности приоритета операций
        int out_idx = ((global_y * in.width + x) * in.channels) + c;
        out.data[out_idx] = ApplyKernelToPixel(local_y, x, c);
      }
    }
  }
}

void KazennovaAImageSmoothMPI::ExchangeBoundaries() {
  const auto &in = GetInput();
  int world_size = 0;  // Инициализировано
  int world_rank = 0;  // Инициализировано
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  if (world_size == 1) {
    int row_size = in.width * in.channels;
    std::copy(local_strip_.begin() + row_size, local_strip_.begin() + row_size + static_cast<ptrdiff_t>(row_size),
              local_strip_.begin());
    int last_data_row = strip_height_ * row_size;
    std::copy(local_strip_.begin() + last_data_row - row_size, local_strip_.begin() + last_data_row,
              local_strip_.begin() + last_data_row + row_size);
    return;
  }

  int row_size = in.width * in.channels;

  if (world_rank > 0) {
    MPI_Sendrecv(local_strip_.data() + row_size, row_size, MPI_BYTE, world_rank - 1, 0, local_strip_.data(), row_size,
                 MPI_BYTE, world_rank - 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  } else {
    std::copy(local_strip_.begin() + row_size, local_strip_.begin() + row_size + static_cast<ptrdiff_t>(row_size),
              local_strip_.begin());
  }

  if (world_rank < world_size - 1) {
    int last_data_row = strip_height_ * row_size;
    MPI_Sendrecv(local_strip_.data() + last_data_row, row_size, MPI_BYTE, world_rank + 1, 1,
                 local_strip_.data() + last_data_row + row_size, row_size, MPI_BYTE, world_rank + 1, 0, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);
  } else {
    int last_data_row = strip_height_ * row_size;
    std::copy(local_strip_.begin() + last_data_row - row_size, local_strip_.begin() + last_data_row,
              local_strip_.begin() + last_data_row + row_size);
  }
}

void KazennovaAImageSmoothMPI::GatherResult() {
  const auto &in = GetInput();
  auto &out = GetOutput();
  int world_size = 0;
  int world_rank = 0;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  if (world_size == 1) {
    return;
  }

  std::vector<int> recv_counts(world_size);
  std::vector<int> recv_displs(world_size);

  int row_size = in.width * in.channels;
  int my_bytes = strip_height_ * row_size;

  MPI_Gather(&my_bytes, 1, MPI_INT, recv_counts.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (world_rank == 0) {
    recv_displs[0] = 0;
    for (int i = 1; i < world_size; ++i) {
      recv_displs[i] = recv_displs[i - 1] + recv_counts[i - 1];
    }
  }

  MPI_Gatherv(local_strip_.data() + row_size, my_bytes, MPI_BYTE, out.data.data(), recv_counts.data(),
              recv_displs.data(), MPI_BYTE, 0, MPI_COMM_WORLD);
}

bool KazennovaAImageSmoothMPI::RunImpl() {
  DistributeImage();
  ExchangeBoundaries();
  ApplyKernelToStrip();
  GatherResult();
  return true;
}

bool KazennovaAImageSmoothMPI::PostProcessingImpl() {
  return !GetOutput().data.empty();
}

}  // namespace kazennova_a_image_smooth
