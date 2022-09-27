#!/usr/bin/env bash

# How to build Morello toolchain using this script
# ================================================
#
# Prerequisites
# -------------
#  - Host LLVM (11.0 or above).
#  - Cmake (3.13.4 or above)
#  - Make (4.2)
#  - Python (3.6 or above)
#
# Note: be aware of use of the env variables in the below commands.
# Note: all paths should be absolute.
#
#
# Configure and build Clang
# -------------------------
#
# LLVM_TARGETS="<targets>" LLVM_LIT_ARGS="<args-for-tests>" \
#   bash build-morello.sh clang \
#   /path/to/llvm-project \
#   /path/to/host/llvm \
#   /path/to/morello/llvm \
#   /path/to/build/directory
#
# For targets use "AArch64" for AArch64-hosted toolchain and "X86;AArch64" for x86-hosted
# toolchain. Clang will be installed in the "/path/to/morello/llvm" directory.
#
#
# Install Musl headers
# --------------------
#
# This installs Musl headers for Morello:
#
# CC=/path/to/morello/llvm/bin/clang \
#   bash build-morello.sh musl-headers \
#   /path/to/musl/sources \
#   /path/to/morello/musl/install \
#   aarch64-unknown-linux-musl_purecap
#
# To install headers for AArch64, use `aarch64-unknown-linux-gnu` triple.
#
#
# Build CRT objects
# -----------------
#
# This builds CRT objects for Morello:
#
# CC=/path/to/morello/llvm/bin/clang \
#   bash build-morello.sh crt \
#   /path/to/llvm-project \
#   /path/to/morello/musl/install \
#   aarch64-unknown-linux-musl_purecap
#
# To build CRT objects for AArch64, use `aarch64-unknown-linux-gnu` triple.
#
#
# Built Compiler-RT
# -----------------
#
# This builds Compiler-RT for Morello:
#
# CC=/path/to/morello/llvm/bin/clang \
#   bash build-morello.sh compiler-rt \
#   /path/to/llvm-project \
#   /path/to/morello/llvm \
#   /path/to/comp-rt/build/directory \
#   /path/to/morello/musl/install \
#   aarch64-unknown-linux-musl_purecap
#
# To build Compiler-RT for AArch64, use `aarch64-unknown-linux-gnu` triple.
#
#
# Build Musl libc
# ---------------
#
# CC=/path/to/morello/llvm/bin/clang \
#   bash build-morello.sh musl \
#   /path/to/musl/sources \
#   /path/to/musl/install /path/to/libshim/sources \
#   aarch64-unknown-linux-musl_purecap
#
# Provide last argument to build libshim-based Musl and omit it to build Musl without
# libshim. Musl will be installed in the "/path/to/musl/install" directory.
# Use `NOSHIM` instead of `/path/to/libshim/sources` for no-libshim build.
#
#
# Compile a hello world app
# -------------------------
#
# /path/to/morello/llvm/bin/clang --target=aarch64-linux-musl_purecap \
#   -march=morello+c64 --sysroot /path/to/musl/install \
#   hello.c -o hello [--static]
#
# END-OF-HOWTO

STAGE=${1} # stage to run: clang, clang-test, musl, crt, compiler-rt, musl-test, package

case ${STAGE} in
  -help|--help|help)
      echo "Usage: ${0} STAGE [ARGS]"
      echo "Stages: clang, clang-test, musl-headers, musl, crt, compiler-rt, musl-test, libc-test, package"
      echo ""
      sed -n '/^# How to/,${p;/^# END-OF-HOWTO/q}' ${0}
      echo ""
      exit 0
      ;;
esac

set -x
set -e

MORELLO_TRIPLE=aarch64-unknown-linux-musl_purecap
AARCH64_TRIPLE=aarch64-unknown-linux-gnu

