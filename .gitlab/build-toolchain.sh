#!/bin/bash

set -xe

sudo apt install -q=2 --yes --no-install-recommends --no-install-suggests wget

# Env vars for AArch64-hosted build
export MUSL_PATH=${PWD}
export CLANG_WORKSPACE=${PWD}/clang
export LLVM_PROJECT_PATH=${CLANG_WORKSPACE}/llvm-project
export MORELLO_LLVM_PATH=${CLANG_WORKSPACE}/${BUNDLE_NAME}
export BUILD_PATH=${CLANG_WORKSPACE}/build
export HOST_LLVM_PATH=/usr/lib/llvm-13
export AARCH64_SYSROOT=${CLANG_WORKSPACE}/sysroot-aarch64
export PURECAP_SYSROOT=${CLANG_WORKSPACE}/sysroot-purecap
export LLVM_LIT_ARGS="-s --no-progress-bar --workers=20 --xunit-xml-output ${CLANG_WORKSPACE}/${LLVM_TEST_REPORT}"

# Checkout LLVM sources
mkdir -p ${LLVM_PROJECT_PATH}
pushd ${LLVM_PROJECT_PATH}
git init
repo=https://git.morello-project.org/morello/llvm-project.git
git fetch --depth=1 -- ${repo} +refs/heads/${LLVM_BRANCH}:refs/remotes/origin/${LLVM_BRANCH}
git checkout origin/${LLVM_BRANCH} -b ${LLVM_BRANCH}
git status
popd

# Build Clang
bash ${MUSL_PATH}/tools/build-morello.sh clang ${LLVM_PROJECT_PATH} ${HOST_LLVM_PATH} ${MORELLO_LLVM_PATH} ${BUILD_PATH}
${MORELLO_LLVM_PATH}/bin/clang --version

# Run Clang tests
LD_LIBRARY_PATH=${BUILD_PATH}/lib bash ${MUSL_PATH}/tools/build-morello.sh clang-test ${BUILD_PATH}

# Install Musl headers
export CC=${MORELLO_LLVM_PATH}/bin/clang
bash ${MUSL_PATH}/tools/build-morello.sh musl-headers ${MUSL_PATH} ${AARCH64_SYSROOT} aarch64-unknown-linux-gnu
bash ${MUSL_PATH}/tools/build-morello.sh musl-headers ${MUSL_PATH} ${PURECAP_SYSROOT} aarch64-unknown-linux-musl_purecap

# Build CRT objects
export CC=${MORELLO_LLVM_PATH}/bin/clang
bash ${MUSL_PATH}/tools/build-morello.sh crt ${LLVM_PROJECT_PATH} ${AARCH64_SYSROOT} aarch64-unknown-linux-gnu
bash ${MUSL_PATH}/tools/build-morello.sh crt ${LLVM_PROJECT_PATH} ${PURECAP_SYSROOT} aarch64-unknown-linux-musl_purecap

# Build compiler-rt
export CC=${MORELLO_LLVM_PATH}/bin/clang
bash ${MUSL_PATH}/tools/build-morello.sh compiler-rt ${LLVM_PROJECT_PATH} ${MORELLO_LLVM_PATH} ${BUILD_PATH}-rt ${AARCH64_SYSROOT} aarch64-unknown-linux-gnu
bash ${MUSL_PATH}/tools/build-morello.sh compiler-rt ${LLVM_PROJECT_PATH} ${MORELLO_LLVM_PATH} ${BUILD_PATH}-rt ${PURECAP_SYSROOT} aarch64-unknown-linux-musl_purecap

# Package Clang
pushd ${CLANG_WORKSPACE}
echo ${LLVM_VERSION} > ${MORELLO_LLVM_PATH}/VERSION.txt
rm -vf ${BUNDLE_NAME}*.tar.gz
bash ${MUSL_PATH}/tools/build-morello.sh package ${MORELLO_LLVM_PATH} ${BUNDLE_NAME}
popd
