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

template<typename T>
void log_message(T message) {
  std::cout << message << "\n";
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
  std::memset(stbuf, 0, sizeof(struct stat));

  auto ptr = fs_service->get_entities_in_path(std::string(path));
  if (!ptr.get()) {
    errno = ENOENT;
    return -errno;
  }

  stbuf->st_size = ptr->size;

  if (dynamic_cast<Directory*>(ptr.get())) {
    stbuf->st_mode =
        S_IFDIR | S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP | S_IROTH | S_IWOTH;
  } else if (dynamic_cast<File*>(ptr.get())) {
    stbuf->st_mode =
        S_IFREG | S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP | S_IROTH | S_IWOTH;
  } else {
    return -errno;
  }

  return 0;
}

int myfs_readdir(const char* path, void* buf, fuse_fill_dir_t filler,
                 off_t offset, struct fuse_file_info* fi) {
  auto ptr = fs_service->get_entities_in_path(std::string(path));
  if (!ptr.get()) {
    errno = ENOENT;
    return -errno;
  }

  if (!dynamic_cast<Directory*>(ptr.get())) {
    return -errno;
  }

  auto* dir = dynamic_cast<Directory*>(ptr.get());
  for (const auto& entity : dir->entities) {
    struct stat st {};
    myfs_getattr(entity->path.data(), &st);

    filler(buf, entity->name.data(), &st, 0);
  }
  return 0;
}

int myfs_open(const char* path, struct fuse_file_info* fi) {
  auto ptr = fs_service->get_entities_in_path(std::string(path));

  if (!ptr.get()) {
    errno = ENOENT;
    return -errno;
  }
  if (dynamic_cast<Directory*>(ptr.get())) {
    errno = EISDIR;
    return -errno;
  }
  // if (fi->flags & O_TRUNC)
  // {
  //   fout << "O_TRUNC\n";
  //   std::ifstream file(dynamic_cast<MockFile*>(ptr.get())->local_path,
  //   std::ios::trunc);
  // }

  return 0;
}

int myfs_truncate(const char* path, off_t size) {
  log_message("Truncate start");
  auto ptr = fs_service->get_entities_in_path(std::string(path));
  if (!ptr.get()) {
    errno = ENOENT;
    return -errno;
  }

  auto* file = dynamic_cast<File*>(ptr.get());
  if (!file) {
    errno = EISDIR;
    return -errno;
  }

  size_t current_size = file->size;

  if (size == 0) {
    std::ofstream ofs(file->local_path);
    if (!ofs.is_open()) {
      return -EACCES;
    }
  } else if (size > current_size) {
    std::ofstream ofs(file->local_path, std::ios::app);
    if (!ofs.is_open()) {
      return -EACCES;
    }
    for (off_t i = current_size; i < size; ++i) {
      ofs.put('\0');
    }
  }

  fs_service->write_file(file);
  log_message("Truncate end");
  return 0;
}

int myfs_read(const char* path, char* buf, size_t size, off_t offset,
              struct fuse_file_info* fi) {
  auto ptr = fs_service->get_entities_in_path(std::string(path));
  if (!ptr.get()) {
    errno = ENOENT;
    return -errno;
  }
  auto* file = dynamic_cast<File*>(ptr.get());
  if (!file) {
    errno = EINVAL;
    return -errno;
  }
  if (offset > file->size) {
    return 0;
  }
  std::ifstream file_d(file->local_path);
  file_d.seekg(offset, std::ios::beg);

  if (size <= file->size - offset) {
    file_d.read(buf, static_cast<int64_t>(size));
    return static_cast<int32_t>(size);
  }

  file_d.read(buf, file->size - offset);
  return file->size - offset;
}

int myfs_write(const char* path, const char* buf, size_t size, off_t offset,
               struct fuse_file_info* fi) {
  log_message("Write start");
  log_message(path);
  log_message(buf);
  log_message(size);
  log_message(offset);

  auto ptr = fs_service->get_entities_in_path(std::string(path));
  if (!ptr.get()) {
    errno = ENOENT;
    return -errno;
  }
  auto* file = dynamic_cast<File*>(ptr.get());
  if (!file) {
    errno = EINVAL;
    return -errno;
  }
  log_message(file->local_path);
  log_message(file->name);
  log_message(file->path);
  log_message(file->size);
  std::ofstream file_d;
  if (offset == 0) {
    file_d.open(file->local_path, std::ios::out);
  } else {
    file_d.open(file->local_path, std::ios::app);
  }
  file_d.write(buf, size);
  file_d.close();

  log_message("write_file start");
  fs_service->write_file(file);
  log_message("Write end");
  return size;
}

int myfs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
  fs_service->create_file(std::string(path));
  return 0;
}

int myfs_unlink(const char* path) {
  auto ptr = fs_service->get_entities_in_path(std::string(path));
  if (!ptr.get()) {
    errno = ENOENT;
    return -errno;
  }
  auto* file = dynamic_cast<File*>(ptr.get());
  if (!file) {
    errno = EINVAL;
    return -errno;
  }

  fs_service->delete_file(file);
  return 0;
}

int myfs_rename(const char* from, const char* to) {
  auto ptr = fs_service->get_entities_in_path(std::string(from));
  if (!ptr.get()) {
    errno = ENOENT;
    return -errno;
  }
  auto* file = dynamic_cast<File*>(ptr.get());
  if (!file) {
    errno = EINVAL;
    return -errno;
  }

  fs_service->move_file(file, std::string(to));
  return 0;
}

int myfs_release(const char* path, struct fuse_file_info* fi) { return 0; }

int myfs_mkdir(const char* path, mode_t mode) {
  // fs_service->create_dir(std::string(path));
  return 0;
}

int myfs_rmdir(const char* path) {
  auto ptr = fs_service->get_entities_in_path(std::string(path));
  if (!ptr.get()) {
    errno = ENOENT;
    return -errno;
  }
  auto* dir = dynamic_cast<Directory*>(ptr.get());
  if (!dir) {
    errno = EINVAL;
    return -errno;
  }

  // fs_service->remove_dir(dir);
  return 0;
}