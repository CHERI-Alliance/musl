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

function __configure_comp_rt() {
    local LLVM_PROJECT=${1}         # path to LLVM sources
    local BAKEWELL_LLVM_PATH=${2}    # path where Morello LLVM has been installed
    local BUILD_PATH=${3}           # path to the build folder
    local SYSROOT=${4}              # path to sysroot with the required libc headers
    local TRIPLE=${5}               # triple to target
    local TFLAGS="-march=rv64imafdc_zcheri-legacy -mabi=l64pc128d"
    mkdir -p ${BUILD_PATH}
    pushd ${BUILD_PATH}
    cat << EOF > toolchain.cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR riscv64)
set(CMAKE_ASM_COMPILER_TARGET "${TRIPLE} ${TFLAGS}")
set(CMAKE_C_COMPILER_TARGET "${TRIPLE} ${TFLAGS}")
set(CMAKE_CXX_COMPILER_TARGET "${TRIPLE} ${TFLAGS}")

set(CMAKE_ASM_COMPILER_WORKS 1 CACHE INTERNAL "")
set(CMAKE_C_COMPILER_WORKS 1 CACHE INTERNAL "")
set(CMAKE_CXX_COMPILER_WORKS 1 CACHE INTERNAL "")

set(CMAKE_ASM_COMPILER "${BAKEWELL_LLVM_PATH}/bin/clang" CACHE FILEPATH "" FORCE)
set(CMAKE_C_COMPILER "${BAKEWELL_LLVM_PATH}/bin/clang" CACHE FILEPATH "" FORCE)
set(CMAKE_CXX_COMPILER "${BAKEWELL_LLVM_PATH}/bin/clang++" CACHE FILEPATH "" FORCE)
set(CMAKE_AR "${BAKEWELL_LLVM_PATH}/bin/llvm-ar" CACHE FILEPATH "" FORCE)
set(CMAKE_RANLIB "${BAKEWELL_LLVM_PATH}/bin/llvm-ranlib" CACHE FILEPATH "" FORCE)
set(CMAKE_NM "${BAKEWELL_LLVM_PATH}/bin/llvm-nm" CACHE FILEPATH "" FORCE)
set(CMAKE_LINKER "${BAKEWELL_LLVM_PATH}/bin/ld.lld" CACHE FILEPATH "" FORCE)
set(CMAKE_OBJDUMP "${BAKEWELL_LLVM_PATH}/bin/llvm-objdump" CACHE FILEPATH "" FORCE)
set(CMAKE_OBJCOPY "${BAKEWELL_LLVM_PATH}/bin/llvm-objcopy" CACHE FILEPATH "" FORCE)

set(LLVM_CONFIG_PATH "${BAKEWELL_LLVM_PATH}/bin/llvm-config" CACHE FILEPATH "" FORCE)
EOF
    cmake -Wno-dev \
        -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_C_FLAGS="-nostdinc -isystem ${SYSROOT}/include ${TFLAGS}" \
        -DCMAKE_CXX_FLAGS="-nostdinc -isystem ${SYSROOT}/include ${TFLAGS}" \
        -DCMAKE_ASM_FLAGS="-nostdinc -isystem ${SYSROOT}/include ${TFLAGS}" \
        -DCMAKE_EXE_LINKER_FLAGS="${TFLAGS} -nostartfiles -nostdlib ${SWITCHES}" \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        -DCMAKE_SKIP_BUILD_RPATH=OFF \
        -DCMAKE_INSTALL_RPATH=\$ORIGIN/../lib \
        -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON \
        -DLLVM_TARGETS_TO_BUILD="RISCV" \
        -DLLVM_ENABLE_ASSERTIONS=OFF \
        -DBUILD_SHARED_LIBS=ON \
        -DCOMPILER_RT_BUILD_BUILTINS=ON \
        -DCOMPILER_RT_BUILD_SANITIZERS=OFF \
        -DCOMPILER_RT_BUILD_XRAY=OFF \
        -DCOMPILER_RT_BUILD_LIBFUZZER=OFF \
        -DCOMPILER_RT_BUILD_PROFILE=OFF \
        -DCOMPILER_RT_BUILD_MEMPROF=OFF \
        -DCOMPILER_RT_DEFAULT_TARGET_ONLY=ON \
   ${LLVM_PROJECT}/compiler-rt
    popd
}


# Environment variables:
#  - BAKEWELL_NPROC: number of parallel jobs (default: 4)
function build_compiler_rt() {
    local LLVM_PROJECT=${1}         # path to LLVM sources
    local BAKEWELL_LLVM_PATH=${2}    # path where Morello LLVM has been installed
    local BUILD_PATH=${3}           # path to the build folder
    local SYSROOT=${4}              # path to sysroot with the required libc headers
    local TRIPLE=${5}               # expanded target triple
    local DESTDIR=$(${BAKEWELL_LLVM_PATH}/bin/clang -print-resource-dir)/lib/${TRIPLE}
    rm -rf ${BUILD_PATH}
    __configure_comp_rt ${LLVM_PROJECT} ${BAKEWELL_LLVM_PATH} ${BUILD_PATH} ${SYSROOT} ${TRIPLE}
    pushd ${BUILD_PATH}
    make -j ${BAKEWELL_NPROC:-4} clang_rt.builtins-riscv64
    mkdir -p ${DESTDIR}
    cp lib/linux/libclang_rt.builtins-riscv64.a ${DESTDIR}/libclang_rt.builtins.a
    popd
}

build_compiler_rt ${LLVM_PROJECT} ${BAKEWELL_HOME}/cherillvm ${BUILDPREFIX}/compiler-rt ${BAKEWELL_HOME} riscv64-unknown-linux-gnu
