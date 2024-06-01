#pragma once
#include <string>
#include <fstream>

#include "MockFileSystemEntity.h"

struct MockFile : public MockFileSystemEntity {
    // File(std::string n, std::string p, size_t s) : FileSystemEntity(n, p, s) { }
    MockFile(std::ifstream&& file_d, std::string n, std::string p, size_t s) : 
        MockFileSystemEntity(n, p, s), file_data(std::move(file_d)){ }

    std::ifstream file_data;
};