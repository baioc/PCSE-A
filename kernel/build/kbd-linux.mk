#
# Ensimag - Projet système
# Copyright (C) 2012-2014 - Damien Dejean <dam.dejean@gmail.com>
#

#
# Generate a the scancode keyboard mapping.
#
# Dependencies:
#  utils.mk, OUTPUT, CC, AR
# Parameters:
#  KBD_LINUX_DIR: directory where to look for sources
#  OUTPUT: directory for build output
# Provides:
#  KBD_LINUX_TARGET: the target to build the scancode mapping.


### Check pre-requisites ###
ifndef OUTPUT
	$(error Build output directory undefined \(OUTPUT undefined\).)
else
ifeq ("$(OUTPUT)", "")
	$(error Build output directory undefined \(OUTPUT is empty\).)
endif
endif

ifndef KBD_LINUX_DIR
	$(error Sourcet directory undefined \(KBD_LINUX_DIR undefined\).)
else
ifeq ("$(KBD_LINUX_DIR)", "")
	$(error Source directory undefined \(KBD_LINUX_DIR is empty\).)
endif
endif

KBD_LINUX_OUT    := $(OUTPUT)/kbd-linux
KBD_LINUX_SRC    := $(KBD_LINUX_DIR)/keyboard.c $(KBD_LINUX_DIR)/scancode.c
KBD_LINUX_INC    := -I$(KBD_LINUX_DIR)/
KBD_LINUX_DEPS   := $(addprefix $(OUTPUT)/, $(call generate-deps, $(KBD_LINUX_SRC)))
KBD_LINUX_OBJS   := $(addprefix $(OUTPUT)/, $(call objetize, $(KBD_LINUX_SRC))) \
	$(KBD_LINUX_OUT)/keyboard-keymap.o
KBD_LINUX_TARGET := $(KBD_LINUX_OUT)/kbd-linux.o


### Dependency management ###
ifeq "$(MAKECMDGOALS)" "kernel.bin"
-include $(KBD_LINUX_DEPS)
endif

# Standard build rules will be handle by generic rules in parent Makefile.
# Just add a dependency to the build output directory
$(KBD_LINUX_DEPS): INCLUDES := $(KBD_LINUX_INC)
$(KBD_LINUX_DEPS): | $(KBD_LINUX_OUT)


### Build rules ###
# Standard build rules will be handle by generic rules in parent Makefile.
# Just add a dependency to the build output directory
$(KBD_LINUX_OBJS): | $(KBD_LINUX_OUT)

.INTERMEDIATE: $(KBD_LINUX_OUT)/keymap.c
$(KBD_LINUX_OUT)/keymap.c: $(KBD_LINUX_DIR)/defkeymap.c
	$(CP) -f $< $@

$(KBD_LINUX_OUT)/keymap.o: $(KBD_LINUX_OUT)/keymap.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@	

$(KBD_LINUX_OUT)/keyboard-keymap.o: $(KBD_LINUX_OUT)/keymap.o
	$(OBJCOPY) `$(NM) -g -f posix $^ | awk '{printf " --redefine-sym "$$1"=keyboard_"$$1}'` $^ $@

$(KBD_LINUX_TARGET): INCLUDES := $(KBD_LINUX_INC)
$(KBD_LINUX_TARGET): $(KBD_LINUX_OBJS)
	$(LD) $(LDFLAGS) -r $^ -o $@

$(KBD_LINUX_OUT):
	$(MKDIR) -p $@

### Special keymap generation targets, call one of them once to generate a keymap ###
.PHONY: frkm
frkm:
	loadkeys --mktable fr > $(KBD_LINUX_SRC)/defkeymap.c

.PHONY: uskm
uskm:
	loadkeys --mktable us > $(KBD_LINUX_SRC)/defkeymap.c

