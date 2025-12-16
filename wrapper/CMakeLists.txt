cmake_minimum_required(VERSION 3.10)
project(wrapper_engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")

# Исходные файлы проекта
set(SOURCES
    main.cpp
    engine.cpp
)

# Заголовочные файлы
set(HEADERS
    engine.hpp
    iwrapper.hpp
    wrapper.hpp
    wrapper.tpp
)

add_executable(wrapper_engine ${SOURCES})

target_include_directories(wrapper_engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Threads REQUIRED)
target_link_libraries(wrapper_engine PRIVATE Threads::Threads)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(wrapper_engine PRIVATE -g -O0)
else()
    target_compile_options(wrapper_engine PRIVATE -O3)
endif()

message(STATUS "Configuratio: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
