#include "kazennova_a_image_smooth/seq/include/ops_seq.hpp"

#include <cmath>
#include <vector>

#include "kazennova_a_image_smooth/common/include/common.hpp"

namespace kazennova_a_image_smooth {

const float kernel[3][3] = {
    {1.0f / 16, 2.0f / 16, 1.0f / 16}, {2.0f / 16, 4.0f / 16, 2.0f / 16}, {1.0f / 16, 2.0f / 16, 1.0f / 16}};

KazennovaAImageSmoothSEQ::KazennovaAImageSmoothSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = in;
}

bool KazennovaAImageSmoothSEQ::ValidationImpl() {
  const auto &in = GetInput();
  return in.width > 0 && in.height > 0 && !in.data.empty() && (in.channels == 1 || in.channels == 3);
}

bool KazennovaAImageSmoothSEQ::PreProcessingImpl() {
  GetOutput().data.resize(GetInput().data.size());
  return true;
}

uint8_t KazennovaAImageSmoothSEQ::ApplyKernelToPixel(int x, int y, int c) {
  const auto &in = GetInput();
  float sum = 0.0f;

  for (int ky = -1; ky <= 1; ++ky) {
    for (int kx = -1; kx <= 1; ++kx) {
      int nx = x + kx;
      int ny = y + ky;

      if (nx < 0) {
        nx = 0;
      }
      if (nx >= in.width) {
        nx = in.width - 1;
      }
      if (ny < 0) {
        ny = 0;
      }
      if (ny >= in.height) {
        ny = in.height - 1;
      }

      int idx = (ny * in.width + nx) * in.channels + c;
      sum += in.data[idx] * kernel[ky + 1][kx + 1];
    }
  }

  return static_cast<uint8_t>(std::round(sum));
}

bool KazennovaAImageSmoothSEQ::RunImpl() {
  const auto &in = GetInput();
  auto &out = GetOutput();

  out.width = in.width;
  out.height = in.height;
  out.channels = in.channels;
  out.data.resize(in.data.size());

  for (int y = 0; y < in.height; ++y) {
    for (int x = 0; x < in.width; ++x) {
      for (int c = 0; c < in.channels; ++c) {
        int idx = (y * in.width + x) * in.channels + c;
        out.data[idx] = ApplyKernelToPixel(x, y, c);
      }
    }
  }

  return true;
}

bool KazennovaAImageSmoothSEQ::PostProcessingImpl() {
  return !GetOutput().data.empty();
}

}  // namespace kazennova_a_image_smooth
