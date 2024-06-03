#pragma once
#include <string>
#include <vector>

struct Message {
  std::int64_t id;
  std::string content;
  std::string attachment;
};