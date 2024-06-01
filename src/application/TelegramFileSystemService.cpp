#include "TelegramFileSystemService.h"

#include "../infrastructure/TelegramIntegration.h"

std::vector<std::string> TelegramFileSystemService::split_path(
    const std::string& path) {
  std::vector<std::string> exploded_path;

  std::string tmp;
  std::stringstream ss(path);

  while (getline(ss, tmp, '/')) {
    exploded_path.push_back(tmp);
  }

  return exploded_path;
}

TelegramFileSystemService::TelegramFileSystemService() {
  telegram_integration_ =
      std::shared_ptr<ITelegramIntegration>(new TelegramIntegration());
  telegram_integration_->auth_loop();
  telegram_integration_->start_event_loop();
}

std::shared_ptr<FileSystemEntity> TelegramFileSystemService::getEntitiesInPath(
    const std::string& path) {
  auto entities = telegram_integration_->searchChats("penis");

  for (auto& entity : entities) {
    std::cout << entity.name << "\n";
    for (auto& message : entity.messages) {
      std::cout << message.content << "\n";
    }
  }

  return std::shared_ptr<FileSystemEntity>();
}