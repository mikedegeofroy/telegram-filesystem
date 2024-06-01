#pragma once

#include <vector>

#include "MockFileSystemEntity.h"

struct MockDirectory : public MockFileSystemEntity {
  MockDirectory(std::vector<std::shared_ptr<MockFileSystemEntity>> ent, std::string n, std::string p, size_t s) : 
      MockFileSystemEntity(n, p, s), entities(ent) { }
  
  std::vector<std::shared_ptr<MockFileSystemEntity>> entities;
};