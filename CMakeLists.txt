cmake_minimum_required(VERSION 3.30)
project(simple-db)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

#-----------------------------------------------------------------------------
# Warning flags for GCC/Clang
#-----------------------------------------------------------------------------
set(UNUSED_WARN_FLAGS
        -Wall               # enable the majority of “sane” warnings (overflow, uninitialized vars, etc.)
        -Wextra             # turn on extra, more pedantic warnings beyond -Wall
        -Wunused-variable   # warn whenever a local variable is declared but never used
)

add_compile_options(${UNUSED_WARN_FLAGS}) # Add to all targets

#-----------------------------------------------------------------------------
# Enable Testing with CTest
#-----------------------------------------------------------------------------
enable_testing()
include(FetchContent) # Include module for fetching dependencies

#-----------------------------------------------------------------------------
# GoogleTest Dependency
#-----------------------------------------------------------------------------
FetchContent_Declare(
        googletest
        # Specify the Git repository and a stable tag or commit hash
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.16.0 # Or choose the latest stable release tag
)
FetchContent_MakeAvailable(googletest)

#-----------------------------------------------------------------------------
# spdlog Dependency
#-----------------------------------------------------------------------------
FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG        v1.13.0        # or whatever latest stable
)
FetchContent_MakeAvailable(spdlog)

# --- Use PkgConfig to find Readline ---
# Find the pkg-config utility itself. CMake needs this to run pkg-config.
find_package(PkgConfig REQUIRED)

# Use pkg-config to check for the 'readline' module.
# This will search for 'readline.pc' using the PKG_CONFIG_PATH.
# If found, it defines variables like:
#   READLINE_FOUND
#   READLINE_INCLUDE_DIRS (path(s) for -I flags)
#   READLINE_LIBRARIES    (library name(s) for linking, e.g., -lreadline)
#   READLINE_LIBRARY_DIRS (path(s) for -L flags)
# REQUIRED makes CMake fail if readline is not found via pkg-config.
pkg_check_modules(READLINE REQUIRED readline)
# ------------------------------------

# Define the executable target.
# List all your source files (.cpp) here.
add_executable(simple-db
        src/main.cpp
        src/config.cpp
        src/parser.cpp
        src/utils/stringutils.cpp
        src/history.cpp
)

# --- Link the executable against Readline using pkg-config variables ---

# This tells CMake to add the 'include' directory to the search path
# for headers when compiling the 'simple-db' target.
# This allows `#include "simpledb/config.h"` to work.
# Use PRIVATE since only this target needs it directly for compilation.
# ${CMAKE_CURRENT_SOURCE_DIR} is the directory containing this CMakeLists.txt
target_include_directories(simple-db PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Add the include directories provided by pkg-config to the target's include paths.
# This allows your code to find <readline/readline.h> etc.
target_include_directories(simple-db PRIVATE ${READLINE_INCLUDE_DIRS})

# Link the target against the libraries specified by pkg-config.
# This handles the -L and -l flags needed by the linker.
target_link_libraries(
        simple-db PRIVATE
        ${READLINE_LIBRARIES}
        spdlog::spdlog
)

#-----------------------------------------------------------------------------
# Unit Test Definition
#-----------------------------------------------------------------------------
# Define the test executable target
add_executable(run_tests
        tests/parser_test.cpp
        # Add other tests/*.cpp files here

        # Include source files needed by the tests that aren't part of a library yet
        # This links their compiled object code into the test executable.
        src/config.cpp
        src/parser.cpp
        src/utils/stringutils.cpp
)

# --- Configure 'run_tests' Target ---
target_include_directories(
        run_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${READLINE_INCLUDE_DIRS}
)

target_link_libraries(
        run_tests PRIVATE
        ${READLINE_LIBRARIES}
        GTest::gtest_main
        spdlog::spdlog
)

# Discover and add tests to CTest automatically
include(GoogleTest)
gtest_discover_tests(run_tests)
# This finds all TEST(...) cases in run_tests and registers them with CTest
