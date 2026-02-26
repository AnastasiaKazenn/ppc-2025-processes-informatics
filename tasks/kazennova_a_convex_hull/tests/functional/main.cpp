#include <gtest/gtest.h>
#include <mpi.h>
#include <stb/stb_image.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "kazennova_a_convex_hull/common/include/common.hpp"
#include "kazennova_a_convex_hull/mpi/include/ops_mpi.hpp"
#include "kazennova_a_convex_hull/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace kazennova_a_convex_hull {

class KazennovaARunFuncTestsProcesses3 : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    return std::to_string(std::get<0>(test_param)) + "_" + std::get<1>(test_param);
  }

 protected:
  void SetUp() override {
    int width = -1;
    int height = -1;
    int channels = -1;
    std::vector<uint8_t> img;

    {
      std::string abs_path = ppc::util::GetAbsoluteTaskPath(PPC_ID_kazennova_a_convex_hull, "pic.jpg");
      auto *data = stbi_load(abs_path.c_str(), &width, &height, &channels, STBI_rgb);
      if (data == nullptr) {
        throw std::runtime_error("Failed to load image: " + std::string(stbi_failure_reason()));
      }
      channels = STBI_rgb;
      img = std::vector<uint8_t>(data, data + (static_cast<ptrdiff_t>(width * height * channels)));
      stbi_image_free(data);
      if (std::cmp_not_equal(width, height)) {
        throw std::runtime_error("width != height");
      }
    }

    TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());

    int num_points = std::get<0>(params);
    input_data_.clear();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib_x(0, width - 1);
    std::uniform_int_distribution<> distrib_y(0, height - 1);

    for (int i = 0; i < num_points; ++i) {
      auto x = static_cast<double>(distrib_x(gen));
      auto y = static_cast<double>(distrib_y(gen));
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

 private:
  InType input_data_;
};

namespace {

TEST_P(KazennovaARunFuncTestsProcesses3, ConvexHullFromPic) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 3> kTestParam = {std::make_tuple(10, "10_points"), std::make_tuple(50, "50_points"),
                                            std::make_tuple(100, "100_points")};

const auto kTestTasksList = std::tuple_cat(
    ppc::util::AddFuncTask<KazennovaAConvexHullMPI, InType>(kTestParam, PPC_SETTINGS_example_processes_3),
    ppc::util::AddFuncTask<KazennovaAConvexHullSEQ, InType>(kTestParam, PPC_SETTINGS_example_processes_3));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName = KazennovaARunFuncTestsProcesses3::PrintFuncTestName<KazennovaARunFuncTestsProcesses3>;

INSTANTIATE_TEST_SUITE_P(ConvexHullTests, KazennovaARunFuncTestsProcesses3, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace kazennova_a_convex_hull
