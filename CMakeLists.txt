project(cpascpt)
cmake_minimum_required(VERSION 3.4)

# Find antlr
find_package(antlr4 NAMES antlr4-runtime)
find_library(antlr4-runtime NAMES antlr4-runtime HINTS ${ANTLR4_LIB_DIR})

if (NOT ${antlr4})
    message(FATAL_ERROR "could not find anltr4 package")
endif()

if (NOT EXISTS parser)
    # Generate antlr files
    execute_process(COMMAND antlr -Dlanguage=Cpp Generic.g4 -o parser)
    execute_process(COMMAND antlr4 -Dlanguage=Cpp Generic.g4 -o parser)
endif()

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/parser)
include_directories(${ANTLR4_INCLUDE_DIR})

set(SOURCE_FILES
    parser/GenericListener.cpp
    parser/GenericBaseListener.cpp
    parser/GenericLexer.cpp
    parser/GenericParser.cpp
    compile.cc
)

add_library(cpascpt SHARED ${SOURCE_FILES})
add_executable(cpascpt-bin ${SOURCE_FILES} interface.cc main.cc)

set_property(TARGET cpascpt PROPERTY CXX_STANDARD 17)
set_property(TARGET cpascpt-bin PROPERTY CXX_STANDARD 17)
set_property(TARGET cpascpt-bin PROPERTY OUTPUT_NAME cpascpt)

target_link_libraries(cpascpt-bin cpascpt)
target_link_libraries(cpascpt ${antlr4-runtime})
