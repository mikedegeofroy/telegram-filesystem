#define FUSE_USE_VERSION 26
#include <fuse.h>

#include "./application/TelegramFileSystemService.h"

int main(int argc, char* argv[]) { 
  std::shared_ptr<TelegramFileSystemService> fs(new TelegramFileSystemService);

  fs->getEntitiesInPath("fs");

  return 0; 
  }
