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

#include <chrono>
#include <cstdint>

#include "iceberg/util/temporal_util.h"

namespace iceberg {

using namespace std::chrono;  // NOLINT
using namespace compat;       // NOLINT for temporal compatibility

struct DateParts {
  int32_t year{0};
  uint8_t month{0};
  uint8_t day{0};
};

struct TimeParts {
  int32_t hour{0};
  int32_t minute{0};
  int32_t second{0};
  int32_t microsecond{0};
};

struct TimestampParts {
  int32_t year{0};
  uint8_t month{0};
  uint8_t day{0};
  int32_t hour{0};
  int32_t minute{0};
  int32_t second{0};
  int32_t microsecond{0};
  // e.g. -480 for PST (UTC-8:00), +480 for Asia/Shanghai (UTC+8:00)
  int32_t tz_offset_minutes{0};
};

struct TimestampNanosParts {
  int32_t year{0};
  uint8_t month{0};
  uint8_t day{0};
  int32_t hour{0};
  int32_t minute{0};
  int32_t second{0};
  int32_t nanosecond{0};
  // e.g. -480 for PST (UTC-8:00), +480 for Asia/Shanghai (UTC+8:00)
  int32_t tz_offset_minutes{0};
};

class TemporalTestHelper {
  // Helper function to calculate days since Unix epoch (1970-01-01)
  static constexpr int32_t DaysSinceEpoch(int32_t year, int32_t month, int32_t day) {
    // Julian day calculation - simplified for post-1970 dates
    int32_t days = 0;
    
    // Add days for complete years since 1970
    for (int32_t y = 1970; y < year; ++y) {
      days += 365;
      if (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0)) {
        days += 1; // leap year
      }
    }
    
    // Add days for complete months in the current year
    int32_t days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) {
      days_in_month[1] = 29; // leap year
    }
    
    for (int32_t m = 1; m < month; ++m) {
      days += days_in_month[m - 1];
    }
    
    // Add remaining days
    days += day - 1; // day is 1-based
    
    return days;
  }

 public:
  /// \brief Construct a Calendar date without timezone or time
  static int32_t CreateDate(const DateParts& parts) {
    return DaysSinceEpoch(parts.year, parts.month, parts.day);
  }

  /// \brief Construct a time-of-day, microsecond precision, without date, timezone
  static int64_t CreateTime(const TimeParts& parts) {
    using namespace std::chrono;
    return duration_cast<microseconds>(hours(parts.hour) + minutes(parts.minute) +
                                       seconds(parts.second) +
                                       microseconds(parts.microsecond))
        .count();
  }

  /// \brief Construct a timestamp, microsecond precision, without timezone
  static int64_t CreateTimestamp(const TimestampParts& parts) {
    // Calculate days since epoch
    int32_t days = DaysSinceEpoch(parts.year, parts.month, parts.day);
    
    // Convert to microseconds since epoch
    int64_t days_in_micros = static_cast<int64_t>(days) * 24 * 60 * 60 * 1000000LL;
    int64_t time_in_micros = static_cast<int64_t>(parts.hour) * 60 * 60 * 1000000LL +
                             static_cast<int64_t>(parts.minute) * 60 * 1000000LL +
                             static_cast<int64_t>(parts.second) * 1000000LL +
                             static_cast<int64_t>(parts.microsecond);
    
    return days_in_micros + time_in_micros;
  }

  /// \brief Construct a timestamp, microsecond precision, with timezone
  static int64_t CreateTimestampTz(const TimestampParts& parts) {
    // Calculate timestamp without timezone first
    int64_t timestamp_micros = CreateTimestamp(parts);
    
    // Adjust for timezone offset (convert from local time to UTC)
    int64_t tz_adjustment_micros = static_cast<int64_t>(parts.tz_offset_minutes) * 60 * 1000000LL;
    
    return timestamp_micros - tz_adjustment_micros;
  }

  /// \brief Construct a timestamp, nanosecond precision, without timezone
  static int64_t CreateTimestampNanos(const TimestampNanosParts& parts) {
    // Calculate days since epoch
    int32_t days = DaysSinceEpoch(parts.year, parts.month, parts.day);
    
    // Convert to nanoseconds since epoch
    int64_t days_in_nanos = static_cast<int64_t>(days) * 24 * 60 * 60 * 1000000000LL;
    int64_t time_in_nanos = static_cast<int64_t>(parts.hour) * 60 * 60 * 1000000000LL +
                            static_cast<int64_t>(parts.minute) * 60 * 1000000000LL +
                            static_cast<int64_t>(parts.second) * 1000000000LL +
                            static_cast<int64_t>(parts.nanosecond);
    
    return days_in_nanos + time_in_nanos;
  }

  /// \brief Construct a timestamp, nanosecond precision, with timezone
  static int64_t CreateTimestampTzNanos(const TimestampNanosParts& parts) {
    // Calculate timestamp without timezone first
    int64_t timestamp_nanos = CreateTimestampNanos(parts);
    
    // Adjust for timezone offset (convert from local time to UTC)
    int64_t tz_adjustment_nanos = static_cast<int64_t>(parts.tz_offset_minutes) * 60 * 1000000000LL;
    
    return timestamp_nanos - tz_adjustment_nanos;
  }
};

}  // namespace iceberg
