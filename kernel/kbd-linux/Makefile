# Copyright (C) 2002 by Simon Nieuviarts
FILES=keyboard.c scancode.c
OBJS=$(notdir $(patsubst %.S,%.o,$(patsubst %.c,%.o,$(FILES))))
CC=gcc -m32
AS=gcc -m32
LD=ld -melf_i386
DIRS=.

KFLAGS=-Wall -O2 -fomit-frame-pointer -pipe -nostdinc
DEF=-D__KERNEL__
INC=$(patsubst %,-I%,$(DIRS))
CCL=$(CC) $(KFLAGS) $(DEF) $(INC)
ASL=$(AS) -DASSEMBLER $(DEF) $(INC)
DEPS=$(patsubst %.o,deps/%.d,$(OBJS))

vpath %.c $(DIRS)
vpath %.h $(DIRS)
vpath %.S $(DIRS)

.PHONY: clean all fr frkm us uskm

all: kbd_linux.o

deps/%.d: %.c
	@$(SHELL) -ec '$(CCL) -M $< | sed '\''s|\($*\)\.o[ :]*|\1.o $@ : Makefile |g'\'' > $@; [ -s $@ ] || rm -f $@'

deps/%.d: %.S
	@$(SHELL) -ec '$(ASL) -M $< | sed '\''s|\($*\)\.o[ :]*|\1.o $@ : Makefile |g'\'' > $@; [ -s $@ ] || rm -f $@'

-include $(DEPS)

%.o: %.S
	$(ASL) -c $< -o $@

%.o: %.c
	$(CCL) -c $< -o $@

%.s: %.c
	$(CCL) -S -c $< -o $@

keymap.c: defkeymap.c
	cp -f defkeymap.c keymap.c

fr: frkm kbd_linux.o

us: uskm kbd_linux.o

frkm:
	loadkeys --mktable fr > keymap.c

uskm:
	cp -f defkeymap.c keymap.c

keymap2.o: keymap.o
	objcopy `nm -g -f posix $^ | awk '{printf " --redefine-sym "$$1"=keyboard_"$$1}'` $^ $@

kbd_linux.o: $(OBJS) keymap2.o
	$(LD) $(LDFLAGS) -r $^ -o $@

clean:
	rm -f keymap.c $(OBJS) keymap.o keymap2.o kbd_linux.o $(DEPS)
