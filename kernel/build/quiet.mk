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
QDEP := $(DEP)
QMKDIR := $(MKDIR)
QOBJCOPY := $(OBJCOPY)
QGENSECTIONS := $(GEN_SECTIONS)
QGENTABLE := $(GEN_TABLE)

CC           = @$(ECHO) "  CC [K]  $@"; $(QCC)
AS           = @$(ECHO) "  AS [K]  $@"; $(QAS)
LD           = @$(ECHO) "  LD [K]  $@"; $(QLD)
AR           = @$(ECHO) "  AR [K]  $@"; $(QAR)
DEP          = @$(ECHO) "  DEP [K] $@"; $(QDEP)
CP           = @$(ECHO) "  CP      $< -> $@"; $(QCP)
MKDIR        = @$(ECHO) "  MKDIR   $@"; $(QMKDIR)
OBJCOPY      = @$(ECHO) "  OBJCOPY $@"; $(QOBJCOPY)
GEN_SECTIONS = @$(ECHO) "  GEN-SECTIONS $@"; $(QGENSECTIONS)
GEN_TABLE    = @$(ECHO) "  GEN-TABLE    $@"; $(QGENTABLE)

