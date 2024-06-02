#define FUSE_USE_VERSION 26
#include <fuse.h>

#include <iostream>
#include <memory>

#include "application/TelegramFileSystemService.h"

int main(int argc, char* argv[]) {
  std::unique_ptr<IFileSystemService> fs(new TelegramFileSystemService);

  auto penis = fs->getEntitiesInPath("fs-");

  std::cout << "end\n";

  return 0;
}
