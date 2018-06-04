cmake_minimum_required(VERSION 2.6)
include(CHECKSYMBOLEXISTS)

project(righack)

add_definitions(-Werror -Wall -Wextra)

# Look for openpty, which exists in different places
CHECK_SYMBOL_EXISTS(openpty pty.h HAVE_PTY_H)
if (HAVE_PTY_H)
 MESSAGE(STATUS "using pty.h")
 add_definitions(-DHAVE_PTY_H=1)
else()
 CHECK_SYMBOL_EXISTS(openpty util.h HAVE_UTIL_H)
 if (HAVE_UTIL_H)
   MESSAGE(STATUS "using util.h")
   add_definitions(-DHAVE_UTIL_H=1)
 else()
   MESSAGE(FATAL_ERROR "can't find openpty")
 endif()
endif()

find_package (Threads)
find_library(HAMLIB_LIBRARY NAMES hamlib)

add_executable(righack
    main.c guts.c log.c util.c vserial.c runloop.c
    configfile.c
    drivers/test.c drivers/ptt.c
    external/toml.c external/autodie.c
)
set_property(TARGET righack PROPERTY C_STANDARD 99)

target_link_libraries(righack util)
target_link_libraries(righack ${HAMLIB_LIBRARY})
target_link_libraries(righack ${CMAKE_THREAD_LIBS_INIT})
