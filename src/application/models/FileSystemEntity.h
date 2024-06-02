#pragma once
#include <string>
#include <fstream>

struct FileSystemEntity {
  std::string name;
  std::string path;
  size_t size;
  std::ifstream data;
};
