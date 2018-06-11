cmake_minimum_required(VERSION 2.6)

project(righack)

set(CMAKE_C_FLAGS "-std=gnu99")
add_definitions(-Werror -Wall -Wextra)

add_executable(righack
    main.c guts.c log.c util.c
    runloop.c configfile.c module.c driver.c
    drivers/vserial.c
    modules/test.c
    external/toml.c external/autodie.c
)
TARGET_LINK_LIBRARIES(righack hamlib)
TARGET_LINK_LIBRARIES(righack uv)
TARGET_LINK_LIBRARIES(righack pthread)
TARGET_LINK_LIBRARIES(righack util)
