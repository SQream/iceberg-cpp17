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

#include "iceberg/statistics_file.h"

#include "iceberg/format_compat.h"

#include "iceberg/util/formatter_internal.h"

namespace iceberg {

std::string ToString(const BlobMetadata& blob_metadata) {
  std::string repr = "BlobMetadata[";
  repr += "type='" + blob_metadata.type + "'";
  repr += ",sourceSnapshotId=" + std::to_string(blob_metadata.source_snapshot_id);
  repr += ",sourceSnapshotSequenceNumber=" + std::to_string(blob_metadata.source_snapshot_sequence_number);
  
  // Format fields vector
  repr += ",fields=[";
  for (size_t i = 0; i < blob_metadata.fields.size(); ++i) {
    if (i > 0) repr += ",";
    repr += std::to_string(blob_metadata.fields[i]);
  }
  repr += "]";
  
  // Format properties map
  repr += ",properties={";
  bool first = true;
  for (const auto& pair : blob_metadata.properties) {
    if (!first) repr += ",";
    repr += pair.first + ":" + pair.second;
    first = false;
  }
  repr += "}";
  repr += "]";
  return repr;
}

std::string ToString(const StatisticsFile& statistics_file) {
  std::string repr = "StatisticsFile[";
  repr += "snapshotId=" + std::to_string(statistics_file.snapshot_id);
  repr += ",path=" + statistics_file.path;
  repr += ",fileSizeInBytes=" + std::to_string(statistics_file.file_size_in_bytes);
  repr += ",fileFooterSizeInBytes=" + std::to_string(statistics_file.file_footer_size_in_bytes);
  
  // Format blob_metadata vector
  repr += ",blobMetadata=[";
  for (size_t i = 0; i < statistics_file.blob_metadata.size(); ++i) {
    if (i > 0) repr += ",";
    repr += ToString(statistics_file.blob_metadata[i]);
  }
  repr += "]";
  repr += "]";
  return repr;
}

std::string ToString(const PartitionStatisticsFile& partition_statistics_file) {
  std::string repr = "PartitionStatisticsFile[";
  repr += "snapshotId=" + std::to_string(partition_statistics_file.snapshot_id);
  repr += ",path=" + partition_statistics_file.path;
  repr += ",fileSizeInBytes=" + std::to_string(partition_statistics_file.file_size_in_bytes);
  repr += "]";
  return repr;
}

}  // namespace iceberg
