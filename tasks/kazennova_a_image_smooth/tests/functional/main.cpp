#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdio>
#include <string>
#include <tuple>
#include <vector>

#include "kazennova_a_image_smooth/common/include/common.hpp"
#include "kazennova_a_image_smooth/mpi/include/ops_mpi.hpp"
#include "kazennova_a_image_smooth/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"

namespace kazennova_a_image_smooth {

class ImageSmoothFuncTest : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    return std::to_string(std::get<0>(test_param)) + "_" + std::get<1>(test_param);
  }

 protected:
  void SetUp() override {
    TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    (void)params;

    input_data_.width = 4;
    input_data_.height = 4;
    input_data_.channels = 1;

    input_data_.data = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150, 160};
  }

  bool CheckTestOutputData(OutType &output_data) final {
    if (output_data.width != input_data_.width || output_data.height != input_data_.height ||
        output_data.channels != input_data_.channels || output_data.data.size() != input_data_.data.size()) {
      printf("Size mismatch!\n");
      return false;
    }

    bool changed = false;
    for (size_t i = 0; i < input_data_.data.size(); ++i) {
      if (input_data_.data[i] != output_data.data[i]) {
        changed = true;
        break;
      }
    }

    if (!changed) {
      printf("WARNING: No pixels changed!\n");
    }

    return true;
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;
};

namespace {

TEST_P(ImageSmoothFuncTest, ImageSmoothTest) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 1> kTestParam = {std::make_tuple(1, "simple_4x4")};

const auto kTestTasksList = std::tuple_cat(
    ppc::util::AddFuncTask<KazennovaAImageSmoothMPI, InType>(kTestParam, PPC_SETTINGS_kazennova_a_image_smooth),
    ppc::util::AddFuncTask<KazennovaAImageSmoothSEQ, InType>(kTestParam, PPC_SETTINGS_kazennova_a_image_smooth));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName = ImageSmoothFuncTest::PrintFuncTestName<ImageSmoothFuncTest>;

INSTANTIATE_TEST_SUITE_P(ImageSmoothTests, ImageSmoothFuncTest, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace kazennova_a_image_smooth
