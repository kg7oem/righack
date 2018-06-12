cmake_minimum_required(VERSION 2.6)

project(righack)

add_definitions(-Werror -Wall -Wextra)

find_package (Threads)

find_library(HAMLIB_LIBRARY NAMES hamlib)
find_library(UV_LIBRARY NAMES uv)

add_executable(righack
    main.c guts.c log.c util.c
    runloop.c configfile.c module.c driver.c
    drivers/vserial.c
    modules/test.c
    external/toml.c external/autodie.c
)

set_property(TARGET righack PROPERTY C_STANDARD 99)

target_link_libraries(righack ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(righack util)
target_link_libraries(righack ${HAMLIB_LIBRARY})
target_link_libraries(righack ${UV_LIBRARY})
