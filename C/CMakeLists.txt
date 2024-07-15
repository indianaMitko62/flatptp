add_library(flatptp flatptp.c)

target_sources(flatptp PRIVATE
    flatptp.c
)

target_include_directories(flatptp INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)