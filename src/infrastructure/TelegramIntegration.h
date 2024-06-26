#pragma once
#include <td/telegram/Client.h>
#include <td/telegram/td_api.h>

#include <cstdint>
#include <map>
#include <memory>
#include <td/telegram/td_api.hpp>
#include <thread>

#include "../application/abstractions/ITelegramIntegration.h"
#include "./Query.h"

// overloaded
namespace detail {
template <class... Fs>
struct overload;

template <class F>
struct overload<F> : public F {
  explicit overload(F f) : F(f) {}
};
template <class F, class... Fs>
struct overload<F, Fs...> : public overload<F>, public overload<Fs...> {
  overload(F f, Fs... fs) : overload<F>(f), overload<Fs...>(fs...) {}
  using overload<F>::operator();
  using overload<Fs...>::operator();
};
}  // namespace detail

template <class... F>
auto overloaded(F... f) {
  return detail::overload<F...>(f...);
}

namespace td_api = td::td_api;
class TelegramIntegration : public ITelegramIntegration {
 private:
  using Object = td_api::object_ptr<td_api::Object>;
  std::unique_ptr<td::ClientManager> client_manager_;
  std::int32_t client_id_{0};

  td_api::object_ptr<td_api::AuthorizationState> authorization_state_;
  std::uint64_t current_query_id_{0};
  std::uint64_t authentication_query_id_{0};
  bool are_authorized_{false};
  bool need_restart_{false};

  std::map<std::uint64_t, std::shared_ptr<Query>> handlers_;
  std::map<std::int64_t, td_api::object_ptr<td_api::user>> users_;

  std::map<std::int64_t, std::string> chat_title_;

  std::thread event_loop_thread_;
  bool running_{true};

  void restart();

  void send_query(td_api::object_ptr<td_api::Function> f,
                  std::shared_ptr<Query> query);

  void process_response(td::ClientManager::Response response);

  void process_update(td_api::object_ptr<td_api::Object> update);

  auto create_authentication_query_handler();

  void on_authorization_state_update();

  void check_authentication_error(Object object);

  std::uint64_t next_query_id();

 public:
  TelegramIntegration();

  Chat getChat(int64_t chat_id);

  std::vector<Chat> searchChats(const std::string &string);

  std::vector<Message> getChatMessages(int64_t chat_id, int64_t from_id = 0);

  std::string download_file(int64_t file_id);

  void send_message(std::int64_t chat_id, Message content);

  void edit_message(std::int64_t message_id, std::int64_t chat_id,
                    Message content);

  void delete_messages(std::int64_t chat_id,
                       std::vector<std::int64_t> message_ids);

  void start_event_loop();

  void stop_event_loop();

  void auth_loop();

  ~TelegramIntegration();
};