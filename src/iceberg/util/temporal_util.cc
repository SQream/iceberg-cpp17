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

#include "iceberg/util/temporal_util.h"

#include <chrono>
#include <cstdint>
#include <utility>

#include "iceberg/expression/literal.h"

// C++17 compatibility for chrono features
namespace compat {
  using namespace std::chrono;
  
  // Simple year/month/day structure for C++17
  struct year_month_day {
    int year_val;
    int month_val;
    int day_val;
    
    year_month_day(int y, int m, int d) : year_val(y), month_val(m), day_val(d) {}
    
    constexpr int year() const { return year_val; }
    constexpr unsigned month() const { return month_val; }
    constexpr unsigned day() const { return day_val; }
  };
  
  using days = std::chrono::duration<int, std::ratio<86400>>;
  using hours = std::chrono::hours;
  using microseconds = std::chrono::microseconds;
  
  // Convert days since epoch to year/month/day
  inline year_month_day DateToYmd(int32_t days_since_epoch) {
    // Unix epoch: 1970-01-01
    // Simple approximation - can be made more accurate if needed
    const int days_per_year = 365;
    const int days_per_4_years = days_per_year * 4 + 1; // accounting for leap years
    
    int year = 1970;
    int remaining_days = days_since_epoch;
    
    // Handle years
    while (remaining_days >= days_per_4_years) {
      year += 4;
      remaining_days -= days_per_4_years;
    }
    while (remaining_days >= days_per_year) {
      year += 1;
      remaining_days -= days_per_year;
      // Simple leap year check
      if (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) {
        if (remaining_days >= 0) {
          remaining_days -= 1; // account for leap day
        }
      }
    }
    
    // Handle months (simplified)
    int month = 1;
    int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) {
      days_in_month[1] = 29; // leap year
    }
    
    while (remaining_days >= days_in_month[month - 1]) {
      remaining_days -= days_in_month[month - 1];
      month++;
      if (month > 12) {
        month = 1;
        year++;
        // Recalculate leap year for new year
        days_in_month[1] = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) ? 29 : 28;
      }
    }
    
    return year_month_day(year, month, remaining_days + 1);
  }
  
  // Convert microseconds since epoch to year/month/day
  inline year_month_day TimestampToYmd(int64_t micros_since_epoch) {
    int32_t days_since_epoch = static_cast<int32_t>(micros_since_epoch / (1000000LL * 86400LL));
    return DateToYmd(days_since_epoch);
  }
  
  // Convert timestamp to duration units
  template <typename Duration>
  inline int32_t TimestampToDuration(int64_t micros_since_epoch) {
    if constexpr (std::is_same_v<Duration, days>) {
      return static_cast<int32_t>(micros_since_epoch / (1000000LL * 86400LL));
    } else if constexpr (std::is_same_v<Duration, hours>) {
      return static_cast<int32_t>(micros_since_epoch / (1000000LL * 3600LL));
    } else {
      static_assert(std::is_same_v<Duration, days> || std::is_same_v<Duration, hours>, 
                    "Only days and hours are supported");
    }
  }
}

