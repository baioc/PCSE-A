/**
 * Wrapper sur les structures de verrouillage disponibles dans le kernel:
 * les files de messages, ou les sémaphores.
 */


#include "sysapi.h"

#if defined WITH_SEM
/** Implémentation de verrous en se basant sur les sémaphores. */
void xwait(union sem *s)
{
        assert(wait(s->sem) == 0);
}

void xsignal(union sem *s)
{
        assert(signal(s->sem) == 0);
}

void xscreate(union sem *s)
{
        assert((s->sem = screate(0)) >= 0);
}

void xsdelete(union sem *s)
{
        assert(sdelete(s->sem) == 0);
}


#elif defined WITH_MSG
/** Implémentation de verrous en se basant sur les files de message. */
void xwait(union sem *s)
{
        assert(preceive(s->fid, 0) == 0);
}

void xsignal(union sem *s)
{
        int count;
        assert(psend(s->fid, 1) == 0);
        assert(pcount(s->fid, &count) == 0);
        //assert(count == 1); XXX
        assert(count < 2);
}

void xscreate(union sem *s)
{
        assert((s->fid = pcreate(2)) >= 0);
}

void xsdelete(union sem *s)
{
        assert(pdelete(s->fid) == 0);
}


#else
# error "WITH_SEM" ou "WITH_MSG" doit être définie.
#endif
