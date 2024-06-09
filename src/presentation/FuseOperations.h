#include <fuse.h>
#include <syslog.h>

#include <cstring>
#include <fstream>
#include <iostream>

#include "../application/TelegramFileSystemService.h"
#include "../application/abstractions/ITelegramIntegration.h"
#include "../application/contracts/IFileSystemService.h"
#include "../application/models/Directory.h"
#include "../application/models/File.h"
#include "../infrastructure/TelegramIntegration.h"

std::shared_ptr<IFileSystemService> fs_service;
std::shared_ptr<ITelegramIntegration> tg_integration;

void log_message(const char* message) {
  std::ofstream fout("/var/log/fslog", std::ios::app);
  fout << message << "\n";
}

void* myfs_init(struct fuse_conn_info* conn) {
  log_message("init called");
  if (!tg_integration) {
    tg_integration = std::make_shared<TelegramIntegration>();
  }
  if (!fs_service) {
    fs_service = std::make_shared<TelegramFileSystemService>(tg_integration);
  }
  return nullptr;
}

void myfs_destroy(void* private_data) {
  log_message("destroy called");
  fs_service.reset();
  tg_integration.reset();
}

int myfs_getattr(const char* path, struct stat* stbuf) {
  log_message("get attr called");
  log_message(path);
  std::memset(stbuf, 0, sizeof(struct stat));
  log_message("memset called");

  auto ptr = fs_service->get_entities_in_path(std::string(path));
  log_message("get_entities_in_path called");

  stbuf->st_size = ptr->size;
  log_message("st_size called");


  if (dynamic_cast<Directory*>(ptr.get())) {
    stbuf->st_mode =
        S_IFDIR | S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP | S_IROTH | S_IWOTH;
  } else if (dynamic_cast<File*>(ptr.get())) {
    stbuf->st_mode =
        S_IFREG | S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP | S_IROTH | S_IWOTH;
  } else {
    log_message("Wrong get attr");
    return -errno;
  }

  log_message("get attr ended");

  return 0;
}

int myfs_readdir(const char* path, void* buf, fuse_fill_dir_t filler,
                 off_t offset, struct fuse_file_info* fi) {
  log_message("myfs_readdir called");

  log_message("fs_service");
  auto ptr = fs_service->get_entities_in_path(std::string(path));
  log_message("ptr");

  if (!dynamic_cast<Directory*>(ptr.get())) {
    return -errno;
  }

  Directory* dir = dynamic_cast<Directory*>(ptr.get());
  for (auto entity : dir->entities) {
    struct stat st;
    myfs_getattr(entity->path.data(), &st);

    if (dynamic_cast<Directory*>(ptr.get())) {
      filler(buf, entity->name.data(), &st, 0);
    } else if (dynamic_cast<File*>(ptr.get())) {
      filler(buf, entity->name.data(), &st, 0);
    }
  }

  log_message("myfs_readdir ended");
  
  return 0;
}

int myfs_open(const char* path, struct fuse_file_info* fi) {
  auto ptr = fs_service->get_entities_in_path(std::string(path));

  if (!ptr.get() || dynamic_cast<Directory*>(ptr.get())) {
    return -errno;
  }
  return 0;
}

int myfs_read(const char* path, char* buf, size_t size, off_t offset,
              struct fuse_file_info* fi) {
  auto ptr = fs_service->get_entities_in_path(std::string(path));
  auto* file = dynamic_cast<File*>(ptr.get());
  if (!file) {
    return -errno;
  }

  std::ifstream file_d(file->local_path);

  file_d.read(buf + offset, file->size);
  return file->size;
}

int myfs_release(const char* path, struct fuse_file_info* fi) { return 0; }