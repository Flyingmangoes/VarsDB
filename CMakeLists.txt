cmake_minimum_required(VERSION 3.21)

project(redis
    VERSION 0.0.2
    LANGUAGES CXX
)

if(NOT CMAKE_BUILD_TYPE) 
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/out)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_executable(redis
    core/main.cpp
    core/server/server.cpp
    core/server/client.cpp
    core/cli/resp.cpp
    core/command/all_cmd.cpp
    core/command/store.cpp
    utils/message.cpp
    utils/parse.cpp
)

target_include_directories(redis
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)