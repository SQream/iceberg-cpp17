// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#pragma once

#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

// C++17 compatibility layer for C++20/23 features

namespace iceberg {
namespace cpp17_compat {

// std::span replacement for C++17
template <typename T>
class span {
 public:
  using element_type = T;
  using value_type = typename std::remove_cv<T>::type;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using pointer = T*;
  using const_pointer = const T*;
  using reference = T&;
  using const_reference = const T&;
  using iterator = T*;
  using const_iterator = const T*;

  constexpr span() noexcept : data_(nullptr), size_(0) {}

  constexpr span(pointer ptr, size_type count) : data_(ptr), size_(count) {}

  constexpr span(pointer first, pointer last) : data_(first), size_(last - first) {}

  template <size_t N>
  constexpr span(T (&arr)[N]) noexcept : data_(arr), size_(N) {}

  template <typename Container>
  constexpr span(Container& cont) : data_(cont.data()), size_(cont.size()) {}

  template <typename Container>
  constexpr span(const Container& cont) : data_(cont.data()), size_(cont.size()) {}

  constexpr iterator begin() const noexcept { return data_; }
  constexpr iterator end() const noexcept { return data_ + size_; }
  constexpr const_iterator cbegin() const noexcept { return data_; }
  constexpr const_iterator cend() const noexcept { return data_ + size_; }

  constexpr reference operator[](size_type idx) const { return data_[idx]; }
  constexpr reference front() const { return data_[0]; }
  constexpr reference back() const { return data_[size_ - 1]; }
  constexpr pointer data() const noexcept { return data_; }

  constexpr size_type size() const noexcept { return size_; }
  constexpr size_type size_bytes() const noexcept { return size_ * sizeof(element_type); }
  constexpr bool empty() const noexcept { return size_ == 0; }

  constexpr span<element_type> subspan(size_type offset, size_type count = std::size_t(-1)) const {
    return span(data_ + offset, count == std::size_t(-1) ? size_ - offset : count);
  }

 private:
  pointer data_;
  size_type size_;
};

// std::expected replacement for C++17
template <class E>
class unexpected {
 public:
  template <class Err>
  constexpr explicit unexpected(Err&& e) : error_(std::forward<Err>(e)) {}
  
  constexpr unexpected(const E& e) : error_(e) {}
  constexpr unexpected(E&& e) : error_(std::move(e)) {}

  constexpr const E& error() const& { return error_; }
  constexpr E& error() & { return error_; }
  constexpr const E&& error() const&& { return std::move(error_); }
  constexpr E&& error() && { return std::move(error_); }

 private:
  E error_;
};

template <class T, class E>
class expected {
 public:
  using value_type = T;
  using error_type = E;
  using unexpected_type = unexpected<E>;

  constexpr expected() : has_value_(true) {
    new (&value_) T();
  }

  template <class U, typename = typename std::enable_if_t<!std::is_same<typename std::decay<U>::type, expected>::value && std::is_constructible<T, U>::value>>
  constexpr expected(U&& val) : has_value_(true) {
    new (&value_) T(std::forward<U>(val));
  }

  constexpr expected(const unexpected_type& e) : has_value_(false) {
    new (&error_) E(e.error());
  }

  constexpr expected(unexpected_type&& e) : has_value_(false) {
    new (&error_) E(std::move(e.error()));
  }

  constexpr expected(const expected& other) : has_value_(other.has_value_) {
    if (has_value_) {
      new (&value_) T(other.value_);
    } else {
      new (&error_) E(other.error_);
    }
  }

  constexpr expected(expected&& other) noexcept : has_value_(other.has_value_) {
    if (has_value_) {
      new (&value_) T(std::move(other.value_));
    } else {
      new (&error_) E(std::move(other.error_));
    }
  }

  ~expected() {
    if (has_value_) {
      value_.~T();
    } else {
      error_.~E();
    }
  }

  constexpr expected& operator=(const expected& other) {
    if (this != &other) {
      if (has_value_) {
        value_.~T();
      } else {
        error_.~E();
      }
      
      has_value_ = other.has_value_;
      if (has_value_) {
        new (&value_) T(other.value_);
      } else {
        new (&error_) E(other.error_);
      }
    }
    return *this;
  }

  constexpr expected& operator=(expected&& other) noexcept {
    if (this != &other) {
      if (has_value_) {
        value_.~T();
      } else {
        error_.~E();
      }
      
      has_value_ = other.has_value_;
      if (has_value_) {
        new (&value_) T(std::move(other.value_));
      } else {
        new (&error_) E(std::move(other.error_));
      }
    }
    return *this;
  }

  constexpr bool has_value() const noexcept { return has_value_; }
  constexpr explicit operator bool() const noexcept { return has_value_; }

  constexpr T& value() & {
    if (!has_value_) {
      throw std::runtime_error("expected has no value");
    }
    return value_;
  }

  constexpr const T& value() const & {
    if (!has_value_) {
      throw std::runtime_error("expected has no value");
    }
    return value_;
  }

  constexpr T&& value() && {
    if (!has_value_) {
      throw std::runtime_error("expected has no value");
    }
    return std::move(value_);
  }

  constexpr const T&& value() const && {
    if (!has_value_) {
      throw std::runtime_error("expected has no value");
    }
    return std::move(value_);
  }

  constexpr E& error() & { return error_; }
  constexpr const E& error() const & { return error_; }
  constexpr E&& error() && { return std::move(error_); }
  constexpr const E&& error() const && { return std::move(error_); }

  template <class U>
  constexpr T value_or(U&& default_value) const & {
    return has_value_ ? value_ : static_cast<T>(std::forward<U>(default_value));
  }

