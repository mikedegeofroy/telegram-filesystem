#define FUSE_USE_VERSION 26
#include <fuse.h>

#include "./application/TelegramFileSystemService.h"
#include "./application/models/Directory.h"

int main(int argc, char* argv[]) {
  std::shared_ptr<TelegramFileSystemService> fs(new TelegramFileSystemService);

  auto root = fs->getEntitiesInPath("fs");

  if (std::shared_ptr<Directory> d = std::dynamic_pointer_cast<Directory>(root);
      d != nullptr) {
    for (auto& entity : d->entities) {
      std::cout << entity->name << " : " << entity->path << "\n";
      if (std::shared_ptr<Directory> dir =
              std::dynamic_pointer_cast<Directory>(entity);
          d != nullptr) {
        for (auto& file : dir->entities) {
          std::cout << file->name << ", " << file->path << ", " << file->size
                    << "\n";
        }
      }
    }
  }

  return 0;
}