namespace iceberg {

namespace {

using namespace std::chrono;  // NOLINT

// Use compat implementations for C++17
using compat::year_month_day;
using compat::DateToYmd;
using compat::TimestampToYmd;
using compat::TimestampToDuration;
using compat::days;
using compat::hours;

inline constexpr int32_t MonthsSinceEpoch(const year_month_day& ymd) {
  // Simple calculation: years since 1970 * 12 + month difference
  int years_since_epoch = ymd.year() - 1970;
  return years_since_epoch * 12 + (ymd.month() - 1);
}

template <TypeId type_id>
Result<Literal> ExtractYearImpl(const Literal& literal) {
  compat::unreachable();
  return Literal::Int(0); // Never reached
}

template <>
Result<Literal> ExtractYearImpl<TypeId::kDate>(const Literal& literal) {
  auto value = std::get<int32_t>(literal.value());
  auto ymd = DateToYmd(value);
  return Literal::Int((ymd.year() - kEpochYmd.year()).count());
}

template <>
Result<Literal> ExtractYearImpl<TypeId::kTimestamp>(const Literal& literal) {
  auto value = std::get<int64_t>(literal.value());
  auto ymd = TimestampToYmd(value);
  return Literal::Int((ymd.year() - kEpochYmd.year()).count());
}

template <>
Result<Literal> ExtractYearImpl<TypeId::kTimestampTz>(const Literal& literal) {
  return ExtractYearImpl<TypeId::kTimestamp>(literal);
}

template <TypeId type_id>
Result<Literal> ExtractMonthImpl(const Literal& literal) {
  compat::unreachable();
  return Literal::Int(0); // Never reached
}

template <>
Result<Literal> ExtractMonthImpl<TypeId::kDate>(const Literal& literal) {
  auto value = std::get<int32_t>(literal.value());
  auto ymd = DateToYmd(value);
  return Literal::Int(MonthsSinceEpoch(ymd));
}

template <>
Result<Literal> ExtractMonthImpl<TypeId::kTimestamp>(const Literal& literal) {
  auto value = std::get<int64_t>(literal.value());
  auto ymd = TimestampToYmd(value);
  return Literal::Int(MonthsSinceEpoch(ymd));
}

template <>
Result<Literal> ExtractMonthImpl<TypeId::kTimestampTz>(const Literal& literal) {
  return ExtractMonthImpl<TypeId::kTimestamp>(literal);
}

template <TypeId type_id>
Result<Literal> ExtractDayImpl(const Literal& literal) {
  compat::unreachable();
  return Literal::Int(0); // Never reached
}

template <>
Result<Literal> ExtractDayImpl<TypeId::kDate>(const Literal& literal) {
  return Literal::Int(std::get<int32_t>(literal.value()));
}

template <>
Result<Literal> ExtractDayImpl<TypeId::kTimestamp>(const Literal& literal) {
  auto value = std::get<int64_t>(literal.value());
  return Literal::Int(TimestampToDuration<days>(value));
}

template <>
Result<Literal> ExtractDayImpl<TypeId::kTimestampTz>(const Literal& literal) {
  return ExtractDayImpl<TypeId::kTimestamp>(literal);
}

template <TypeId type_id>
Result<Literal> ExtractHourImpl(const Literal& literal) {
  compat::unreachable();
  return Literal::Int(0); // Never reached
}

template <>
Result<Literal> ExtractHourImpl<TypeId::kTimestamp>(const Literal& literal) {
  auto value = std::get<int64_t>(literal.value());
  return Literal::Int(TimestampToDuration<hours>(value));
}

template <>
Result<Literal> ExtractHourImpl<TypeId::kTimestampTz>(const Literal& literal) {
  return ExtractHourImpl<TypeId::kTimestamp>(literal);
}

}  // namespace

#define DISPATCH_EXTRACT_YEAR(type_id) \
  case type_id:                        \
    return ExtractYearImpl<type_id>(literal);

Result<Literal> TemporalUtils::ExtractYear(const Literal& literal) {
  if (literal.IsNull()) [[unlikely]] {
    return Literal::Null(int32());
  }

  if (literal.IsAboveMax() || literal.IsBelowMin()) [[unlikely]] {
    return NotSupported("Cannot extract year from {}", literal.ToString());
  }

  switch (literal.type()->type_id()) {
    DISPATCH_EXTRACT_YEAR(TypeId::kDate)
    DISPATCH_EXTRACT_YEAR(TypeId::kTimestamp)
    DISPATCH_EXTRACT_YEAR(TypeId::kTimestampTz)
    default:
      return NotSupported("Extract year from type {} is not supported",
                          literal.type()->ToString());
  }
}

#define DISPATCH_EXTRACT_MONTH(type_id) \
  case type_id:                         \
    return ExtractMonthImpl<type_id>(literal);

Result<Literal> TemporalUtils::ExtractMonth(const Literal& literal) {
  if (literal.IsNull()) [[unlikely]] {
    return Literal::Null(int32());
  }

  if (literal.IsAboveMax() || literal.IsBelowMin()) [[unlikely]] {
    return NotSupported("Cannot extract month from {}", literal.ToString());
  }

  switch (literal.type()->type_id()) {
    DISPATCH_EXTRACT_MONTH(TypeId::kDate)
    DISPATCH_EXTRACT_MONTH(TypeId::kTimestamp)
    DISPATCH_EXTRACT_MONTH(TypeId::kTimestampTz)
    default:
      return NotSupported("Extract month from type {} is not supported",
                          literal.type()->ToString());
  }
}

#define DISPATCH_EXTRACT_DAY(type_id) \
  case type_id:                       \
    return ExtractDayImpl<type_id>(literal);

Result<Literal> TemporalUtils::ExtractDay(const Literal& literal) {
  if (literal.IsNull()) [[unlikely]] {
    return Literal::Null(int32());
  }

  if (literal.IsAboveMax() || literal.IsBelowMin()) [[unlikely]] {
    return NotSupported("Cannot extract day from {}", literal.ToString());
  }

  switch (literal.type()->type_id()) {
    DISPATCH_EXTRACT_DAY(TypeId::kDate)
    DISPATCH_EXTRACT_DAY(TypeId::kTimestamp)
    DISPATCH_EXTRACT_DAY(TypeId::kTimestampTz)
    default:
      return NotSupported("Extract day from type {} is not supported",
                          literal.type()->ToString());
  }
}

#define DISPATCH_EXTRACT_HOUR(type_id) \
  case type_id:                        \
    return ExtractHourImpl<type_id>(literal);

Result<Literal> TemporalUtils::ExtractHour(const Literal& literal) {
  if (literal.IsNull()) [[unlikely]] {
    return Literal::Null(int32());
  }

  if (literal.IsAboveMax() || literal.IsBelowMin()) [[unlikely]] {
    return NotSupported("Cannot extract hour from {}", literal.ToString());
  }

  switch (literal.type()->type_id()) {
    DISPATCH_EXTRACT_HOUR(TypeId::kTimestamp)
    DISPATCH_EXTRACT_HOUR(TypeId::kTimestampTz)
    default:
      return NotSupported("Extract hour from type {} is not supported",
                          literal.type()->ToString());
  }
}

}  // namespace iceberg
