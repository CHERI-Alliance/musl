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
* No support for multi-threaded applications.
* Only objects listed in ``arch/aarch64/morello.objects`` are ported to Morello.

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
   configure command
   CC=${MORELLO_HOME}/bin/clang ./configure \
       --disable-shared --enable-morello --enable-libshim  --prefix=${MUSL_HOME}

We use ``--disable-shared`` because dynamic linking and dynamic loading is currently not
supported. We use ``--enable-morello`` to build the Morello version of the library. When
this is disabled, an AArch64 version of the library will be built. Finally, using
``--enable-libshim`` is required to produce build which uses the ``libshim`` library for
system calls. This option only works when Morello is enabled.

To build and install, just run

.. code-block::

   make
   make install

When ``--enable-libshim`` is used, source code for
`libshim <https://git.morello-project.org/morello/android/platform/external/libshim>`_
and
`libarchcap <https://git.morello-project.org/morello/android/platform/external/libarchcap>`_
is downloaded and built. The ``libshim`` objects are then added to ``libc.a`` which can then
be used in a usual way.


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

   ${MORELLO_HOME}/bin/clang -fuse-ld=lld -Wl,--morello-c64-plt -o foo \
       ${MUSL_HOME}/lib/crt1.o \
       ${MUSL_HOME}/lib/crti.o \
       ${MORELLO_HOME}/lib/clang/11.0.0/lib/linux/clang_rt.crtbegin-morello.o \
       hello.c.o \
       ${MORELLO_HOME}/lib/clang/11.0.0/lib/linux/clang_rt.crtend-morello.o \
       ${MUSL_HOME}/lib/crtn.o \
       -nostdlib -L${MUSL_HOME}/lib -lc -lm \
       -static

The ``-nostdlib`` and ``-L${MUSL_HOME}/lib`` options are used to make sure the right
libraries for ``-lc`` and ``-lm`` are used. The ``-static`` is necessary because only
static linking is currently supported. See `Morello LLVM toolchain`_ for more details
about the ``crtbegin`` and ``crtend`` objects.

Morello LLVM toolchain
----------------------

Building Morello toolchain for Linux
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This describes how to build Morello LLVM toolchain natively from
`source <https://git.morello-project.org/morello/llvm-project>`_. This relies on existing
LLVM toolchain of version 9.0.x or newer (the "host LLVM") and can be used on an AArch64
Linux system.

Setup:

.. code-block::

   # where host LLVM is installed
   export HOST_LLVM_BIN=/usr/local/llvm/bin
   # path to sources of the Morello toolchain
   export LLVM_PROJECT="$(pwd)/llvm-project"
   # target installation path for the Morello toolchain
   export MORELLO_HOME="${HOME}/morello"

Get sources:

.. code-block::

   git clone https://git.morello-project.org/morello/llvm-project.git

Configure:

.. code-block::

   mkdir build && cd build
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
      -DLLVM_TARGETS_TO_BUILD="AArch64" \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
      -DBUILD_SHARED_LIBS=ON \
      -DCMAKE_SKIP_BUILD_RPATH=OFF \
      -DCMAKE_INSTALL_RPATH=\$ORIGIN/../lib \
      -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON \
      -DLLVM_ENABLE_ASSERTIONS=ON \
      -DLLVM_ENABLE_LIBCXX=ON \
      -DLLVM_ENABLE_LLD=ON \
      -DLIBCXX_CXX_ABI=libcxxabi \
      -DLIBCXX_CXX_ABI_INCLUDE_PATHS="${LLVM_PROJECT}/libcxxabi/include" \
      -DLIBCXXABI_USE_LLVM_UNWINDER=ON \
      -DLIBCXX_USE_COMPILER_RT=ON \
      -DLIBCXXABI_USE_COMPILER_RT=ON \
      -DLIBCXX_ENABLE_THREADS=ON \
      -DLIBCXXABI_ENABLE_THREADS=ON \
      -DLIBUNWIND_ENABLE_THREADS=ON \
      -DCMAKE_INSTALL_PREFIX=${MORELLO_HOME} \
      -DLLVM_ENABLE_EH=ON -DLLVM_ENABLE_RTTI=ON \
      -DLLVM_ENABLE_PROJECTS="clang;lld;lldb;libcxx;libcxxabi;compiler-rt;libunwind" \
      ${LLVM_PROJECT}/llvm

Build:

.. code-block::

   make -j16
   make install

Compiling crtbegin and crtend objects
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To build purecap Morello applications, startup code needs to be built in purecap mode too.
This includes the ``crtbegin`` and ``crtend`` objects which are provided by the toolchain.

.. code-block::

   ${MORELLO_HOME}/bin/clang -march=morello+c64 -mabi=purecap \
       -c ${LLVM_PROJECT}/compiler-rt/lib/crt/crtbegin.c \
       -o ${MORELLO_HOME}/lib/clang/11.0.0/lib/linux/clang_rt.crtbegin-morello.o

   ${MORELLO_HOME}/bin/clang -march=morello+c64 -mabi=purecap \
       -c ${LLVM_PROJECT}/compiler-rt/lib/crt/crtend.c \
       -o ${MORELLO_HOME}/lib/clang/11.0.0/lib/linux/clang_rt.crtend-morello.o

Original README
---------------

.. include:: README