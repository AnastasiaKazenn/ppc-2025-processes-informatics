#include <gtest/gtest.h>
#include <stb/stb_image.h>

#include <array>
#include <cstddef>
#include <string>
#include <tuple>

#include "kazenova_a_vec_change_sign/common/include/common.hpp"
#include "kazenova_a_vec_change_sign/mpi/include/ops_mpi.hpp"
#include "kazenova_a_vec_change_sign/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace kazenova_a_vec_change_sign {

class KazenovaAVecChangeSignFuncTests : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    return std::to_string(std::get<0>(test_param)) + "_" + std::get<1>(test_param);
  }

 protected:
  void SetUp() override {
    TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    int test_case = std::get<0>(params);

    switch (test_case) {
      case 3:
        input_data_ = {1, -2, 3, -4, 5};
        break;
      case 5:
        input_data_ = {1, 2, -3, -4, 5};
        break;
      case 7:
        input_data_ = {-1, -2, -3, -4};
        break;
      default:
        input_data_ = {1, -1, 1, -1};
    }
  }

  bool CheckTestOutputData(OutType &output_data) final {
    int expected = 0;
    for (size_t i = 1; i < input_data_.size(); i++) {
      bool prev_pos = (input_data_[i - 1] >= 0);
      bool curr_pos = (input_data_[i] >= 0);
      if (prev_pos != curr_pos) {
        expected++;
      }
    }
    return output_data == expected;
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;
};

namespace {

TEST_P(KazenovaAVecChangeSignFuncTests, VecChangeSignTest) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 3> kTestParam = {std::make_tuple(3, "alternating_signs"), std::make_tuple(5, "mixed_signs"),
                                            std::make_tuple(7, "same_signs")};

const auto kTestTasksList = std::tuple_cat(
    ppc::util::AddFuncTask<KazenovaAVecChangeSignMPI, InType>(kTestParam, PPC_SETTINGS_kazenova_a_vec_change_sign),
    ppc::util::AddFuncTask<KazenovaAVecChangeSignSEQ, InType>(kTestParam, PPC_SETTINGS_kazenova_a_vec_change_sign));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName = KazenovaAVecChangeSignFuncTests::PrintFuncTestName<KazenovaAVecChangeSignFuncTests>;

INSTANTIATE_TEST_SUITE_P(VecChangeSignTests, KazenovaAVecChangeSignFuncTests, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace kazenova_a_vec_change_sign
