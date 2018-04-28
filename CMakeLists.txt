 cmake_minimum_required(VERSION 3.10)
 project(arcport)

 set(CMAKE_C_STANDARD 99)
 set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -g -Wall -Werror")

 ADD_EXECUTABLE(arcport queue.c arcport.c)

 TARGET_LINK_LIBRARIES(arcport -lpthread)

