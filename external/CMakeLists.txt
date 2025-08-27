# Copyright (c) Maia

if(MSVC)
  add_compile_options(/bigobj)
endif()

# if(WIN32)
#   add_compile_definitions(WIN32_LEAN_AND_MEAN)
# endif()

add_subdirectory(lsp-framework)
