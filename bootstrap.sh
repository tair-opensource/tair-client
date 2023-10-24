#!/usr/bin/env bash

PROJECT_DIR=`pwd`

# =========================================================================================

# abseil: C++ Common Libraries from Google
#
# https://github.com/abseil/abseil-cpp
#
ABSEIL_CPP_VER=20220623.1

# fmt: A modern formatting library
#
# https://github.com/fmtlib/fmt
#
FMT_VER=9.1.0

# spdlog: Fast C++ logging library
#
# https://github.com/gabime/spdlog
#
SPDLOG_VER=1.11.0

# libevent: Event notification library
#
# https://github.com/libevent/libevent
#
# merged patch:
#   https://github.com/libevent/libevent/commit/2f184f8bbf23377bddc8daa1a2c7b40735ee7e2a
#   https://github.com/libevent/libevent/commit/9a9b92ed06249be8326d82e2483b87e1a1b4caac
#
LIBEVENT_VER=2.1.12-stable-patch

# jemalloc: A general purpose malloc(3) implementation
#
# https://github.com/jemalloc/jemalloc
#
JEMALLOC_VER=5.3.0

# gflags: Commandline Flags Libraries
#
# https://github.com/gflags/gflags
#
GFLAGS_VER=2.2.2

# openssl: Is a toolkit for the TLS and SSL protocols.
#          Is also a general-purpose cryptography library.
#
# https://github.com/openssl/openssl
#
OPENSSL_VER=1.1.1m

# googletest: Google Testing and Mocking Framework
#
# https://github.com/google/googletest
#
GOOGLETEST_VER=1.12.1

# benchmark: A microbenchmark support library
#
# https://github.com/google/benchmark
#
BENCHMARK_VER=1.7.0

#
# cpp-linenoise: A single file multi-platform (Unix, Windows) C++ header-only linenoise-based readline library
#
# https://github.com/yhirose/cpp-linenoise
#
CPP_LINENOISE_VER=1.0.0

# =========================================================================================

