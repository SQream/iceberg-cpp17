#pragma once

#include <iostream>

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

// C++23 print/println compatibility layer for C++17/C++20
#if __cplusplus >= 202302L  // C++23 or later
#include <print>
namespace compat {
  using std::print;
  using std::println;
}
#else
namespace compat {
  // print to stdout
  template<typename... Args>
  void print(fmt::format_string<Args...> fmt, Args&&... args) {
    std::cout << fmt::format(fmt, std::forward<Args>(args)...);
  }

  // print to file
  template<typename... Args>
  void print(std::FILE* f, fmt::format_string<Args...> fmt, Args&&... args) {
    fmt::print(f, fmt, std::forward<Args>(args)...);
  }

  // println to stdout
  template<typename... Args>
  void println(fmt::format_string<Args...> fmt, Args&&... args) {
    std::cout << fmt::format(fmt, std::forward<Args>(args)...) << '\n';
  }

  // println to file
  template<typename... Args>
  void println(std::FILE* f, fmt::format_string<Args...> fmt, Args&&... args) {
    fmt::print(f, fmt, std::forward<Args>(args)...);
    std::fputc('\n', f);
  }
}
#endif
