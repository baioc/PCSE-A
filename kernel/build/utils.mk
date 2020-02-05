#
# Ensimag - Projet système
# Copyright (C) 2012 - Damien Dejean <dam.dejean@gmail.com>
#
# Smart utils for user space compilation tool chain
#

# Remove trailing slash at the end of a string
define remove-trailing-slash
$(patsubst %/, %, $(strip $(1)))
endef

# List all c files into a directory
# all-c-files-under(directory)
define all-c-files-under
$(strip 															\
	$(patsubst ./%, %, 												\
		$(wildcard 													\
			$(addsuffix /*.c, $(call remove-trailing-slash, $(1)))	\
		)															\
	)																\
)
endef

# List all assembly files into a directory
# all-asm-files-under(directory)
define all-asm-files-under
$(strip 															\
	$(patsubst ./%, %, 												\
		$(wildcard 													\
			$(addsuffix /*.S, $(call remove-trailing-slash, $(1)))	\
		)															\
	)																\
)
endef

# Generate objects file names from compilable file names
# objetize
define objetize
$(patsubst %.S, %.o, $(patsubst %.c, %.o, $(1)))
endef

# Generate dependencies file names from compilable file names
# objetize
define generate-deps
$(strip $(patsubst %.S, %.d, $(patsubst %.c, %.d, $(1))))
endef

# Generate a dependency to the containing directory for a provided file.
define generate-build-dir-dep
$(1): | $(patsubst %/, %, $(dir $(1)))
endef

# Generate a target to create a build directory
define generate-build-dir-target
$(1):
	$$(MKDIR) -p $$@
endef
