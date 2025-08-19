# Copyright (c) Maia

find_package(Boost COMPONENTS Process REQUIRED)

add_executable(telecodex)
target_sources(telecodex PRIVATE main.cpp)
target_link_libraries(telecodex PRIVATE Boost::process)

target_compile_definitions(telecodex PRIVATE _WIN32_WINNT=0x0A00)
