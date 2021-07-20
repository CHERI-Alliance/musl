override SHELL = /bin/bash

MUSL_LIB = $(MUSL_ROOT)/lib
MUSL_INC = $(MUSL_ROOT)/include

CFLAGS += -nostdinc -isystem $(MUSL_INC) -march=morello+c64 -mabi=purecap -g -DMORELLO -fno-builtin-mempcpy -Werror
LDFLAGS += -g -fuse-ld=lld -nostdlib -L$(MUSL_LIB) -Wl,-morello-c64-plt
LDLIBS += -lc -lgcc

CRT_BEGIN = $(shell $(CC) -print-file-name=clang_rt.crtbegin-morello.o)
CRT_END = $(shell $(CC) -print-file-name=clang_rt.crtend-morello.o)
CRT_PRE = $(MUSL_LIB)/crt1.o $(MUSL_LIB)/crti.o $(CRT_BEGIN)
CRT_POST = $(CRT_END) $(MUSL_LIB)/crtn.o

SOURCES := $(wildcard src/*.c)
OBJS := $(patsubst %.c, %.o, $(SOURCES))
EXE := $(patsubst %.c, %.exe, $(SOURCES))

test: $(EXE) check_morelloie
	python3 runner.py src morello-tests.json $(MORELLOIE) musl.custom.morello

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
	$(CC) $(LDFLAGS) $(CRT_PRE) $< $(CRT_POST) -o $@ $(LDLIBS) -static

run-%: src/%.exe
	$(MORELLOIE) $(MIE_ARGS) -- $< $(TEST_ARGS)

clean:
	@rm -vf suite-morello-results.xml $(EXE) $(OBJS)
	@rm -vrf /tmp/morello-musl-tests-*

.PHONY: clean check_morelloie check_musl_root test build
