#define FUSE_USE_VERSION 26
#include <fuse.h>

#include <iostream>
#include <memory>

#include "application/TelegramFileSystemService.h"

int main(int argc, char* argv[]) {
  std::shared_ptr<IFileSystemService> fs(new TelegramFileSystemService);

  auto penis = fs->getEntitiesInPath("some path");

  return 0;
}
