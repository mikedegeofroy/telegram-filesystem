#pragma once
#include "MockDirectory.h"
#include "MockFile.h"
#include<fstream>
#include<memory>

struct MockFileSystemService {
  MockFileSystemService() = default;

  std::shared_ptr<MockFileSystemEntity> getFilesInPath(const std::string& path) {  
    std::vector<std::shared_ptr<MockFileSystemEntity>> ent;
    ent.emplace_back(new MockFile(std::ifstream("/home/etozheraf/telegram-filesystem/1.txt"), "1",
                             "/channel/1", 10));
    ent.emplace_back(new MockFile(std::ifstream("/home/etozheraf/telegram-filesystem/2.txt"), "2", 
                              "/channel/2", 5));
    std::shared_ptr<MockFileSystemEntity> channel(new MockDirectory(ent, "channel", "/channel", 15));
    std::vector<std::shared_ptr<MockFileSystemEntity>> v = {channel};
    std::shared_ptr<MockFileSystemEntity> mnt(new MockDirectory(v, "", "/", 15));
    
    if (path == "/")
    {
      return mnt;
    } else if (path == "/channel") {
      return channel;
    } else if (path == "/channel/1") {
      return ent[0];
    } else if (path == "/channel/2") {
      return ent[1];
    }

    return nullptr;
  }
};
