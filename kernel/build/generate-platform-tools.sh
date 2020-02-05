#!/bin/bash
#
# Ensimag - Projet Système
# Copyright (C) 2014 - Damien Dejean <dam.dejean@gmail.com>
#

#
# Define a toolchain for MacOS and check it is available
#
OSX=`uname -a | grep -c '^Darwin'`
if [ ${OSX} = 1 ]; then
    TOOLCHAIN_PREFIX="../darwin-x86_64/bin/i686-linux-android-"
else
    TOOLCHAIN_PREFIX=""
fi

if [ -x ${TOOLCHAIN_PREFIX}gcc ]; then
    echo "TOOLCHAIN_PREFIX := ${TOOLCHAIN_PREFIX}"
else
    echo "TOOLCHAIN_PREFIX :="
fi


#
# Kernel cannot use the gold linker. On platforms where the both linkers exist
# (bfd and gold), explicitely choose the gold one.
#
BFD_LINKER=`which ${TOOLCHAIN_PREFIX}ld.bfd 2> /dev/null`
if [ -n "${BFD_LINKER}" -a -x "${BFD_LINKER}" ]; then
    echo 'LD := $(TOOLCHAIN_PREFIX)ld.bfd'
else
    echo 'LD := $(TOOLCHAIN_PREFIX)ld'
fi
