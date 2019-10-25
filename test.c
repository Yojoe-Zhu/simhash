#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "simhash.h"

#define DEBUG 1

static void thread_time_start(struct timespec *start)
{
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, start);
}

static void thread_time_stop(struct timespec *start, uint64_t count)
{
    struct timespec stop;
    float sec;

    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &stop);

    sec = (stop.tv_sec - start->tv_sec) + (stop.tv_nsec - start->tv_nsec) * 1e-9f;
    printf("Total: %f, Avg: %f MQPS\n", sec, count/sec/1000000);
}

static void loop_run(uint64_t loop, const char *str)
{
    int i;
    struct timespec start;

    printf("Performance test, LOOP: %llu\n", loop);

    thread_time_start(&start);
    for (i = 0; i < loop; i++) {
        asm volatile("");
        simhash(str);
    }
    thread_time_stop(&start, loop);

    printf("Test finished!\n");
}

static void print_bin(uint64_t i)
{
    uint64_t t = i;
    int c = 0;
    int m = sizeof(uint64_t) * 8;

    while (c < m) {
#if DEBUG
        printf("%llu%s", (t >> (m-1)) & 0x01, ((c & 0x03) == 0x03) ? "." : "");
#endif
        t = t << 1;
        c++;
    }
    printf("  %llu\n", i);
}

static void test_distance(uint64_t a, uint64_t b)
{
    print_bin(a);
    print_bin(b);
    printf("Distance: %d\n", sim_distance(a, b));
}

int main()
{
    test_distance(17831459094038722629U, 17831459094038722629U);
    test_distance(17831459094038722629U, 17831459094038722630U);
    test_distance(17831459094038722629U, 17831447584778722630U);

    loop_run(1000000, "This is a nice book");
    loop_run(1000000, "This is a bad man");

    return 0;
}

