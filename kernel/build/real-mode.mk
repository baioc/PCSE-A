#
# Ensimag - Projet système
# Copyright (C) 2012 - Damien Dejean <dam.dejean@gmail.com>
#

#
# Provides an object file that contains real mode code linked to run at address
# 0x2000.
#
# Dependencies:
#  OUTPUT, REAL_MODE_DIR
# Provides:
#  REAL_MODE_OBJS: The application object files


### Check pre-requisites ###
ifndef OUTPUT
	$(error Build output directory undefined \(OUTPUT undefined\).)
else
ifeq ("$(OUTPUT)", "")
	$(error Build output directory undefined \(OUTPUT is empty\).)
endif
endif

ifndef REAL_MODE_DIR
	$(error Sourcet directory undefined \(REAL_MODE_DIR undefined\).)
else
ifeq ("$(REAL_MODE_DIR)", "")
	$(error Source directory undefined \(REAL_MODE_DIR is empty\).)
endif
endif

REAL_MODE_OUT := $(OUTPUT)/real-mode
REAL_MODE_SRC := $(REAL_MODE_DIR)/real-mode.S
REAL_MODE_OBJ := $(addprefix $(OUTPUT)/, $(call objetize, $(REAL_MODE_SRC)))
REAL_MODE_TARGET := $(REAL_MODE_OUT)/relocable-real-mode.o
REAL_MODE_BIN := $(REAL_MODE_OUT)/real-mode.bin

# Ensure build directory will be created, but let generic rules build the source
# oject file
$(REAL_MODE_OBJ): | $(REAL_MODE_OUT)

# Link the code to the address 0x2000
$(REAL_MODE_BIN): $(REAL_MODE_OBJ)
	$(LD) $(LDFLAGS) -e do_bios_call_rm -Ttext 0x2000 $^ --oformat binary -o $@

$(REAL_MODE_TARGET): $(OUTPUT)/empty.o $(REAL_MODE_BIN)
	$(OBJCOPY) $< \
		--add-section=.real-mode=$(filter-out $<, $^) \
		--set-section-flags=.real-mode=contents,alloc,load,data $@

$(REAL_MODE_OUT):
	$(MKDIR) -p $@

