#
# Ensimag - Projet système
# Copyright (C) 2012-2014 - Damien Dejean <dam.dejean@gmail.com>
#
# User space compile tool chain.
# This Makefile use sub makefiles in build/ to build and link each app and
# the standard library for our custom kernel/userspace environment.

ifeq ($(OUTPUT),)
    $(error Output build directory not specified by caller)
endif


### Platform tools ###
$(PLATFORM_TOOLS): | $(OUTPUT)
	build/generate-platform-tools.sh > $@

ifneq "$(MAKECMDGOALS)" "$(PLATFORM_TOOLS)"
include $(PLATFORM_TOOLS)
endif


### Tools ###
ifeq ($(OS_PCSEA),macOS)
    CC      := i386-pc-elf-gcc
    AS      := i386-pc-elf-gcc
    LD      := i386-pc-elf-ld
    AR      := i386-pc-elf-ar
    OBJCOPY := i386-pc-elf-objcopy
endif

ifeq ($(OS_PCSEA),)
    TOOLCHAIN_PREFIX ?=
    CC      := $(TOOLCHAIN_PREFIX)gcc
    AS      := $(TOOLCHAIN_PREFIX)gcc
    LD      ?= $(TOOLCHAIN_PREFIX)ld
    AR      := $(TOOLCHAIN_PREFIX)ar
    OBJCOPY := $(TOOLCHAIN_PREFIX)objcopy
endif

DEPS    := $(CC)
ECHO    ?= echo
MKDIR   := mkdir
CP      := cp


### Override with quiet tools by default
ifneq "$(VERBOSE)" "1"
include build/quiet.mk
endif


# Global compilation flags
CFLAGS := -m32 -march=pentium \
		  -Wall -Wextra -Werror -Wmissing-include-dirs -std=c99 \
		  -g \
		  -pipe \
		  -nostdinc \
		  -fno-pic \
		  -fno-stack-protector
LDFLAGS := -melf_i386 \
	-nostdlib


# Include makefile utilities
include build/utils.mk

### Import standard library build toolchain ###
STDLIB_DIRS := ../shared lib
STDLIB_NAME := std
include build/stdlib.mk

### Compile userspace tests ###
TESTS_DIR     := tests
TESTS_OPTIONS := WITH_MSG WITH_SEM CONS_READ_LINE
include build/tests.mk

### Import userspace program build toolchain ###
APPS_NAMES      := init
APPS_STD_INC    := $(STDLIB_DIRS)
APPS_STD_LIB    := $(OUTPUT)/lib$(STDLIB_NAME).a
APPS_STD_TARGET := $(STDLIB_TARGET)
include build/apps.mk

# The all target is virtual, it is just a way to call all
.PHONY: all
all: $(APPS_TARGETS) $(TESTS_TARGETS)

### Standard output directory ###
$(OUTPUT):
	$(MKDIR) -p $@

.PHONY: clean
clean:
	rm -rf $(OUTPUT)