  template <class U>
  constexpr T value_or(U&& default_value) && {
    return has_value_ ? std::move(value_) : static_cast<T>(std::forward<U>(default_value));
  }

  constexpr T* operator->() { return &value_; }
  constexpr const T* operator->() const { return &value_; }
  constexpr T& operator*() & { return value_; }
  constexpr const T& operator*() const & { return value_; }
  constexpr T&& operator*() && { return std::move(value_); }
  constexpr const T&& operator*() const && { return std::move(value_); }

  // Monadic operations (C++23 feature, but useful for compatibility)
  template <class F>
  constexpr auto and_then(F&& f) const & -> decltype(f(std::declval<const T&>())) {
    if (has_value_) {
      return f(value_);
    } else {
      return unexpected_type(error_);
    }
  }

  template <class F>
  constexpr auto and_then(F&& f) && -> decltype(f(std::declval<T&&>())) {
    if (has_value_) {
      return f(std::move(value_));
    } else {
      return unexpected_type(std::move(error_));
    }
  }

 private:
  bool has_value_;
  union {
    T value_;
    E error_;
  };
};

// Specialization for void
template <class E>
class expected<void, E> {
 public:
  using value_type = void;
  using error_type = E;
  using unexpected_type = unexpected<E>;

  constexpr expected() : has_value_(true) {}

  constexpr expected(const unexpected_type& e) : has_value_(false), error_(e.error()) {}

  constexpr expected(unexpected_type&& e) : has_value_(false), error_(std::move(e.error())) {}

  constexpr bool has_value() const noexcept { return has_value_; }
  constexpr explicit operator bool() const noexcept { return has_value_; }

  constexpr void value() const {
    if (!has_value_) {
      throw std::runtime_error("expected has no value");
    }
  }

  constexpr E& error() & { return error_; }
  constexpr const E& error() const & { return error_; }
  constexpr E&& error() && { return std::move(error_); }
  constexpr const E&& error() const && { return std::move(error_); }

  // Monadic operations (C++23 feature, but useful for compatibility)
  template <class F>
  constexpr auto and_then(F&& f) const & -> decltype(f()) {
    if (has_value_) {
      return f();
    } else {
      return unexpected_type(error_);
    }
  }

  template <class F>
  constexpr auto and_then(F&& f) && -> decltype(f()) {
    if (has_value_) {
      return f();
    } else {
      return unexpected_type(std::move(error_));
    }
  }

 private:
  bool has_value_;
  E error_;
};

// std::unreachable replacement
[[noreturn]] inline void unreachable() {
  __builtin_unreachable();
}

// Comparison operators helpers (C++17 doesn't have automatic generation)
struct strong_ordering {
  signed char value;
  
  static const strong_ordering less;
  static const strong_ordering equal;
  static const strong_ordering greater;
  
  constexpr strong_ordering(signed char v) : value(v) {}
  
  constexpr bool operator==(const strong_ordering& other) const { return value == other.value; }
  constexpr bool operator!=(const strong_ordering& other) const { return value != other.value; }
};

inline constexpr strong_ordering strong_ordering::less{-1};
inline constexpr strong_ordering strong_ordering::equal{0};
inline constexpr strong_ordering strong_ordering::greater{1};

struct partial_ordering {
  signed char value;
  
  static const partial_ordering less;
  static const partial_ordering equivalent;
  static const partial_ordering greater;
  static const partial_ordering unordered;
  
  constexpr partial_ordering(signed char v) : value(v) {}
  
  constexpr bool operator==(const partial_ordering& other) const { return value == other.value; }
  constexpr bool operator!=(const partial_ordering& other) const { return value != other.value; }
};

inline constexpr partial_ordering partial_ordering::less{-1};
inline constexpr partial_ordering partial_ordering::equivalent{0};
inline constexpr partial_ordering partial_ordering::greater{1};
inline constexpr partial_ordering partial_ordering::unordered{-127};

}  // namespace cpp17_compat
}  // namespace iceberg

// Convenience aliases in std namespace for easier migration
// Note: avoiding std::unexpected due to name collision with C++ exception function
namespace std {
  template<typename T>
  using span = iceberg::cpp17_compat::span<T>;
  
  template<typename T, typename E>
  using expected = iceberg::cpp17_compat::expected<T, E>;
  
  using strong_ordering = iceberg::cpp17_compat::strong_ordering;
  using partial_ordering = iceberg::cpp17_compat::partial_ordering;
}

// Convenience namespace for compatibility
namespace compat {
  template<typename T>
  using span = iceberg::cpp17_compat::span<T>;
  
  template<typename T, typename E>
  using expected = iceberg::cpp17_compat::expected<T, E>;
  
  template<typename E>
  using unexpected = iceberg::cpp17_compat::unexpected<E>;
  
  using strong_ordering = iceberg::cpp17_compat::strong_ordering;
  using partial_ordering = iceberg::cpp17_compat::partial_ordering;
  
  inline void unreachable() {
    iceberg::cpp17_compat::unreachable();
  }
}

// Handle std::variant compatibility - GCC 12.3 has bugs with variant
// For now, try to use the standard variant but provide fallback
#if __cplusplus >= 201703L && !defined(__GNUC__) || (__GNUC__ > 12) || (__GNUC__ == 12 && __GNUC_MINOR__ >= 4)
// Use standard variant for newer compilers or non-GCC
#include <variant>
#else
// For problematic GCC versions, we'll need to work around std::variant issues
// For now, just include it and hope for the best - we may need boost::variant later
#include <variant>
#endif
