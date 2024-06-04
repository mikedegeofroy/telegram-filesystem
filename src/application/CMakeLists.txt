add_library(telegram-application TelegramFileSystemService.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE ${Td_INCLUDE_DIRS})