build_deps()
{
    BUILD_MODE=""
    if [[ $1 == "debug" ]]; then
        BUILD_MODE="DEBUG"
    elif [[ $1 == "release" ]]; then
        BUILD_MODE="Release"
    fi

    DEPS_NAME="all"
    if [[ $2 != "" ]]; then
        DEPS_NAME=$2
    fi
    clean_deps $2

    # for opensll
    SAVED_RELEASE=${RELEASE}
    unset RELEASE

    # abseil-cpp
    if [[ ${DEPS_NAME} == "abseil-cpp" || ${DEPS_NAME} == "all" ]]; then
        cd ${PROJECT_DIR}/deps
        tar xf abseil-cpp-${ABSEIL_CPP_VER}.tar.gz
        cd abseil-cpp-${ABSEIL_CPP_VER}
        mkdir -p build && cd build
        cmake -DCMAKE_BUILD_TYPE=${BUILD_MODE} -DCMAKE_CXX_FLAGS="-fPIC" -DCMAKE_C_FLAGS="-fPIC" -DCMAKE_CXX_STANDARD=17 -DCMAKE_INSTALL_PREFIX=${PROJECT_DIR}/deps/abseil-cpp ..
        make -j VERBOSE=1 && make install
    fi

    # fmt
    if [[ ${DEPS_NAME} == "fmt" || ${DEPS_NAME} == "all" ]]; then
        cd ${PROJECT_DIR}/deps
        tar xf fmt-${FMT_VER}.tar.gz
        cd fmt-${FMT_VER}
        mkdir -p build && cd build
        cmake -DCMAKE_BUILD_TYPE=${BUILD_MODE} -DCMAKE_CXX_FLAGS="-fPIC" -DCMAKE_C_FLAGS="-fPIC" -DFMT_TEST=off -DCMAKE_CXX_VISIBILITY_PRESET=default -DCMAKE_VISIBILITY_INLINES_HIDDEN=OFF \
              -DFMT_DOC=OFF -DFMT_PEDANTIC=ON -DFMT_WERROR=ON -DCMAKE_INSTALL_PREFIX=${PROJECT_DIR}/deps/fmt ..
        make -j VERBOSE=1 && make install
    fi

    # spdlog
    if [[ ${DEPS_NAME} == "spdlog" || ${DEPS_NAME} == "all" ]]; then
        cd ${PROJECT_DIR}/deps
        tar xf spdlog-${SPDLOG_VER}.tar.gz
        cd spdlog-${SPDLOG_VER}
        mkdir -p build && cd build
        FMT_CMAKE_DIR=${PROJECT_DIR}/deps/fmt/lib/cmake/fmt
        if [[ `uname` == "Linux" ]]; then
            FMT_CMAKE_DIR=${PROJECT_DIR}/deps/fmt/lib64/cmake/fmt
        fi
        cmake -DCMAKE_BUILD_TYPE=${BUILD_MODE} -DCMAKE_CXX_FLAGS="-fPIC" -DCMAKE_C_FLAGS="-fPIC" -DCMAKE_INSTALL_PREFIX=${PROJECT_DIR}/deps/spdlog \
              -DSPDLOG_FMT_EXTERNAL=ON -DCMAKE_PREFIX_PATH=${FMT_CMAKE_DIR} ..
        make -j VERBOSE=1 && make install
    fi

    # libevent
    if [[ ${DEPS_NAME} == "libevent" || ${DEPS_NAME} == "all" ]]; then
        cd ${PROJECT_DIR}/deps
        tar xf libevent-${LIBEVENT_VER}.tar.gz
        cd libevent-${LIBEVENT_VER}
        mkdir -p build && cd build
        cmake -DCMAKE_BUILD_TYPE=${BUILD_MODE} -DEVENT__LIBRARY_TYPE=STATIC -DEVENT__DISABLE_OPENSSL=ON -DEVENT__DISABLE_TESTS=ON \
              -DCMAKE_INSTALL_PREFIX=${PROJECT_DIR}/deps/libevent -DCMAKE_CXX_FLAGS="-fPIC" -DCMAKE_C_FLAGS="-fPIC" ..
        make -j VERBOSE=1 && make install
    fi

    # jemalloc
    if [[ ${DEPS_NAME} == "jemalloc" || ${DEPS_NAME} == "all" ]]; then
        cd ${PROJECT_DIR}/deps
        tar xf jemalloc-${JEMALLOC_VER}.tar.gz
        cd jemalloc-${JEMALLOC_VER}
        autoconf
        mkdir -p build && cd build
        LG_QUANTUM=""
        if [[ `uname` == "Linux" ]]; then
            LG_QUANTUM="--with-lg-quantum=3"
        fi
        JEMALLOC_BUILD_ARG=""
        if [[ ${BUILD_MODE} == "DEBUG" ]]; then
            JEMALLOC_BUILD_ARG="--enable-debug"
        fi
        ../configure --prefix=${PROJECT_DIR}/deps/jemalloc --enable-shared=no --disable-initial-exec-tls ${JEMALLOC_BUILD_ARG} ${LG_QUANTUM}
        make -j VERBOSE=1 && make install
    fi

    # gflags
    if [[ ${DEPS_NAME} == "gflags" || ${DEPS_NAME} == "all" ]]; then
        cd ${PROJECT_DIR}/deps
        tar xf gflags-${GFLAGS_VER}.tar.gz
        cd gflags-${GFLAGS_VER} && rm -f BUILD
        mkdir -p build && cd build
        cmake -DCMAKE_BUILD_TYPE=${BUILD_MODE} -DCMAKE_INSTALL_PREFIX=${PROJECT_DIR}/deps/gflags -DCMAKE_CXX_FLAGS="-fPIC" -DCMAKE_C_FLAGS="-fPIC" ..
        make -j VERBOSE=1 && make install
    fi

    # openssl
    if [[ ${DEPS_NAME} == "openssl" || ${DEPS_NAME} == "all" ]]; then
        cd ${PROJECT_DIR}/deps
        tar xf openssl-${OPENSSL_VER}.tar.gz
        cd openssl-${OPENSSL_VER}
        mkdir -p build
        cd build
        OPENSSL_BUILD_ARG=""
        if [[ ${BUILD_MODE} == "DEBUG" ]]; then
            OPENSSL_BUILD_ARG="-d enable-asan"
        fi
        ../config --prefix=${PROJECT_DIR}/deps/openssl --openssldir=${PROJECT_DIR}/deps/openssl no-shared no-tests ${OPENSSL_BUILD_ARG}
        make -j VERBOSE=1 && make install_sw
    fi

    # googletest
    if [[ ${DEPS_NAME} == "googletest" || ${DEPS_NAME} == "all" ]]; then
        cd ${PROJECT_DIR}/deps
        tar xf googletest-release-${GOOGLETEST_VER}.tar.gz
        ln -s googletest-release-${GOOGLETEST_VER} googletest-release
    fi

    # benchmark
    if [[ ${DEPS_NAME} == "benchmark" || ${DEPS_NAME} == "all" ]]; then
        cd ${PROJECT_DIR}/deps
        tar xf benchmark-${BENCHMARK_VER}.tar.gz
        cd benchmark-${BENCHMARK_VER}
        mkdir -p build && cd build
        cmake -DCMAKE_BUILD_TYPE=${BUILD_MODE} -DCMAKE_INSTALL_PREFIX=${PROJECT_DIR}/deps/benchmark -DBENCHMARK_ENABLE_TESTING=off \
              -DCMAKE_CROSSCOMPILING=1 -DRUN_HAVE_STD_REGEX=0 -DRUN_HAVE_POSIX_REGEX=0 ..
        make -j VERBOSE=1 && make install
    fi

    # cpp-linenoise
    if [[ $DEPS_NAME == "cpp-linenoise" || $DEPS_NAME == "all" ]]; then
        cd ${PROJECT_DIR}/deps
        tar xf cpp-linenoise-${CPP_LINENOISE_VER}.tar.gz
        ln -s cpp-linenoise-${CPP_LINENOISE_VER} cpp-linenoise
    fi
    
    export RELEASE=${SAVED_RELEASE}
}

