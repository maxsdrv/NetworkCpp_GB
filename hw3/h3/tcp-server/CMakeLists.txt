cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(tcp-server C CXX)

set(SOURCE)
set(HEADER)

list(APPEND SOURCE
        main.cpp
        TcpServer.cpp)

list(APPEND HEADER
        TcpServer.h)

source_group(source FILES ${HEADER}} ${SOURCE})

add_executable("${PROJECT_NAME}" ${HEADER} ${SOURCE})
target_link_libraries("${PROJECT_NAME}" socket-wrapper)

if(WIN32)
  target_link_libraries("${PROJECT_NAME}" wsock32 ws2_32)
endif()

