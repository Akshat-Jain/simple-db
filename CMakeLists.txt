cmake_minimum_required(VERSION 3.30)
project(simple-db)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

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
#        src/config.cpp
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
target_link_libraries(simple-db PRIVATE ${READLINE_LIBRARIES})
