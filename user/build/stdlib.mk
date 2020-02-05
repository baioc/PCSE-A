#
# Ensimag - Projet système
# Copyright (C) 2012-2014 - Damien Dejean <dam.dejean@gmail.com>
#

#
# Generates a standard static library for the user space. This library will be
# built from the source file found in the provided directories (STDLIB_DIRS
# variable). The generated library file will be named libXXX.a where XXX is
# the provided library name (STDLIB_NAME variable). The compilation
# products will be stored in $(OUTPUT)/$(STDLIB_NAME).
#
# Dependencies:
#  utils.mk, OUTPUT, CC, AR
# Parameters:
#  STDLIB_DIRS: directories where library files are located.
#  STDLIB_NAME: name of the standard library.
#  OUTPUT: directory for build output
# Provides:
#  STDLIB_TARGET: the target to build the library which is the path to the
#  library file.


### Check pre-requisites ###
ifndef OUTPUT
	$(error Build output directory undefined \(OUTPUT undefined\).)
else
ifeq ("$(OUTPUT)", "")
	$(error Build output directory undefined \(OUTPUT is empty\).)
endif
endif

ifndef STDLIB_DIRS
	$(error No directory defined for standard library source files \(STDLIB_DIRS undefined\).)
else
ifeq ("$(STDLIB_DIRS)", "")
	$(error Empty directory defined for standard library source files \(STDLIB_DIRS is empty\).)
endif
endif

ifndef STDLIB_NAME
	$(error No name defined for the standard library \(STDLIB_NAME undefined\).)
else
ifeq ("$(STDLIB_NAME)", "")
	$(error Empty name defined for standard library source files \(STDLIB_NAME is empty\).)
endif
endif


### Setup library creation environment ###

# As we have our custom kernel, user space programs cannot use the system
# standard library. The following lines define an environment to build a
# custom standard library for userspace.
STDLIB_OUT    := $(OUTPUT)/$(STDLIB_NAME)
STDLIB_SRC    := $(call all-c-files-under, $(STDLIB_DIRS)) \
	$(call all-asm-files-under, $(STDLIB_DIRS))
STDLIB_INC    := $(addprefix -I, $(STDLIB_DIRS))
STDLIB_DEPS   := $(addprefix $(STDLIB_OUT)/, $(notdir $(call generate-deps, $(STDLIB_SRC))))
STDLIB_OBJS   := $(addprefix $(STDLIB_OUT)/, $(notdir $(call objetize, $(STDLIB_SRC))))
STDLIB_TARGET := $(OUTPUT)/lib$(STDLIB_NAME).a

# Our local compilation flags
STDLIB_CFLAGS  := $(CFLAGS)
STDLIB_ARFLAGS := rcs

# Notify GNUmake where he can find source files
VPATH := $(STDLIB_DIRS)


### Library dependency file ###
ifneq "$(MAKECMDGOALS)" "clean"
    ifneq "$(MAKECMDGOALS)" "$(PLATFORM_TOOLS)"
        -include $(STDLIB_DEPS)
    endif
endif

$(STDLIB_OUT)/%.d: %.c
	@printf "$(@D)/" > $@
	$(DEPS) $(CFLAGS) -MM $< $(STDLIB_INC) >> $@

$(STDLIB_OUT)/%.d: %.S
	@printf "$(@D)/" > $@
	$(DEPS) $(CFLAGS) -MM $< $(STDLIB_INC) >> $@


### Library compilation and archive ###
$(STDLIB_TARGET): $(STDLIB_OBJS)
	$(AR) $(STDLIB_ARFLAGS) $@ $^

$(STDLIB_OUT)/%.o: %.c
	$(CC) $(STDLIB_CFLAGS) $(STDLIB_INC) -c $< -o $@

$(STDLIB_OUT)/%.o: %.S
	$(AS) $(STDLIB_CFLAGS) $(STDLIB_INC) -c $< -o $@


### Create the build directory for the library objects ###

# Build deps and products need the directory
$(STDLIB_DEPS): | $(STDLIB_OUT)
$(STDLIB_OBJS): | $(STDLIB_OUT)

$(STDLIB_OUT):
	$(MKDIR) -p $@

