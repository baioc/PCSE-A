.PHONY: clean all debug run run_gdb

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
endif

ifeq ($(OS_PCSEA),)
GDB=gdb
endif

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
	qemu-system-i386 -cpu pentium -kernel kernel/kernel.bin -m 256M -gdb tcp::1234 -S &
	$(GDB) --tui -f kernel/kernel.bin -ex "target remote localhost:1234" -ex "dir kernel" -ex "tbreak kernel_start" -ex "continue"

run: all
	qemu-system-i386 -curses -cpu pentium -kernel kernel/kernel.bin -m 256M -gdb tcp::1234 -S

run_gdb:
	$(GDB) --tui -f kernel/kernel.bin -ex "target remote localhost:1234" -ex "dir kernel" -ex "tbreak kernel_start" -ex "continue"
