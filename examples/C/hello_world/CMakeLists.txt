cmake_minimum_required(VERSION 3.12)

project(hello_world)

add_subdirectory(./src)
add_subdirectory(./vendor/flatptp/C EXCLUDE_FROM_ALL)
