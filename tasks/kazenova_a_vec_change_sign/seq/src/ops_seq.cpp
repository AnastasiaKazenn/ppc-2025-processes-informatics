#include "kazenova_a_vec_change_sign/seq/include/ops_seq.hpp"

#include <cstddef>
#include <vector>

#include "kazenova_a_vec_change_sign/common/include/common.hpp"

namespace kazenova_a_vec_change_sign {

KazenovaAVecChangeSignSEQ::KazenovaAVecChangeSignSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool KazenovaAVecChangeSignSEQ::ValidationImpl() {
  return (!GetInput().empty()) && (GetOutput() == 0);
}

bool KazenovaAVecChangeSignSEQ::PreProcessingImpl() {
  return true;
}

bool KazenovaAVecChangeSignSEQ::RunImpl() {
  const auto &input_vec = GetInput();
  int change_count = 0;

  for (size_t i = 1; i < input_vec.size(); i++) {
    if ((input_vec[i] >= 0 && input_vec[i - 1] < 0) || (input_vec[i] < 0 && input_vec[i - 1] >= 0)) {
      change_count++;
    }
  }

  GetOutput() = change_count;
  return true;
}

bool KazenovaAVecChangeSignSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace kazenova_a_vec_change_sign
