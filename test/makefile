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
MKFS := $(patsubst %.c, %.mk, $(SOURCES))
EXE_STATIC := $(patsubst %.c, %-static.exe, $(SOURCES))

MIE_ARGS ?= -fail-early c64-a64

all: test

-include $(MKFS)

test: test-static

test-static: $(EXE_STATIC) check_morelloie $(CURDIR)/runner.py
	python3 runner.py --kind static --nproc $(MORELLO_TESTS_PROCS) --mie-args "$(MIE_ARGS)" src morello-tests.json $(MORELLOIE) musl.custom.morello.static

test-dynamic: $(EXE_STATIC) check_morelloie $(CURDIR)/runner.py
	python3 runner.py --kind dynamic --nproc $(MORELLO_TESTS_PROCS) --mie-args "$(MIE_ARGS)" src morello-tests.json $(MORELLOIE) musl.custom.morello.dynamic

build: $(EXE_STATIC)

check_morelloie:
ifndef MORELLOIE
	$(error MORELLOIE is undefined - please supply path to emulator binary)
endif

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%-static.exe: %.o $(addprefix $(MUSLLIB)/,$(MUSL_BITS_STATIC))
	$(CC) $(LDFLAGS) $< -o $@ $(LDLIBS) -static

%-dynamic.exe: %.o $(addprefix $(MUSLLIB)/,$(MUSL_BITS_STATIC))
	$(CC) $(LDFLAGS) $< -o $@ $(LDLIBS)

run-%: src/%.exe
	$(MORELLOIE) $(MIE_ARGS) -- $< $(TEST_ARGS)

clean:
	@rm -vf suite-morello-results.xml $(EXE_STATIC) $(OBJS)
	@rm -vrf /tmp/morello-musl-tests-*

.PHONY: clean check_morelloie test build
