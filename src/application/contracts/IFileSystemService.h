#pragma once
#include <vector>
#include <memory>

#include "../models/File.h"

struct IFileSystemService {
  virtual std::shared_ptr<FileSystemEntity> getEntitiesInPath(const std::string& path) = 0;
};