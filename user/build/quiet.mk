#
# Ensimag - Projet système
# Copyright (C) 2012-2014 - Damien Dejean <dam.dejean@gmail.com>
#

# Use temporary variables to store the tools
QCC := $(CC)
QAS := $(AS)
QLD := $(LD)
QAR := $(AR)
QCP := $(CP)
QDEPS := $(DEPS)
QMKDIR := $(MKDIR)
QOBJCOPY := $(OBJCOPY)
QGENSECTIONS := $(GEN_SECTIONS)
QGENTABLE := $(GEN_TABLE)

CC      = @$(ECHO) "  CC   [U] $@"; $(QCC)
AS      = @$(ECHO) "  AS   [U] $@"; $(QAS)
LD      = @$(ECHO) "  LD   [U] $@"; $(QLD)
AR      = @$(ECHO) "  AR   [U] $@"; $(QAR)
DEPS    = @$(ECHO) "  DEPS [U] $@"; $(QDEPS)
CP      = @$(ECHO) "  CP       $< -> $@"; $(QCP)
MKDIR   = @$(ECHO) "  MKDIR    $@"; $(QMKDIR)
OBJCOPY = @$(ECHO) "  OBJCOPY  $@"; $(QOBJCOPY)


