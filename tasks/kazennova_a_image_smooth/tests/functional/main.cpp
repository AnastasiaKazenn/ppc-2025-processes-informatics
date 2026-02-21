#include <gtest/gtest.h>
#include <mpi.h>

#include <array>
#include <cstddef>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include "kazennova_a_image_smooth/common/include/common.hpp"
#include "kazennova_a_image_smooth/mpi/include/ops_mpi.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

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
    int world_rank = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    // Только процесс 0 проверяет данные
    if (world_rank != 0) {
      return true;
    }

    std::cout << "\n=== CheckTestOutputData (Process 0) ===\n";
    std::cout << "Input data:  ";
    for (size_t i = 0; i < input_data_.data.size(); ++i) {
      std::cout << static_cast<int>(input_data_.data[i]) << " ";
    }
    std::cout << "\n";

    std::cout << "Output data: ";
    for (size_t i = 0; i < output_data.data.size(); ++i) {
      std::cout << static_cast<int>(output_data.data[i]) << " ";
    }
    std::cout << "\n";

    if (output_data.width != input_data_.width || output_data.height != input_data_.height ||
        output_data.channels != input_data_.channels || output_data.data.size() != input_data_.data.size()) {
      std::cout << "ERROR: Output data size mismatch!\n";
      return false;
    }

    bool changed = false;
    for (size_t i = 0; i < input_data_.data.size(); ++i) {
      if (input_data_.data[i] != output_data.data[i]) {
        if (!changed) {
          std::cout << "Changes detected:\n";
          changed = true;
        }
        std::cout << "  Position " << i << ": " << static_cast<int>(input_data_.data[i]) << " -> "
                  << static_cast<int>(output_data.data[i]) << "\n";
      }
    }

    if (!changed) {
      std::cout << "ERROR: Output data is identical to input data!\n";
      return false;
    }

    std::cout << "SUCCESS: Output data changed from input\n";
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
  int initialized = 0;
  MPI_Initialized(&initialized);
  if (initialized == 0) {
    MPI_Init(nullptr, nullptr);
  }

  ExecuteTest(GetParam());

  MPI_Barrier(MPI_COMM_WORLD);
}

const std::array<TestType, 1> kTestParam = {std::make_tuple(1, "simple_4x4")};

const auto kTestTasksList = std::tuple_cat(
    ppc::util::AddFuncTask<KazennovaAImageSmoothMPI, InType>(kTestParam, PPC_SETTINGS_kazennova_a_image_smooth));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName = ImageSmoothFuncTest::PrintFuncTestName<ImageSmoothFuncTest>;

INSTANTIATE_TEST_SUITE_P(ImageSmoothTests, ImageSmoothFuncTest, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace kazennova_a_image_smooth
