cmake_minimum_required(VERSION 3.10)

project(call-intercepter C CXX)

#set(CMAKE_COMPILER_IS_GNUCC TRUE)

set(HWFiles)

list(APPEND HWFiles
        SFileLogger.h
        SFileLogger.cpp)

add_library("${PROJECT_NAME}" SHARED call_intercepter.cpp ${HWFiles})
target_link_libraries("${PROJECT_NAME}" dl)
set_property(TARGET "${PROJECT_NAME}" PROPERTY POSITION_INDEPENDENT_CODE ON)