# Environment variables:
#  - LLVM_TARGETS: AArch64 or X86;AArch64
#  - LLVM_LIT_ARGS: LIT args for tests (can be unset or empty)
function configure_clang() {
    local LLVM_PROJECT=${1}
    local HOST_LLVM_BIN=${2}
    local MORELLO_HOME=${3}
    mkdir -p ${MORELLO_HOME}
    cmake -Wno-dev \
        -DCMAKE_C_COMPILER=${HOST_LLVM_BIN}/clang \
        -DCMAKE_C_COMPILER_WORKS=YES \
        -DCMAKE_ASM_COMPILER=${HOST_LLVM_BIN}/clang \
        -DCMAKE_ASM_COMPILER_WORKS=YES \
        -DCMAKE_CXX_COMPILER=${HOST_LLVM_BIN}/clang++ \
        -DCMAKE_CXX_COMPILER_WORKS=YES \
        -DCMAKE_AR=${HOST_LLVM_BIN}/llvm-ar \
        -DCMAKE_RANLIB=${HOST_LLVM_BIN}/llvm-ranlib \
        -DCMAKE_NM=${HOST_LLVM_BIN}/llvm-nm \
        -DCMAKE_LINKER=${HOST_LLVM_BIN}/ld.lld \
        -DCMAKE_OBJDUMP=${HOST_LLVM_BIN}/llvm-objdump \
        -DCMAKE_OBJCOPY=${HOST_LLVM_BIN}/llvm-objcopy \
        -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=lld" \
        -DCMAKE_SHARED_LINKER_FLAGS="-fuse-ld=lld" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=${MORELLO_HOME} \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        -DCMAKE_SKIP_BUILD_RPATH=OFF \
        -DCMAKE_INSTALL_RPATH=\$ORIGIN/../lib \
        -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON \
        -DLLVM_ENABLE_PROJECTS="clang;lld;lldb;libcxx;libcxxabi;compiler-rt;libunwind" \
        -DLLVM_TARGETS_TO_BUILD="${LLVM_TARGETS}" \
        -DLLVM_ENABLE_ASSERTIONS=OFF \
        -DLLVM_ENABLE_LIBCXX=ON \
        -DLLVM_ENABLE_LLD=ON \
        -DLLVM_ENABLE_EH=ON \
        -DLLVM_ENABLE_RTTI=ON \
        -DBUILD_SHARED_LIBS=ON \
        -DCOMPILER_RT_BUILD_BUILTINS=ON \
        -DCOMPILER_RT_BUILD_XRAY=OFF \
        -DCOMPILER_RT_BUILD_LIBFUZZER=OFF \
        -DCOMPILER_RT_BUILD_PROFILE=OFF \
        -DLIBCXX_CXX_ABI=libcxxabi \
        -DLIBCXX_CXX_ABI_INCLUDE_PATHS="${LLVM_PROJECT}/libcxxabi/include" \
        -DLIBCXX_USE_COMPILER_RT=ON \
        -DLIBCXX_ENABLE_THREADS=ON \
        -DLIBCXXABI_ENABLE_THREADS=ON \
        -DLIBCXXABI_USE_LLVM_UNWINDER=ON \
        -DLIBCXXABI_USE_COMPILER_RT=ON \
        -DLIBUNWIND_ENABLE_THREADS=ON \
        -DLLVM_LIT_ARGS="${LLVM_LIT_ARGS}" \
        -DCLANG_DEFAULT_RTLIB="compiler-rt" \
        -DCLANG_DEFAULT_CXX_STDLIB="libc++" \
        -DCLANG_DEFAULT_LINKER="lld" \
        -DCLANG_DEFAULT_OBJCOPY="llvm-objcopy" \
        ${LLVM_PROJECT}/llvm
}

function configure_comp_rt() {
    local LLVM_PROJECT=${1}         # path to LLVM sources
    local MORELLO_LLVM_PATH=${2}    # path where Morello LLVM has been installed
    local BUILD_PATH=${3}           # path to the build folder
    local SYSROOT=${4}              # path to sysroot with the required libc headers
    local TRIPLE=${5}               # triple to target
    if [[ "${TRIPLE}" == "${MORELLO_TRIPLE}" ]]; then
        local TFLAGS="-march=morello+c64 -mabi=purecap"
    else
        local TFLAGS="-march=armv8"
    fi
    mkdir -p ${BUILD_PATH}
    pushd ${BUILD_PATH}
    cat << EOF > toolchain.cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_C_COMPILER_TARGET "${TRIPLE} ${TFLAGS}")

set(CMAKE_C_COMPILER_WORKS 1 CACHE INTERNAL "")
set(CMAKE_CXX_COMPILER_WORKS 1 CACHE INTERNAL "")

