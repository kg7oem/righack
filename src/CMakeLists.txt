cmake_minimum_required(VERSION 2.6)

project(righack)

add_definitions(-Werror -Wall -Wextra)

add_executable(righack
    main.c guts.c log.c util.c vserial.c runloop.c
    configfile.c
    drivers/test.c drivers/ptt.c
    external/toml.c external/autodie.c
)
set_property(TARGET righack PROPERTY C_STANDARD 99)

TARGET_LINK_LIBRARIES(righack util)
TARGET_LINK_LIBRARIES(righack pthread)
TARGET_LINK_LIBRARIES(righack hamlib)
