#!/bin/bash

set -xe

export MORELLOIE_PREFIX=${HOME}/morelloie
export MORELLOIE=${MORELLOIE_PREFIX}/bin/morelloie

# Install Morello IE
rm -rf ${HOME}/morelloie-* ${MORELLOIE_PREFIX}
wget -q ${MORELLOIE_DOWNLOAD_URL}/morelloie-${MORELLOIE_VERSION}.tgz.sh -O ${HOME}/morelloie-${MORELLOIE_VERSION}.tgz.sh
bash ${HOME}/morelloie-${MORELLOIE_VERSION}.tgz.sh --i-agree-to-the-contained-eula --prefix=${MORELLOIE_PREFIX}

# Install Morello LLVM
export LLVM_PREFIX=${HOME}/llvm
rm -rf ${LLVM_PREFIX}
mkdir -p ${LLVM_PREFIX}
pushd ${LLVM_PREFIX}
git init
repo=https://git.morello-project.org/morello/llvm-project-releases.git
branch=morello/linux-aarch64-release-${MORELLO_LLVM_VERSION}
git fetch --depth=1 -- ${repo} +refs/heads/${branch}:refs/remotes/origin/${branch}
git checkout origin/${branch} -b ${branch}
popd

# Build Musl for AArch64
export MUSL_PREFIX_AARCH64=${HOME}/musl-sysroot-hybrid
rm -rf ${MUSL_PREFIX_AARCH64}
make distclean
CC=${LLVM_PREFIX}/bin/clang ./configure --prefix=${MUSL_PREFIX_AARCH64} --disable-morello --target=aarch64-linux-gnu
make -j8
make install

# Build Musl for Purecap
export MUSL_PREFIX_PURECAP=${HOME}/musl-sysroot-purecap
rm -rf ${MUSL_PREFIX_PURECAP}
make distclean
CC=${LLVM_PREFIX}/bin/clang ./configure --prefix=${MUSL_PREFIX_PURECAP} --target=aarch64-linux-musl_purecap
make -j8
make install

# Compile tests for Purecap
make -j8 -C test build

# Run Purecap tests on Morello IE
export TEST_RUNNER_TAGS='mie purecap'
export TEST_DRIVER=${MORELLOIE}
make test

# Prepare test package
export TEST_RUNNER_TAGS='morello purecap'
make -C test bundle-tests
pushd test
gzip morello-musl-tests.tar
mv morello-musl-tests.tar.gz morello-musl-tests-purecap.tar.gz
popd
