#include <gtest/gtest.h>
#include <mpi.h>

#include <random>

#include "kazennova_a_convex_hull/common/include/common.hpp"
#include "kazennova_a_convex_hull/mpi/include/ops_mpi.hpp"
#include "kazennova_a_convex_hull/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace kazennova_a_convex_hull {

class ExampleRunPerfTestKazennovaA : public ppc::util::BaseRunPerfTests<InType, OutType> {
  const int kNumPoints_ = 1000;
  InType input_data_;

  void SetUp() override {
    input_data_.clear();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 999);

    for (int i = 0; i < kNumPoints_; ++i) {
      auto x = static_cast<double>(distrib(gen));
      auto y = static_cast<double>(distrib(gen));
      input_data_.emplace_back(x, y);
    }
  }

  bool CheckTestOutputData(OutType &output_data) final {
    int initialized = 0;
    MPI_Initialized(&initialized);
    if (initialized != 0) {
      int world_rank = 0;
      MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
      if (world_rank != 0) {
        return true;
      }
    }
    return !output_data.empty();
  }

  InType GetTestInputData() final {
    return input_data_;
  }
};

TEST_P(ExampleRunPerfTestKazennovaA, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks = ppc::util::MakeAllPerfTasks<InType, KazennovaAConvexHullMPI, KazennovaAConvexHullSEQ>(
    PPC_SETTINGS_example_processes_3);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = ExampleRunPerfTestKazennovaA::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, ExampleRunPerfTestKazennovaA, kGtestValues, kPerfTestName);

}  // namespace kazennova_a_convex_hull
