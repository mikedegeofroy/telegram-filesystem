#include "TelegramFileSystemService.h"

#include <regex>
#include <sstream>
#include <iostream>

#include "./models/Directory.h"

template<typename T>
void logT_message(T message) {
  std::cout << message << "\n";
}

std::vector<std::string> TelegramFileSystemService::split_path(
    const std::string& path) {
  if (path == "/") {
    return {};
  }

  std::vector<std::string> exploded_path;
  std::string tmp;
  std::stringstream ss(path);

  while (getline(ss, tmp, '/')) {
    if (!tmp.empty()) {
      exploded_path.push_back(tmp);
    }
  }

  return exploded_path;
}

std::string TelegramFileSystemService::file_to_string(const File& file) {
  return "path: " + file.path + "\nsize: " + std::to_string(file.size) + "b";
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

std::shared_ptr<FileSystemEntity>
TelegramFileSystemService::get_entities_in_path(const std::string& path) {
  auto location = split_path(path);

  std::shared_ptr<Directory> root(nullptr);
  std::vector<Chat> chats;

  if (location.empty()) {
    root = std::make_shared<Directory>();
    chats = telegram_integration_->searchChats("fs-");
  } else if (location.size() == 1) {
    chats = telegram_integration_->searchChats(location[0]);
  } else if (location.size() == 2) {
    chats = telegram_integration_->searchChats(location[0]);
    for (auto& message : chats[0].messages) {
      std::shared_ptr<File> file(string_to_file(
          message.content, message.attachment, "/" + chats[0].name));
      if (file->name == location[1]) {
        if (file->local_path.empty())
          file->local_path = "/root/customfs/build/src/tdlib/documents" + file->path;
        return file;
      }
    }
    return nullptr;
  }

  for (auto& chat : chats) {
    std::shared_ptr<Directory> dir(new Directory());
    dir->name = chat.name;
    dir->path = "/" + chat.name;

    for (auto& message : chat.messages) {
      std::shared_ptr<File> file(
          string_to_file(message.content, message.attachment, dir->path));
      dir->entities.push_back(file);
    }

    if (location.empty()) {
      root->entities.push_back(dir);
    } else {
      root = dir;
      break;
    }
  }

  return root;
}

void TelegramFileSystemService::create_file(const std::string& path) {
  Message msg;
  auto location = split_path(path);
  msg.content = "path: /" + location.back() + "\nsize: 0b";
  msg.attachment = "/root/customfs/build/src/tdlib/documents" + path;

  auto chats = telegram_integration_->searchChats("fs-");

  auto chat = std::find_if(chats.begin(), chats.end(),
                           [&](Chat chat) { return location[0] == chat.name; });

  telegram_integration_->send_message(chat->id, msg);
}

void TelegramFileSystemService::write_file(File* file) {
  auto chats = telegram_integration_->searchChats("fs-");
  auto location = split_path(file->path);

  auto chat = std::find_if(chats.begin(), chats.end(),
                           [&](Chat chat) { return location[0] == chat.name; });

  for (auto& message : chat->messages) {
    std::shared_ptr<File> message_file(
        string_to_file(message.content, message.attachment, "/" + chat->name));
    if (message_file->path == file->path) {
      
      Message outbound;
      outbound.content = file_to_string(*message_file);
      outbound.attachment = file->local_path;

      telegram_integration_->edit_message(message.id, chat->id, outbound);
      break;
    }
  }
}

void TelegramFileSystemService::move_file(File* from, const std::string& to) {
  File* file_to = from;
  file_to->path = to;

  write_file(file_to);
  delete_file(from);
}

void TelegramFileSystemService::delete_file(File* file) {
  auto chats = telegram_integration_->searchChats("fs-");
  auto location = split_path(file->path);

  auto chat = std::find_if(chats.begin(), chats.end(),
                           [&](Chat chat) { return location[0] == chat.name; });

  for (auto& message : chat->messages) {
    std::shared_ptr<File> message_file(
        string_to_file(message.content, message.attachment, "/" + chat->name));
    if (message_file->path == file->path) {
      telegram_integration_->delete_messages(chat->id, {message.id});
      break;
    }
  }
}

TelegramFileSystemService::~TelegramFileSystemService() {}
