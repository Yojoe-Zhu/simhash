#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "simhash.h"

#define DEBUG 1

static void thread_time_start(struct timespec *start)
{
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, start);
}

static void thread_time_stop(struct timespec *start, uint64_t count, unsigned int len)
{
    struct timespec stop;
    float sec;

    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &stop);

    sec = (stop.tv_sec - start->tv_sec) + (stop.tv_nsec - start->tv_nsec) * 1e-9f;
    printf("Total: %f, Avg: %f MQPS, %f MBPS\n",
        sec, count/sec/1000000, count/sec/1000000*len);
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
    thread_time_stop(&start, loop, strlen(str));

    printf("== Test finished! ==\n");
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
    printf("-- Distance: %d --\n", sim_distance(a, b));
}

int main()
{
    const char *a, *b;

    a = "The lady who has long hair is nice, She helps me a lot, But why does she looks so antry. I think she needs help.";
    b = "The lady who has long hair is kind, She helps me a lot, But why does she looks so antry. I think she needs help.";
    test_distance(simhash(a), simhash(b));
    loop_run(500000, a);
    loop_run(500000, b);
	printf("\n");

    a = "The man who has short hair is nice,";
    b = "The man who has long hair is dangrous,";
    test_distance(simhash(a), simhash(b));
    loop_run(500000, a);
    loop_run(500000, b);
	printf("\n");

    a = "There're a lot of students in the classroom.";
    b = "But they're doing theirs homework, so it very quite.";
    test_distance(simhash(a), simhash(b));
    loop_run(500000, a);
    loop_run(500000, b);
	printf("\n");

    return 0;
}

