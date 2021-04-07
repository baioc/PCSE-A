#ifndef _TEST11_H_
#define _TEST11_H_

union sem {
    int fid;
    int sem;
};

struct test11_shared {
        union sem sem;
        int in_mutex;
};

#endif
