#!/bin/bash

# PROJECT_ROOT_DIR=$(dirname $PWD)

# echo "${PROJECT_ROOT_DIR}"

run-clang-tidy -p build/ 'src/.*\.cpp' 'tests/.*\.cpp'
