override SHELL = /bin/bash

MORELLO_TESTS_PROCS = 8

MUSL_LIB = $(MUSL_ROOT)/lib
MUSL_INC = $(MUSL_ROOT)/include

CFLAGS += -nostdinc -isystem $(MUSL_INC) -march=morello+c64 -mabi=purecap -g -DMORELLO -fno-builtin-mempcpy
CFLAGS += -Wall -Wpedantic -Werror
CFLAGS += -Wno-gnu-empty-initializer -Wno-gnu-folding-constant -Wno-gnu-statement-expression
LDFLAGS += -g -fuse-ld=lld -nostdlib -L$(MUSL_LIB) -march=morello+c64 -mabi=purecap -Wl,-disable-warn-morello-abi-mismatch
LDLIBS += -lc

CRT_BEGIN = $(shell $(CC) -print-file-name=clang_rt.crtbegin-morello.o)
CRT_END = $(shell $(CC) -print-file-name=clang_rt.crtend-morello.o)
CRT_PRE = $(MUSL_LIB)/crt1.o $(MUSL_LIB)/crti.o $(CRT_BEGIN)
CRT_POST = $(CRT_END) $(MUSL_LIB)/crtn.o

RT_BUILTINS = $(shell $(CC) -print-file-name=libclang_rt.builtins-morello.a)

SOURCES := $(wildcard src/*.c)
OBJS := $(patsubst %.c, %.o, $(SOURCES))
MKFS := $(patsubst %.c, %.mk, $(SOURCES))
EXE := $(patsubst %.c, %.exe, $(SOURCES))

MIE_ARGS ?= -fail-early c64-a64

all: test

-include $(MKFS)

test: $(EXE) check_morelloie
	python3 runner.py --nproc $(MORELLO_TESTS_PROCS) --mie-args "$(MIE_ARGS)" src morello-tests.json $(MORELLOIE) musl.custom.morello

build: $(EXE)

check_musl_root:
ifndef MUSL_ROOT
	$(error MUSL_ROOT is undefined)
endif

check_morelloie:
ifndef MORELLOIE
	$(error MORELLOIE is undefined - please supply path to emulator binary)
endif

%.o: %.c check_musl_root
	$(CC) $(CFLAGS) -c $< -o $@

%.exe: %.o
	$(CC) $(LDFLAGS) $(CRT_PRE) $< $(RT_BUILTINS) $(CRT_POST) -o $@ $(LDLIBS) -static

run-%: src/%.exe
	$(MORELLOIE) $(MIE_ARGS) -- $< $(TEST_ARGS)

clean:
	@rm -vf suite-morello-results.xml $(EXE) $(OBJS)
	@rm -vrf /tmp/morello-musl-tests-*

.PHONY: clean check_morelloie check_musl_root test build
