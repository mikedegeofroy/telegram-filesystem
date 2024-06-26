#define FUSE_USE_VERSION 26
#include "presentation/FuseOperations.h"

static struct fuse_operations myOperations = {};

int main(int argc, char *argv[]) {
  myOperations.init = myfs_init;
  myOperations.destroy = myfs_destroy;

  myOperations.getattr = myfs_getattr;
  myOperations.readdir = myfs_readdir;

  myOperations.open = myfs_open;
  myOperations.truncate = myfs_truncate;
  myOperations.read = myfs_read;
  myOperations.write = myfs_write;
  myOperations.create = myfs_create;
  myOperations.unlink = myfs_unlink;
  myOperations.rename = myfs_rename;
  myOperations.release = myfs_release;
  myOperations.mkdir = myfs_mkdir;
  myOperations.rmdir = myfs_rmdir;

  return fuse_main(argc, argv, &myOperations, 0);
}