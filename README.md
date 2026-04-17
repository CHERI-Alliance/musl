# CHERI RISC-V Port of musl C Library

This is based on the morello port of musl libc at
`https://git.morello-project.org/morello/musl-libc` with
RISC-V customization added.

## Building musl-libc for CHERI RISC-V Linux

### Prerequisites

In order to build musl you will need:

* The CHERI-enabled musl sources
* The CHERI-enabled LLVM compiler
* Linux Kernel headers from a CHERI-enabled Linux Kernel

The build is slightly complicated by the fact that LLVM's compiler
runtime must be compiled using libc headers from musl but the musl
build needs this compiler runtime for its linking step.

### Build Process Overview

1) Compile clang and lld
2) Setup a sysroot environment with musl and linux kernel headers
3) Build and install the compiler runtime from the LLVM project
4) Build and install musl
5) Compile your first test program.

In the rest of this document we will assume that the following
environment variables are set:

* `INSTALL`: Installation directory of the LLVM toolchain
* `SYSROOT`: The sysroot directory used for cross compiling
* `TARGET=riscv64-unknown-linux-gnu`
* `MARCH=-march=rv64imafdc_zcherihybrid_zcherilevels_zish4add`
* `MABI=-mabi=l64pc128d`

### Compile clang and lld

Please refer to the LLVM build instructions for details on
how to build LLVM for CHERI. Please make sure you include
these options in your `cmake` command line:

```
	-DCMAKE_INSTALL_PREFIX=$INSTALL
	-DCMAKE_C_COMPILER_TARGET=$TARGET
	-DLLVM_ENABLE_PROJECTS="clang;llvm;lld"
	-DCLANG_DEFAULT_RTLIB="compiler-rt"
```

### Setup the sysroot Environment

To install the kernel headers, run the command below from the kernel's
top-level directory. There is no need to configure or build the kernel
before.

```
	make ARCH=riscv INSTALL_HDR_PATH=$SYSROOT/usr headers_install
```

To build the musl headers go to the musl source directory and
do
```
	mkdir build_headers && cd build_headers
	../configure CC="$INSTALL/bin/clang -target $TARGET" \
		--enable-shared --prefix=/usr
	make install-headers DESTDIR=$SYSROOT
```

### Build and install the Compiler Runtime

As we want to cross compile the compiler runtime with the
CHERI compiler we need a `toolchain.cmake` file that points
to the LLVM compiler. You can create one like this:

```
	TCFLAGS="-target $TARGET \
		$MARCH $MABI -mlittle-endian \
		--sysroot=$SYSROOT \
		-fuse-ld=lld -nostdlib"
	cat <<EOF >toolchain.cmake
	set(CMAKE_SYSTEM_NAME Linux)
	set(CMAKE_SYSTEM_PROCESSOR riscv64)
	set(CMAKE_C_COMPILER $INSTALL/bin/clang)
	set(CMAKE_CXX_COMPILER $INSTALL/bin/clang++)
	set(CMAKE_ASM_COMPILER $INSTALL/bin/clang)
	set(CMAKE_SYSROOT $SYSROOT)
	set(CMAKE_C_FLAGS "$TCFLAGS")
	set(CMAKE_CXX_FLAGS "$TCFLAGS")
	set(CMAKE_ASM_FLAGS "$TCFLAGS")
	set(CMAKE_C_LINK_FLAGS "$TCFLAGS")
	set(CMAKE_CXX_LINK_FLAGS "$TCFLAGS")
	EOF
```

Then go the LLVM source directory and build the compiler
runtime like this:
```
	cmake -G Ninja -B buildrt -S $PWD/compiler-rt/ \
		-DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain.cmake \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_INSTALL_PREFIX=$($INSTALL/bin/clang -print-resource-dir) \
		-DCOMPILER_RT_STANDALONE_BUILD=OFF \
		-DCOMPILER_RT_DEFAULT_TARGET_ONLY=ON \
		-DCMAKE_C_COMPILER_TARGET=riscv64-unknown-linux-gnu \
		-DCOMPILER_RT_BUILD_CRT=ON \
		-DCOMPILER_RT_BUILD_STANDALONE_LIBATOMIC=ON \
		-DLLVM_ENABLE_PER_TARGET_RUNTIME_DIR=ON \
		-DCMAKE_POSITION_INDEPENDENT_CODE=ON \
		-DLLVM_LIBDIR_SUFFIX= \
		-DLLVM_CMAKE_DIR=$INSTALL/lib/cmake/llvm \
		-DCOMPILER_RT_INCLUDE_TESTS=OFF \
		-DCOMPILER_RT_BUILD_XRAY=OFF \
		-DCOMPILER_RT_BUILD_SANITIZERS=OFF \
		-DCOMPILER_RT_BUILD_MEMPROF=OFF \
		-DCOMPILER_RT_BUILD_LIBFUZZER=OFF \
		-DCOMPILER_RT_BUILD_PROFILE=OFF \
		-DCOMPILER_RT_BUILD_CTX_PROFILE=OFF
	ninja -C buildrt install
```

### Build and install musl

In the musl source directory run

```
	INC=$( $INSTALL/bin/clang -print-resource-dir)/include
	XFLAGS="-target $TARGET \
		$MARCH $MABI -mlittle-endian \
		--sysroot=$SYSROOT \
		-isystem $INC \
		-fuse-ld=lld"
	./configure \
		CC="$INSTALL/bin/clang $XFLAGS" \
		LD=$INSTALL/bin/lld \
		--enable-shared --enable-static \
		--prefix=/
	make \
		CC="$INSTALL/bin/clang $XFLAGS" \
		LD=$INSTALL/bin/lld \
		DESTDIR=$SYSROOT \
		install
```

### Build your first Binary

Now you should be able to cross compile a standard HelloWorld.c
like this:

```
	$INSTALL/bin/clang -target $TARGET $MARCH $MABI \
		--sysroot $SYSROOT -fuse-ld=lld \
		-o HelloWorld HelloWorld.c
```