clean_deps()
{
    DEPS_NAME="all"
    if [[ $1 != "" ]]; then
        DEPS_NAME=$1
    fi

    # abseil-cpp
    if [[ ${DEPS_NAME} == "abseil-cpp" || ${DEPS_NAME} == "all" ]]; then
        rm -rf ${PROJECT_DIR}/deps/abseil-cpp
        rm -rf ${PROJECT_DIR}/deps/abseil-cpp-${ABSEIL_CPP_VER}
    fi

    # fmt
    if [[ ${DEPS_NAME} == "fmt" || ${DEPS_NAME} == "all" ]]; then
        rm -rf ${PROJECT_DIR}/deps/fmt
        rm -rf ${PROJECT_DIR}/deps/fmt-${FMT_VER}
    fi

    # spdlog
    if [[ ${DEPS_NAME} == "spdlog" || ${DEPS_NAME} == "all" ]]; then
        rm -rf ${PROJECT_DIR}/deps/spdlog
        rm -rf ${PROJECT_DIR}/deps/spdlog-${SPDLOG_VER}
    fi

    # libevent
    if [[ ${DEPS_NAME} == "libevent" || ${DEPS_NAME} == "all" ]]; then
        rm -rf ${PROJECT_DIR}/deps/libevent
        rm -rf ${PROJECT_DIR}/deps/libevent-${LIBEVENT_VER}
    fi

    # jemalloc
    if [[ ${DEPS_NAME} == "jemalloc" || ${DEPS_NAME} == "all" ]]; then
        rm -rf ${PROJECT_DIR}/deps/jemalloc
        rm -rf ${PROJECT_DIR}/deps/jemalloc-${JEMALLOC_VER}
    fi

    # gflags
    if [[ ${DEPS_NAME} == "gflags" || ${DEPS_NAME} == "all" ]]; then
        rm -rf ${PROJECT_DIR}/deps/gflags
        rm -rf ${PROJECT_DIR}/deps/gflags-${GFLAGS_VER}
    fi

    # openssl
    if [[ ${DEPS_NAME} == "openssl" || ${DEPS_NAME} == "all" ]]; then
        rm -rf ${PROJECT_DIR}/deps/openssl
        rm -rf ${PROJECT_DIR}/deps/openssl-${OPENSSL_VER}
    fi

    # googletest
    if [[ ${DEPS_NAME} == "googletest" || ${DEPS_NAME} == "all" ]]; then
        rm -rf ${PROJECT_DIR}/deps/googletest-build
        rm -rf ${PROJECT_DIR}/deps/googletest-release
        rm -rf ${PROJECT_DIR}/deps/googletest-release-${GOOGLETEST_VER}
    fi

    # benchmark
    if [[ ${DEPS_NAME} == "benchmark" || ${DEPS_NAME} == "all" ]]; then
        rm -rf ${PROJECT_DIR}/deps/benchmark-${BENCHMARK_VER}
        rm -rf ${PROJECT_DIR}/deps/benchmark
    fi

    # cpp-linenoise
    if [[ $DEPS_NAME == "cpp-linenoise" || $DEPS_NAME == "all" ]]; then
        rm -rf ${PROJECT_DIR}/deps/cpp-linenoise-${CPP_LINENOISE_VER}
        rm -rf ${PROJECT_DIR}/deps/cpp-linenoise
    fi
}

