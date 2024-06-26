#pragma once
#include <string>
#include <vector>

#include "../models/Chat.h"

struct ITelegramIntegration {
  virtual std::vector<Chat> searchChats(const std::string& query) = 0;
  virtual void start_event_loop() = 0;
  virtual void stop_event_loop() = 0;
  virtual void auth_loop() = 0;
  virtual std::string download_file(std::int64_t file_id) = 0;
  virtual void send_message(std::int64_t chat_id, Message content) = 0;
  virtual void edit_message(std::int64_t message_id, std::int64_t chat_id,
                            Message content) = 0;
  virtual void delete_messages(std::int64_t chat_id,
                               std::vector<std::int64_t> message_ids) = 0;
};