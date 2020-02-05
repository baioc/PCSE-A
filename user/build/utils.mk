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

# Figure out where we are
define my-dir
$(strip                                                             \
    $(eval LOCAL_MODULE_MAKEFILE := $$(lastword $$(MAKEFILE_LIST))) \
    $(patsubst %/,%,$(dir $(LOCAL_MODULE_MAKEFILE)))                \
)
endef
