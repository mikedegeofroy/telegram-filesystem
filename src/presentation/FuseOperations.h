#include <fuse.h>
#include <iostream>
#include <cstring>
#include "../application/MockFileSystemService.h"



int myfs_getattr(const char *path, struct stat *stbuf) {
  std::ofstream fout("/home/etozheraf/telegram-filesystem/output.txt", std::ios::app);
  fout << "Start getattr\n";
  std::memset(stbuf, 0, sizeof(struct stat));

  MockFileSystemService f;
  auto ptr = f.getFilesInPath(std::string(path));
  fout << "Get " + ptr->name + " " + ptr->path + "\n";
  stbuf->st_size = ptr->size;
  
  if (dynamic_cast<MockDirectory*>(ptr.get())) {
      stbuf->st_mode = S_IFDIR | S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP | S_IROTH | S_IWOTH;
  } else if (dynamic_cast<MockFile*>(ptr.get())) {
      stbuf->st_mode = S_IFREG | S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP | S_IROTH | S_IWOTH;
  } else {
    return -errno;
  }

  fout << "End gettatr\n";
  return 0;
}

int myfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                 off_t offset, struct fuse_file_info *fi) 
{
  MockFileSystemService f;
  std::ofstream fout("/home/etozheraf/telegram-filesystem/output.txt", std::ios::app);
  fout << "Start ls\n";

  auto ptr = f.getFilesInPath(std::string(path));

  if (!dynamic_cast<MockDirectory*>(ptr.get())) {
    return -errno;
  }
  fout << "Get " + ptr->name + " " + ptr->path + "\n";


  MockDirectory* dir = dynamic_cast<MockDirectory*>(ptr.get());
  for (auto entity : dir->entities)
  {
    struct stat st;
    fout << "Get " + entity->name + " " + entity->path + "\n";
    myfs_getattr(entity->path.data(), &st);

    if (dynamic_cast<MockDirectory*>(ptr.get())) {
      filler(buf, entity->name.data(), &st, 0);
    } else if (dynamic_cast<MockFile*>(ptr.get()))
    {
      filler(buf, entity->name.data(), &st, 0); 
    }
  }
  fout << "End ls\n";
  return 0;
}

int myfs_open(const char* path, struct fuse_file_info* fi) {
  std::ofstream fout("/home/etozheraf/telegram-filesystem/output.txt", std::ios::app);
  fout << "Start open\n";
  MockFileSystemService f;
  auto ptr = f.getFilesInPath(std::string(path));

  if (!ptr.get() || dynamic_cast<MockDirectory*>(ptr.get()))
  {
    fout << "Wrong open\n";
    return -errno;
  }
  fout << "Success open\n";
  return 0;
}

int myfs_read(const char* path, char *buf, size_t size, off_t offset, struct fuse_file_info* fi) {
  std::ofstream fout("/home/etozheraf/telegram-filesystem/output.txt", std::ios::app);
  fout << "Start read\n";
  MockFileSystemService f;
  auto ptr = f.getFilesInPath(std::string(path));
  auto* file = dynamic_cast<MockFile*>(ptr.get());
  if (!file)
  {
    fout << "Wrong read\n";
    return -errno;
  }
  
  std::ifstream& file_d = file->file_data;

  file_d.read(buf + offset, file->size);
  fout << offset << " " << size << " Success read\n";
  return file->size;
}

int myfs_release(const char* path, struct fuse_file_info* fi) {
    return 0;
}