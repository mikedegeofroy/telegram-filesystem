#define FUSE_USE_VERSION 26
#include <fuse.h>

#include "./application/TelegramFileSystemService.h"
#include "./application/models/Directory.h"

#define FUSE_USE_VERSION 26
#include <fuse.h>

#include <iostream>

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

    if (std::shared_ptr<Directory> d =
            std::dynamic_pointer_cast<Directory>(root);
        d != nullptr) {
      for (auto &entity : d->entities) {
        filler(buf, entity->name.c_str(), NULL, 0);
      }
    }

    return 0;
  }
};

static struct fuse_operations myOperations = {
    .getattr = MyFuseOperations::myfs_getattr,
    .readdir = MyFuseOperations::myfs_readdir};

int main(int argc, char *argv[]) {
  std::shared_ptr<TelegramFileSystemService> fs(new TelegramFileSystemService);

  MyFuseOperations::initialize(fs);

  return fuse_main(argc, argv, &myOperations, NULL);
}
