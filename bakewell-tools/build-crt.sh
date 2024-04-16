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

if [ "${LINUX_HEADERS}" == "" ]; then
    LINUX_HEADERS=${PWD}/musl-libc/bakewell-linux-libc-headers
fi

set -e

# Environment variables:
#  - CC: path to Morello clang
function build_musl_headers() {
    local MUSL_PATH=${1}            # path to Musl sources
    local PREFIX_PATH=${2}          # where to install Musl headers
    local TRIPLE=${3}               # target triple
    local CFGFLAGS="--enable-bakewell --enable-shared"
    mkdir -p ${BUILDPREFIX}/musl-headers
    pushd ${BUILDPREFIX}/musl-headers
    ${MUSL_PATH}/configure ${CFGFLAGS} --prefix=${PREFIX_PATH} --target=${TRIPLE}
    make install-headers
    popd
}

# Environment variables:
#  - CC: path to Morello clang
function build_crt() {
    local LLVM_PROJECT=${1}         # path to LLVM sources
    local SYSROOT=${2}              # path to sysroot with the required libc headers
    local TRIPLE=${3}               # expanded target triple
    local CRT=${LLVM_PROJECT}/compiler-rt/lib/crt
    local DESTDIR=$(${CC} -print-resource-dir)/lib/${TRIPLE}
    local INCLUDE=${SYSROOT}/include
    local XFLAGS="-DCRT_HAS_INITFINI_ARRAY --target=${TRIPLE} -march=rv64imafdc_zcheri-mode -mabi=l64pc128d -nostdinc -isystem ${INCLUDE}"
    mkdir -p ${DESTDIR}
    ${CC} ${XFLAGS} -c ${CRT}/crtbegin.c -o ${DESTDIR}/clang_rt.crtbegin.o
    ${CC} ${XFLAGS} -c ${CRT}/crtend.c -o ${DESTDIR}/clang_rt.crtend.o
}

function copy_linux_headers() {
    local LINUX_HEADERS=${1}
    local SYSROOT=${2}
    cp -rp ${LINUX_HEADERS}/usr/include/* ${SYSROOT}/include
}

export CC=${BAKEWELL_HOME}/cherillvm/bin/riscv64-unknown-linux-gnu-cc
build_musl_headers ${MUSL_PATH} ${BAKEWELL_HOME} riscv64-uknown-linux-gnu
build_crt ${LLVM_PROJECT} ${BAKEWELL_HOME} riscv64-unknown-linux-gnu
copy_linux_headers ${LINUX_HEADERS} ${BAKEWELL_HOME}
