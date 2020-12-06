#ifndef _FLUSHRLD_H_
#define _FLUSHRLD_H_

inline unsigned long long rdtsc() {
  unsigned long long a, d;
  asm volatile ("lfence");
  asm volatile ("rdtsc" : "=a" (a), "=d" (d));
  asm volatile ("lfence");
  a = (d << 32) | a;
  return a;
}

inline size_t flush(void* p) {
  asm volatile ( "mfence" );
  unsigned long long start = rdtsc();
  asm volatile ( "clflush (%0)" :: "r"( p ) );
  size_t delta = rdtsc() - start;
  return delta;
}

inline size_t reload(void* p)
{
  asm volatile ( "mfence" );
  unsigned long long start = rdtsc();
  asm volatile ("movq (%0), %%rax\n"
  :
  : "c" (p)
  : "%rax");
  size_t delta = rdtsc() - start;
  return delta;
}

inline void wait(size_t cycles) {
  asm volatile("mfence");
  unsigned long long start = rdtsc();
  size_t elapsed = 0;
  while(elapsed < cycles)
    elapsed = rdtsc() - start;
}

#endif /* _FLUSHRLD_H_ */
