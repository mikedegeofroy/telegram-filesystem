add_library(telegram-integration TelegramIntegration.cpp)

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/external/td")

target_include_directories(telegram-integration PUBLIC ${Td_INCLUDE_DIRS})
target_link_libraries(telegram-integration Td::TdStatic)