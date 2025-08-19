# Copyright (c) Maia

find_package(Boost COMPONENTS Process REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(scn CONFIG REQUIRED)

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
