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

target_link_libraries(calculator)

# Compile all plugins from operations directory
file(GLOB PLUGIN_SOURCES "operations/*.cpp")

foreach(PLUGIN_SOURCE ${PLUGIN_SOURCES})
    get_filename_component(PLUGIN_NAME ${PLUGIN_SOURCE} NAME_WE)
    
    add_library(${PLUGIN_NAME} SHARED ${PLUGIN_SOURCE})
    set_target_properties(${PLUGIN_NAME} PROPERTIES PREFIX "")
    set_target_properties(${PLUGIN_NAME} PROPERTIES SUFFIX ".dll")
    
    # Define PLUGIN_EXPORTS for proper dllexport on Windows
    target_compile_definitions(${PLUGIN_NAME} PRIVATE PLUGIN_EXPORTS)
    
    # Set output directory for plugins
    set_target_properties(${PLUGIN_NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    )
endforeach()