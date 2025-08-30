# Copyright (c) Maia

add_subdirectory(ttgen)

find_package(Boost COMPONENTS Process REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(scn CONFIG REQUIRED)

if(WIN32)
  add_compile_definitions(
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    NOGDI
    NOUSER
  )
endif()

add_library(teletex.lib)
target_sources(
  teletex.lib
  PRIVATE
    file_finder.cpp
    file_finder.h
)
target_link_libraries(
  teletex.lib
  PUBLIC
    Boost::process
    nlohmann_json::nlohmann_json
    scn::scn
)

if(WIN32)
  target_compile_definitions(teletex.lib PUBLIC _WIN32_WINNT=0x0A00)
endif()

add_executable(teletex)
target_sources(teletex PRIVATE main.cpp)
target_link_libraries(
  teletex
  PRIVATE
    teletex.lib
    lsp
)

# add_executable(teletex_test)
# target_sources(teletex_test PRIVATE server_test.cpp)
# target_link_libraries(
#   teletex_test
#   PRIVATE
#     GTest::gtest_main
#     teletex.lib
# )
# gtest_discover_tests(teletex_test DISCOVERY_TIMEOUT 60)

# set(METAMODEL_INPUT "${CMAKE_CURRENT_SOURCE_DIR}/meta/metamodel.json")

# file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/meta")

# set(
#   LSP_GENERATED_SOURCES
#   "${CMAKE_CURRENT_BINARY_DIR}/meta/messages.h"
#   "${CMAKE_CURRENT_BINARY_DIR}/meta/types.cpp"
#   "${CMAKE_CURRENT_BINARY_DIR}/meta/types.h"
# )

# add_custom_command(
#   OUTPUT
#     ${LSP_GENERATED_SOURCES}
#   COMMAND
#     lspgen "${METAMODEL_INPUT}"
#   DEPENDS
#     ${METAMODEL_INPUT}
#   WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/meta"
#   COMMENT "Generating lsp types from ${METAMODEL_INPUT} ..."
#   VERBATIM
# )

# target_sources(
#   test_app
#   PRIVATE
#     test_app.cpp
#     ${LSP_GENERATED_SOURCES}
# )

# target_include_directories(test_app PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

# if(MSVC)
#   target_compile_options(test_app PRIVATE /bigobj)
# endif()

# add_dependencies(test_app lspgen)
