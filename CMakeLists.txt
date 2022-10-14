cmake_minimum_required(VERSION 3.0.0)
project(SerialMessageParser VERSION 0.1.0)

add_executable(SerialMessageParser SerialMessageParser.cpp HelperFunctions.h HelperFunctions.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)