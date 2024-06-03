#pragma once

#include <memory>
#include <regex>;
#include <string>
#include <vector>

#include "../infrastructure/TelegramIntegration.h"
#include "contracts/IFileSystemService.h"

class TelegramFileSystemService : public IFileSystemService {
 private:
  std::shared_ptr<ITelegramIntegration> telegram_integration_;
  std::vector<std::string> split_path(const std::string& path);
  File* string_to_file(const std::string& content, const std::string& root_dir);

 public:
  TelegramFileSystemService();
  std::shared_ptr<FileSystemEntity> getEntitiesInPath(const std::string& path);
  ~TelegramFileSystemService();
};
