cmake_minimum_required(VERSION 3.21)

project(redis
    VERSION 0.0.10
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
    utils/message.cpp
    utils/parse.cpp
    core/net/server.cpp
    core/net/net_utils.cpp
    core/net/client.cpp
    core/cli/resp.cpp
    core/command/store.cpp
    core/command/basicmd.cpp
    core/command/listcmd.cpp
    core/command/streamcmd.cpp
)

target_include_directories(redis
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)