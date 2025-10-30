/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
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
/// A specialization of formatter for Formattable objects.  This header
/// is separate from iceberg/util/formattable.h so that the latter (which is
/// meant to be included widely) does not leak format libraries unnecessarily into
/// other headers.  You must include this header to format a Formattable.

#include <type_traits>
#include <string_view>
#include "iceberg/format_compat.h"
#include "iceberg/util/formattable.h"

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

// SFINAE helper to detect Formattable types
template <typename T>
struct is_formattable : std::is_base_of<iceberg::util::Formattable, T> {};

template <typename T>
constexpr bool is_formattable_v = is_formattable<T>::value;

// Specializations for fmt library (C++17 mode)
namespace fmt {

// Formatter for Formattable-derived types
template <typename T>
struct formatter<T, typename std::enable_if_t<is_formattable_v<T>, char>> : formatter<std::string_view> {
  template <typename FormatContext>
  auto format(const T& obj, FormatContext& ctx) -> decltype(ctx.out()) {
    return formatter<std::string_view>::format(obj.ToString(), ctx);
  }
};

// Formatter for any type that has a ToString function
template <typename T>
struct formatter<T, typename std::enable_if_t<has_ToString_v<T> && !is_formattable_v<T>, char>> : formatter<std::string_view> {
  template <typename FormatContext>
  auto format(const T& value, FormatContext& ctx) -> decltype(ctx.out()) {
    return formatter<std::string_view>::format(ToString(value), ctx);
  }
};

}  // namespace fmt
