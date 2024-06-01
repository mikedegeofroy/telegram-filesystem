#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../infrastructure/TelegramIntegration.h"
#include "contracts/IFileSystemService.h"
#include "models/FileSystemEntity.h"

class TelegramFileSystemService : public IFileSystemService {
 private:
  std::shared_ptr<ITelegramIntegration> telegram_integration_;
  std::vector<std::string> split_path(const std::string& path);

 public:
  TelegramFileSystemService();
  std::shared_ptr<FileSystemEntity> getEntitiesInPath(const std::string& path);
};
