#!/bin/bash

set -xe

git config --global --add safe.directory ${PWD}

# Install Morello LLVM
export LLVM_PREFIX=${HOME}/llvm
rm -rf ${LLVM_PREFIX}
mkdir -p ${LLVM_PREFIX}
pushd ${LLVM_PREFIX}
git init
repo=https://git.morello-project.org/morello/llvm-project-releases.git
branch=morello/linux-release-${MORELLO_LLVM_VERSION}
git fetch --depth=1 -- ${repo} +refs/heads/${branch}:refs/remotes/origin/${branch}
git checkout origin/${branch} -b ${branch}
popd

# Build Musl for AArch64
export MUSL_PREFIX_AARCH64=${HOME}/musl-sysroot-hybrid
make distclean
CC=${LLVM_PREFIX}/bin/clang ./configure --prefix=${MUSL_PREFIX_AARCH64} --disable-morello --target=aarch64-linux-gnu
make -j8
make install

# Build Musl for Purecap
export MUSL_PREFIX_PURECAP=${HOME}/musl-sysroot-purecap
make distclean
CC=${LLVM_PREFIX}/bin/clang ./configure --prefix=${MUSL_PREFIX_PURECAP} --target=aarch64-linux-musl_purecap
make -j8
make install

# Compile tests for Purecap
make -j8 -C test build
