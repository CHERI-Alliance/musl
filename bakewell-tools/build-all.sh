#!/usr/bin/env bash
SCRIPT_DIR=`dirname "$0"`
${SCRIPT_DIR}/build-clang.sh || exit 1
${SCRIPT_DIR}/build-crt.sh || exit 1
${SCRIPT_DIR}/build-compiler-rt.sh || exit 1
${SCRIPT_DIR}/build-musl-libc.sh || exit 1
${SCRIPT_DIR}/build-libcxx.sh || exit 1
