#include <td/telegram/td_api.h>
#include <td/telegram/td_api.hpp>

#include <future>

#include "../application/models/Chat.h"

namespace td_api = td::td_api;

class Query {
 private:
  using Object = td_api::object_ptr<td_api::Object>;
  std::promise<void> promise_;
  std::future<void> future_;
  std::thread thread_;
  std::function<void(Object)> task_;
  Object object_;

  void run_task() {
    task_(std::move(object_));
    promise_.set_value();
  }

 public:
  Query(std::function<void(Object)> task)
      : promise_(),
        future_(promise_.get_future()),
        task_(task),
        object_(nullptr) {}

  ~Query() {
    if (thread_.joinable()) {
      thread_.join();
    }
  }

  void handle(Object obj) {
    object_ = std::move(obj);
    thread_ = std::thread(&Query::run_task, this);
  }

  void wait() { future_.wait(); }
};