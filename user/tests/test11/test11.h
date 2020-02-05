#ifndef _TEST11_H_
#define _TEST11_H_

#include "sysapi.h"

struct test11_shared {
        union sem sem;
        int in_mutex;
};

#endif
