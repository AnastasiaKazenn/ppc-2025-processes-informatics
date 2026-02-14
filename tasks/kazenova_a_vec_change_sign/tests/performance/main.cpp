#include <gtest/gtest.h>

#include <cstddef>
#include <utility>

#include "kazenova_a_vec_change_sign/common/include/common.hpp"
#include "kazenova_a_vec_change_sign/mpi/include/ops_mpi.hpp"
#include "kazenova_a_vec_change_sign/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace kazenova_a_vec_change_sign {

class KazenovaAVecChangeSignPerfTest : public ppc::util::BaseRunPerfTests<InType, OutType> {
 protected:
  void SetUp() override {
    int vector_size = 100000000;
    input_data_.resize(vector_size);
    for (int i = 0; i < vector_size; i++) {
      input_data_[i] = (i % 2 == 0) ? 1 : -1;
    }
  }
  bool CheckTestOutputData(OutType &output_data) final {
    size_t max_possible = input_data_.size() - 1;
    return output_data >= 0 && std::cmp_less_equal(output_data, max_possible);
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;
};

TEST_P(KazenovaAVecChangeSignPerfTest, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks = ppc::util::MakeAllPerfTasks<InType, KazenovaAVecChangeSignMPI, KazenovaAVecChangeSignSEQ>(
    PPC_SETTINGS_kazenova_a_vec_change_sign);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = KazenovaAVecChangeSignPerfTest::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, KazenovaAVecChangeSignPerfTest, kGtestValues, kPerfTestName);

}  // namespace kazenova_a_vec_change_sign
