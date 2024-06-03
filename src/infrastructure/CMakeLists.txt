add_library(telegram-integration TelegramIntegration.h)

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/external/td")

target_link_libraries(telegram-integration Td::TdStatic)
target_include_directories(telegram-integration PUBLIC ${Td_INCLUDE_DIRS})