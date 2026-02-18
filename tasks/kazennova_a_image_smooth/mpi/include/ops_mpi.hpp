#pragma once

#include <cstdint>
#include <vector>

#include "kazennova_a_image_smooth/common/include/common.hpp"
#include "task/include/task.hpp"

namespace kazennova_a_image_smooth {

class KazennovaAImageSmoothMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit KazennovaAImageSmoothMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;

  void DistributeImage();
  void ApplyKernelToStrip();
  void ExchangeBoundaries();
  void GatherResult();
  uint8_t ApplyKernelToPixel(int x, int y, int c);

  std::vector<uint8_t> local_strip_;
  int strip_height_;
  int strip_offset_;
};

}  // namespace kazennova_a_image_smooth
