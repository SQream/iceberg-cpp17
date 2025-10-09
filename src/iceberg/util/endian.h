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

#include <cstdint>
#include <cstring>
#include <type_traits>

/// \file iceberg/util/endian.h
/// \brief Endianness conversion utilities

namespace iceberg {

// C++17 compatible bit_cast replacement
template <class To, class From>
typename std::enable_if_t<
    sizeof(To) == sizeof(From) &&
    std::is_trivially_copyable_v<From> &&
    std::is_trivially_copyable_v<To>,
    To>
bit_cast(const From& src) noexcept {
  static_assert(std::is_trivially_constructible_v<To>,
                "This implementation requires To to be trivially constructible");
  To dst;
  std::memcpy(&dst, &src, sizeof(To));
  return dst;
}

namespace detail {

// Compile-time endianness detection using a constexpr technique that works in C++17
constexpr bool is_little_endian_constexpr() {
  return (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__);
}

// Manual byteswap implementations
template <typename T>
constexpr typename std::enable_if_t<sizeof(T) == 1, T> byteswap_impl(T value) {
  return value;
}

template <typename T>
constexpr typename std::enable_if_t<sizeof(T) == 2, T> byteswap_impl(T value) {
  uint16_t v = static_cast<uint16_t>(value);
  v = ((v & 0x00FFU) << 8) | ((v & 0xFF00U) >> 8);
  return static_cast<T>(v);
}

template <typename T>
constexpr typename std::enable_if_t<sizeof(T) == 4, T> byteswap_impl(T value) {
  uint32_t v = static_cast<uint32_t>(value);
  v = ((v & 0x000000FFU) << 24) | ((v & 0x0000FF00U) << 8) |
      ((v & 0x00FF0000U) >> 8) | ((v & 0xFF000000U) >> 24);
  return static_cast<T>(v);
}

template <typename T>
constexpr typename std::enable_if_t<sizeof(T) == 8, T> byteswap_impl(T value) {
  uint64_t v = static_cast<uint64_t>(value);
  v = ((v & 0x00000000000000FFULL) << 56) | ((v & 0x000000000000FF00ULL) << 40) |
      ((v & 0x0000000000FF0000ULL) << 24) | ((v & 0x00000000FF000000ULL) << 8) |
      ((v & 0x000000FF00000000ULL) >> 8) | ((v & 0x0000FF0000000000ULL) >> 24) |
      ((v & 0x00FF000000000000ULL) >> 40) | ((v & 0xFF00000000000000ULL) >> 56);
  return static_cast<T>(v);
}
} // namespace detail

/// \brief Byte-swap a value. For floating-point types, only support 32-bit and 64-bit
/// floats.
template <typename T>
constexpr typename std::enable_if_t<std::is_arithmetic_v<T>, T>
ByteSwap(T value) {
  if constexpr (sizeof(T) <= 1) {
    return value;
  } else if constexpr (std::is_integral_v<T>) {
    return detail::byteswap_impl(value);
  } else if constexpr (std::is_floating_point_v<T>) {
    if constexpr (sizeof(T) == sizeof(uint16_t)) {
      return bit_cast<T>(detail::byteswap_impl(bit_cast<uint16_t>(value)));
    } else if constexpr (sizeof(T) == sizeof(uint32_t)) {
      return bit_cast<T>(detail::byteswap_impl(bit_cast<uint32_t>(value)));
    } else if constexpr (sizeof(T) == sizeof(uint64_t)) {
      return bit_cast<T>(detail::byteswap_impl(bit_cast<uint64_t>(value)));
    } else {
      static_assert(sizeof(T) == 0,
                    "Unsupported floating-point size for endian conversion.");
    }
  }
}

/// \brief Convert a value to little-endian format.
template <typename T>
constexpr typename std::enable_if_t<std::is_arithmetic_v<T>, T>
ToLittleEndian(T value) {
  if constexpr (detail::is_little_endian_constexpr()) {
    return value;
  } else {
    return ByteSwap(value);
  }
}

/// \brief Convert a value from little-endian format.
template <typename T>
constexpr typename std::enable_if_t<std::is_arithmetic_v<T>, T>
FromLittleEndian(T value) {
  if constexpr (detail::is_little_endian_constexpr()) {
    return value;
  } else {
    return ByteSwap(value);
  }
}

/// \brief Convert a value to big-endian format.
template <typename T>
constexpr typename std::enable_if_t<std::is_arithmetic_v<T>, T>
ToBigEndian(T value) {
  if constexpr (!detail::is_little_endian_constexpr()) {
    return value;
  } else {
    return ByteSwap(value);
  }
}

/// \brief Convert a value from big-endian format.
template <typename T>
constexpr typename std::enable_if_t<std::is_arithmetic_v<T>, T>
FromBigEndian(T value) {
  if constexpr (!detail::is_little_endian_constexpr()) {
    return value;
  } else {
    return ByteSwap(value);
  }
}

}  // namespace iceberg
