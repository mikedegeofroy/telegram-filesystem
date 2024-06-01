#pragma once
#include <string>

struct FileSystemEntity {
  std::string name;
  std::string path;
  size_t size;
};
