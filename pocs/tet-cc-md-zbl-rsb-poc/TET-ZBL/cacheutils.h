#ifndef _CACHEUTILS_H_
#define _CACHEUTILS_H_

#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/utsname.h>
#include <stdlib.h>
#include <fcntl.h>
#include <memory.h>
#include <sys/mman.h>
#ifdef _MSC_VER
#include <intrin.h> /* for rdtscp and clflush */
#pragma optimize("gt", on)
#else
#include <x86intrin.h> /* for rdtscp and clflush */
#endif

/* ============================================================
 *                    User configuration
 * ============================================================ */
size_t CACHE_MISS = 150;

#define USE_RDTSC_BEGIN_END     0

#define USE_RDTSCP              1

/* ============================================================
 *                  User configuration End
 * ============================================================ */

// ---------------------------------------------------------------------------
// uint64_t rdtsc() {
//   uint64_t a, d;
//   asm volatile("mfence");
// #if USE_RDTSCP
//   asm volatile("rdtscp" : "=a"(a), "=d"(d) :: "rcx");
// #else
//   asm volatile("rdtsc" : "=a"(a), "=d"(d));
// #endif
//   a = (d << 32) | a;
//   asm volatile("mfence");
//   return a;
// }
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

// ---------------------------------------------------------------------------
void flush(void *p) { asm volatile("clflush 0(%0)\n" : : "c"(p) : "rax"); }

// ---------------------------------------------------------------------------
void maccess(void *p) { asm volatile("movq (%0), %%rax\n" : : "c"(p) : "rax"); }

// ---------------------------------------------------------------------------
void mfence() { asm volatile("mfence"); }

// ---------------------------------------------------------------------------
int flush_reload(void *ptr) {
  uint64_t start = 0, end = 0;

#if USE_RDTSC_BEGIN_END
  start = rdtsc_begin();
#else
  start = __rdtsc();
  mfence();
#endif
  maccess(ptr);
#if USE_RDTSC_BEGIN_END
  end = rdtsc_end();
#else
  mfence();
  end = __rdtsc();
#endif

  mfence();

  flush(ptr);

  if (end - start < CACHE_MISS) {
    return 1;
  }
  return 0;
}

// ---------------------------------------------------------------------------
int flush_reload_t(void *ptr) {
  uint64_t start = 0, end = 0;

#if USE_RDTSC_BEGIN_END
  start = rdtsc_begin();
#else
  start = rdtsc();
#endif
  maccess(ptr);
#if USE_RDTSC_BEGIN_END
  end = rdtsc_end();
#else
  end = rdtsc();
#endif

  mfence();

  flush(ptr);

  return (int)(end - start);
}

// ---------------------------------------------------------------------------
int reload_t(void *ptr) {
  uint64_t start = 0, end = 0;

#if USE_RDTSC_BEGIN_END
  start = rdtsc_begin();
#else
  start = rdtsc();
#endif
  maccess(ptr);
#if USE_RDTSC_BEGIN_END
  end = rdtsc_end();
#else
  end = rdtsc();
#endif

  mfence();

  return (int)(end - start);
}


// ---------------------------------------------------------------------------
size_t detect_flush_reload_threshold() {
  size_t reload_time = 0, flush_reload_time = 0, i, count = 1000000;
  size_t dummy[16];
  size_t *ptr = dummy + 8;

  maccess(ptr);
  for (i = 0; i < count; i++) {
    reload_time += reload_t(ptr);
  }
  for (i = 0; i < count; i++) {
    flush_reload_time += flush_reload_t(ptr);
  }
  reload_time /= count;
  flush_reload_time /= count;

  return (flush_reload_time + reload_time * 2) / 3;
}


// ---------------------------------------------------------------------------
size_t get_physical_address(size_t vaddr) {
    int fd = open("/proc/self/pagemap", O_RDONLY);
    uint64_t virtual_addr = (uint64_t)vaddr;
    size_t value = 0;
    off_t offset = (virtual_addr / 4096) * sizeof(value);
    int got = pread(fd, &value, sizeof(value), offset);
    if(got != sizeof(value)) {
        return 0;
    }
    close(fd);
    return (value << 12) | ((size_t)vaddr & 0xFFFULL);
}

// ---------------------------------------------------------------------------
size_t get_direct_physical_map() {
    struct utsname buf;
    uname(&buf);
    int major = atoi(strtok(buf.release, "."));
    int minor = atoi(strtok(NULL, "."));
    
    if((major == 4 && minor < 19) || major < 4) {
        return 0xffff880000000000ull;
    } else {
        return 0xffff888000000000ull;
    }
}


// ---------------------------------------------------------------------------
static jmp_buf trycatch_buf;

// ---------------------------------------------------------------------------
void unblock_signal(int signum __attribute__((__unused__))) {
  sigset_t sigs;
  sigemptyset(&sigs);
  sigaddset(&sigs, signum);
  sigprocmask(SIG_UNBLOCK, &sigs, NULL);
}

// ---------------------------------------------------------------------------
void trycatch_segfault_handler(int signum) {
  (void)signum;
  unblock_signal(SIGSEGV);
  unblock_signal(SIGFPE);
  longjmp(trycatch_buf, 1);
}


// ---------------------------------------------------------------------------

#include <cpuid.h>
// ---------------------------------------------------------------------------
// unsigned int xbegin() {
//   unsigned status;
//   asm volatile(".byte 0xc7,0xf8,0x00,0x00,0x00,0x00" : "=a"(status) : "a"(-1UL) : "memory");
//   return status;
// }
static __attribute__((always_inline)) inline unsigned int xbegin(void)
{
    int ret = (~0u);
    asm volatile(".byte 0xc7,0xf8 ; .long 0"
                 : "+a"(ret)::"memory");
    return ret;
}

// ---------------------------------------------------------------------------
void xend() {
  asm volatile(".byte 0x0f; .byte 0x01; .byte 0xd5" ::: "memory");
}

// ---------------------------------------------------------------------------
int has_tsx() {
  if (__get_cpuid_max(0, NULL) >= 7) {
    unsigned a, b, c, d;
    __cpuid_count(7, 0, a, b, c, d);
    return (b & (1 << 11)) ? 1 : 0;
  } else {
    return 0;
  }
}

#endif
