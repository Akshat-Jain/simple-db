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
include(FetchContent)

#-----------------------------------------------------------------------------
# GoogleTest Dependency
#-----------------------------------------------------------------------------
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.16.0
)
FetchContent_MakeAvailable(googletest)

#-----------------------------------------------------------------------------
# spdlog Dependency
#-----------------------------------------------------------------------------
FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG        v1.13.0
)
FetchContent_MakeAvailable(spdlog)

#-----------------------------------------------------------------------------
# nlohmann/json Dependency
#-----------------------------------------------------------------------------
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
FetchContent_MakeAvailable(json)

#-----------------------------------------------------------------------------
# ANTLR Integration
#-----------------------------------------------------------------------------

# 1. Fetch the ANTLR C++ runtime library source code from Git.
FetchContent_Declare(
        antlr4_runtime_source
        GIT_REPOSITORY https://github.com/antlr/antlr4.git
        GIT_TAG        4.13.1
)
FetchContent_MakeAvailable(antlr4_runtime_source)
# The C++ runtime's CMakeLists.txt is in a subdirectory. Add it to our build.
add_subdirectory(
        ${antlr4_runtime_source_SOURCE_DIR}/runtime/Cpp
        ${antlr4_runtime_source_BINARY_DIR}/runtime
)

# Manually add the ANTLR runtime's include directory to the main include path.
# This ensures that when compiling the generated sources, the compiler can find 'antlr4-runtime.h'.
include_directories(${antlr4_runtime_source_SOURCE_DIR}/runtime/Cpp/runtime/src)

# 2. Download the ANTLR generator JAR using a direct command.
set(ANTLR_JAR ${CMAKE_CURRENT_BINARY_DIR}/antlr4-4.13.1-complete.jar)
file(DOWNLOAD
        https://repo1.maven.org/maven2/org/antlr/antlr4/4.13.1/antlr4-4.13.1-complete.jar
        ${ANTLR_JAR}
        EXPECTED_HASH SHA256=bc13a9c57a8dd7d5196888211e5ede657cb64a3ce968608697e4f668251a8487
        TLS_VERIFY ON
)

# 3. Find a Java installation on the build machine.
find_package(Java REQUIRED)

# 4. Define paths and output directory for generated parser files.
set(ANTLR_GRAMMAR ${CMAKE_SOURCE_DIR}/src/parser/antlr/SimpleDB.g4)
set(ANTLR_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated/antlr)

# 5. List the files that ANTLR will generate.
set(ANTLR_GENERATED_SOURCES
        ${ANTLR_OUTPUT_DIR}/SimpleDBLexer.cpp
        ${ANTLR_OUTPUT_DIR}/SimpleDBParser.cpp
        ${ANTLR_OUTPUT_DIR}/SimpleDBBaseVisitor.cpp
)
set(ANTLR_GENERATED_HEADERS
        ${ANTLR_OUTPUT_DIR}/SimpleDBLexer.h
        ${ANTLR_OUTPUT_DIR}/SimpleDBParser.h
        ${ANTLR_OUTPUT_DIR}/SimpleDBBaseVisitor.h
        ${ANTLR_OUTPUT_DIR}/SimpleDBVisitor.h
)

# 6. Define the custom command that runs the ANTLR tool to generate C++ files.
add_custom_command(
        OUTPUT  ${ANTLR_GENERATED_SOURCES} ${ANTLR_GENERATED_HEADERS}
        COMMAND ${Java_JAVA_EXECUTABLE} -jar ${ANTLR_JAR}
        -Dlanguage=Cpp
        -o ${ANTLR_OUTPUT_DIR}
        -visitor
        -Xexact-output-dir
        ${ANTLR_GRAMMAR}
        DEPENDS ${ANTLR_GRAMMAR} ${ANTLR_JAR}
        COMMENT "Generating C++ sources from ANTLR grammar..."
)

# 7. Create a custom target to ensure the generation step is triggered.
add_custom_target(AntlrGeneratedSources DEPENDS ${ANTLR_GENERATED_SOURCES})

# 8. Add the generated directory to the include path for all subsequent targets.
include_directories(${ANTLR_OUTPUT_DIR})


# --- Use PkgConfig to find Readline ---
find_package(PkgConfig REQUIRED)
pkg_check_modules(READLINE REQUIRED readline)
# ------------------------------------

# Define the executable target.
# List all your source files (.cpp) here.
add_executable(simple-db
        src/main.cpp
        src/config.cpp
        src/parser.cpp
        src/history.cpp
        src/catalog.cpp
        src/executor.cpp
        src/result.cpp
        src/storage/page.cpp
        src/storage/table_heap.cpp
        src/serializer.cpp
        src/execution/table_scan_operator.cpp
        src/execution/projection_operator.cpp
        src/planner.cpp
        src/parser/ast_builder_visitor.cpp
        ${ANTLR_GENERATED_SOURCES}
)
add_dependencies(simple-db AntlrGeneratedSources)

# --- Link the executable against Readline using pkg-config variables ---
target_include_directories(simple-db PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${READLINE_INCLUDE_DIRS}
)

target_link_libraries(
        simple-db PRIVATE
        ${READLINE_LIBRARIES}
        spdlog::spdlog
        nlohmann_json::nlohmann_json
        antlr4_static
)

#-----------------------------------------------------------------------------
# Unit Test Definition
#-----------------------------------------------------------------------------
add_executable(run_tests
        tests/parser_test.cpp
        tests/catalog_test.cpp
        tests/storage/page_test.cpp
        tests/storage/table_heap_test.cpp
        tests/storage/table_heap_iterator_test.cpp
        tests/execution/table_scan_operator_test.cpp
        tests/execution/projection_operator_test.cpp
        tests/select_integration_test.cpp
        # Add other tests/*.cpp files here

        # Include source files needed by the tests that aren't part of a library yet
        # This links their compiled object code into the test executable.
        src/config.cpp
        src/parser.cpp
        src/catalog.cpp
        src/executor.cpp
        tests/executor_test.cpp
        src/result.cpp
        src/storage/page.cpp
        src/storage/table_heap.cpp
        src/serializer.cpp
        src/execution/table_scan_operator.cpp
        src/execution/projection_operator.cpp
        src/planner.cpp
        src/parser/ast_builder_visitor.cpp
        ${ANTLR_GENERATED_SOURCES}
)
add_dependencies(run_tests AntlrGeneratedSources)

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
        nlohmann_json::nlohmann_json
        antlr4_static
)

# Discover and add tests to CTest automatically
include(GoogleTest)
gtest_discover_tests(run_tests)
# This finds all TEST(...) cases in run_tests and registers them with CTest
