project(mydll CXX)
cmake_minimum_required(VERSION 3.5)
add_library(mydll SHARED jvm.cpp)
include_directories("C:/Program Files (x86)/Java/jdk1.8.0_91/include" "C:/Program Files (x86)/Java/jdk1.8.0_91/include/win32")
add_custom_command(TARGET mydll POST_BUILD
    COMMAND echo command test
)
