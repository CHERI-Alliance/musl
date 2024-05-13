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

function __configure_libcxx() {
    local LLVM_PROJECT=${1}         # path to LLVM sources
    local TARGET_LLVM_PATH=${2}     # path where target LLVM has been installed
    local BUILD_PATH=${3}           # path to the build folder
    local SYSROOT=${4}              # path to sysroot with the required libc header
    local TRIPLE=${5}               # triple to target
    local TFLAGS="-march=rv64imafdc_zcheri-legacy -mabi=l64pc128d"
    mkdir -p ${BUILD_PATH}
    pushd ${BUILD_PATH}

    cat << EOF > toolchain.cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
set(CMAKE_ASM_COMPILER_TARGET "${TRIPLE}")
set(CMAKE_C_COMPILER_TARGET "${TRIPLE}")
set(CMAKE_CXX_COMPILER_TARGET "${TRIPLE}")

set(CMAKE_ASM_COMPILER_WORKS 1 CACHE INTERNAL "")
set(CMAKE_C_COMPILER_WORKS 1 CACHE INTERNAL "")
set(CMAKE_CXX_COMPILER_WORKS 1 CACHE INTERNAL "")

set(LIBCXX_TARGET_TRIPLE "${TARGET}" CACHE STRING "" FORCE)

set(CMAKE_ASM_COMPILER "${TARGET_LLVM_PATH}/bin/clang" CACHE FILEPATH "" FORCE)
set(CMAKE_C_COMPILER "${TARGET_LLVM_PATH}/bin/clang" CACHE FILEPATH "" FORCE)
set(CMAKE_CXX_COMPILER "${TARGET_LLVM_PATH}/bin/clang++" CACHE FILEPATH "" FORCE)
set(CMAKE_AR "${TARGET_LLVM_PATH}/bin/llvm-ar" CACHE FILEPATH "" FORCE)
set(CMAKE_RANLIB "${TARGET_LLVM_PATH}/bin/llvm-ranlib" CACHE FILEPATH "" FORCE)
set(CMAKE_NM "${TARGET_LLVM_PATH}/bin/llvm-nm" CACHE FILEPATH "" FORCE)
set(CMAKE_LINKER "${TARGET_LLVM_PATH}/bin/ld.lld" CACHE FILEPATH "" FORCE)
set(CMAKE_OBJDUMP "${TARGET_LLVM_PATH}/bin/llvm-objdump" CACHE FILEPATH "" FORCE)
set(CMAKE_OBJCOPY "${TARGET_LLVM_PATH}/bin/llvm-objcopy" CACHE FILEPATH "" FORCE)

set(LLVM_CONFIG_PATH "${TARGET_LLVM_PATH}/bin/llvm-config" CACHE FILEPATH "" FORCE)
set(CMAKE_ASM_FLAGS "--sysroot=${SYSROOT} ${TFLAGS}" CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS "--sysroot=${SYSROOT} ${TFLAGS}" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "--sysroot=${SYSROOT} ${TFLAGS}" CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS "-fuse-ld=lld -nostdlib --rtlib=compiler-rt" CACHE STRING "" FORCE)
set(CMAKE_SHARED_LINKER_FLAGS "-fuse-ld=lld -nostdlib --rtlib=compiler-rt" CACHE STRING "" FORCE)
EOF
    cmake -S ${LLVM_PROJECT}/runtimes \
    -B ${BUILD_PATH}\
    -Wno-dev \
    -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi;libunwind" \
    -DLIBCXX_ENABLE_STATIC=ON \
    -DLIBCXX_ENABLE_SHARED=ON \
    -DLIBCXX_INCLUDE_TESTS=ON \
    -DLIBCXX_INCLUDE_BENCHMARKS=OFF \
    -DLIBCXX_ENABLE_EXPERIMENTAL_LIBRARY=NO \
    -DLIBCXXABI_USE_LLVM_UNWINDER=ON \
    -DLIBCXX_USE_COMPILER_RT=ON \
    -DLIBCXX_TARGET_TRIPLE=${TRIPLE} \
    -DLIBCXX_HAS_MUSL_LIBC=ON \
    -DLIBCXX_ENABLE_EXCEPTIONS=ON \
    -DLIBCXX_SYSROOT="${SYSROOT}" \
    -DLIBCXX_CXX_ABI="libcxxabi" \
    -DLIBCXX_ENABLE_ABI_LINKER_SCRIPT=OFF \
    -DLIBCXX_INSTALL_INCLUDE_TARGET_DIR="${TARGET_LLVM_PATH}/include/${TRIPLE}/c++/v1" \
    -DCMAKE_INSTALL_PREFIX=${SYSROOT} \
    -DLIBCXX_TARGET_INFO="libcxx.test.target_info.LinuxLocalTI" \
    -DLIBCXX_TEST_COMPILER_FLAGS="--sysroot=${SYSROOT} ${TFLAGS} -isystem ${BUILD_PATH}/kernel-headers/usr/include" \
    -DLIBCXX_TEST_LINKER_FLAGS="--sysroot=${SYSROOT} -fuse-ld=lld -nostdlib --rtlib=compiler-rt -Wl,--dynamic-linker=${SYSROOT}/lib/libc.so ${SYSROOT}/lib/crt1.o ${SYSROOT}/lib/crti.o ${SYSROOT}/lib/crtn.o" \
    -DLIBCXX_CXX_ABI_LIBRARY_PATH="${SYSROOT}/lib" \
    -DLIBUNWIND_TARGET_TRIPLE=${TRIPLE} \
    -DLIBUNWIND_SYSROOT=${SYSROOT} \
    -DLIBUNWIND_ENABLE_STATIC=ON \
    -DLIBUNWIND_ENABLE_SHARED=ON \
    -DLIBUNWIND_ENABLE_THREADS=ON \
    -DLIBUNWIND_USE_COMPILER_RT=ON
    popd
}

# Environment variables:
#  - BAKEWELL_NPROC: number of parallel jobs (default: 4)
function build_libcxx() {
    local LLVM_PROJECT=${1}                      # path to LLVM sources
    local TARGET_LLVM_PATH=${2}                 # path where target LLVM has been installed
    local BUILD_PATH=${3}                        # path to the build folder
    local SYSROOT=${4}                           # path to sysroot with the required libc headers
    local TRIPLE=${5}                            # triple to target
    rm -rf ${BUILD_PATH}
    mkdir -p ${BUILD_PATH}
    __configure_libcxx ${LLVM_PROJECT} ${TARGET_LLVM_PATH} ${BUILD_PATH} ${SYSROOT} ${TRIPLE} ${KERNEL_BRANCH}
    pushd ${BUILD_PATH}
    make -j${BAKEWELL_NPROC:-4}
    make install
    popd
}

build_libcxx ${LLVM_PROJECT} ${BAKEWELL_HOME}/cherillvm ${BUILDPREFIX}/libcxx ${BAKEWELL_HOME} riscv64-unknown-linux-gnu
