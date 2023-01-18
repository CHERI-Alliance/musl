Musl libc for Morello
=====================

Work-in-progress port to Morello
--------------------------------

Disclaimer
^^^^^^^^^^

This repository contains *work-in-progress* port of Musl libc to `Morello`_ targeting
purecap ABI. It is intended for experimental use.

.. _Morello: https://www.arm.com/architecture/cpu/morello

Using this library
------------------

To build Morello Musl libc, use Morello toolchain, for example, Morello LLVM (see below).
This toolchain will cross-compile Musl libc. This works in the same way on both AArch64
and x86 hosts.

Building consists of two stages: configure and build.

To configure for purecap target, run

.. code-block::

   CC=${MORELLO}/bin/clang ./configure --enable-morello --prefix=${PREFIX} \
        --target=aarch64-linux-musl_purecap

Here, ``${MORELLO}`` is directory where Morello LLVM is installed and ``${PREFIX}`` is
where Musl will be installed. You may use this folder as sysroot for compiling and
linking purecap Morello applications based on.

To build and install, just run

.. code-block::

   make
   make install

The following example demonstrates how to build a purecap Morello application and link
it to this C library. We presume the code of the application is in the ``hello.c`` file.

Compile and link application:

.. code-block::

   ${MORELLO_HOME}/bin/clang \
        -march=morello+c64 --target=aarch64-linux-musl_purecap \
        --sysroot ${PREFIX} hello.c -o hello -static

Running unit tests
------------------

TBD

Morello LLVM toolchain
----------------------

To build Morello LLVM toolchain, you can use script ``tools/build-morello.sh``. This
script will work on AArch64 and x86 hosts, but different parameters should be supplied
to the sub-commands depending on the host platform. Note, that some sub-commands use
environment variables.

The following input is required:

* ``${LLVM_PROJECT}`` -- absolute path to folder with `LLVM sources`_.
* ``${MUSL}`` -- absolute path to folder with Musl sources.
* ``${LLVM}`` -- where host LLVM is installed (LLVM 11.0 or newer is required).

.. _LLVM sources: https://git.morello-project.org/morello/llvm-project

The following folders will be needed (should be different directories):

* ``${BUILD_LLVM}`` -- build folder for clang.
* ``${BUILD_RT}`` -- build folder for compiler-rt.
* ``${MORELLO}`` -- where toolchain will be installed.
* ``${SYSROOT}`` -- where Musl headers will be installed.

Building Clang
^^^^^^^^^^^^^^

On AArch64 host use this command to build Clang:

.. code-block::

   # clang
   LLVM_TARGETS='AArch64' ./tools/build-morello.sh clang \
        ${LLVM_PROJECT} ${LLVM} ${MORELLO} ${BUILD_LLVM}

On x86 host use this command to build Clang:

.. code-block::

   # clang
   LLVM_TARGETS='AArch64' ./tools/build-morello.sh clang \
        ${LLVM_PROJECT} ${LLVM} ${MORELLO} ${BUILD_LLVM}

Building Runtime
^^^^^^^^^^^^^^^^

The following commands will build remaining components of the toolchain:

.. code-block::

   # musl-headers
   CC=${MORELLO}/bin/clang ./tools/build-morello.sh musl-headers \
        ${MUSL} ${SYSROOT} aarch64-unknown-linux-musl_purecap

   # CRT
   CC=${MORELLO}/bin/clang ./tools/build-morello.sh crt \
        ${LLVM_PROJECT} ${SYSROOT} aarch64-unknown-linux-musl_purecap

   ## Compiler-RT
   CC=${MORELLO}/bin/clang ./tools/build-morello.sh compiler-rt \
        ${LLVM_PROJECT} ${MORELLO} ${BUILD_RT} ${SYSROOT} aarch64-unknown-linux-musl_purecap

Original README
---------------

Original Musl `README <README>`_ file.
