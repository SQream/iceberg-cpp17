#pragma once

#if __cplusplus >= 202002L  // C++20 or later
#include <format>
namespace compat {
  using std::format;
  using std::format_to;
}
#else  // C++17 - use fmt library
#include <fmt/format.h>
namespace compat {
  using fmt::format;
  using fmt::format_to;
}
#endif
