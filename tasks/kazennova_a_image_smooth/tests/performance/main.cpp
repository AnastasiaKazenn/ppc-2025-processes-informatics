#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <vector>

#include "kazennova_a_image_smooth/common/include/common.hpp"
#include "kazennova_a_image_smooth/mpi/include/ops_mpi.hpp"
#include "kazennova_a_image_smooth/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace kazennova_a_image_smooth {

class ImageSmoothPerfTest : public ppc::util::BaseRunPerfTests<InType, OutType> {
 protected:
  void SetUp() override {
    int size = 4000;
    input_data_.width = size;
    input_data_.height = size;
    input_data_.channels = 1;

    input_data_.data.resize(static_cast<size_t>(size) * static_cast<size_t>(size) *
                            static_cast<size_t>(input_data_.channels));

    for (int i = 0; i < size * size; ++i) {
      input_data_.data[i] = static_cast<uint8_t>(i % 256);
    }
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return output_data.width == input_data_.width && output_data.height == input_data_.height &&
           output_data.channels == input_data_.channels && output_data.data.size() == input_data_.data.size();
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;
};

TEST_P(ImageSmoothPerfTest, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks = ppc::util::MakeAllPerfTasks<InType, KazennovaAImageSmoothMPI, KazennovaAImageSmoothSEQ>(
    PPC_SETTINGS_kazennova_a_image_smooth);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = ImageSmoothPerfTest::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, ImageSmoothPerfTest, kGtestValues, kPerfTestName);

}  // namespace kazennova_a_image_smooth