clean_build()
{
    rm -rf ${PROJECT_DIR}/build
}

HAS_NO_ASAN=0

get_free_prt() {
    BASE_PORT=7777
    INCREMENT=1

    port=$BASE_PORT
    isfree=$(netstat -taln | grep $port)

    while [[ -n "$isfree" ]]; do
        port=$[port+INCREMENT]
        isfree=$(netstat -taln | grep $port)
    done
    echo $port
}

gen_profraw()
{
    TAIR_BIN_DIR="${PROJECT_DIR}/build/bin"
    RDB_TEMP_SAVE_DIR="${PROJECT_DIR}/build/rdb-temp-saved"
    mkdir -p "${RDB_TEMP_SAVE_DIR}"

    rdb_server_port=`get_free_prt`
    "${TAIR_BIN_DIR}/rdb-server" --port "${rdb_server_port}" --dir "${RDB_TEMP_SAVE_DIR}" &
    rdb_server_run_pid=$!
    sleep 5

    "${REDIS_BENCHMARK_PATH}" -p "${rdb_server_port}" -n 50000 -r 100000 &
    rdb_server_benchmark_run_pid=$!

    wait $rdb_server_benchmark_run_pid

    kill $rdb_server_run_pid
    wait $rdb_server_run_pid

    rm -rf "${RDB_TEMP_SAVE_DIR}"
}

build_tair()
{
    BUILD_MODE=""
    BUILD_WITH_LTO_PGO=0
    if [[ $1 == "debug" ]]; then
        BUILD_MODE="DEBUG"
    elif [ $1 == "release" ]; then
        BUILD_MODE="RelWithDebInfo"
    elif [ $1 == "release-lto-pgo" ]; then
        BUILD_MODE="RelWithDebInfo"
        BUILD_WITH_LTO_PGO=1
    fi

    CMAKE_BUILD_ARGS=""
    if [[ ${HAS_NO_ASAN} == 1 ]]; then
        CMAKE_BUILD_ARGS="${CMAKE_BUILD_ARGS} -DNO_SANITIZER=ON"
    fi

    mkdir -p build && cd build
    if [[ $1 == "debug" || $1 == "release" ]]; then
        cmake -DCMAKE_BUILD_TYPE=${BUILD_MODE} ${CMAKE_BUILD_ARGS} ..
        if [[ `uname` == "Linux" ]]; then
            make -j
        else
            make -j6
        fi
    fi
}

run_tests()
{
    # Disable container overflow detection for false positives
    # see https://github.com/google/sanitizers/wiki/AddressSanitizerContainerOverflow
    export ASAN_OPTIONS=detect_container_overflow=0
    export LLVM_PROFILE_FILE=coverage-%p-%m.profraw

    # gtest ut
    export CTEST_OUTPUT_ON_FAILURE=1
    cd ${PROJECT_DIR}/build && make test
    if [[ $? != 0 ]]; then
        exit 1
    fi
}

