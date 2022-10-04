# CHERIseed requires that some implementations are never in assembly.
CHERISEED_NOT_ARCH_SRCS=\
	%/string/$(ARCH)/memcpy.s \
	%/string/$(ARCH)/memcpy.S \
	%/string/$(ARCH)/memset.s \
	%/string/$(ARCH)/memset.S \
	%/string/$(ARCH)/memmove.s \
	%/string/$(ARCH)/memmove.S

ARCH_SRCS:=$(filter-out $(CHERISEED_NOT_ARCH_SRCS),$(ARCH_SRCS))

# CHERIseed has support for multiple architectures, but it is not
# yet supported for all source files.
CHERISEED_OBJECTS=$(filter-out $(CHERISEED_EXCLUDED_OBJECTS),$(ALL_OBJS))
$(CHERISEED_OBJECTS): CFLAGS += $(CFLAGS_CHERISEED)

CHERISEED_LDSO_OBJECTS=$(filter-out $(CHERISEED_EXCLUDED_OBJECTS),$(ALL_OBJS:%.o=%.lo))
$(CHERISEED_LDSO_OBJECTS): CFLAGS_ALL += $(CFLAGS_CHERISEED)

# Make the list of files a dependency of all targets so that when it changes
# everything is rebuilt.
$(ALL_OBJS): cheriseed_excluded.objects

# 'lite_malloc' is replaced with 'mallocng'
AOBJS := $(filter-out %/lite_malloc.o,$(AOBJS))
LOBJS := $(filter-out %/lite_malloc.lo,$(LOBJS))

# Link cheriseed static library to the dynamic linker.
lib/libc.so: LDFLAGS_ALL += $(shell $(CC) -print-file-name=libclang_rt.cheriseed-$(ARCH).a)

# Add include directory for some files which use the sanitizer's interface.
obj/crt/rcrt1.o obj/ldso/dlstart.lo obj/ldso/dynlink.lo: CFLAGS_ALL += -I $(shell $(CC) -print-resource-dir)/include
