# CHERIseed has support for multiple architectures, but it is not
# yet supported for all source files.
CHERISEED_OBJECTS=$(filter-out $(CHERISEED_EXCLUDED_OBJECTS),$(ALL_OBJS))
$(CHERISEED_OBJECTS): CFLAGS += $(CFLAGS_CHERISEED)

# Make the list of files a dependency of all targets so that when it changes
# everything is rebuilt.
$(ALL_OBJS): cheriseed_excluded.objects
