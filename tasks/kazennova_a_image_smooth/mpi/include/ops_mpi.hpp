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
  uint8_t ApplyKernelToPixel(int local_y, int x, int c, const std::vector<uint8_t> &strip);

  std::vector<uint8_t> local_strip_;
  std::vector<uint8_t> result_strip_;  // буфер для обработанной полосы
  int strip_height_ = 0;
  int strip_offset_ = 0;
};

}  // namespace kazennova_a_image_smooth
