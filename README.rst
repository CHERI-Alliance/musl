Musl libc for Morello
=====================

Work-in-progress port to Morello
--------------------------------

Disclaimer
^^^^^^^^^^

This repository contains *work-in-progress* port of Musl libc to
`Morello <https://developer.arm.com/morello/>`_. It is intended for experimental use
and does not currently provide complete and accurate implementation of the entire Musl
libc functionality. Current limitations include:

* No support for dynamic linking and dynamic loading (only static linking to ``libc.a``
  is supported).
* No support for networking.
* Only objects listed in ``arch/morello/morello.objects`` are ported to Morello.

Kernel ABI
^^^^^^^^^^

Current version relies on the
`libshim <https://git.morello-project.org/morello/android/platform/external/libshim>`_
library for implementing system calls to non-Morello kernel. This allows using purecap
Morello applications on systems with non-Morello kernel subject to common libshim
limitations.

Using this library
------------------

The following describes how to build this library natively and how to use it to link user
space applications. For brevity, we use native Morello LLVM toolchain below (see
`Morello LLVM toolchain`_ for more details).

Building Musl libc
^^^^^^^^^^^^^^^^^^

To configure the build, run

.. code-block::

   # where Morello toolchain is installed
   export MORELLO_HOME=/path/to/morello/llvm
   # where Musl will be installed
   export MUSL_HOME=/path/to/install/musl
   # configure command
   CC=${MORELLO_HOME}/bin/clang ./configure \
       --disable-shared --enable-morello --enable-libshim --prefix=${MUSL_HOME}

We use ``--disable-shared`` because dynamic linking and dynamic loading is currently not
supported. We use ``--enable-morello`` to build the Morello version of the library. When
this is disabled, an AArch64 version of the library will be built. Finally, using
``--enable-libshim`` is required to produce build which uses the ``libshim`` library for
system calls. This option only works when Morello is enabled. To build this library
targeting system with Morello kernel, use ``--disable-libshim`` in the command above.

Currently, by default, Morello and use of libshim is enabled and building shared library
is disabled.

To build and install, just run

.. code-block::

   make
   make install

When ``--enable-libshim`` is used, source code for
`libshim <https://git.morello-project.org/morello/android/platform/external/libshim>`_
and
`libarchcap <https://git.morello-project.org/morello/android/platform/external/libarchcap>`_
is downloaded (from ``mainline`` branch) and built. The ``libshim`` objects are then added
to the ``libc.a`` archive which can then be used in a usual way.

When libshim is compiled, it needs access to kernel headers that should correspond to the
kernel on the target system. The makefile of libshim tries to locate these headers, but,
if that is unsuccessful, the following message may appear: ``Cross compilation on <OS>
is not supported``. Should this happen, you may install kernel header manually. To do this,
download kernel sources of the required version and run the following command:

.. code-block::

   make headers_install ARCH=arm64 INSTALL_HDR_PATH=/path/to/kernel/headers

To use those headers while building Musl, use:

.. code-block::

   KERNEL_HEADER_INCLUDES="-isystem /path/to/kernel/headers/include" make

Building Musl libc without libshim
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To produce build without libshim, use the following configure command:

.. code-block::

   # configure command
   CC=${MORELLO_HOME}/bin/clang ./configure \
       --disable-shared --enable-morello --disable-libshim --prefix=${MUSL_HOME}

The rest of the build process is the same. Please note that this configuration is
experimental and is not currently covered by tests described below.

Building applications with this library
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The following example demonstrates how to build a purecap Morello application and link
it to this C library. We presume the code of the application is in the ``hello.c`` file.

Compile objects:

.. code-block::

   ${MORELLO_HOME}/bin/clang -c -g -nostdinc -isystem ${MUSL_HOME}/include \
       -march=morello+c64 -mabi=purecap hello.c -o hello.c.o

The ``-nostdinc`` and ``-isystem ${MUSL_HOME}/include`` options are used to make sure we
are using the correct headers from Musl.

