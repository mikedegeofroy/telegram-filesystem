add_library(telegram-application TelegramFileSystemService.cpp models/Chat.h)

target_link_libraries(telegram-application telegram-integration)
target_include_directories(${PROJECT_NAME} PRIVATE ${Td_INCLUDE_DIRS})