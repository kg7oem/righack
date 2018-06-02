cmake_minimum_required(VERSION 2.6)

project(righack)

set(CMAKE_C_FLAGS "-std=gnu99")
add_definitions(-Werror -Wall -Wextra)

add_executable(righack
    main.c util.c vserial.c runloop.c config.c
    drivers/test.c
    external/toml.c
)
TARGET_LINK_LIBRARIES(righack util)
TARGET_LINK_LIBRARIES(righack pthread)