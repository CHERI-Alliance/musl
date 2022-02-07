override SHELL = /bin/bash

override TMP = tmp
override SOURCES = \
	test/libc-test-enabled-tests.txt
override SOURCES := $(notdir $(SOURCES))

override CHECKS = $(SOURCES:%=$(TMP)/%)

check: $(CHECKS)
	@cmp test/libc-test-enabled-tests.txt $(TMP)/libc-test-enabled-tests.txt
	@rm -rf $(TMP)

debug:
	@echo $(CHECKS)

$(TMP)/libc-test-enabled-tests.txt: test/libc-test-enabled-tests.txt | $(TMP)
	@LC_COLLATE=C sort -uf $< -o $@

check/obj/%.o: obj/%.o
	@[[ -f $< ]]

$(TMP):
	@mkdir -p $(TMP)

clean:
	@rm -rvf $(TMP)

.PHONY: clean check
