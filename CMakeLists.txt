cmake_minimum_required(VERSION 3.21)

project(VarsDB
    VERSION 0.0.11
    LANGUAGES CXX
)

if(NOT CMAKE_BUILD_TYPE) 
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/out)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_executable(VarsDB
    entry/main.cpp
    internal/message/message_factory.cpp
    internal/tcp/server.cpp
    internal/tcp/net_utils.cpp
    internal/tcp/client.cpp
    internal/resp/resp.cpp
    internal/resp/resp_parser.cpp
    internal/memory/memory_store.cpp
    internal/cmd/general.cpp
    internal/cmd/list.cpp
    internal/cmd/stream.cpp
)

target_include_directories(VarsDB
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)