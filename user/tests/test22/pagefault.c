/*******************************************************************************
 * Ensimag - Projet Systeme
 * Copyright 2013 - Damien Dejean <dam.dejean@gmail.com>
 *******************************************************************************/

#include "sysapi.h"
#include "test22.h"

int main(void *arg)
{
        (void)arg;
        /* Try to fault */
        *((int*)0) = 0;
        /* We should have been killed ... */
        return (int)MALICIOUS_SUCCESS;
}
