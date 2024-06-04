#pragma once
#include <string>
#include <memory>

struct MockFileSystemEntity {
  MockFileSystemEntity(std::string n, std::string p, size_t s) : name(n), path(p), size(s) { }
  
  std::string name;
  std::string path;
  size_t size;

  virtual ~MockFileSystemEntity() = default;
};