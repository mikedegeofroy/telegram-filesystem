#include "TelegramIntegration.h"

#include <functional>
// #include <future>
#include <iostream>
#include <regex>
#include <sstream>

void TelegramIntegration::restart() {
  stop_event_loop();
  client_manager_.reset();
  client_manager_ = std::make_unique<td::ClientManager>();
  client_id_ = client_manager_->create_client_id();
  send_query(td_api::make_object<td_api::getOption>("version"), {});
}

void TelegramIntegration::send_query(td_api::object_ptr<td_api::Function> f,
                                     std::shared_ptr<Query> query) {
  auto query_id = next_query_id();
  if (query) handlers_.emplace(query_id, query);

  client_manager_->send(client_id_, query_id, std::move(f));
}

void TelegramIntegration::process_response(
    td::ClientManager::Response response) {
  if (!response.object) {
    return;
  }
  if (response.request_id == 0) {
    return process_update(std::move(response.object));
  }
  auto it = handlers_.find(response.request_id);
  if (it != handlers_.end()) {
    it->second->handle(std::move(response.object));
    handlers_.erase(it);
  }
}

void TelegramIntegration::process_update(
    td_api::object_ptr<td_api::Object> update) {
  td_api::downcast_call(
      *update,
      overloaded(
          [this](td_api::updateAuthorizationState &update_authorization_state) {
            authorization_state_ =
                std::move(update_authorization_state.authorization_state_);
            on_authorization_state_update();
          },
          [](auto &update) {}));
}

auto TelegramIntegration::create_authentication_query_handler() {
  return [this, id = authentication_query_id_](Object object) {
    if (id == authentication_query_id_) {
      check_authentication_error(std::move(object));
    }
  };
}

void TelegramIntegration::on_authorization_state_update() {
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
            std::shared_ptr<Query> query(
                new Query(create_authentication_query_handler()));
            send_query(
                td_api::make_object<td_api::setAuthenticationPhoneNumber>(
                    phone_number, nullptr),
                query);
          },
          [this](td_api::authorizationStateWaitEmailAddress &) {
            std::cout << "Enter email address: " << std::flush;
            std::string email_address;
            std::cin >> email_address;
            std::shared_ptr<Query> query(
                new Query(create_authentication_query_handler()));
            send_query(
                td_api::make_object<td_api::setAuthenticationEmailAddress>(
                    email_address),
                query);
          },
          [this](td_api::authorizationStateWaitEmailCode &) {
            std::cout << "Enter email authentication code: " << std::flush;
            std::string code;
            std::cin >> code;
            std::shared_ptr<Query> query(
                new Query(create_authentication_query_handler()));
            send_query(
                td_api::make_object<td_api::checkAuthenticationEmailCode>(
                    td_api::make_object<td_api::emailAddressAuthenticationCode>(
                        code)),
                query);
          },
          [this](td_api::authorizationStateWaitCode &) {
            std::cout << "Enter authentication code: " << std::flush;
            std::string code;
            std::cin >> code;
            std::shared_ptr<Query> query(
                new Query(create_authentication_query_handler()));
            send_query(
                td_api::make_object<td_api::checkAuthenticationCode>(code),
                query);
          },
          [this](td_api::authorizationStateWaitRegistration &) {
            std::string first_name;
            std::string last_name;
            std::cout << "Enter your first name: " << std::flush;
            std::cin >> first_name;
            std::cout << "Enter your last name: " << std::flush;
            std::cin >> last_name;

            std::shared_ptr<Query> query(
                new Query(create_authentication_query_handler()));
            send_query(td_api::make_object<td_api::registerUser>(
                           first_name, last_name, false),
                       query);
          },
          [this](td_api::authorizationStateWaitPassword &) {
            std::cout << "Enter authentication password: " << std::flush;
            std::string password;
            std::getline(std::cin, password);
            std::shared_ptr<Query> query(
                new Query(create_authentication_query_handler()));
            send_query(td_api::make_object<td_api::checkAuthenticationPassword>(
                           password),
                       query);
          },
          [this](td_api::authorizationStateWaitOtherDeviceConfirmation &state) {
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
            std::shared_ptr<Query> query(
                new Query(create_authentication_query_handler()));
            send_query(std::move(request), query);
          }));
}

