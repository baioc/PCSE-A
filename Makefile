.PHONY: all clean build test run debug gdb

# default rule
all: test

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

ifeq ($(OS_PCSEA),macOS)
GDB=i386-pc-elf-gdb
else
GDB=gdb
endif

QEMU=qemu-system-i386 -cpu pentium -m 256M

kernel/$(PLATFORM_TOOLS):
	$(MAKE) -C kernel/ $(PLATFORM_TOOLS)

user/$(PLATFORM_TOOLS):
	$(MAKE) -C user/ $(PLATFORM_TOOLS)

clean:
	$(MAKE) clean -C kernel/
	$(MAKE) clean -C user/

#
# Build modes are EITHER 'build' or 'test'.
# A 'clean' is needed when changing between them.
#
build: | kernel/$(PLATFORM_TOOLS) user/$(PLATFORM_TOOLS)
	$(MAKE) -C user/ all VERBOSE=$(VERBOSE)
	$(MAKE) -C kernel/ kernel.bin VERBOSE=$(VERBOSE)

test: | kernel/$(PLATFORM_TOOLS) user/$(PLATFORM_TOOLS)
	$(MAKE) -C user/ all VERBOSE=$(VERBOSE)
	$(MAKE) -C kernel/ kernel.bin VERBOSE=$(VERBOSE) KERNEL_TEST=1

#
# 'run' and 'debug' rules require the kernel to be already built, for example:
# - "make build run" builds the non-test kernel and runs qemu WITHOUT gdb
# - since we'll follow up with a test build: "make clean test"
# - "make debug & make gdb" runs the built kernel WITH gdb
#
run:
	$(QEMU) -kernel kernel/kernel.bin

debug:
	$(QEMU) -kernel kernel/kernel.bin -gdb tcp::1234 -S

# connects gdb to an already-running qemu process on localhost:1234
gdb:
	$(GDB) --tui -f kernel/kernel.bin \
	       -ex "dir kernel" \
	       -ex "target remote localhost:1234" \
	       -ex "break panic" \
	       -ex "tbreak kernel_start" -ex "continue"
