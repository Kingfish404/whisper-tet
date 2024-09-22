#include <stdio.h>
#include <pthread.h>
#include "cacheutils.h"

int main(int argc, char const *argv[])
{
    int try_times = 100000, flag = 1;
    uint64_t timer[4];
    while (1)
    {
        timer[0] = rdtsc();
        timer[1] = rdtsc();
        for (int i = 0; i < try_times; i++)
        {
            for (int test_num = 0; test_num < 16; test_num++)
            {
                if (xbegin() == (~0u))
                {
                    asm volatile(
                        "mov %[test_num], %%ebx\n"
                        "movq (%1), %%rax\n"
                        :
                        : [test_num] "r"(test_num), "c"(0)
                        :);
                }
            }
        }
        timer[2] = rdtsc();

        if (flag == 0)
        {
            double t1 = timer[1] - timer[0];
            double t2 = timer[2] - timer[1];
            printf("t1 = %.3lf, t2 = %.3lf\t", t1, t2);
            double div = (t2 / 100000000 - 4) * 100;
            div -= 150;
            div *= 4;
            printf("(%2.3lf) ", div);
            for (int j = 0; j < div; j++)
            {
                printf("#");
            }
            printf("\n");
            flag = 1;
        }
        else
        {
            flag--;
        }
    }
    return 0;
}