Link executable objects:

.. code-block::

   ${MORELLO_HOME}/bin/clang -fuse-ld=lld -march=morello+c64 -mabi=purecap -o foo \
       ${MUSL_HOME}/lib/crt1.o \
       ${MUSL_HOME}/lib/crti.o \
       ${MORELLO_HOME}/lib/clang/11.0.0/lib/linux/clang_rt.crtbegin-morello.o \
       hello.c.o \
       ${MORELLO_HOME}/lib/clang/11.0.0/lib/linux/libclang_rt.builtins-morello.a \
       ${MORELLO_HOME}/lib/clang/11.0.0/lib/linux/clang_rt.crtend-morello.o \
       ${MUSL_HOME}/lib/crtn.o \
       -nostdlib -L${MUSL_HOME}/lib -lc -static

The ``-nostdlib`` and ``-L${MUSL_HOME}/lib`` options are used to make sure the right
library for ``-lc`` is used. The ``-static`` is necessary because only
static linking is currently supported. See `Morello LLVM toolchain`_ for more details
about the ``crtbegin`` and ``crtend`` objects and about ``libclang_rt.builtins-morello.a``
static library. Note that paths to these objects provided by the toolchain can be obtained
dynamically with these command:

.. code-block::

   ${MORELLO_HOME}/bin/clang -print-file-name=<file-name>

For example:

.. code-block::

   ${MORELLO_HOME}/bin/clang -print-file-name=libclang_rt.builtins-morello.a


Cross-compiling
^^^^^^^^^^^^^^^
Both steps above can be cross-compiled from an x86 host to Morello target. To do so,
append ``--target=aarch64-linux-gnu`` to the ``configure`` script command and clang
invocations (for both compiling and linking). Clang is a cross-compiler by default so it
can output code for any architecture on demand. The configure script will also try to use
LLVM's binutils instead of gcc's. They can be overridden in the same way as ``CC``. It
might also be necessary to run ``configure`` with ``CFLAGS=--target=aarch64-linux-gnu``.

Morello LLVM toolchain
----------------------

Building Morello toolchain for Linux
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This describes how to build Morello LLVM toolchain natively from
`source <https://git.morello-project.org/morello/llvm-project>`_. This relies on existing
LLVM toolchain of version 9.0.x or newer (the "host LLVM") and can be used on an AArch64
Linux system.

The essential differences of cross-compiling the toolchain on x86 hosts are explained below.

Setup:

.. code-block::

   # where host LLVM is installed
   export HOST_LLVM_BIN=/path/to/host/llvm/bin
   # path to sources of the Morello toolchain
   export LLVM_PROJECT="$(pwd)/llvm-project"
   # target installation path for the Morello toolchain
   export MORELLO_HOME="${HOME}/morello"

Get sources:

.. code-block::

   git clone https://git.morello-project.org/morello/llvm-project.git

Configure:

.. code-block::

   mkdir -p build && cd build
   cmake \
      -DCMAKE_C_COMPILER=${HOST_LLVM_BIN}/clang \
      -DCMAKE_C_COMPILER_WORKS=YES \
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
      -DLLVM_TARGETS_TO_BUILD="AArch64" \
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
      -DLIBCXXABI_USE_LLVM_UNWINDER=ON \
      -DLIBUNWIND_ENABLE_THREADS=ON \
      -DCLANG_DEFAULT_RTLIB="compiler-rt" \
      -DCLANG_DEFAULT_CXX_STDLIB="libc++" \
      -DCLANG_DEFAULT_LINKER="lld" \
      -DCLANG_DEFAULT_OBJCOPY="llvm-objcopy" \
      ${LLVM_PROJECT}/llvm

Build:

.. code-block::

   make -j16
   make install

This step is the same for native and cross compilation except that you need to extend
targets to build in ``LLVM_TARGETS_TO_BUILD`` with your host target.

