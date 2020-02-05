#
# Ensimag - Projet système
# Copyright (C) 2012-2014 - Damien Dejean <dam.dejean@gmail.com>
#

#
# Generate a library from shared sources between kernel an userspace.
#
# Dependencies:
#  utils.mk, OUTPUT, CC, AR
# Parameters:
#  SHARED_LIB_DIR: directory where to look for sources
#  OUTPUT: directory for build output
# Provides:
#  SHARED_LIB_TARGET: the target to build the library which is the path to the
#  library file.


### Check pre-requisites ###
ifndef OUTPUT
	$(error Build output directory undefined \(OUTPUT undefined\).)
else
ifeq ("$(OUTPUT)", "")
	$(error Build output directory undefined \(OUTPUT is empty\).)
endif
endif

ifndef SHARED_LIB_DIR
	$(error Sourcet directory undefined \(SHARED_LIB_DIR undefined\).)
else
ifeq ("$(SHARED_LIB_DIR)", "")
	$(error Source directory undefined \(SHARED_LIB_DIR is empty\).)
endif
endif

### Environment ###
SHARED_LIB_OUT  := $(OUTPUT)/shared
SHARED_LIB_SRC  := $(call all-c-files-under, $(SHARED_LIB_DIR)) \
	$(call all-asm-files-under, $(SHARED_LIB_DIR))
SHARED_LIB_INC   := $(addprefix -I, $(SHARED_LIB_DIR))
SHARED_LIB_DEPS := $(addprefix $(SHARED_LIB_OUT)/, $(call generate-deps, $(notdir $(SHARED_LIB_SRC))))
SHARED_LIB_OBJS := $(addprefix $(SHARED_LIB_OUT)/, $(call objetize, $(notdir $(SHARED_LIB_SRC))))

# Library target file
SHARED_LIB_TARGET := $(OUTPUT)/libshared.a

# Set the path where to look for files
vpath %.c $(SHARED_LIB_DIR)
vpath %.S $(SHARED_LIB_DIR)


### Library dependency file ###
ifeq "$(MAKECMDGOALS)" "kernel.bin"
-include $(SHARED_LIB_DEPS)
endif

$(SHARED_LIB_OUT)/%.d: %.c | $(SHARED_LIB_OUT)
	@printf "$(@D)/" > $@
	$(DEP) -MM $< $(SHARED_LIB_INC) >> $@

$(SHARED_LIB_OUT)/%.d: %.S | $(SHARED_LIB_OUT)
	@printf "$(@D)/" > $@
	$(DEP) -MM $< $(SHARED_LIB_INC) >> $@


### Library build ###
$(SHARED_LIB_TARGET): $(SHARED_LIB_OBJS)
	$(AR) $(ARFLAGS) $@ $^

$(SHARED_LIB_OUT)/%.o: %.c | $(SHARED_LIB_OUT)
	$(CC) $(CFLAGS) $(SHARED_LIB_INC) -c $< -o $@

$(SHARED_LIB_OUT)/%.o: %.S | $(SHARED_LIB_OUT)
	$(AS) $(ASFLAGS) $(SHARE_LIB_INC) -c $< -o $@

$(SHARED_LIB_OUT):
	$(MKDIR) -p $@

