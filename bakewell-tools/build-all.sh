#!/usr/bin/env bash
SCRIPT_DIR=`dirname "$0"`

set -e

${SCRIPT_DIR}/build-clang.sh
${SCRIPT_DIR}/build-crt.sh
${SCRIPT_DIR}/build-compiler-rt.sh
${SCRIPT_DIR}/build-musl-libc.sh
${SCRIPT_DIR}/build-libcxx.sh
