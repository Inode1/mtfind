cmake_minimum_required(VERSION 2.8)

set(PROJECT mtfind)

project(${PROJECT})

set (MAJOR_VERSION  0)
set (MINOR_VERSION  1)

option(${PROJECT}_TESTS "Enable tests for ${PROJECT}" On)

include_directories(src)

find_package( Threads )
add_executable(${PROJECT} mtfind.cpp)
target_link_libraries(${PROJECT} ${CMAKE_THREAD_LIBS_INIT})

if(${PROJECT}_TESTS)
    add_subdirectory(test)
    add_dependencies(${PROJECT} unit_test)
endif()
