/*
 * Ensimag - Projet système
 * Copyright (C) 2013 - Damien Dejean <dam.dejean@gmail.com>
 */

#include "sysapi.h"

// TODO: missing tests
const char *tests[] = {
        "test0",
        "test1",
        "test2",
        "test3",
        "test4",
        "test5",
        "test6",
        // "test7", // FIXME
        "test8",
        // "test9", // FIXME
        "test10",
        "test11",
        // "test12", // FIXME
        // "test13", // FIXME
        // "test14", // FIXME
        "test15",
        "test16",
        "test17",
        "test18",
        // "test19", // FIXME
        // "test20", // FIXME
        // "test21", // FIXME
        // "test22", // FIXME
};

#define TESTS_NUMBER ((int)(sizeof(tests) / sizeof(tests[0])))

int main(void)
{
        int i;
        int pid;
        int ret;

        for (i = 0; i < TESTS_NUMBER; i++) {
                printf("Test %s : ", tests[i]);
                pid = start(tests[i], 4000, 128, NULL);
                waitpid(pid, &ret);
                assert(ret == 0);
        }
}