set(CMAKE_C_COMPILER "${MORELLO_LLVM_PATH}/bin/clang" CACHE FILEPATH "" FORCE)
set(CMAKE_CXX_COMPILER "${MORELLO_LLVM_PATH}/bin/clang++" CACHE FILEPATH "" FORCE)
set(CMAKE_AR "${MORELLO_LLVM_PATH}/bin/llvm-ar" CACHE FILEPATH "" FORCE)
set(CMAKE_RANLIB "${MORELLO_LLVM_PATH}/bin/llvm-ranlib" CACHE FILEPATH "" FORCE)
set(CMAKE_NM "${MORELLO_LLVM_PATH}/bin/llvm-nm" CACHE FILEPATH "" FORCE)
set(CMAKE_LINKER "${MORELLO_LLVM_PATH}/bin/ld.lld" CACHE FILEPATH "" FORCE)
set(CMAKE_OBJDUMP "${MORELLO_LLVM_PATH}/bin/llvm-objdump" CACHE FILEPATH "" FORCE)
set(CMAKE_OBJCOPY "${MORELLO_LLVM_PATH}/bin/llvm-objcopy" CACHE FILEPATH "" FORCE)

set(LLVM_CONFIG_PATH "${MORELLO_LLVM_PATH}/bin/llvm-config" CACHE FILEPATH "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS "-fuse-ld=lld" CACHE FILEPATH "" FORCE)
set(CMAKE_SHARED_LINKER_FLAGS "-fuse-ld=lld" CACHE FILEPATH "" FORCE)
EOF
    cmake -Wno-dev \
        -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_C_FLAGS="-nostdinc -isystem ${SYSROOT}/include" \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        -DCMAKE_SKIP_BUILD_RPATH=OFF \
        -DCMAKE_INSTALL_RPATH=\$ORIGIN/../lib \
        -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON \
        -DLLVM_TARGETS_TO_BUILD="AArch64" \
        -DLLVM_ENABLE_ASSERTIONS=OFF \
        -DBUILD_SHARED_LIBS=ON \
        -DCOMPILER_RT_DEFAULT_TARGET_TRIPLE=${TRIPLE} \
        -DCOMPILER_RT_BUILD_BUILTINS=ON \
        -DCOMPILER_RT_BUILD_SANITIZERS=OFF \
        -DCOMPILER_RT_BUILD_XRAY=OFF \
        -DCOMPILER_RT_BUILD_LIBFUZZER=OFF \
        -DCOMPILER_RT_BUILD_PROFILE=OFF \
        ${LLVM_PROJECT}/compiler-rt
    popd
}

# Environment variables:
#  - LLVM_TARGETS: AArch64 or X86;AArch64
#  - LLVM_LIT_ARGS: LIT args for tests (can be unset or empty)
#  - MORELLO_NPROC: number of parallel jobs (default: 16)
function build_clang() {
    local LLVM_PROJECT=${1}         # path to LLVM sources
    local HOST_LLVM_PATH=${2}       # path to host LLVM (11.0 or newer)
    local MORELLO_LLVM_PATH=${3}    # path to install Morello LLVM
    local BUILD_PATH=${4}           # path to the build folder
    local TPIP_PATH=${MORELLO_LLVM_PATH}/thirdpartylicences
    echo "Building clang from ${LLVM_PROJECT} with ${HOST_LLVM_PATH} ..."
    mkdir -p ${BUILD_PATH}
    pushd ${BUILD_PATH}
    configure_clang ${LLVM_PROJECT} ${HOST_LLVM_PATH}/bin ${MORELLO_LLVM_PATH}
    make -j${MORELLO_NPROC:-16}
    make install
    popd
    mkdir -p ${TPIP_PATH}
    cp ${LLVM_PROJECT}/llvm/LICENSE.TXT ${TPIP_PATH}/LLVM-LICENSE.TXT
    cp ${LLVM_PROJECT}/clang/LICENSE.TXT ${TPIP_PATH}/CLANG-LICENSE.TXT
    cp ${LLVM_PROJECT}/lldb/LICENSE.TXT ${TPIP_PATH}/LLDB-LICENSE.TXT
    cp ${LLVM_PROJECT}/lld/LICENSE.TXT ${TPIP_PATH}/LLD-LICENSE.TXT
    cp ${LLVM_PROJECT}/libcxx/LICENSE.TXT ${TPIP_PATH}/LIBCXX-LICENSE.TXT
    cp ${LLVM_PROJECT}/libcxxabi/LICENSE.TXT ${TPIP_PATH}/LIBCXXABI-LICENSE.TXT
    cp ${LLVM_PROJECT}/libunwind/LICENSE.TXT ${TPIP_PATH}/LIBUNWIND-LICENSE.TXT
    cp ${LLVM_PROJECT}/compiler-rt/LICENSE.TXT ${TPIP_PATH}/COMPILER-RT-LICENSE.TXT
    cp ${LLVM_PROJECT}/libclc/LICENSE.TXT ${TPIP_PATH}/LIBCLC-LICENSE.TXT
    cp ${LLVM_PROJECT}/openmp/LICENSE.TXT ${TPIP_PATH}/OPENMP-LICENSE.TXT
    cp ${LLVM_PROJECT}/parallel-libs/acxxel/LICENSE.TXT ${TPIP_PATH}/PARALLEL-LIBS-ACXXEL-LICENSE.TXT
    cp ${LLVM_PROJECT}/polly/LICENSE.TXT ${TPIP_PATH}/POLLY-LICENSE.TXT
    cp ${LLVM_PROJECT}/pstl/LICENSE.TXT ${TPIP_PATH}/PSTL-LICENSE.TXT
    cp ${LLVM_PROJECT}/clang-tools-extra/LICENSE.TXT ${TPIP_PATH}/CLANG-TOOLS-EXTRA-LICENSE.TXT
}

