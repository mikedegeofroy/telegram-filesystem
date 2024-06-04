#pragma once
#include <string>
#include <vector>

#include "Message.h"

struct Chat {
  std::string name;
  std::vector<Message> messages;
};