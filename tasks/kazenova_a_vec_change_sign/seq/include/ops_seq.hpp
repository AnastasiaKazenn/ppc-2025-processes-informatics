#pragma once

#include "kazenova_a_vec_change_sign/common/include/common.hpp"
#include "task/include/task.hpp"

namespace kazenova_a_vec_change_sign {

class KazenovaAVecChangeSignSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit KazenovaAVecChangeSignSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace kazenova_a_vec_change_sign
