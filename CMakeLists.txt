# Copyright (c) Maia

cmake_minimum_required(VERSION 3.31)

project(
  telecodex
  VERSION 0.0.1
  LANGUAGES
    C
    CXX
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

option(
  TC_COMPILE_COMMANDS
  "Create a symbolic link for compile_commands.json"
  OFF
)
if(TC_COMPILE_COMMANDS)
  include(compile_commands_link)
endif()

if(WIN32)
  add_compile_definitions(
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    NOGDI
    NOUSER
  )
endif()

enable_testing()
include(GoogleTest)

include_directories("${CMAKE_SOURCE_DIR}/src")

add_subdirectory(src/tc)
