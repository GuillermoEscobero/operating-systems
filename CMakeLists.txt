cmake_minimum_required(VERSION 3.8)
project(operating_systems)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-Wall -ansi -o")


include_directories(person.h)

add_executable(printFile printFile.c person.h)
add_executable(statistics statistics.c person.h)
add_executable(split split.c person.h)
add_executable(filter filter.c person.h)
add_executable(combine combine.c person.h)