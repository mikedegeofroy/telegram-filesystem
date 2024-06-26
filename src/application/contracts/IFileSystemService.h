#pragma once
#include <vector>
#include <memory>

#include "../models/File.h"

struct IFileSystemService {
  virtual std::shared_ptr<FileSystemEntity> get_entities_in_path(const std::string& path) = 0;
  virtual void create_file(File file) = 0;
  virtual void write_file(File file) = 0;
};