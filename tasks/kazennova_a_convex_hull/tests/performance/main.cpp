#include <gtest/gtest.h>
#include <mpi.h>

#include "kazennova_a_convex_hull/common/include/common.hpp"
#include "kazennova_a_convex_hull/mpi/include/ops_mpi.hpp"
#include "kazennova_a_convex_hull/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace kazennova_a_convex_hull {

class ExampleRunPerfTestKazennovaA : public ppc::util::BaseRunPerfTests<InType, OutType> {
  const int kNumPoints_ = 1000;
  InType input_data_{};

  void SetUp() override {
    // Создаем большой тестовый набор точек
    input_data_.clear();
    for (int i = 0; i < kNumPoints_; ++i) {
      double x = rand() % 1000;
      double y = rand() % 1000;
      input_data_.push_back(Point(x, y));
    }
  }

  bool CheckTestOutputData(OutType &output_data) final {
    int world_rank = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    if (world_rank == 0) {
      return !output_data.empty();
    }
    return true;
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
