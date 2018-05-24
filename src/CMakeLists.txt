cmake_minimum_required(VERSION 2.6)

project(righack)
add_definitions(-Werror -Wall -Wextra)

add_executable(righack main.c util.c vserial.c runloop.c)
TARGET_LINK_LIBRARIES(righack util)