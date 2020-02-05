#
# Ensimag - Projet système
# Copyright (C) 2012 - Damien Dejean <dam.dejean@gmail.com>
#

#
# Generates object files from userspace binaries and integrates them in the
# kernel binary. Generates a table to be able to find theses programs in
# the binary when the kernel is loaded.
#
# Dependencies:
#  OUTPUT
# Provides:
#  APPS_OBJS: The application object files

# Defines build environment
APPS_BIN  := $(wildcard ../user/out/*.bin)
APPS_OUT  := $(OUTPUT)/apps
APPS_OBJS := $(addprefix $(APPS_OUT)/, $(notdir $(addsuffix .o, $(APPS_BIN)))) $(APPS_OUT)/symbols-table.o
vpath %.bin $(dir $(APPS_BIN))

# Transform binary files in linkable files
$(APPS_OUT)/%.bin.o: $(OUTPUT)/empty.o %.bin | $(APPS_OUT)
	$(OBJCOPY) $< \
		--add-section=.$*.bin=$(filter-out $<, $^) \
		--set-section-flags=.$*.bin=contents,alloc,load,data $@

$(APPS_OUT):
	$(MKDIR) -p $@

# Generate a linker file that:
# - creates a table with an entry (address, size) for each application
# - include each binary in its own section
# This is a intermediate target that will be automagically deleted by make
# at the end of the build. It ensures that the script is entirely
# re-generated at each build.

.INTERMEDIATE: $(APPS_OUT)/symbols-table.o
$(APPS_OUT)/symbols-table.o: $(APPS_OUT)/symbols-table.c | $(APPS_OUT)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

.INTERMEDIATE: $(APPS_OUT)/symbols-table.c
$(APPS_OUT)/symbols-table.c: | $(APPS_OUT)
	$(GEN_TABLE) "$@" $(APPS_BIN)

.INTERMEDIATE: $(APPS_OUT)/apps.lds
$(APPS_OUT)/apps.lds: | $(APPS_OUT)
	$(GEN_SECTIONS) "$@" $(APPS_BIN)

.PHONY: build/kernel.lds
build/kernel.lds: $(APPS_OUT)/apps.lds

# Add the apps dir to LDFLAGS to ensure the linker will find our script
LDFLAGS += -L$(APPS_OUT)/

