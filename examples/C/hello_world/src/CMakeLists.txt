add_executable(hello_world)

target_sources(hello_world PRIVATE
    hello_world.c
)

target_include_directories(hello_world PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(hello_world PRIVATE
    flatptp
)