# Environment variables:
#  - LD_LIBRARY_PATH: path to the `lib` folder in the LLVM build directory
#  - MORELLO_NPROC: number of parallel jobs (default: 16)
function build_clang_test() {
    local BUILD_PATH=${1}           # path to the LLVM build folder
    pushd ${BUILD_PATH}
    make -j${MORELLO_NPROC:-16} UnitTests
    make check-llvm
    popd
}

# Environment variables:
#  - CC: path to Morello clang
function build_musl_headers() {
    local MUSL_PATH=${1}            # path to Musl sources
    local PREFIX_PATH=${2}          # where to install Musl headers
    local TRIPLE=${3}               # target triple
    if [[ "${TRIPLE}" == "${MORELLO_TRIPLE}" ]]; then
        local CFGFLAGS="--enable-morello --disable-libshim --disable-shared"
    else
        local CFGFLAGS="--disable-morello --disable-libshim --disable-shared"
    fi
    rm -rf ${PREFIX_PATH}
    pushd ${MUSL_PATH}
    make distclean
    ./configure ${CFGFLAGS} --prefix=${PREFIX_PATH} --target=${TRIPLE}
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
    if [[ "${TRIPLE}" == "${MORELLO_TRIPLE}" ]]; then
        local XFLAGS="--target=${TRIPLE} -march=morello+c64 -mabi=purecap -nostdinc -isystem ${INCLUDE}"
    else
        local XFLAGS="--target=${TRIPLE} -nostdinc -isystem ${INCLUDE}"
    fi
    mkdir -p ${DESTDIR}
    ${CC} ${XFLAGS} -c ${CRT}/crtbegin.c -o ${DESTDIR}/clang_rt.crtbegin.o
    ${CC} ${XFLAGS} -c ${CRT}/crtend.c -o ${DESTDIR}/clang_rt.crtend.o
}

# Environment variables:
#  - CC: path to Morello clang
#  - MORELLO_NPROC: number of parallel jobs (default: 4)
function build_compiler_rt() {
    local LLVM_PROJECT=${1}         # path to LLVM sources
    local MORELLO_LLVM_PATH=${2}    # path where Morello LLVM has been installed
    local BUILD_PATH=${3}           # path to the build folder
    local SYSROOT=${4}              # path to sysroot with the required libc headers
    local TRIPLE=${5}               # expanded target triple
    local DESTDIR=$(${CC} -print-resource-dir)/lib/${TRIPLE}
    rm -rf ${BUILD_PATH}
    configure_comp_rt ${LLVM_PROJECT} ${MORELLO_LLVM_PATH} ${BUILD_PATH} ${SYSROOT} ${TRIPLE}
    pushd ${BUILD_PATH}
    make -j${MORELLO_NPROC:-4} clang_rt.builtins-aarch64
    cp lib/linux/libclang_rt.builtins-aarch64.a ${DESTDIR}/libclang_rt.builtins.a
    popd
}

