#pragma once

#include <memory>
#include <string>
#include <vector>

#include "./abstractions/ITelegramIntegration.h"
#include "contracts/IFileSystemService.h"

class TelegramFileSystemService : public IFileSystemService {
 private:
  std::shared_ptr<ITelegramIntegration> telegram_integration_;
  std::vector<std::string> split_path(const std::string& path);
  File* string_to_file(const std::string& content,
                       const std::string& local_path,
                       const std::string& root_dir);

 public:
  TelegramFileSystemService(
      std::shared_ptr<ITelegramIntegration> telegram_integration);
  std::shared_ptr<FileSystemEntity> get_entities_in_path(const std::string& path);
  void create_file(File file);
  void write_file(File file);
  ~TelegramFileSystemService();
};
