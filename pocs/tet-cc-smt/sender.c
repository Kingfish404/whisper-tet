#include <stdio.h>
#include <pthread.h>
#include "cacheutils.h"

int secret[] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1};

int main(int argc, char const *argv[])
{
    int offset = 0;
    int try_times = 10000, flag = 1, mark = secret[offset];
    uint64_t timer[4];
    while (1)
    {
        timer[0] = rdtsc();
        if (mark == 0)
        {
            for (int i = 0; i < try_times; i++)
            {
                for (int test_num = 0; test_num < 256; test_num++)
                {
                    // NOP
                    asm volatile(
                        ".rept 50\n"
                        "nop\n"
                        ".endr\n");
                }
            }
        }

        timer[1] = rdtsc();
        for (int i = 0; i < try_times; i++)
        {
            for (int test_num = 0; test_num < 16; test_num++)
            {
                if (mark == 1)
                {
                    // MIS PREDICT
                    if (xbegin() == (~0u))
                    {
                        asm volatile(
                            "mov %[test_num], %%ebx\n"
                            "movq (%1), %%rax\n"
                            "sub $12, %%bl\n"
                            "jz equal\n"
                            "nop\n"
                            "jmp notequal\n"
                            :
                            : [test_num] "r"(test_num), "c"(0)
                            :);
                    }

                    asm volatile(
                        "equal: nop\n"
                        "notequal: nop\n");
                }
            }
        }
        timer[2] = rdtsc();
        if (flag == 0)
        {
            printf("t1: %ld, t2: %ld\t send: %d\n",
                   timer[1] - timer[0],
                   timer[2] - timer[1],
                   mark);
            flag = 8;
            offset++;
            if (offset > sizeof(secret) / sizeof(int))
            {
                offset = 0;
            }
            mark = secret[offset];
        }
        else
        {
            flag--;
        }
    }
    return 0;
}
