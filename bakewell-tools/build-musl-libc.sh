#!/usr/bin/env bash

if [ "${LLVM_PROJECT}" == "" ]; then
    LLVM_PROJECT=${PWD}/cherillvm
fi

if [ "${MUSL_PATH}" == "" ]; then
    MUSL_PATH=${PWD}/musl-libc
fi

if [ "${BAKEWELL_HOME}" == "" ]; then
    BAKEWELL_HOME=${PWD}/install
fi

if [ "${BUILDPREFIX}" == "" ]; then
    BUILDPREFIX=${PWD}/build
fi

set -e

# Environment variables:
#  - CC: path to bakewell clang
#  - BAKEWELL_NPROC: number of parallel jobs (default: 8)
function build_musl() {
    local MUSL_PATH=${1}            # path to Musl sources
    local PREFIX_PATH=${2}          # where to install Musl
    local TRIPLE=${4:-${3}}         # target triple
    local CFGFLAGS="--enable-bakewell"
    ${MUSL_PATH}/configure --prefix=${PREFIX_PATH} --target=${TRIPLE} ${CFGFLAGS} --enable-debug
    make -j${BAKEWELL_NPROC:-8}
    make install
    mkdir -p ${PREFIX_PATH}/share
    cp ${MUSL_PATH}/COPYRIGHT ${PREFIX_PATH}/share/MUSL-LICENSE.txt
    wget -q https://www.apache.org/licenses/LICENSE-2.0.txt -O ${PREFIX_PATH}/LICENSE.txt
    cat << EOF > ${PREFIX_PATH}/NOTICE.txt
This product embeds and uses the following pieces of software
which have additional or alternate licenses:
 - Musl libc: share/MUSL-LICENSE.txt
EOF
}

export CC=${BAKEWELL_HOME}/cherillvm/bin/riscv64-unknown-linux-gnu-cc
export CFLAGS="-isystem ${BAKEWELL_HOME}/cherillvm/lib/clang/15.0.0/include"
mkdir -p ${BUILDPREFIX}/musl
cd ${BUILDPREFIX}/musl
build_musl ${MUSL_PATH} ${BAKEWELL_HOME} riscv64-unknown-linux-gnu