void TelegramIntegration::check_authentication_error(Object object) {
  if (object->get_id() == td_api::error::ID) {
    auto error = td::move_tl_object_as<td_api::error>(object);
    std::cout << "Error: " << to_string(error) << std::flush;
    on_authorization_state_update();
  }
}

std::uint64_t TelegramIntegration::next_query_id() {
  return ++current_query_id_;
}

TelegramIntegration::TelegramIntegration() {
  td::ClientManager::execute(
      td_api::make_object<td_api::setLogVerbosityLevel>(1));
  client_manager_ = std::make_unique<td::ClientManager>();
  client_id_ = client_manager_->create_client_id();
  send_query(td_api::make_object<td_api::getOption>("version"), {});
}

Chat TelegramIntegration::getChat(int64_t chat_id) {
  Chat result;

  std::shared_ptr<Query> query(new Query([&](Object object) {
    if (object->get_id() == td_api::error::ID) {
      return;
    }
    auto chat = td::move_tl_object_as<td_api::chat>(object);

    result.name = chat->title_;
    result.messages = getChatMessages(chat_id);
  }));

  send_query(td_api::make_object<td_api::getChat>(chat_id), query);
  query->wait();

  return result;
}

std::vector<Chat> TelegramIntegration::searchChats(const std::string &string) {
  std::vector<Chat> results;

  std::shared_ptr<Query> query(new Query([&](Object object) {
    if (object->get_id() == td_api::error::ID) {
      return;
    }
    auto chats = td::move_tl_object_as<td_api::chats>(object);

    for (auto chat_id : chats->chat_ids_) {
      if (chat_id > 0) continue;
      Chat test = getChat(chat_id);
      results.push_back(test);
    }
  }));

  send_query(td_api::make_object<td_api::searchChatsOnServer>(string, 100),
             query);

  query->wait();

  return results;
}

std::vector<Message> TelegramIntegration::getChatMessages(int64_t chat_id,
                                                          int64_t from_id) {
  std::vector<Message> results;

  std::shared_ptr<Query> query(new Query([&](Object object) {
    if (object->get_id() == td_api::error::ID) {
      return;
    }

    auto messages = td::move_tl_object_as<td_api::messages>(object);
    bool are_messages_empty = messages->messages_.empty();

    for (auto &message : messages->messages_) {
      int64_t id = message->id_;
      std::string text;
      std::string document;

      if (message->content_->get_id() == td_api::messageDocument::ID) {
        auto &document_message =
            static_cast<td_api::messageDocument &>(*message->content_);
        text = document_message.caption_->text_;
        document = download_file(document_message.document_->document_->id_);
      } else if (message->content_->get_id() == td_api::messageText::ID) {
        auto &text_message =
            static_cast<td_api::messageText &>(*message->content_);
        text = text_message.text_->text_;
      }

      // Create the message object only if text or document is present
      if (!text.empty() || !document.empty()) {
        Message my_message = {
            .id = id, .content = text, .attachment = document};
        results.push_back(my_message);
      }
    }

    if (!are_messages_empty) {
      std::vector<Message> remaining =
          getChatMessages(chat_id, messages->messages_.back()->id_);
      results.insert(results.end(), remaining.begin(), remaining.end());
    }
  }));

  send_query(td_api::make_object<td_api::getChatHistory>(chat_id, from_id, 0,
                                                         100, false),
             query);
  query->wait();

  return results;
}

std::string TelegramIntegration::download_file(int32_t file_id) {
  std::string local_path;

  std::shared_ptr<Query> query(new Query([&](Object object) {
    if (object->get_id() == td_api::error::ID) {
      return;
    }

    auto file = td::move_tl_object_as<td_api::file>(object);
    local_path = file->local_->path_;
  }));

  send_query(td_api::make_object<td_api::downloadFile>(file_id, 1, 0, 0, true),
             query);

  query->wait();

  return local_path;
}

void TelegramIntegration::send_message(Chat chat) {}

void TelegramIntegration::start_event_loop() {
  event_loop_thread_ = std::thread([this]() {
    while (running_) {
      auto response = client_manager_->receive(10);
      if (response.object) {
        process_response(std::move(response));
      }
    }
  });
}

void TelegramIntegration::stop_event_loop() {
  running_ = false;
  if (event_loop_thread_.joinable()) {
    event_loop_thread_.join();
  }
}

void TelegramIntegration::auth_loop() {
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

TelegramIntegration::~TelegramIntegration() { stop_event_loop(); }
