cmake_minimum_required(VERSION 3.10)
project(Calculator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

add_executable(calculator
    main.cpp
    calculator.cpp
    calculator.hpp
    evaluator.cpp
    evaluator.hpp
    functionRegistry.cpp
    functionRegistry.hpp
    parser.cpp
    parser.hpp
    baseClasses.hpp
    IPluginFunction.hpp
)

add_custom_command(TARGET calculator POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/plugins
        $<TARGET_FILE_DIR:calculator>/plugins
    COMMENT "Copying pre-built plugins to output directory"
)

target_link_libraries(calculator stdc++fs)