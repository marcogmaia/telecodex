# Copyright (c) Maia

add_executable(ttgen.executable)
target_sources(ttgen.executable PRIVATE ttgen.cpp)
target_link_libraries(ttgen.executable PRIVATE lsp)

set(TT_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
set(TT_METAMODEL "${CMAKE_CURRENT_SOURCE_DIR}/meta/metamodel.json")
set(
  TTGEN_GENERATED_HEADERS
  "${TT_GEN_DIR}/tt/types.h"
  "${TT_GEN_DIR}/tt/messages.h"
)
set(TTGEN_GENERATED_SOURCES ${TT_GEN_DIR}/tt/types.cpp)
file(MAKE_DIRECTORY "${TT_GEN_DIR}")
add_custom_command(
  COMMAND ttgen.executable ${TT_METAMODEL}
  WORKING_DIRECTORY ${TT_GEN_DIR}/tt
  OUTPUT ${TTGEN_GENERATED_SOURCES} ${TTGEN_GENERATED_HEADERS}
  DEPENDS ttgen.executable ${TT_METAMODEL}
  COMMENT "Generating tt types from meta model..."
)

add_library(tt.messages)
target_sources(
  tt.messages
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS "${TT_GEN_DIR}"
  PRIVATE
    ${TTGEN_GENERATED_SOURCES}
)
target_link_libraries(tt.messages PUBLIC lsp)
