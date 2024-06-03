#define FUSE_USE_VERSION 26
#define FUSE_DEFAULT_MAX_BACKGROUND 120
#include <fuse.h>

#include <iostream>
#include <memory>

#include "./application/TelegramFileSystemService.h"
#include "./application/models/Directory.h"
#include "./infrastructure/TelegramIntegration.h"

class MyFuseOperations {
 public:
  static std::shared_ptr<TelegramFileSystemService> fs;

  static void initialize(std::shared_ptr<TelegramFileSystemService> service) {
    fs = service;
  }

  static int myfs_getattr(const char *path, struct stat *stbuf) {
    stbuf->st_mode = S_IFDIR | 00400;
    return 0;
  }

  static int myfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                          off_t offset, struct fuse_file_info *fi) {
    auto root = fs->getEntitiesInPath("fs");

    std::shared_ptr<Directory> d = std::dynamic_pointer_cast<Directory>(root);
    if (d != nullptr) {
      for (auto &entity : d->entities) {
        filler(buf, entity->name.c_str(), NULL, 0);
      }
    }

    return 0;
  }
};

// Definition of the static member
std::shared_ptr<TelegramFileSystemService> MyFuseOperations::fs;

static struct fuse_operations myOperations = {
    .getattr = MyFuseOperations::myfs_getattr,
    .readdir = MyFuseOperations::myfs_readdir};

int main(int argc, char *argv[]) {
  std::shared_ptr<ITelegramIntegration> telegram_integration(
      new TelegramIntegration());
  std::shared_ptr<TelegramFileSystemService> fs(
      new TelegramFileSystemService(telegram_integration));

  MyFuseOperations::initialize(fs);

  return fuse_main(argc, argv, &myOperations, NULL);
}
