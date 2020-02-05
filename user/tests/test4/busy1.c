#include "sysapi.h"
#include "test4.h"

int main(void *arg)
{
        (void)arg;
        while (1) {
                int i, j;

                printf(" A");
                for (i=0; i<loop_count1; i++) {
                        test_it();
                        for (j=0; j<loop_count0; j++);
                }
        }
        return 0;
}
