Musl libc for Morello
=====================

Work-in-progress port to Morello
--------------------------------

Disclaimer
^^^^^^^^^^

This repository contains *work-in-progress* port of Musl libc to `Morello`_ targeting
purecap ABI. It is intended for experimental use.

.. _Morello: https://www.arm.com/architecture/cpu/morello

Kernel ABI
^^^^^^^^^^

It is possible to use `libshim`_ library for implementing system calls to non-Morello
kernel. This allows using purecap Morello applications on systems with non-Morello kernel
subject to common libshim limitations. Use the ``--enable-libshim`` parameter of the
configure script to activate this option. You need to clone libshim separately and place
it next to Musl's root folder. An arbitrary path to libshim can be used as well via the
``--libshim-path`` parameter of the configure script.

.. _libshim: https://git.morello-project.org/morello/android/platform/external/libshim

Using this library
------------------

To build Morello Musl libc, use Morello toolchain, for example, Morello LLVM (see below).
This toolchain will cross-compile Musl libc. This works in the same way on both AArch64
and x86 hosts.

Building consists of two stages: configure and build.

To configure without libshim, run

.. code-block::

   CC=${MORELLO}/bin/clang ./configure --enable-morello --prefix=${PREFIX} \
        --target=aarch64-linux-musl_purecap

To configure with libshim, run

.. code-block::

   CC=${MORELLO}/bin/clang ./configure --enable-morello --prefix=${PREFIX} \
        --target=aarch64-linux-musl_purecap \
        --enable-libshim --libshim-path=${LIBSHIM}

Here, ``${MORELLO}`` is directory where Morello LLVM is installed and ``${PREFIX}`` is
where Musl will be installed. You may use this folder as sysroot for compiling and
linking purecap Morello applications based on. Optionally use path ``${LIBSHIM}`` to the
libshim sources.

To build and install, just run

.. code-block::

   make
   make install

When libshim is compiled, it needs access to kernel headers that should correspond to the
kernel on the target system. The makefile of libshim tries to locate these headers, but,
if that is unsuccessful, the following message may appear: ``Cross compilation on <OS>
is not supported``. Should this happen, you may install kernel header manually. To do
this, download kernel sources of the required version and run the following command:

.. code-block::

   make headers_install ARCH=arm64 INSTALL_HDR_PATH=/path/to/kernel/headers

To use those headers while building Musl, use:

.. code-block::

   KERNEL_HEADER_INCLUDES="-isystem /path/to/kernel/headers/include" make

The following example demonstrates how to build a purecap Morello application and link
it to this C library. We presume the code of the application is in the ``hello.c`` file.

Compile and link application:

.. code-block::

   ${MORELLO_HOME}/bin/clang \
        -march=morello+c64 --target=aarch64-linux-musl_purecap \
        --sysroot ${PREFIX} hello.c -o hello -static

Running unit tests
------------------

Unit tests can be built and executed on `Morello IE`_ using the commands below:

.. code-block::

   export MORELLOIE=/path/to/bin/morelloie
   make -C test test

.. _Morello IE: https://developer.arm.com/downloads/-/morello-instruction-emulator

Alternatively, you can build the tests and then run them elsewhere:

.. code-block::

   export MORELLOIE=/path/to/bin/morelloie
   make -C test build

Note that you will need to use libshim to run tests on Morello IE and *not* use libshim
if you aim to run or a Morello board with PCuABI kernel.

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

CHERIseed
---------

This version of Musl also includes changes to enable CHERIseed, a software-only implementation of
`CHERI`_ semantics.

.. _CHERI: https://www.cl.cam.ac.uk/research/security/ctsrd/cheri/

The aim of CHERIseed is to facilitate the porting effort of existing code to CHERI
hardware platforms, by providing some of the functionality while running on a host
machine that is not capability aware. This functionality includes:

* 128-bit pointers for a 64-bit address space (64 bits of “metadata”).
* Bounds checking on pointer dereferences.
* Permissions checking for pointers where permissions are restricted.

By compiling and running code with CHERIseed a user can experiment with CHERI programming
(see the `CHERI C/C++ Programming Guide`_), and identify potentially unsafe code that
would fault on real CHERI hardware.

.. _CHERI C/C++ Programming Guide: https://www.cl.cam.ac.uk/techreports/UCAM-CL-TR-947.pdf

The CHERIseed LLVM Project can be found `here`_. See: `CHERIseed.rst`_ for how to build
CHERIseed-enabled Clang. `CHERIseed-enabled libshim`_ is also required.

.. _here: https://git.morello-project.org/morello/llvm-project/-/tree/cheriseed
.. _CHERIseed.rst: https://git.morello-project.org/morello/llvm-project/-/blob/cheriseed/clang/docs/CHERIseed.rst
.. _CHERIseed-enabled libshim: https://git.morello-project.org/morello/android/platform/external/libshim/-/tree/cheriseed

To build Musl with CHERIseed enabled, Musl should be configured with:

.. code-block::

   # configure command
   CC=${CHERISEED_LLVM}/build/bin/clang ./configure \
       --disable-shared \
       --disable-morello \
       --enable-cheriseed \
       --libshim-path=${CHERISEED_LIBSHIM}  \
       --prefix=${MUSL_HOME}

Original README
---------------

Original Musl `README <README>`_ file.
