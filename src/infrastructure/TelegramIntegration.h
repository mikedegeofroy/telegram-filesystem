#pragma once
#include <td/telegram/Client.h>
#include <td/telegram/td_api.h>

#include <cstdint>
#include <functional>
#include <future>
#include <iostream>
#include <map>
#include <memory>
#include <regex>
#include <sstream>
#include <string>
#include <td/telegram/td_api.hpp>
#include <thread>
#include <vector>

#include "../application/abstractions/ITelegramIntegration.h"

// Simple single-threaded example of TDLib usage.
// Real world programs should use separate thread for the user input.
// Example includes user authentication, receiving updates, getting chat list
// and sending text messages.

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
  bool are_authorized_{false};
  bool need_restart_{false};
  std::uint64_t current_query_id_{0};
  std::uint64_t authentication_query_id_{0};

  std::map<std::uint64_t, std::function<void(Object)>> handlers_;

  std::map<std::int64_t, td_api::object_ptr<td_api::user>> users_;

  std::map<std::int64_t, std::string> chat_title_;

  std::thread event_loop_thread_;
  bool running_{true};

  void restart() {
    stop_event_loop();
    client_manager_.reset();
    client_manager_ = std::make_unique<td::ClientManager>();
    client_id_ = client_manager_->create_client_id();
    send_query(td_api::make_object<td_api::getOption>("version"), {});
  }

  void send_query(td_api::object_ptr<td_api::Function> f,
                  std::function<void(Object)> handler) {
    auto query_id = next_query_id();
    if (handler) {
      handlers_.emplace(query_id, std::move(handler));
    }
    client_manager_->send(client_id_, query_id, std::move(f));
  }

  void process_response(td::ClientManager::Response response) {
    std::cout << "Received response with id: " << response.request_id << "\n";
    if (!response.object) {
      std::cout << "Received empty response object.\n";
      return;
    }
    if (response.request_id == 0) {
      return process_update(std::move(response.object));
    }
    std::cout << "started handling this fucker\n";
    auto it = handlers_.find(response.request_id);
    if (it != handlers_.end()) {
      std::cout << "found handler\n";
      it->second(std::move(response.object));
      handlers_.erase(it);
    }
    std::cout << "handled this fucker\n";
  }

  std::string get_user_name(std::int64_t user_id) const {
    auto it = users_.find(user_id);
    if (it == users_.end()) {
      return "unknown user";
    }
    return it->second->first_name_ + " " + it->second->last_name_;
  }

  std::string get_chat_title(std::int64_t chat_id) const {
    auto it = chat_title_.find(chat_id);
    if (it == chat_title_.end()) {
      return "unknown chat";
    }
    return it->second;
  }

  void process_update(td_api::object_ptr<td_api::Object> update) {
    std::cout << "Processing update\n";
    td_api::downcast_call(
        *update,
        overloaded(
            [this](
                td_api::updateAuthorizationState &update_authorization_state) {
              authorization_state_ =
                  std::move(update_authorization_state.authorization_state_);
              on_authorization_state_update();
            },
            [this](td_api::updateNewChat &update_new_chat) {
              chat_title_[update_new_chat.chat_->id_] =
                  update_new_chat.chat_->title_;
            },
            [this](td_api::updateChatTitle &update_chat_title) {
              chat_title_[update_chat_title.chat_id_] =
                  update_chat_title.title_;
            },
            [this](td_api::updateUser &update_user) {
              auto user_id = update_user.user_->id_;
              users_[user_id] = std::move(update_user.user_);
            },
            [this](td_api::updateNewMessage &update_new_message) {
              auto chat_id = update_new_message.message_->chat_id_;
              std::string sender_name;
              td_api::downcast_call(
                  *update_new_message.message_->sender_id_,
                  overloaded(
                      [this, &sender_name](td_api::messageSenderUser &user) {
                        sender_name = get_user_name(user.user_id_);
                      },
                      [this, &sender_name](td_api::messageSenderChat &chat) {
                        sender_name = get_chat_title(chat.chat_id_);
                      }));
              std::string text;
              if (update_new_message.message_->content_->get_id() ==
                  td_api::messageText::ID) {
                text = static_cast<td_api::messageText &>(
                           *update_new_message.message_->content_)
                           .text_->text_;
              }
              std::cout << "Receive message: [chat_id:" << chat_id
                        << "] [from:" << sender_name << "] [" << text << "]"
                        << std::endl;
            },
            [](auto &update) {}));
  }

  auto create_authentication_query_handler() {
    return [this, id = authentication_query_id_](Object object) {
      if (id == authentication_query_id_) {
        check_authentication_error(std::move(object));
      }
    };
  }

  void on_authorization_state_update() {
    authentication_query_id_++;
    td_api::downcast_call(
        *authorization_state_,
        overloaded(
            [this](td_api::authorizationStateReady &) {
              are_authorized_ = true;
              std::cout << "Authorization is completed" << std::endl;
            },
            [this](td_api::authorizationStateLoggingOut &) {
              are_authorized_ = false;
              std::cout << "Logging out" << std::endl;
            },
            [this](td_api::authorizationStateClosing &) {
              std::cout << "Closing" << std::endl;
            },
            [this](td_api::authorizationStateClosed &) {
              are_authorized_ = false;
              need_restart_ = true;
              std::cout << "Terminated" << std::endl;
            },
            [this](td_api::authorizationStateWaitPhoneNumber &) {
              std::cout << "Enter phone number: " << std::flush;
              std::string phone_number;
              std::cin >> phone_number;
              send_query(
                  td_api::make_object<td_api::setAuthenticationPhoneNumber>(
                      phone_number, nullptr),
                  create_authentication_query_handler());
            },
            [this](td_api::authorizationStateWaitEmailAddress &) {
              std::cout << "Enter email address: " << std::flush;
              std::string email_address;
              std::cin >> email_address;
              send_query(
                  td_api::make_object<td_api::setAuthenticationEmailAddress>(
                      email_address),
                  create_authentication_query_handler());
            },
            [this](td_api::authorizationStateWaitEmailCode &) {
              std::cout << "Enter email authentication code: " << std::flush;
              std::string code;
              std::cin >> code;
              send_query(
                  td_api::make_object<td_api::checkAuthenticationEmailCode>(
                      td_api::make_object<
                          td_api::emailAddressAuthenticationCode>(code)),
                  create_authentication_query_handler());
            },
            [this](td_api::authorizationStateWaitCode &) {
              std::cout << "Enter authentication code: " << std::flush;
              std::string code;
              std::cin >> code;
              send_query(
                  td_api::make_object<td_api::checkAuthenticationCode>(code),
                  create_authentication_query_handler());
            },
            [this](td_api::authorizationStateWaitRegistration &) {
              std::string first_name;
              std::string last_name;
              std::cout << "Enter your first name: " << std::flush;
              std::cin >> first_name;
              std::cout << "Enter your last name: " << std::flush;
              std::cin >> last_name;
              send_query(td_api::make_object<td_api::registerUser>(
                             first_name, last_name, false),
                         create_authentication_query_handler());
            },
            [this](td_api::authorizationStateWaitPassword &) {
              std::cout << "Enter authentication password: " << std::flush;
              std::string password;
              std::getline(std::cin, password);
              send_query(
                  td_api::make_object<td_api::checkAuthenticationPassword>(
                      password),
                  create_authentication_query_handler());
            },
            [this](
                td_api::authorizationStateWaitOtherDeviceConfirmation &state) {
              std::cout << "Confirm this login link on another device: "
                        << state.link_ << std::endl;
            },
            [this](td_api::authorizationStateWaitTdlibParameters &) {
              auto request = td_api::make_object<td_api::setTdlibParameters>();
              request->database_directory_ = "tdlib";
              request->use_message_database_ = true;
              request->use_secret_chats_ = true;
              request->api_id_ = 94575;
              request->api_hash_ = "a3406de8d171bb422bb6ddf3bbd800e2";
              request->system_language_code_ = "en";
              request->device_model_ = "Desktop";
              request->application_version_ = "1.0";
              send_query(std::move(request),
                         create_authentication_query_handler());
            }));
  }

  void check_authentication_error(Object object) {
    if (object->get_id() == td_api::error::ID) {
      auto error = td::move_tl_object_as<td_api::error>(object);
      std::cout << "Error: " << to_string(error) << std::flush;
      on_authorization_state_update();
    }
  }

  std::uint64_t next_query_id() { return ++current_query_id_; }

 public:
  TelegramIntegration() {
    td::ClientManager::execute(
        td_api::make_object<td_api::setLogVerbosityLevel>(1));
    client_manager_ = std::make_unique<td::ClientManager>();
    client_id_ = client_manager_->create_client_id();
    send_query(td_api::make_object<td_api::getOption>("version"), {});
  }

  std::vector<Chat> searchChats(const std::string &query) {
    std::vector<Chat> results;
    std::promise<void> promise;
    auto future = promise.get_future();

    send_query(td_api::make_object<td_api::getChats>(nullptr, 100),
               [this, &results, &promise](Object object) {
                 if (object->get_id() == td_api::error::ID) {
                   promise.set_value();
                   return;
                 }
                 auto chats = td::move_tl_object_as<td_api::chats>(object);
                 bool kill = false;
                 for (auto chat_id : chats->chat_ids_) {
                   kill = true;
                   Chat chat = {.name = std::to_string(chat_id),
                                .messages = getChatMessages(chat_id)};
                   results.push_back(chat);
                   if (kill) break;
                 }
                 promise.set_value();
               });

    future.wait();

    return results;
  };

  std::vector<Message> getChatMessages(int64_t chat_id) {
    std::cout << "Getting chat messages for " << chat_id << "\n";
    std::vector<Message> results;

    Message message = {.content = "fuck you"};

    results.push_back(message);
    results.push_back(message);
    results.push_back(message);

    return results;
  }

  std::vector<Message> getChatMessages(int64_t chat_id, int64_t from_message_id,
                                       int32_t limit) {
    std::vector<Message> results;
    std::promise<void> promise;
    auto future = promise.get_future();

    send_query(
        td_api::make_object<td_api::getChatHistory>(chat_id, from_message_id, 0,
                                                    limit, false),
        [this, &results, &promise](Object object) mutable {
          if (object->get_id() == td_api::error::ID) {
            auto error = td::move_tl_object_as<td_api::error>(object);
            std::cerr << "Error getting chat history: " << error->message_
                      << "\n";
            promise.set_value();
            return;
          }

          auto history = td::move_tl_object_as<td_api::messages>(object);
          for (auto &message : history->messages_) {
            std::string text;
            if (message->content_->get_id() == td_api::messageText::ID) {
              text = static_cast<td_api::messageText &>(*message->content_)
                         .text_->text_;
              Message my_message = {.content = text};
              results.push_back(my_message);
            }
          }
          promise.set_value();
        });

    future.wait();  // Wait for the async operation to complete

    return results;
  }

  void start_event_loop() {
    event_loop_thread_ = std::thread([this]() {
      while (running_) {
        auto response = client_manager_->receive(10);
        std::cout << "Event loop iteration\n";
        if (response.object) {
          process_response(std::move(response));
        }
        std::cout << running_ << "\n";
      }
    });
  }

  void stop_event_loop() {
    running_ = false;
    if (event_loop_thread_.joinable()) {
      event_loop_thread_.join();
    }
  }

  void auth_loop() {
    bool auth_needed = true;

    while (auth_needed) {
      if (need_restart_) {
        restart();
      } else if (!are_authorized_) {
        process_response(client_manager_->receive(10));
      } else {
        auth_needed = false;
      }
    }
  }

  ~TelegramIntegration() { stop_event_loop(); }
};