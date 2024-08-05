add_executable(receiver)

target_sources(receiver PRIVATE
    receiver.c
)

target_include_directories(receiver PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(receiver PRIVATE
    flatptp
)