# This works only for kernel version 5.x
function download_kernel_headers() {
    local BUILD_PATH=${1}           # folder where kernel headers will be stored
    local VERSION=${2}              # kernel version
    local URL=https://cdn.kernel.org/pub/linux/kernel/v5.x/linux-${VERSION}.tar.xz
    mkdir -p ${BUILD_PATH}
    pushd ${BUILD_PATH}
    wget -q ${URL}
    tar -xf linux-${VERSION}.tar.xz linux-${VERSION}/arch/arm64 linux-${VERSION}/include linux-${VERSION}/scripts linux-${VERSION}/Makefile
    make -C linux-${VERSION} --silent headers_install ARCH=arm64 INSTALL_HDR_PATH=${BUILD_PATH}/kernel
    popd
    rm -rf ${BUILD_PATH}/linux-${VERSION} ${BUILD_PATH}/linux-${VERSION}.tar.xz
}

# Environment variables:
#  - CC: path to Morello clang
#  - MORELLO_NPROC: number of parallel jobs (default: 8)
function build_musl() {
    local MUSL_PATH=${1}            # path to Musl sources
    local PREFIX_PATH=${2}          # where to install Musl
    local LIBSHIM_PATH=${3:-NOSHIM} # path to libshim (omit for non-libshim build)
    local TRIPLE=${4}               # target triple
    if [[ "${TRIPLE}" == "${MORELLO_TRIPLE}" ]]; then
        if [[ "${LIBSHIM_PATH}" == "NOSHIM" ]]; then
            local CFGFLAGS="--enable-morello --disable-libshim"
        else
            local CFGFLAGS="--enable-morello --enable-libshim --libshim-path=${LIBSHIM_PATH}"
        fi
    else
        local CFGFLAGS="--disable-morello --disable-libshim"
    fi
    rm -rf ${PREFIX_PATH}
    pushd ${MUSL_PATH}
    make distclean
    ./configure --prefix=${PREFIX_PATH} --target=${TRIPLE} ${CFGFLAGS}
    if [[ "${TRIPLE}" == "${MORELLO_TRIPLE}" ]]; then
        if [[ "${LIBSHIM_PATH}" != "NOSHIM" ]]; then
            download_kernel_headers ${MUSL_PATH}/lib 5.19
            KERNEL_HEADER_INCLUDES="-isystem ${MUSL_PATH}/lib/kernel/include" make -j${MORELLO_NPROC:-8}
        else
            make -j${MORELLO_NPROC:-8}
        fi
    else
        make -j${MORELLO_NPROC:-8}
    fi
    make install
    mkdir -p ${PREFIX_PATH}/share
    cp COPYRIGHT ${PREFIX_PATH}/share/MUSL-LICENSE.txt
    wget -q https://www.apache.org/licenses/LICENSE-2.0.txt -O ${PREFIX_PATH}/LICENSE.txt
    cat << EOF > ${PREFIX_PATH}/NOTICE.txt
This product embeds and uses the following pieces of software
which have additional or alternate licenses:
 - Musl libc: share/MUSL-LICENSE.txt
EOF
    popd
    if [[ "${LIBSHIM_PATH}" != "NOSHIM" ]]; then
        cp ${LIBSHIM_PATH}/LICENSE.txt ${PREFIX_PATH}/share/LIBSHIM-LICENSE.txt
        cat << EOF >> ${PREFIX_PATH}/NOTICE.txt
 - Libshim: share/LIBSHIM-LICENSE.txt
EOF
    fi
}

# Environment variables:
#  - TEST_DRIVER: path to the test driver script or Morello IE
#  - CC: path to Morello clang (when libc-test tests are used)
#  - MORELLO_NPROC: number of parallel jobs (default: 8)
function build_musl_test() {
    local MUSL_PATH=${1}            # path to Musl sources
    local PREFIX_PATH=${2}          # where Musl has been installed
    local TRIPLE=${3}               # target triple
    local SKIP_TEST_RUN=${4:-NO}    # whether to skip running tests
    if [[ "${TRIPLE}" == "${MORELLO_TRIPLE}" ]]; then  # these test only for Morello
        local ARCHFLAGS=${ARCHFLAGS:--march=morello+c64}
        local CFGFLAGS="--enable-morello --disable-libshim"
        pushd ${MUSL_PATH}
        make distclean
        ./configure --prefix=${PREFIX_PATH} --target=${TRIPLE} ${CFGFLAGS}

        make -C test clean
        make -C test build -j${MORELLO_NPROC:-8}
        if [[ "${SKIP_TEST_RUN}" == "NO" ]]; then
            make -C test test
        fi
        popd
    fi
}

