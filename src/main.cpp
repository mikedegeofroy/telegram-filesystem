#define FUSE_USE_VERSION 26
#include "presentation/FuseOperations.h"

static struct fuse_operations myOperations = {};

int main(int argc, char *argv[]) {
  myOperations.init = myfs_init;
  myOperations.destroy = myfs_destroy;

  myOperations.getattr = myfs_getattr;
  myOperations.readdir = myfs_readdir;

  myOperations.open = myfs_open;
  myOperations.read = myfs_read;
  myOperations.release = myfs_release;

  return fuse_main(argc, argv, &myOperations, 0);
}