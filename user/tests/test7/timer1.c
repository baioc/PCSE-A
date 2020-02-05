#include "sysapi.h"

int main(void *arg)
{
        (void)arg;

        unsigned long quartz;
        unsigned long ticks;
        unsigned long dur;
        int i;

        clock_settings(&quartz, &ticks);
        dur = (quartz + ticks) / ticks;
        printf(" 2");
        for (i = 4; i < 8; i++) {
                wait_clock(current_clock() + dur);
                printf(" %d", i);
        }
        return 0;
}

