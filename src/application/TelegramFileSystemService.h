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
  std::string file_to_string(const File& file);

 public:
  TelegramFileSystemService(
      std::shared_ptr<ITelegramIntegration> telegram_integration);
  std::shared_ptr<FileSystemEntity> get_entities_in_path(const std::string& path) override;
  void create_file(const std::string& path) override;
  void write_file(File* file) override;
  void delete_file(File* file) override;
  void move_file(File* from, const std::string& to) override;
  ~TelegramFileSystemService();
};
