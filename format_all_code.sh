#!/bin/bash

set -ex

CLANG_FORMAT=""

if [[ "$(uname)" == "Darwin" ]]; then
    if [[ "$(uname -m)" == "x86_64" ]]; then
        CLANG_FORMAT="./scripts/code_format/clang-format-darwin-x86_64"
    elif [[ "$(uname -m)" == "arm64" ]]; then
        CLANG_FORMAT="./scripts/code_format/clang-format-darwin-aarch64"
    else
        echo "not support this arch now"
        exit 1
    fi
elif [[ "$(uname)" == "Linux" ]]; then
    if [[ "$(uname -m)" == "x86_64" ]]; then
        CLANG_FORMAT="./scripts/code_format/clang-format-linux-x86_64"
    else
        echo "not support this arch now"
        exit 1
    fi
else
    echo "not support this system: $(uname)"
    exit 1
fi

FORMAT_DIRS=("src" "tests/ut_tests")

for dir in ${FORMAT_DIRS};
do
    find ${dir} -name "*.cpp" -o -name "*.hpp" -o -name "*.h" | xargs ${CLANG_FORMAT} -style=file -i
done

