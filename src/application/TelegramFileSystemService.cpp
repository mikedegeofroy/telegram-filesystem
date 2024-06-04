#include "TelegramFileSystemService.h"

#include <regex>
#include <sstream>

#include "./models/Directory.h"

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

File* TelegramFileSystemService::string_to_file(const std::string& input,
                                                const std::string& local_path,
                                                const std::string& root_dir) {
  std::string path;
  std::string filename;
  std::size_t size;

  std::regex path_regex(R"(path:\s(.+))");
  std::regex size_regex(R"(size:\s(\d+\w+))");

  std::smatch match;

  if (std::regex_search(input, match, path_regex) && match.size() > 1) {
    path = match.str(1);
    filename = path.substr(path.find_last_of("/") + 1);
  }

  if (std::regex_search(input, match, size_regex) && match.size() > 1) {
    size = static_cast<size_t>(std::stoull(match.str(1)));
  }

  File* file = new File();

  file->name = filename;
  file->path = root_dir + path;
  file->size = size;
  file->local_path = local_path;

  return file;
}

TelegramFileSystemService::TelegramFileSystemService(
    std::shared_ptr<ITelegramIntegration> telegram_integration) {
  telegram_integration_ = telegram_integration;
  telegram_integration_->auth_loop();
  telegram_integration_->start_event_loop();
}

std::shared_ptr<FileSystemEntity> TelegramFileSystemService::get_entities_in_path(
    const std::string& path) {
  auto location = split_path(path);

  std::shared_ptr<Directory> root(new Directory());
  auto chats = telegram_integration_->searchChats("fs-");

  for (auto& chat : chats) {
    std::shared_ptr<Directory> dir(new Directory());
    dir->name = chat.name;
    dir->path = "/" + chat.name;

    for (auto& message : chat.messages) {
      std::shared_ptr<File> file(
          string_to_file(message.content, message.attachment, dir->path));
      dir->entities.push_back(file);
    }

    root->entities.push_back(dir);
  }

  return root;
}

void TelegramFileSystemService::create_file(File file) {
}

void TelegramFileSystemService::write_file(File file) {
}

TelegramFileSystemService::~TelegramFileSystemService() {}
