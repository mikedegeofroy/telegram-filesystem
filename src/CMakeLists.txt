add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} ${FUSE_LIBRARIES})
# target_link_libraries(${PROJECT_NAME} telegram-integration)
target_compile_definitions(${PROJECT_NAME} PRIVATE -D_FILE_OFFSET_BITS=64)

include_directories(${FUSE_INCLUDE_DIRS})

add_subdirectory(application)
target_link_libraries(${PROJECT_NAME} telegram-application)

add_subdirectory(infrastructure)
target_link_libraries(${PROJECT_NAME} telegram-integration)
