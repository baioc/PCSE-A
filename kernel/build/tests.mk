# Inspired by kbd-linux.mk

### Check pre-requisites ###
ifndef OUTPUT
	$(error Build output directory undefined \(OUTPUT undefined\).)
else
ifeq ("$(OUTPUT)", "")
	$(error Build output directory undefined \(OUTPUT is empty\).)
endif
endif

ifndef TESTS_DIR
	$(error Sourcet directory undefined \(TESTS_DIR undefined\).)
else
ifeq ("$(TESTS_DIR)", "")
	$(error Source directory undefined \(TESTS_DIR is empty\).)
endif
endif

# Add define for KERNEL_TEST. If KERNEL_TEST is present, kernel tests will be
# launched right after kernel startup (see start.c for usage of KERNEL_TEST)
CFLAGS += -D KERNEL_TEST

TESTS_OUT    := $(OUTPUT)/tests
TESTS_SRC    := $(wildcard $(TESTS_DIR)/*.c)
TESTS_INC    := -I$(TESTS_DIR)/
TESTS_DEPS   := $(addprefix $(OUTPUT)/, $(call generate-deps, $(TESTS_SRC)))
TESTS_OBJS   := $(addprefix $(OUTPUT)/, $(call objetize, $(TESTS_SRC)))


### Dependency management ###
ifeq "$(MAKECMDGOALS)" "kernel.bin"
-include $(TESTS_DEPS)
endif

# Standard build rules will be handle by generic rules in parent Makefile.
# Just add a dependency to the build output directory
# Added ../shared includes
$(TESTS_DEPS): INCLUDES := $(TESTS_INC) -I../shared -I.
$(TESTS_DEPS): | $(TESTS_OUT)

### Build rules ###
# Standard build rules will be handle by generic rules in parent Makefile.
# Just add a dependency to the build output directory
# Added ../shared includes
$(TESTS_OBJS): INCLUDES := $(TESTS_INC) -I../shared -I.
$(TESTS_OBJS): | $(TESTS_OUT)

$(TESTS_OUT):
	$(MKDIR) -p $@
