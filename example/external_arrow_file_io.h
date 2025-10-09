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

#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include <arrow/filesystem/localfs.h>
#include <arrow/io/interfaces.h>
#include <arrow/result.h>
#include <arrow/buffer.h>

#include "iceberg/file_io.h"
#include "iceberg/result.h"

namespace iceberg::example {

/// Simple FileIO implementation using external Arrow installation
/// This provides the same functionality as the bundle's MakeLocalFileIO()
/// but works with external Arrow installations (ICEBERG_BUILD_BUNDLE=OFF)
class ExternalArrowFileIO : public FileIO {
 public:
  ExternalArrowFileIO() : fs_(std::make_shared<arrow::fs::LocalFileSystem>()) {}

  ~ExternalArrowFileIO() override = default;

  /// \brief Read the content of the file at the given location.
  Result<std::string> ReadFile(const std::string& file_location,
                               std::optional<size_t> length) override {
    auto result = fs_->OpenInputFile(file_location);
    if (!result.ok()) {
      return IOError("Failed to open file: {}", result.status().ToString());
    }
    
    auto file = result.ValueOrDie();
    auto size_result = file->GetSize();
    if (!size_result.ok()) {
      return IOError("Failed to get file size: {}", size_result.status().ToString());
    }
    
    auto file_size = size_result.ValueOrDie();
    size_t read_size = length.value_or(file_size);
    if (read_size > file_size) {
      read_size = file_size;
    }
    
    auto buffer_result = file->Read(read_size);
    if (!buffer_result.ok()) {
      return IOError("Failed to read file: {}", buffer_result.status().ToString());
    }
    
    auto buffer = buffer_result.ValueOrDie();
    return std::string(reinterpret_cast<const char*>(buffer->data()), buffer->size());
  }

  /// \brief Write the given content to the file at the given location.
  Status WriteFile(const std::string& file_location, std::string_view content) override {
    auto result = fs_->OpenOutputStream(file_location);
    if (!result.ok()) {
      return IOError("Failed to open output stream: {}", result.status().ToString());
    }
    
    auto file = result.ValueOrDie();
    auto write_result = file->Write(content.data(), content.size());
    if (!write_result.ok()) {
      return IOError("Failed to write to file: {}", write_result.ToString());
    }
    
    auto flush_result = file->Flush();
    if (!flush_result.ok()) {
      return IOError("Failed to flush file: {}", flush_result.ToString());
    }
    
    auto close_result = file->Close();
    if (!close_result.ok()) {
      return IOError("Failed to close file: {}", close_result.ToString());
    }
    
    return {};
  }

  /// \brief Delete a file at the given location.
  Status DeleteFile(const std::string& file_location) override {
    auto result = fs_->DeleteFile(file_location);
    if (!result.ok()) {
      return IOError("Failed to delete file: {}", result.ToString());
    }
    return {};
  }

 private:
  std::shared_ptr<arrow::fs::LocalFileSystem> fs_;
};

/// \brief Create a FileIO instance using external Arrow (equivalent to MakeLocalFileIO)
inline std::unique_ptr<FileIO> MakeExternalArrowFileIO() {
  return std::make_unique<ExternalArrowFileIO>();
}

}  // namespace iceberg::example