#
# Ensimag - Projet système
# Copyright (C) 2012-2014 - Damien Dejean <dam.dejean@gmail.com>
#

#
# Generates a binary for each program provided in APPS_NAMES. Each app will
# be compiled in it's own directory, and two binaries will be created. One
# will be the classic elf binary, the other one will be an object file
# where all the content of the elf binary have been dumped into a section
# named as the application.
# Each app will be linked with the user space standard library.
#
# Dependencies:
#  utils.mk, OUTPUT, CC, LD
# Parameters:
#  APPS_NAMES: the list of the applications names. Each app content must be
#  stored in a directory anmed as the application itself.
#  APPS_STD_INC: a list of directory that contains includes for userspace
#  standard library
#  APPS_STD_LIB: path to the standard static library file
#  APPS_STD_TARGET: the Makefile target we need to depends if we want a
#  standard library file
#  OUTPUT: directory for build output
# Provides:
#  APPS_TARGETS: Will contain the targets to request to build the set of
#  apps.

### Check requirements ###
ifndef OUTPUT
	$(error Build output directory undefined \(OUTPUT undefined\).)
else
ifeq ("$(OUTPUT)", "")
	$(error Build output directory undefined \(OUTPUT is empty\).)
endif
endif
ifndef APPS_NAMES
	$(error Applications names list undefined \(APPS_NAMES undefined\).)
else
ifeq ("$(APPS_NAMES)", "")
	$(error Applications names list undefined \(APPS_NAMES is empty\).)
endif
endif
ifndef APPS_STD_INC
	$(error Standard includes directories undefined \(APPS_STD_INC undefined\).)
else
ifeq ("$(APPS_STD_INC)", "")
	$(error Standard includes directories undefined \(APPS_STD_INC is empty\).)
endif
endif
ifndef APPS_STD_LIB
	$(error Standard library file undefined \(APPS_STD_LIB undefined\).)
else
ifeq ("$(APPS_STD_LIB)", "")
	$(error Standard library file undefined \(APPS_STD_LIB is empty\).)
endif
endif
ifndef APPS_STD_TARGET
	$(error Standard library target undefined \(APPS_STD_TARGET undefined\).)
else
ifeq ("$(APPS_STD_TARGET)", "")
	$(error Standard library target undefined \(APPS_STD_TARGET is empty\).)
endif
endif

### Check that apps names fit directories ###
$(foreach app, $(APPS_NAMES),													\
	$(if $(wildcard $(app)),													\
		,																		\
		$(error '$(app)' declared but $(app) directory does not exist.)			\
	)																			\
)

### Define a compilation environment for an application ###
define APP_BUILD

# Create build context for the app
$(1)_DIR  := $$(call remove-trailing-slash, $(1))
$(1)_OUT  := $$(addprefix $(OUTPUT)/, $$($(1)_DIR))
$(1)_SRC  := $$(call all-c-files-under, $$($(1)_DIR)) \
	$$(call all-asm-files-under, $$($(1)_DIR))
$(1)_OBJS := $$(addprefix $(OUTPUT)/, $$(call objetize, $$($(1)_SRC)))
$(1)_DEPS := $$(addprefix $(OUTPUT)/, $$(call generate-deps, $$($(1)_SRC)))
$(1)_INC  := -I. $$(addprefix -I, $(APPS_STD_INC) $$($(1)_DIR))

# Create final target and add it to the list of target to call
$(1)_TARGET := $$(addprefix $(OUTPUT)/, $$(addsuffix .bin, $$($(1)_DIR)))

# Include app dependency files if required
ifneq "$$(MAKECMDGOALS)" "clean"
    ifneq "$$(MAKECMDGOALS)" "$$(PLATFORM_TOOLS)"
        -include $$($(1)_DEPS)
    endif
endif

# Compilation products depends on output directory
$$($(1)_DEPS) $$($(1)_OBJS)): | $$($(1)_OUT)
$$($(1)_OUT):
	$$(MKDIR) -p $$@

# Define compilation flags/includes for app targets
$$($(1)_DEPS): APP_INC := $$($(1)_INC)
$$($(1)_TARGET): APP_INC := $$($(1)_INC)
$$($(1)_TARGET): APP_CFLAGS := $$(CFLAGS)
$$($(1)_TARGET): APP_LDFLAGS := $$(LDFLAGS) --oformat=binary -T build/apps.lds

$$($(1)_TARGET): $$(APPS_STD_TARGET) $$($(1)_OBJS) build/apps.mk
	$$(LD) $$(APP_LDFLAGS) -o $$@ $$(filter-out build/apps.mk, $$(filter-out $$<, $$^)) $(OUTPUT)/libstd.a

endef

# The following statements are common to all apps compilation processes. We
# assume that all the APP_xxx variables are defined using target-specific
# definitions.
# (see http://www.gnu.org/software/make/manual/make.html#Target_002dspecific)

### Dependency generation targets ###
$(OUTPUT)/%.d: %.c
	@printf "$(@D)/" > $@
	$(DEPS) $(CFLAGS) -MM $< $(APP_INC) >> $@

$(OUTPUT)/%.d: %.S
	@printf "$(@D)/" > $@
	$(DEPS) $(CFLAGS) -MM $< $(APP_INC) >> $@

### Generic targets for compilation ###
$(OUTPUT)/%.o: %.c
	$(CC) $(APP_CFLAGS) $(APP_INC) -c $< -o $@

$(OUTPUT)/%.o: %.S
	$(AS) $(APP_CFLAGS) $(APP_INC) -c $< -o $@

### Create targets for parent Makefile ###
APPS_TARGETS := $(addprefix $(OUTPUT)/, $(addsuffix .bin, $(APPS_NAMES)))

### Apply compile environment to each app ###
$(foreach app, $(APPS_NAMES), $(eval $(call APP_BUILD,$(app))))

