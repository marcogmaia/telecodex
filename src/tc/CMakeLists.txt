# Copyright (c) Maia

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

add_library(telecodex.lib)
target_sources(
  telecodex.lib
  PRIVATE
    file_finder.cpp
    file_finder.h
    server.cpp
    server.h
)
target_link_libraries(
  telecodex.lib
  PUBLIC
    Boost::process
    nlohmann_json::nlohmann_json
    scn::scn
)

# Check if we can determine if it is Windows 7, Windows 10, etc.
if(WIN32)
  target_compile_definitions(telecodex.lib PUBLIC _WIN32_WINNT=0x0A00)
endif()

add_executable(telecodex)
target_sources(telecodex PRIVATE main.cpp)
target_link_libraries(telecodex PRIVATE telecodex.lib)

add_executable(telecodex_test)
target_sources(telecodex_test PRIVATE server_test.cpp)
target_link_libraries(
  telecodex_test
  PRIVATE
    GTest::gtest_main
    telecodex.lib
)

gtest_discover_tests(telecodex_test DISCOVERY_TIMEOUT 60)

add_executable(test_app)

target_link_libraries(
  test_app
  PRIVATE
    telecodex.lib
    lsp
)

set(METAMODEL_INPUT "${CMAKE_CURRENT_SOURCE_DIR}/meta/metamodel.json")

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/meta")

set(
  LSP_GENERATED_SOURCES
  "${CMAKE_CURRENT_BINARY_DIR}/meta/messages.h"
  "${CMAKE_CURRENT_BINARY_DIR}/meta/types.cpp"
  "${CMAKE_CURRENT_BINARY_DIR}/meta/types.h"
)

add_custom_command(
  OUTPUT
    ${LSP_GENERATED_SOURCES}
  COMMAND
    lspgen "${METAMODEL_INPUT}"
  DEPENDS
    ${METAMODEL_INPUT}
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/meta"
  COMMENT "Generating lsp types from ${METAMODEL_INPUT} ..."
  VERBATIM
)

target_sources(
  test_app
  PRIVATE
    test_app.cpp
    ${LSP_GENERATED_SOURCES}
)

target_include_directories(test_app PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/meta")

add_dependencies(test_app lspgen)
