.PHONY: clean all debug

# Notify C compiler
ifdef CC
$(info Environment compiler: $(CC))
endif

# Output directory for each submakefiles
OUTPUT := out
export OUTPUT

#
# Some build tools need to be explicitely defined before building. The toolchain
# creates the following platform tools configuration file before it allows the
# toolchain to build.
#
PLATFORM_TOOLS := $(OUTPUT)/platform-tools.mk
export PLATFORM_TOOLS

all: | kernel/$(PLATFORM_TOOLS) user/$(PLATFORM_TOOLS)
	$(MAKE) -C user/ all VERBOSE=$(VERBOSE)
	$(MAKE) -C kernel/ kernel.bin VERBOSE=$(VERBOSE)

kernel/$(PLATFORM_TOOLS):
	$(MAKE) -C kernel/ $(PLATFORM_TOOLS)

user/$(PLATFORM_TOOLS):
	$(MAKE) -C user/ $(PLATFORM_TOOLS)

clean:
	$(MAKE) clean -C kernel/
	$(MAKE) clean -C user/

debug: all
	qemu-system-i386 -kernel kernel/kernel.bin -m 256M -gdb tcp::1234 -S &
	gdb --tui -f kernel/kernel.bin -ex "target remote localhost:1234" -ex "dir kernel" -ex "tbreak kernel_start" -ex "continue"
