#!/usr/bin/env bash

if [ "${LLVM_PROJECT}" == "" ]; then
    LLVM_PROJECT=${PWD}/cherillvm
fi

if [ "${BAKEWELL_HOME}" == "" ]; then
    BAKEWELL_HOME=${PWD}/install
fi

if [ "${BUILDPREFIX}" == "" ]; then
    BUILDPREFIX=${PWD}/build
fi

if [ "${LLVM_TARGETS}" == "" ]; then
    LLVM_TARGETS="RISCV"
fi

if [ "${LLVM_LIT_ARGS}" == "" ]; then
    LLVM_LIT_ARGS="--max-time 3600 --timeout 300 -s -vv"
fi

function __configure_clang() {
    local LLVM_PROJECT=${1}
    local BAKEWELL_LLVM_PATH=${2}
    mkdir -p ${BAKEWELL_LLVM_PATH}
    cmake -GNinja \
        -Wno-dev \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=${BAKEWELL_LLVM_PATH} \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        -DCMAKE_SKIP_BUILD_RPATH=OFF \
        -DCMAKE_INSTALL_RPATH=\$ORIGIN/../lib \
        -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON \
        -DLLVM_ENABLE_PROJECTS="clang;llvm;lld" \
        -DLLVM_TARGETS_TO_BUILD="RISCV" \
        -DLLVM_ENABLE_ASSERTIONS=OFF \
        -DLLVM_ENABLE_EH=ON \
        -DLLVM_ENABLE_RTTI=ON \
        -DBUILD_SHARED_LIBS=ON \
        -DCOMPILER_RT_BUILD_BUILTINS=ON \
        -DCOMPILER_RT_BUILD_XRAY=OFF \
        -DCOMPILER_RT_BUILD_LIBFUZZER=OFF \
        -DCOMPILER_RT_BUILD_PROFILE=OFF \
        -DLIBUNWIND_ENABLE_THREADS=ON \
        -DLLVM_LIT_ARGS="${LLVM_LIT_ARGS}" \
        -DCLANG_DEFAULT_RTLIB="compiler-rt" \
        -DCLANG_DEFAULT_LINKER="lld" \
        -DCLANG_DEFAULT_OBJCOPY="llvm-objcopy" \
        ${LLVM_PROJECT}/llvm
}

# Environment variables:
#  - LLVM_TARGETS: AArch64 or X86;AArch64
#  - LLVM_LIT_ARGS: LIT args for tests (can be unset or empty)
#  - BAKEWELL_NPROC: number of parallel jobs (default: 2)
function build_clang() {
    local LLVM_PROJECT=${1}         # path to LLVM sources
    local BAKEWELL_LLVM_PATH=${2}    # path to install BAKEWELL LLVM
    local BUILD_PATH=${3}           # path to the build folder
    local TPIP_PATH=${BAKEWELL_LLVM_PATH}/thirdpartylicences
    mkdir -p ${BUILD_PATH}
    pushd ${BUILD_PATH}
    __configure_clang ${LLVM_PROJECT} ${BAKEWELL_LLVM_PATH}
    cmake --build .
    cmake --install .
    pushd ${BAKEWELL_LLVM_PATH}/bin
    ln -sf clang riscv64-unknown-linux-gnu-cc
    popd
    popd
    mkdir -p ${TPIP_PATH}
    declare -a files=(
        llvm/LICENSE.TXT,LLVM-LICENSE.TXT
        clang/LICENSE.TXT,CLANG-LICENSE.TXT
        lldb/LICENSE.TXT,LLDB-LICENSE.TXT
        lld/LICENSE.TXT,LLD-LICENSE.TXT
        libcxx/LICENSE.TXT,LIBCXX-LICENSE.TXT
        libcxxabi/LICENSE.TXT,LIBCXXABI-LICENSE.TXT
        libunwind/LICENSE.TXT,LIBUNWIND-LICENSE.TXT
        compiler-rt/LICENSE.TXT,COMPILER-RT-LICENSE.TXT
        libclc/LICENSE.TXT,LIBCLC-LICENSE.TXT
        openmp/LICENSE.TXT,OPENMP-LICENSE.TXT
        parallel-libs/acxxel/LICENSE.TXT,PARALLEL-LIBS-ACXXEL-LICENSE.TXT
        polly/LICENSE.TXT,POLLY-LICENSE.TXT
        pstl/LICENSE.TXT,PSTL-LICENSE.TXT
        clang-tools-extra/LICENSE.TXT,CLANG-TOOLS-EXTRA-LICENSE.TXT
    )
    for t in ${files[@]}; do
        IFS="," read src dst <<< "${t}"
        if [ -f "${LLVM_PROJECT}/${src}" ]; then
            cp ${LLVM_PROJECT}/${src} ${TPIP_PATH}/${dst}
        fi
    done
}

build_clang $LLVM_PROJECT ${BAKEWELL_HOME}/cherillvm ${BUILDPREFIX}/llvm-bakewell
