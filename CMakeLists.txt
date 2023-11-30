cmake_minimum_required(VERSION 3.9.4)

project(tair-client)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "DEBUG")
endif ()

string(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_TOUPPER)
message("CMAKE_BUILD_TYPE is " ${CMAKE_BUILD_TYPE_TOUPPER})

if (NOT MALLOC AND CMAKE_BUILD_TYPE MATCHES "Rel*")
    set(MALLOC "jemalloc")
    message("not set MALLOC, try use jemalloc")
endif ()

set(DEFAULT_PIC "-fPIC")

if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set(MALLOC "libc")
    message("not support jemalloc in Darwin, just use libc")
endif ()

if (MALLOC MATCHES "jemalloc")
    add_definitions(-DUSE_JEMALLOC=1)
endif ()

find_program(CCACHE_PROGRAM ccache)
if (CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif ()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # enable compile_commands.json for VSCode and Fleet

find_package(absl REQUIRED PATHS ${PROJECT_SOURCE_DIR}/deps/abseil-cpp NO_DEFAULT_PATH)
find_package(fmt REQUIRED PATHS ${PROJECT_SOURCE_DIR}/deps/fmt NO_DEFAULT_PATH)
find_package(spdlog REQUIRED PATHS ${PROJECT_SOURCE_DIR}/deps/spdlog NO_DEFAULT_PATH)
find_package(gflags REQUIRED PATHS ${PROJECT_SOURCE_DIR}/deps/gflags NO_DEFAULT_PATH)
find_package(benchmark REQUIRED PATHS ${PROJECT_SOURCE_DIR}/deps/benchmark NO_DEFAULT_PATH)
find_package(Libevent REQUIRED PATHS ${PROJECT_SOURCE_DIR}/deps/libevent NO_DEFAULT_PATH)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CXX_FLAGS
            -g
            -Wall
            -Werror
            -std=c++2a
            -pthread
            -fno-strict-aliasing
            -fno-omit-frame-pointer
            -fno-var-tracking-assignments
            -Wno-deprecated-declarations
            -Wno-unused-function
            -D_FILE_OFFSET_BITS=64
            -static-libgcc
            -static-libstdc++
        )
    set(C_FLAGS
            -g
            -Wall
            -Werror
            -Wno-empty-body
            -fno-strict-aliasing
            -fno-omit-frame-pointer
            -fno-var-tracking-assignments
            -Wno-deprecated-declarations
            -std=c11
            -pthread
            -D_FILE_OFFSET_BITS=64
            -static-libgcc
            -static-libstdc++
        )
else ()
    set(CXX_FLAGS
            -g
            -Wall
            -Werror
            -std=c++20
            -fno-omit-frame-pointer
            -Wthread-safety
            -Wsign-compare
            -Wunused-variable
            -Wno-deprecated-declarations
            -Wno-unused-function
            -D_FILE_OFFSET_BITS=64
            -D_LIBCPP_NO_EXCEPTIONS
        )
    set(C_FLAGS
            -g
            -Wall
            -Werror
            -Wsign-compare
            -Wno-empty-body
            -Wno-unused-function
            -Wno-deprecated-declarations
            -fno-omit-frame-pointer
            -std=c11
            -D_FILE_OFFSET_BITS=64
        )
endif ()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(DEFAULT_SANITIZER "-fsanitize=address,leak -DUSE_SANITIZER")
    set(DEFAULT_COVERAGE "--coverage")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(DEFAULT_SANITIZER "-fsanitize=address,leak -DUSE_SANITIZER")
    set(DEFAULT_COVERAGE "-fprofile-instr-generate -fcoverage-mapping")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    # AppleClang not support leak check
    set(DEFAULT_SANITIZER "-fsanitize=address -DUSE_SANITIZER")
    set(DEFAULT_COVERAGE "-fprofile-instr-generate -fcoverage-mapping")
endif ()

if (SANITIZER)
    set(DEFAULT_SANITIZER "-fsanitize=${SANITIZER} -DUSE_SANITIZER")
endif ()

if (NO_SANITIZER)
    set(DEFAULT_SANITIZER "-fno-sanitize=all")
endif ()

if (COVERAGE)
    set(DEFAULT_COVERAGE ${COVERAGE})
endif ()

message("PROJECT_BINARY_DIR is " ${PROJECT_BINARY_DIR})
set(PGO_DATA_DIR "${PROJECT_BINARY_DIR}/pgo")

# SPDLOG_LEVEL_TRACE is 0, in DEBUG mode, enable the trace log
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DSPDLOG_ACTIVE_LEVEL=0 -O0 ${DEFAULT_PIC} ${DEFAULT_SANITIZER} ${DEFAULT_COVERAGE}")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 ${DEFAULT_PIC} ${DEFAULT_SANITIZER} ${DEFAULT_COVERAGE}")

set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG ${DEFAULT_PIC} ${DEFAULT_PGO_GENERATE} ${DEFAULT_PGO_USE}")
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG ${DEFAULT_PIC} ${DEFAULT_PGO_GENERATE} ${DEFAULT_PGO_USE}")

set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -DNDEBUG ${DEFAULT_PIC} ${DEFAULT_PGO_GENERATE} ${DEFAULT_PGO_USE}")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -DNDEBUG ${DEFAULT_PIC} ${DEFAULT_PGO_GENERATE} ${DEFAULT_PGO_USE}")

string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")
string(REPLACE ";" " " CMAKE_C_FLAGS "${C_FLAGS}")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

include_directories(${PROJECT_SOURCE_DIR}/src)

# Add deps include dir and lib dir
include_directories(${PROJECT_SOURCE_DIR}/deps/libevent/include)
include_directories(${PROJECT_SOURCE_DIR}/deps/jemalloc/include)
include_directories(${PROJECT_SOURCE_DIR}/deps/openssl/include)
include_directories(${PROJECT_SOURCE_DIR}/deps/cpp-linenoise)

add_subdirectory(src)
add_subdirectory(examples)

if (NOT BUILD_NO_TEST)
    enable_testing()
    add_subdirectory(benchmark)
    add_subdirectory(tests/ut_tests)
endif ()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    link_directories(/usr/local/lib)
endif ()
