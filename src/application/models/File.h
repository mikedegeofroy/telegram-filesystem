#pragma once
#include <string>

#include "FileSystemEntity.h"

struct File : public FileSystemEntity {
  std::string local_path;
};
