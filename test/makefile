override SHELL = /bin/bash

MORELLO_TESTS_PROCS = 8

CONFIG ?= $(CURDIR)/../config.mak

include $(CONFIG)

override SYSROOT = $(prefix)
override MUSLLIB = $(SYSROOT)/lib
override TRIPLE = aarch64-linux-musl_purecap

override MUSL_BITS_STATIC = crt1.o crti.o crtn.o libc.a rcrt1.o Scrt1.o
override MUSL_BITS_DYNAMIC = crt1.o crti.o crtn.o libc.so rcrt1.o Scrt1.o

CFLAGS += --target=$(TRIPLE) --sysroot=$(SYSROOT) -march=morello+c64
CFLAGS += ${TEST_CFLAGS}
CFLAGS += -g -fno-builtin-mempcpy
CFLAGS += -Wall -Wpedantic -Werror
CFLAGS += -Wno-gnu-empty-initializer -Wno-gnu-folding-constant -Wno-gnu-statement-expression

LDFLAGS += --target=$(TRIPLE) --sysroot=$(SYSROOT) -march=morello+c64
LDFLAGS += ${TEST_LDFLAGS}
LDFLAGS += -g -fuse-ld=lld
LDLIBS += -lc

SOURCES := $(wildcard src/*.c)
OBJS := $(patsubst %.c, %.o, $(SOURCES))
LOBJS := $(patsubst %.c, %.lo, $(SOURCES))
MKFS := $(patsubst %.c, %.mk, $(SOURCES))
EXE_STATIC := $(patsubst %.c, %-static.exe, $(SOURCES))
EXE_DYNAMIC := $(patsubst %.c, %-dynamic.exe, $(SOURCES))

# Third party library tests
LIBRARY_SOURCES := $(wildcard src/lib/*.c)
SLOBJS := $(patsubst src/lib/%.c, src/lib/lib%.so, $(LIBRARY_SOURCES))

MIE_ARGS ?=

all: test

-include $(MKFS)

test: test-static test-dynamic

test-native: test-static-native test-dynamic-native

test-static-native: $(EXE_STATIC) runner.py morello-tests.json
	python3 runner.py static $(MORELLO_TESTS_PROCS) $(CURDIR)/src morello-tests.json \
	"native" custom morello-custom-static-results.xml

test-dynamic-native: $(EXE_DYNAMIC) runner.py morello-tests.json
	python3 runner.py dynamic $(MORELLO_TESTS_PROCS) $(CURDIR)/src morello-tests.json \
	"native" custom morello-custom-dynamic-results.xml

test-static: $(EXE_STATIC) runner.py morello-tests.json check_morelloie
	TEST_RUNNER_MIEARGS='$(MIE_ARGS)' python3 runner.py static $(MORELLO_TESTS_PROCS) \
	$(CURDIR)/src morello-tests.json $(MORELLOIE) custom morello-custom-static-results.xml

test-dynamic: $(EXE_DYNAMIC) runner.py morello-tests.json check_morelloie
	TEST_RUNNER_MIEARGS='$(MIE_ARGS)' python3 runner.py dynamic $(MORELLO_TESTS_PROCS) \
	$(CURDIR)/src morello-tests.json $(MORELLOIE) custom morello-custom-dynamic-results.xml

build: $(EXE_STATIC) $(EXE_DYNAMIC)

shared-libraries: $(SLOBJS)

check_morelloie:
ifndef MORELLOIE
	$(error MORELLOIE is undefined - please supply path to emulator binary)
endif

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.lo: %.c
	$(CC) $(CFLAGS) -D DYNAMIC -fPIC -c $< -o $@

src/lib/lib%.so: src/lib/%.c
	$(CC) $(CFLAGS) $(LDFLAGS) -shared $< -o $@

src/shared-lib.lo src/shared-lib.o: CFLAGS += -I src/lib

src/shared-lib-static.exe: LDFLAGS += src/lib/third-party-library.o

src/shared-lib-static.exe: src/lib/third-party-library.o

src/shared-lib-dynamic.exe: LDLIBS += -L./src/lib -lthird-party-library '-Wl,-rpath,$$ORIGIN/lib'

src/shared-lib-dynamic.exe: src/lib/libthird-party-library.so

%-static.exe: %.o $(addprefix $(MUSLLIB)/,$(MUSL_BITS_STATIC))
	$(CC) $(LDFLAGS) $< -o $@ $(LDLIBS) -static

%-dynamic.exe: %.lo $(addprefix $(MUSLLIB)/,$(MUSL_BITS_STATIC))
	$(CC) $(LDFLAGS) $< -o $@ $(LDLIBS) -Wl,--dynamic-linker,$(MUSLLIB)/libc.so

run-%: src/%.exe
	$< $(TEST_ARGS)

clean:
	@rm -vf *-results.xml $(EXE_STATIC) $(EXE_DYNAMIC) $(OBJS) $(LOBJS) $(SLOBJS)
	@rm -vrf /tmp/morello-musl-tests-*

.PHONY: clean check_morelloie test build test-native test-static-native test-dynamic-native test-static test-dynamic shared-libraries
