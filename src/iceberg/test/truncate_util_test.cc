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

#include "iceberg/util/truncate_util.h"

#include <gtest/gtest.h>

#include "iceberg/expression/literal.h"
#include "iceberg/test/matchers.h"

namespace iceberg {

// The following tests are from
// https://iceberg.apache.org/spec/#truncate-transform-details
TEST(TruncateUtilTest, TruncateLiteral) {
  // Integer
  auto result1 = TruncateUtils::TruncateLiteral(Literal::Int(1), 10);
  EXPECT_THAT(result1, IsOk());
  EXPECT_EQ(result1.value(), Literal::Int(0));
  
  auto result2 = TruncateUtils::TruncateLiteral(Literal::Int(-1), 10);
  EXPECT_THAT(result2, IsOk());
  EXPECT_EQ(result2.value(), Literal::Int(-10));
  
  auto result3 = TruncateUtils::TruncateLiteral(Literal::Long(1), 10);
  EXPECT_THAT(result3, IsOk());
  EXPECT_EQ(result3.value(), Literal::Long(0));
  
  auto result4 = TruncateUtils::TruncateLiteral(Literal::Long(-1), 10);
  EXPECT_THAT(result4, IsOk());
  EXPECT_EQ(result4.value(), Literal::Long(-10));

  // Decimal
  auto result5 = TruncateUtils::TruncateLiteral(Literal::Decimal(1065, 4, 2), 50);
  EXPECT_THAT(result5, IsOk());
  EXPECT_EQ(result5.value(), Literal::Decimal(1050, 4, 2));

  // String
  auto result6 = TruncateUtils::TruncateLiteral(Literal::String("iceberg"), 3);
  EXPECT_THAT(result6, IsOk());
  EXPECT_EQ(result6.value(), Literal::String("ice"));

  // Binary
  std::string data = "\x01\x02\x03\x04\x05";
  std::string expected = "\x01\x02\x03";
  auto result7 = TruncateUtils::TruncateLiteral(
      Literal::Binary(std::vector<uint8_t>(data.begin(), data.end())), 3);
  EXPECT_THAT(result7, IsOk());
  EXPECT_EQ(result7.value(), Literal::Binary(std::vector<uint8_t>(expected.begin(), expected.end())));
}

}  // namespace iceberg
