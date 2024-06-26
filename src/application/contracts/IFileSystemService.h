#pragma once
#include <vector>
#include <memory>

#include "../models/File.h"

struct IFileSystemService {
  virtual std::shared_ptr<FileSystemEntity> get_entities_in_path(const std::string& path) = 0;
  virtual void create_file(const std::string& path) = 0;
  virtual void move_file(File* from, const std::string& to) = 0;
  virtual void delete_file(File* file) = 0;
  virtual void write_file(File* file) = 0;

  // virtual void create_dir(const std::string& path) = 0;
  // virtual void remove_dir(Directory* dir) = 0;
};