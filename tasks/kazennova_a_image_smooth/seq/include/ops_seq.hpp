#pragma once

#include <cstdint>

#include "kazennova_a_image_smooth/common/include/common.hpp"
#include "task/include/task.hpp"

namespace kazennova_a_image_smooth {

class KazennovaAImageSmoothSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit KazennovaAImageSmoothSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;

  uint8_t ApplyKernelToPixel(int x, int y, int c);
};

}  // namespace kazennova_a_image_smooth
