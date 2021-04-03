.PHONY: all clean build run debug gdb

# default rule
all: build

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

build: kernel/kernel.bin

kernel/kernel.bin: | kernel/$(PLATFORM_TOOLS) user/$(PLATFORM_TOOLS)
	$(MAKE) -C user/ all VERBOSE=$(VERBOSE)
	$(MAKE) -C kernel/ kernel.bin VERBOSE=$(VERBOSE)

#
# "make run" launches Qemu WITHOUT gdb
# "make debug & make gdb" runs Qemu with a connected gdb session
#

run: kernel/kernel.bin
	$(QEMU) -kernel kernel/kernel.bin

debug: kernel/kernel.bin
	$(QEMU) -kernel kernel/kernel.bin -gdb tcp::1234 -S

# connects gdb to an already-running qemu process on localhost:1234
gdb: kernel/kernel.bin
	$(GDB) --tui -f kernel/kernel.bin \
	       -ex "dir kernel" \
	       -ex "target remote localhost:1234" \
	       -ex "tbreak kernel_start" -ex "continue"
