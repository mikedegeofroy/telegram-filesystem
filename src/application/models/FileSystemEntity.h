#pragma once
#include <fstream>
#include <string>

struct FileSystemEntity {
  std::string name;
  std::string path;
  size_t size = 0;
  virtual ~FileSystemEntity() = default;
};
