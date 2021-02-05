#
# Ensimag - Projet système
# Copyright (C) 2001-2003 - Simon Nieuviarts
# Copyright (C) 2012-2014 - Damien Dejean <dam.dejean@gmail.com>
# Copyright (C) 2019 - Grégory Mounié <Gregory.Mounie@imag.fr>

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
TOOLCHAIN_PREFIX ?=
CC      := $(TOOLCHAIN_PREFIX)gcc
AS      := $(TOOLCHAIN_PREFIX)gcc
LD      ?= $(TOOLCHAIN_PREFIX)ld
AR      := $(TOOLCHAIN_PREFIX)ar
OBJCOPY := $(TOOLCHAIN_PREFIX)objcopy
NM      := $(TOOLCHAIN_PREFIX)nm
DEP     := $(CC)
MKDIR   := mkdir
CP      := cp
ECHO    ?= echo
GEN_SECTIONS := build/generate-link-sections.sh
GEN_TABLE    := build/generate-symbols-table.sh


### Override with quiet tools by default
ifneq "$(VERBOSE)" "1"
include build/quiet.mk
endif

### Tools flags ###
DEFS   := -D__KERNEL__
CFLAGS := -m32 -march=pentium \
		  -Wall -Wextra -Werror -std=c99 \
		  -g \
		  -pipe \
		  -nostdinc \
		  -nostdlib \
		  -fno-stack-protector \
		  $(DEFS)
ASFLAGS := -m32 \
		   -DASSEMBLER \
		   $(DEFS)
ARFLAGS := rcs
LDFLAGS := -melf_i386

# Import facilities
include build/utils.mk

### Compilation sources ###
# Use DIRS variable to add source sub-directories
DIRS := . boot
SRC  :=	\
	$(call all-asm-files-under, $(DIRS)) \
	$(call all-c-files-under , $(DIRS))
INCLUDES := $(addprefix -I, $(DIRS) ../shared)

### Output files ###
DEPS        := $(addprefix $(OUTPUT)/, $(call generate-deps, $(SRC)))
OBJS        := $(addprefix $(OUTPUT)/, $(call objetize, $(SRC))) \
	$(OUTPUT)/task_dump_screen.o
OUTPUT_DIRS := $(addprefix $(OUTPUT)/, $(DIRS))

### Shared lib build ###
SHARED_LIB_DIR := ../shared
include build/shared-lib.mk


### Linux keyboard support build ###
KBD_LINUX_DIR := kbd-linux
include build/kbd-linux.mk


### Build code for real-mode ###
#REAL_MODE_DIR := real-mode
#include build/real-mode.mk

### Generate object files for userspace binaries ###
include build/apps-integration.mk


### Generate the kernel, ready to run ###
kernel.bin: build/kernel.lds $(OBJS) $(SHARED_LIB_TARGET) $(KBD_LINUX_TARGET) $(APPS_OBJS) $(REAL_MODE_TARGET)
	$(LD) $(LDFLAGS) -e entry -T$< $(filter-out %.lds, $^) -o $@

### insert the kernel in an iso file (cdrom) suitable for bochs and USB key
cdrom.iso: kernel.bin
	mkdir -p iso/boot/grub
	cp kernel.bin iso/boot
	printf 'set timeout=0\nset default=0\nmenuentry "kernel.bin" {\n multiboot /boot/kernel.bin\n boot\n}' > iso/boot/grub/grub.cfg
	grub-mkrescue -o cdrom.iso iso
	rm -r iso/

### Running bochs: do [6] Begin Smulation then 'cont'
### to run the PC simulation
.PHONY: bochs
bochs: cd.iso
	bochs -q 'memory: guest=256,host=256' 'boot: cdrom' 'ata0-master: type=cdrom, path="cdrom.iso", status=inserted'

### Generic rules ###
$(OUTPUT)/%.d: %.c | $(OUTPUT)
	@printf "$(@D)/" > $@
	$(DEP) $(INCLUDES) -MM $< >> $@

$(OUTPUT)/%.d: %.S | $(OUTPUT)
	@printf "$(@D)/" > $@
	$(DEP) $(INCLUDES) -MM $< >> $@

$(OUTPUT)/%.o: %.c | $(OUTPUT)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OUTPUT)/%.o: %.S | $(OUTPUT)
	$(AS) $(ASFLAGS) $(INCLUDES) -c $< -o $@

# A text screen for exception dump
$(OUTPUT)/task_dump_screen.o: $(OUTPUT)/empty.o task_dump_screen.txt
	$(OBJCOPY) $< \
		--add-section=.task_dump_screen=$(filter-out $<, $^) \
		--set-section-flags=.task_dump_screen=contents,alloc,load,data $@


# Add a dependency between each object file and the directory that will contain
# it.
$(foreach obj-file, $(OBJS), $(eval 			\
	$(call generate-build-dir-dep, $(obj-file)) \
))
$(foreach dep-file, $(DEPS), $(eval 			\
	$(call generate-build-dir-dep, $(dep-file)) \
))

# Create a target for each build output directory
$(foreach build-dir, $(OUTPUT_DIRS), $(eval 		\
	$(call generate-build-dir-target, $(build-dir))	\
))

# Include app dependency files if required
ifeq "$(MAKECMDGOALS)" "kernel.bin"
-include $(DEPS)
endif

### Build output directory ###
$(OBJS): | $(OUTPUT)
$(DEPS): | $(OUTPUT)
$(OUTPUT):
	$(MKDIR) -p $@


### Clean everything ###
.PHONY: clean
clean:
	rm -rf $(OUTPUT) kernel.bin cdrom.iso

