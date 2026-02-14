#pragma once

#include "kazenova_a_vec_change_sign/common/include/common.hpp"
#include "task/include/task.hpp"

namespace kazenova_a_vec_change_sign {

class KazenovaAVecChangeSignMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit KazenovaAVecChangeSignMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;

  void ProcessSmallVector(int world_rank, int total_size);
  static void ComputeChunkBounds(int world_rank, int world_size, int total_size, int &start_idx, int &end_idx);
  int CountLocalChanges(int start_idx, int end_idx);
  int CheckBoundary(int world_rank, int world_size, int end_idx, int total_size);
};

}  // namespace kazenova_a_vec_change_sign
