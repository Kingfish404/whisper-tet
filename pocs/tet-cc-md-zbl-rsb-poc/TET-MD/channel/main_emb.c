#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>
#include "cacheutils.h"

#define FROM 'A'
#define TO 'Z'

#ifndef SIG_UNBLOCK
#define SIG_UNBLOCK 1
#endif

static jmp_buf jbuf;
unsigned char test_num, argmax;
uint64_t secret, len = 10, try_times = 20;
uint64_t start_time, spend_time, max_time;
int hist[256];
int ***count;

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
    signal(SIGSEGV, segfault_handler);
    count = (int ***)malloc(len * sizeof(int **));
    for (int i = 0; i < len; i++)
    {
        count[i] = (int **)malloc(try_times * sizeof(int *));
        for (int j = 0; j < try_times; j++)
        {
            count[i][j] = (int *)malloc(256 * sizeof(int));
            memset(count[i][j], 0, 256 * sizeof(int));
        }
    }

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
                // if (!setjmp(jbuf))
                if (xbegin() == (~0u))
                {
                    asm volatile(
                        "mov %[test_num], %%bl;"
                        "sub $83, %%bl;" // 83 = 0x53 = 'S', 88 = 0x58 = 'X'
                        "movq (%1), %%rax\n"
                        :
                        : [test_num] "r"(test_num), "c"(0)
                        :);
                }
                asm volatile(
                    "jz equal;"
                    "jmp notequal;"
                    "equal: nop;");
                printf("you can not see me\n");
                asm volatile(
                    "notequal: nop;");
                spend_time = rdtsc() - start_time;
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