gen_code_coverage()
{
    cd ${PROJECT_DIR}
    USED_COMPILER="gcc"
    if [[ `uname` == "Darwin" ]]; then
        USED_COMPILER="clang"
    fi

    if [[ ${USED_COMPILER} == "gcc" ]]; then
        # need lcov >= 1.15, see: https://github.com/linux-test-project/lcov/releases
        python3 scripts/fastcov.py -d ./build --lcov -o coverage.info
    else
        PATH="/Library/Developer/CommandLineTools/usr/bin/:$PATH"
        PROFRAW_FILES=`find . -name "coverage-*.profraw" | xargs`
        llvm-profdata merge -sparse ${PROFRAW_FILES} -o default.profdata
        llvm-cov export -format=lcov ./build/bin/rdb-server -instr-profile=default.profdata -j ${TASK_COUNT} > coverage.info
    fi

    lcov --remove coverage.info '*/usr/*' '*MacOSX*' '*/deps/*' '*/tests/*' -o main_coverage.info
    genhtml main_coverage.info --output-directory gcov_out
    # covert to cobertura XML format (just for jenkins)
    python scripts/lcov_cobertura.py main_coverage.info --output coverage.xml --demangle
}

check_gcc_version()
{
    if [[ `uname` == "Linux" ]]; then
        if [[ `uname -r` =~ aarch64 ]]; then
            return
        fi
        GCC_VERSION=`echo __GNUC__ | gcc -E -xc - | tail -n 1`
        if (( ${GCC_VERSION} < 9 )); then
            printf "\n  gcc version is NOT 9 or Later version , see README.md for more info\n\n"
            exit 1
        fi
    fi
}

print_help_info_and_exit_failure()
{
    set +x
    printf "Usage:\n"
    printf "\t./bootstrap.sh deps debug|release [opt]                   编译依赖, 可选 debug/release, [opt] 可选具体一个依赖的名字\n"
    printf "\t./bootstrap.sh build debug|release [opt]                  编译项目, 可选 debug/release, [opt] 可选 no-asan\n"
    printf "\t./bootstrap.sh test                                       运行测试\n"
    printf "\t./bootstrap.sh gen_code_coverage                          生成代码覆盖率报告 (需要先运行测试生成原始覆盖率文件)\n"
    printf "\t./bootstrap.sh clean deps [opt]|build|all                 清理编译目录, deps/build 分别表示子项, all 表示所有, [opt] 可选具体一个依赖的名字\n\n"
    exit 1
}

# for CI
export LLVM_PROFILE_FILE=coverage-%p-%m.profraw

TASK_COUNT=4
if [[ `uname` == "Linux" ]]; then
    CPU_CORE=`cat /proc/cpuinfo| grep "processor"| wc -l`
    TASK_COUNT=`expr ${CPU_CORE} / 4`
    ulimit -c unlimited
fi

if [[ $# != 0 ]]; then
    set -ex
    if [[ $1 == "deps" ]]; then
        check_gcc_version
        if [[ $2 == "debug" || $2 == "release" ]]; then
            build_deps $2 $3
        else
            print_help_info_and_exit_failure
        fi
    elif [[ $1 == "build" ]]; then
        if [[ $2 == "debug" || $2 == "release" || $2 == "release-lto-pgo" ]]; then
            check_gcc_version
            for ((i=3; i<=$#; i++))
            do
                arg=${!i}
                if [[ ${arg} == "no-asan" ]]; then
                    HAS_NO_ASAN=1
                else
                    print_help_info_and_exit_failure
                fi
            done
            build_tair $2
        else
            print_help_info_and_exit_failure
        fi
    elif [[ $1 == "test" ]]; then
        run_tests $2
    elif [[ $1 == "gen_code_coverage" ]]; then
        check_gcc_version
        gen_code_coverage
    elif [[ $1 == "clean" ]]; then
        if [[ $2 == "deps" ]]; then
            clean_deps $3
        elif [[ $2 == "build" ]]; then
            clean_build
        elif [[ $2 == "all" ]]; then
            clean_deps
            clean_build
        else
            print_help_info_and_exit_failure
        fi
    elif [[ $1 == "-h" || $1 == "--help" ]]; then
        print_help_info
    else
        print_help_info_and_exit_failure
    fi
    exit 0
else
    print_help_info_and_exit_failure
fi