Compiling crtbegin and crtend objects
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To build purecap Morello applications, startup code needs to be built in purecap mode too.
This includes the ``crtbegin`` and ``crtend`` objects which are provided by the toolchain.

.. code-block::

   ${MORELLO_HOME}/bin/clang -march=morello+c64 -mabi=purecap \
       -nostdinc -isystem ${MUSL_HOME}/include \
       -c ${LLVM_PROJECT}/compiler-rt/lib/crt/crtbegin.c \
       -o $(${MORELLO_HOME}/bin/clang -print-resource-dir)/lib/linux/clang_rt.crtbegin-morello.o

   ${MORELLO_HOME}/bin/clang -march=morello+c64 -mabi=purecap \
       -nostdinc -isystem ${MUSL_HOME}/include \
       -c ${LLVM_PROJECT}/compiler-rt/lib/crt/crtend.c \
       -o $(${MORELLO_HOME}/bin/clang -print-resource-dir)/lib/linux/clang_rt.crtend-morello.o

When cross-compiling, you will also need to build these objects for AArch64 (non-Morello)
target:

.. code-block::

   ${MORELLO_HOME}/bin/clang --target=aarch64-linux-gnu \
       -nostdinc -isystem ${MUSL_HOME}/include \
       -c ${LLVM_PROJECT}/compiler-rt/lib/crt/crtbegin.c \
       -o $(${MORELLO_HOME}/bin/clang -print-resource-dir)/lib/linux/clang_rt.crtbegin-aarch64.o

   ${MORELLO_HOME}/bin/clang --target=aarch64-linux-gnu \
       -nostdinc -isystem ${MUSL_HOME}/include \
       -c ${LLVM_PROJECT}/compiler-rt/lib/crt/crtend.c \
       -o $(${MORELLO_HOME}/bin/clang -print-resource-dir)/lib/linux/clang_rt.crtend-aarch64.o

Compiling libclang_rt.builtins-morello.a
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The ``libclang_rt.builtins-morello.a`` binary is required for building purecap applications.
This step requires Musl library built and installed (``libclang_rt.builtins-morello.a``
depends in C library headers).

When cross-compiling, before proceeding to the step of building Morello version of the
``libclang_rt.builtins-morello.a``, you will need to cross-compile AArch64 (non-Morello)
version of this static library ``libclang_rt.builtins-aarch64.a``. This operation is
identical to build Morello version with the following differences:

* The ``CMAKE_C_COMPILER_TARGET`` value should be replaced with only ``aarch64-linux-gnu``
  (Morello-specific flags should be removed).
* The ``MUSL_HOME`` variable will refer to the installation path of the AArch64 (non-Morello)
  version of Musl. It can bui built by using the procedure described above with running
  the ``configure`` script with ``--disable-morello --disable-libshim`` options.
* The destination file name of the ``mv`` command must be ``libclang_rt.builtins-aarch64.a``.

When AArch64 versions of ``libclang_rt.builtins-aarch64.a``, ``clang_rt.crtbegin-aarch64.o``
and ``clang_rt.crtend-aarch64.o`` are installed, you can successfully cross-compile Morello
version of ``libclang_rt.builtins-morello.a`` by following steps described above.

If you compile toolchain on AArch64-based device, you can proceed to the next step straight
away.

Create ``toolchain.cmake`` file with the following contents (note the use of environment
variable ``MORELLO_HOME``, it is supposed to point to the Morello toolchain installation
directory):

