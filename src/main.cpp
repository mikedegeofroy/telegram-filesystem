#define FUSE_USE_VERSION 26
#define FUSE_DEFAULT_MAX_BACKGROUND 120
#include <fuse.h>

#include <iostream>
#include <memory>

#include "./application/TelegramFileSystemService.h"
#include "./application/models/Directory.h"
#include "./infrastructure/TelegramIntegration.h"

// class MyFuseOperations {
//  public:
//   static std::shared_ptr<TelegramFileSystemService> fs;

//   static void* initialize(struct fuse_conn_info* conn) {
//     std::shared_ptr<ITelegramIntegration> telegram_integration(
//         new TelegramIntegration());
//     fs = std::make_shared<TelegramFileSystemService>(telegram_integration);
//   }

//   static int myfs_getattr(const char* path, struct stat* stbuf) {
//     stbuf->st_mode = S_IFDIR | 0755;
//     return 0;
//   }

//   static int myfs_readdir(const char* path, void* buf, fuse_fill_dir_t
//   filler,
//                           off_t offset, struct fuse_file_info* fi) {
//     filler(buf, ".", NULL, 0);
//     filler(buf, "..", NULL, 0);

//     auto root = fs->getEntitiesInPath("fs");

//     std::shared_ptr<Directory> d =
//     std::dynamic_pointer_cast<Directory>(root); if (d != nullptr) {
//       for (auto& entity : d->entities) {
//         filler(buf, entity->name.c_str(), NULL, 0);
//       }
//     }

//     return 0;
//   }
// };

// std::shared_ptr<TelegramFileSystemService> MyFuseOperations::fs;

// static struct fuse_operations myOperations = {};

int main(int argc, char* argv[]) {
  std::shared_ptr<ITelegramIntegration> telegram_integration(
      new TelegramIntegration());

  std::shared_ptr<TelegramFileSystemService> fs(
      new TelegramFileSystemService(telegram_integration));

  auto root = fs->getEntitiesInPath("fs-");
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
            std::cout << file->local_path << "\n";
          }
        }
      }
    }
  }
  // telegram_integration->auth_loop();

  // myOperations.init = MyFuseOperations::initialize;
  // myOperations.getattr = MyFuseOperations::myfs_getattr;
  // myOperations.readdir = MyFuseOperations::myfs_readdir;

  // return fuse_main(argc, argv, &myOperations, NULL);
  return 0;
}
