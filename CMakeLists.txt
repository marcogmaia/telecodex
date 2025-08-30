# Copyright (c) Maia

cmake_minimum_required(VERSION 3.31)

project(
  teletex
  VERSION 0.0.1
  LANGUAGES
    C
    CXX
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

enable_testing()
include(GoogleTest)

include_directories("${CMAKE_SOURCE_DIR}/src")

add_subdirectory(external)

add_subdirectory(src/tt)
