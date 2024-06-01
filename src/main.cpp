#define FUSE_USE_VERSION 26

#include"presentation/FuseOperations.h"

static struct fuse_operations myOperations = {.getattr = myfs_getattr,
                                              .readdir = myfs_readdir};

int main(int argc, char *argv[]) {
  myOperations.open = myfs_open;
  myOperations.read = myfs_read;
  myOperations.release = myfs_release;
  return fuse_main(argc, argv, &myOperations, 0);
}
