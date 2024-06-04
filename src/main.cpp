#define FUSE_USE_VERSION 26
#define FUSE_DEFAULT_MAX_BACKGROUND 120
#include <fuse.h>

#include <iostream>
#include <memory>

#include "./application/TelegramFileSystemService.h"
#include "./application/models/Directory.h"
#include "./infrastructure/TelegramIntegration.h"

int main(int argc, char* argv[]) {
  std::shared_ptr<ITelegramIntegration> telegram_integration(
      new TelegramIntegration());

  std::shared_ptr<TelegramFileSystemService> fs(
      new TelegramFileSystemService(telegram_integration));

  auto root = fs->get_entities_in_path("fs-");
  std::shared_ptr<Directory> d = std::dynamic_pointer_cast<Directory>(root);

  if (d != nullptr) {
    for (auto& entity : d->entities) {
      std::cout << entity->name << " : " << entity->path << "\n";
      std::shared_ptr<Directory> dir =
          std::dynamic_pointer_cast<Directory>(entity);
      if (dir != nullptr) {
        for (auto& dir_entity : dir->entities) {
          std::shared_ptr<File> file =
              std::dynamic_pointer_cast<File>(dir_entity);
          if (file != nullptr) {
            std::cout << ">>> " << file->local_path << "\n";
          }
        }
      }
    }
  }

  return 0;
}