# Environment variables:
#  - TEST_DRIVER: path to the test driver script or Morello IE
#  - CC: path to Morello clang (when libc-test tests are used)
#  - MORELLO_NPROC: number of parallel jobs (default: 8)
#  - TESTPKG: base name for test report
function build_libc_test() {
    local MUSL_PATH=${1}            # path to Musl sources
    local PREFIX_PATH=${2}          # where Musl has been installed
    local TRIPLE=${3}               # target triple
    local LIBC_TEST_PATH=${4}       # path to libc-test suite sources
    local SKIP_TEST_RUN=${5:-NO}    # whether to skip running tests
    if [[ "${TRIPLE}" == "${MORELLO_TRIPLE}" ]]; then
        local ARCHFLAGS=${ARCHFLAGS:--march=morello+c64}
    else
        local ARCHFLAGS=${ARCHFLAGS:--march=armv8-a}
    fi
    local TESTS=${MUSL_PATH}/test/libc-test-enabled-tests.txt
    local TESTPKG=${TESTPKG:-musl.libc-test.${TRIPLE}}
    pushd ${LIBC_TEST_PATH}
    make clean
    make -j${MORELLO_NPROC:-8} build \
        TESTS=${TESTS} TESTPKG=${TESTPKG} SYSROOT=${PREFIX_PATH} TRIPLE=${TRIPLE} ARCHFLAGS=${ARCHFLAGS}
    if [[ "${SKIP_TEST_RUN}" == "NO" ]]; then
        make run TESTS=${TESTS} TESTPKG=${TESTPKG} SYSROOT=${PREFIX_PATH} TRIPLE=${TRIPLE} ARCHFLAGS=${ARCHFLAGS}
    fi
    popd
}

function build_package() {
    local MORELLO_LLVM_PATH=${1}    # path where Morello LLVM has been installed
    local BUNDLE=${2}               # name of the tarball
    local CWD=$(pwd)
    local PDIR=$(dirname ${MORELLO_LLVM_PATH})
    local PNAME=$(basename ${MORELLO_LLVM_PATH})
    wget -q https://www.apache.org/licenses/LICENSE-2.0.txt -O ${MORELLO_LLVM_PATH}/LICENSE.txt
    cat << EOF > ${MORELLO_LLVM_PATH}/NOTICE.txt
This product embeds and uses the following pieces of software which have
additional or alternate licenses:
 - LLVM: thirdpartylicences/LLVM-LICENSE.TXT
 - Clang: thirdpartylicences/CLANG-LICENSE.TXT
 - lldb: thirdpartylicences/LLDB-LICENSE.TXT
 - lld: thirdpartylicences/LLD-LICENSE.TXT
 - libc++: thirdpartylicences/LIBCXX-LICENSE.TXT
 - libc++abi: thirdpartylicences/LIBCXXABI-LICENSE.TXT
 - libunwind: thirdpartylicences/LIBUNWIND-LICENSE.TXT
 - libclc: thirdpartylicences/LIBCLC-LICENSE.TXT
 - openmp: thirdpartylicences/OPENMP-LICENSE.TXT
 - parallel-libs: thirdpartylicences/PARALLEL-LIBS-ACXXEL-LICENSE.TXT
 - polly: thirdpartylicences/POLLY-LICENSE.TXT
 - pstl: thirdpartylicences/PSTL-LICENSE.TXT
 - clang-tools-extra: thirdpartylicences/CLANG-TOOLS-EXTRA-LICENSE.TXT
 - compiler-rt: thirdpartylicences/COMPILER-RT-LICENSE.TXT
EOF
    pushd ${PDIR}
    tar --transform 's|^'${PNAME}'|'${BUNDLE}'|' -czf ${CWD}/${BUNDLE}-clang.tar.gz ${PNAME}
    popd
}

case ${STAGE} in
  clang)
      build_clang ${@:2};
      exit 0;
      ;;
  clang-test)
      build_clang_test ${@:2};
      exit 0;
      ;;
  musl)
      build_musl ${@:2};
      exit 0;
      ;;
  musl-headers)
      build_musl_headers ${@:2};
      exit 0;
      ;;
  crt)
      build_crt ${@:2};
      exit 0;
      ;;
  compiler-rt)
      build_compiler_rt ${@:2};
      exit 0;
      ;;
  musl-test)
      build_musl_test ${@:2};
      exit 0;
      ;;
  libc-test)
      build_libc_test ${@:2};
      exit 0;
      ;;
  package)
      build_package ${@:2};
      exit 0;
      ;;
  *)
      echo "Unknown stage ${STAGE}"
      exit 1
      ;;
esac
