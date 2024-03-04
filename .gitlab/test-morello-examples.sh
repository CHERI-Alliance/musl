#!/bin/bash

set -xe

export MORELLOIE_PREFIX=${HOME}/morelloie
export MORELLOIE=${MORELLOIE_PREFIX}/bin/morelloie
export LLVM_PREFIX=${HOME}/llvm
export TEST_WORKSPACE=${PWD}/morello-examples

git config --global --add safe.directory ${PWD}

# Install Morello IE
rm -rf ${HOME}/morelloie-* ${MORELLOIE_PREFIX}
wget -q ${MORELLOIE_DOWNLOAD_URL}/morelloie-${MORELLOIE_VERSION}.tgz.sh -O ${HOME}/morelloie-${MORELLOIE_VERSION}.tgz.sh
bash ${HOME}/morelloie-${MORELLOIE_VERSION}.tgz.sh --i-agree-to-the-contained-eula --prefix=${MORELLOIE_PREFIX}

# Install Morello LLVM
rm -rf ${LLVM_PREFIX}
mkdir -p ${LLVM_PREFIX}
pushd ${LLVM_PREFIX}
git init
repo=https://git.morello-project.org/morello/llvm-project-releases.git
branch=morello/linux-aarch64-release-${MORELLO_LLVM_VERSION}
git fetch --depth=1 -- ${repo} +refs/heads/${branch}:refs/remotes/origin/${branch}
git checkout origin/${branch} -b ${branch}
popd

# Checkout Morello Examples
rm -rf ${TEST_WORKSPACE}
mkdir -p ${TEST_WORKSPACE}
pushd ${TEST_WORKSPACE}
git init
repo=https://git.morello-project.org/morello/morello-examples.git
git fetch --depth=1 -- ${repo} +refs/heads/${MORELLO_EXAMPLES_BRANCH}:refs/remotes/origin/${MORELLO_EXAMPLES_BRANCH}
git checkout origin/${MORELLO_EXAMPLES_BRANCH} -b ${MORELLO_EXAMPLES_BRANCH}
popd

# Build Musl for AArch64
export MUSL_PREFIX_AARCH64=${HOME}/musl-sysroot-hybrid
rm -rf ${MUSL_PREFIX_AARCH64}
make distclean
CC=${LLVM_PREFIX}/bin/clang ./configure --disable-shared --prefix=${MUSL_PREFIX_AARCH64} --disable-morello --target=aarch64-linux-gnu
make -j8
make install
ls -la ${MUSL_PREFIX_AARCH64}

# Build Musl for Purecap
export MUSL_PREFIX_PURECAP=${HOME}/musl-sysroot-purecap
rm -rf ${MUSL_PREFIX_PURECAP}
make distclean
CC=${LLVM_PREFIX}/bin/clang ./configure --disable-shared --prefix=${MUSL_PREFIX_PURECAP} --target=aarch64-linux-musl_purecap
make -j8
make install
ls -la ${MUSL_PREFIX_PURECAP}

# Run tests
pushd ${TEST_WORKSPACE}
touch config.make
make distclean
./configure CC=${LLVM_PREFIX}/bin/clang --sysroot=${MUSL_PREFIX_PURECAP} --sysroot-hybrid=${MUSL_PREFIX_AARCH64}
make
make test TEST_RUNNER="${MORELLOIE} --"
popd
