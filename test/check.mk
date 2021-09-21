override SHELL = /bin/bash

override TMP = tmp
override SOURCES = \
	arch/morello/morello.objects \
	test/libc-test-enabled-tests.txt
override SOURCES := $(notdir $(SOURCES))

override OBJECTS = $(shell cat arch/morello/morello.objects)
override CHECKS = $(SOURCES:%=$(TMP)/%)
override CHECKS += $(OBJECTS:%.o=check/%.o)

check: $(CHECKS)
	@cmp arch/morello/morello.objects $(TMP)/morello.objects
	@cmp test/libc-test-enabled-tests.txt $(TMP)/libc-test-enabled-tests.txt
	@rm -rf $(TMP)

debug:
	@echo $(CHECKS)

$(TMP)/morello.objects: arch/morello/morello.objects | $(TMP)
	@LC_COLLATE=C sort -uf $< -o $@

$(TMP)/libc-test-enabled-tests.txt: test/libc-test-enabled-tests.txt | $(TMP)
	@LC_COLLATE=C sort -uf $< -o $@

check/obj/%.o: obj/%.o
	@[[ -f $< ]]

$(TMP):
	@mkdir -p $(TMP)

clean:
	@rm -rvf $(TMP)

.PHONY: clean check
