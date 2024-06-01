#pragma once
#include <string>
#include <vector>

#include "../models/Chat.h"

struct ITelegramIntegration {
  virtual std::vector<Chat> searchChats(const std::string &query) = 0;
  virtual void start_event_loop() = 0;
  virtual void stop_event_loop() = 0;
  virtual void auth_loop() = 0;
};