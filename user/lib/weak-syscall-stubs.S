/*
 * Ensimag - Projet système
 * Copyright (C) 2012 - Damien Dejean <djod4556@yahoo.fr>
 *
 * Defines weak symbols to allow the linker create final binaries even if the
 * implementation does not exists. But these weak symbols will be overriden by
 * every implementation.
 */

#define WEAK_STUB(symbol)       .weak symbol; symbol:

WEAK_STUB(start)
WEAK_STUB(getpid)
WEAK_STUB(getprio)
WEAK_STUB(chprio)
WEAK_STUB(kill)
WEAK_STUB(waitpid)
WEAK_STUB(exit)
WEAK_STUB(cons_write)
WEAK_STUB(cons_read)
WEAK_STUB(cons_echo)
WEAK_STUB(scount)
WEAK_STUB(screate)
WEAK_STUB(sdelete)
WEAK_STUB(signal)
WEAK_STUB(signaln)
WEAK_STUB(sreset)
WEAK_STUB(try_wait)
WEAK_STUB(wait)
WEAK_STUB(pcount)
WEAK_STUB(pcreate)
WEAK_STUB(pdelete)
WEAK_STUB(preceive)
WEAK_STUB(preset)
WEAK_STUB(psend)
WEAK_STUB(clock_settings)
WEAK_STUB(current_clock)
WEAK_STUB(wait_clock)
WEAK_STUB(sys_info)
WEAK_STUB(shm_create)
WEAK_STUB(shm_acquire)
WEAK_STUB(shm_release)

.weak __do_a_fault
__do_a_fault:
    int $3
    ret
