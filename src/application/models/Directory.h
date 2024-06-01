#pragma once

#include <vector>

#include "FileSystemEntity.h"

struct Directory : public FileSystemEntity {
  std::vector<std::shared_ptr<FileSystemEntity>> entities;
};