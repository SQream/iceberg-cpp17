/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more conttemplate <typename T>
struct fmt::formatter<T, typename std::enable_if_t<has_ToString_v<T>, char>> : fmt::formatter<std::string_view> {butor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#pragma once

/// \file iceberg/util/formatter.h
/// A specialization of std::formatter for Formattable objects.  This header
/// is separate from iceberg/util/formattable.h so that the latter (which is
/// meant to be included widely) does not leak <format> unnecessarily into
/// other headers.  You must include this header to format a Formattable.

#include <type_traits>
#include "../format_compat.h"
#include <string_view>

#include "iceberg/util/formattable.h"

// std::formatter is only available in C++20, disable for C++17
#if __cplusplus >= 202002L

/// \brief Make all classes deriving from iceberg::util::Formattable
///   formattable with std::format.
template <typename Derived, typename = typename std::enable_if_t<std::is_base_of_v<iceberg::util::Formattable, Derived>>>
struct std::formatter<Derived> : std::formatter<std::string_view> {
  template <class FormatContext>
  auto format(const iceberg::util::Formattable& obj, FormatContext& ctx) const {
    return std::formatter<string_view>::format(obj.ToString(), ctx);
  }
};

/// \brief std::formatter specialization for any type that has a ToString function
template <typename T>
  requires requires(const T& t) {
    { ToString(t) } -> std::convertible_to<std::string_view>;
  }
struct std::formatter<T> : std::formatter<std::string_view> {
  template <class FormatContext>
  auto format(const T& value, FormatContext& ctx) const {
    return std::formatter<std::string_view>::format(ToString(value), ctx);
  }
};

#else // C++17 mode - provide fmt::formatter specializations

// SFINAE helper to detect ToString function
template <typename T>
struct has_ToString {
  template <typename U>
  static auto test(int) -> decltype(ToString(std::declval<const U&>()), std::true_type{});
  template <typename>
  static std::false_type test(...);
  using type = decltype(test<T>(0));
  static constexpr bool value = type::value;
};

template <typename T>
constexpr bool has_ToString_v = has_ToString<T>::value;

#endif // C++20 or later


// For C++17 mode, provide fmt::formatter specializations outside any namespace
#if __cplusplus < 202002L

template <typename T>
struct fmt::formatter<T, typename std::enable_if_t<has_ToString_v<T>, char>> : fmt::formatter<std::string_view> {
  template <typename FormatContext>
  auto format(const T& value, FormatContext& ctx) -> decltype(ctx.out()) {
    return fmt::formatter<std::string_view>::format(ToString(value), ctx);
  }
};

#endif
