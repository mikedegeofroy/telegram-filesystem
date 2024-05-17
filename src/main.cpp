#define FUSE_USE_VERSION 26
#include <fuse.h>

#include <iostream>

#include "telegram-integration.h"

int myfs_getattr(const char *path, struct stat *stbuf) {
  stbuf->st_mode = S_IFDIR | 00400;
  return 0;
}

int myfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                 off_t offset, struct fuse_file_info *fi) {
  std::cout << "wwha??\n";

  filler(buf, "test", NULL, 0);
  filler(buf, "test1", NULL, 0);
  filler(buf, "test2", NULL, 0);
  filler(buf, "test3", NULL, 0);

  return 0;
}

static struct fuse_operations myOperations = {.getattr = myfs_getattr,
                                              .readdir = myfs_readdir};

int main(int argc, char *argv[]) {
  // return fuse_main(argc, argv, &myOperations, 0);

  TelegramIntegration *integration = new TelegramIntegration();

  integration->test();

  return 0;
}
