#pragma once

#include <cstdint>
#include <string>
#include <tuple>
#include <vector>

#include "task/include/task.hpp"

namespace kazennova_a_image_smooth {

struct Image {
  std::vector<uint8_t> data;
  int width = 0;
  int height = 0;
  int channels = 0;
};

using InType = Image;
using OutType = Image;
using TestType = std::tuple<int, std::string>;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace kazennova_a_image_smooth
