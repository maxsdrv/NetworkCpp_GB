cmake_minimum_required(VERSION 3.21)

project(server)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost 1.71 REQUIRED COMPONENTS)

FILE(GLOB_RECURSE SRC_CPP ${PROJECT_SOURCE_DIR}/src *.cpp)
FILE(GLOB_RECURSE SRC_HPP ${PROJECT_SOURCE_DIR}/src *.hpp)
FILE(GLOB_RECURSE SRC_H ${PROJECT_SOURCE_DIR}/src *.h)


add_executable(server ${SRC_CPP} ${SRC_HPP} ${SRC_H})


target_link_libraries(server ${Boost_LIBRARIES} socket_wrapper)

