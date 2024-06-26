#pragma once
#include <string>
#include <vector>

#include "Message.h"

struct Chat {
  std::int64_t id;
  std::string name;
  std::vector<Message> messages;
};