.. code-block::

   set(CMAKE_SYSTEM_NAME Linux)
   set(CMAKE_SYSTEM_PROCESSOR aarch64)
   set(CMAKE_C_COMPILER_TARGET "aarch64-linux-gnu -march=morello+c64 -mabi=purecap")

   set(CMAKE_C_COMPILER_WORKS 1 CACHE INTERNAL "")
   set(CMAKE_CXX_COMPILER_WORKS 1 CACHE INTERNAL "")

   set(CMAKE_C_COMPILER "${MORELLO_HOME}/bin/clang" CACHE FILEPATH "" FORCE)
   set(CMAKE_CXX_COMPILER "${MORELLO_HOME}/bin/clang++" CACHE FILEPATH "" FORCE)
   set(CMAKE_AR "${MORELLO_HOME}/bin/llvm-ar" CACHE FILEPATH "" FORCE)
   set(CMAKE_RANLIB "${MORELLO_HOME}/bin/llvm-ranlib" CACHE FILEPATH "" FORCE)
   set(CMAKE_NM "${MORELLO_HOME}/bin/llvm-nm" CACHE FILEPATH "" FORCE)
   set(CMAKE_LINKER "${MORELLO_HOME}/bin/ld.lld" CACHE FILEPATH "" FORCE)
   set(CMAKE_OBJDUMP "${MORELLO_HOME}/bin/llvm-objdump" CACHE FILEPATH "" FORCE)
   set(CMAKE_OBJCOPY "${MORELLO_HOME}/bin/llvm-objcopy" CACHE FILEPATH "" FORCE)

   set(LLVM_CONFIG_PATH "${MORELLO_HOME}/bin/llvm-config" CACHE FILEPATH "" FORCE)
   set(CMAKE_EXE_LINKER_FLAGS "-fuse-ld=lld" CACHE FILEPATH "" FORCE)
   set(CMAKE_SHARED_LINKER_FLAGS "-fuse-ld=lld" CACHE FILEPATH "" FORCE)

This file is used in the following configure command for compiler-rt (note that the
``MORELLO_HOME`` environment variable must be exported):

.. code-block::

   mkdir p build-rt && cd build-rt
   rm -rf *

   perl -pe 's/\$\{([_A-Z]+)\}/$ENV{$1}/g' < /path/to/toolchain.cmake > toolchain.cmake

   cmake -Wno-dev \
      -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_C_FLAGS="-nostdinc -isystem ${MUSL_HOME}/include" \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
      -DCMAKE_SKIP_BUILD_RPATH=OFF \
      -DCMAKE_INSTALL_RPATH=\$ORIGIN/../lib \
      -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON \
      -DLLVM_TARGETS_TO_BUILD="AArch64" \
      -DLLVM_ENABLE_ASSERTIONS=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DCOMPILER_RT_DEFAULT_TARGET_TRIPLE=aarch64-linux-gnu \
      -DCOMPILER_RT_BUILD_BUILTINS=ON \
      -DCOMPILER_RT_BUILD_SANITIZERS=OFF \
      -DCOMPILER_RT_BUILD_XRAY=OFF \
      -DCOMPILER_RT_BUILD_LIBFUZZER=OFF \
      -DCOMPILER_RT_BUILD_PROFILE=OFF \
      ${LLVM_PROJECT}/compiler-rt

   make clang_rt.builtins-aarch64

   mv lib/linux/libclang_rt.builtins-aarch64.a \
       $(${MORELLO_HOME}/bin/clang -print-resource-dir)/lib/linux/libclang_rt.builtins-morello.a

Contributing
------------

Running unit tests
^^^^^^^^^^^^^^^^^^

Prerequisites: Python 3.6+, `Morello IE <https://developer.arm.com/architectures/cpu-architecture/a-profile/morello/development-tools#instruction-emulator>`_.

.. code-block::

   export MORELLOIE=/path/to/morelloie/bin/morelloie
   make -C test test

Sorting ``morello.objects``
^^^^^^^^^^^^^^^^^^^^^^^^^^^

To sort the ``morello.objects`` file please use the command line ``sort``
utility. This can also be used to remove duplicate entries.

E.g. from musl root directory:

.. code-block::

   LC_COLLATE=C sort -uf arch/morello/morello.objects -o arch/morello/morello.objects

Original README
---------------

Original Musl `README <README>`_ file.
