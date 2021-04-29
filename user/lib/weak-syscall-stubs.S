/*
 * Ensimag - Projet système
 * Copyright (C) 2012 - Damien Dejean <djod4556@yahoo.fr>
 *
 * Defines weak symbols to allow the linker create final binaries even if the
 * implementation does not exists. But these weak symbols will be overriden by
 * every implementation.
 */

#define WEAK_STUB(symbol)       .weak symbol; symbol:

/* TODO: implement missing system subroutines */
WEAK_STUB(sys_info)

.weak __do_a_fault
__do_a_fault:
    int $3
    ret
