#ifndef _FLUSHRLD_H_
#define _FLUSHRLD_H_

inline unsigned long long rdtsc() {
  unsigned long long a, d;
  asm volatile ("mfence");
  asm volatile ("rdtsc" : "=a" (a), "=d" (d));
  a = (d << 32) | a;
  asm volatile ("mfence");
  return a;
}

inline size_t flush(void* p) {
  unsigned long long time = rdtsc();
  asm volatile ( "clflush (%0)" :: "r"( p ) );
  size_t delta = rdtsc() - time;
  return delta;
}

inline size_t reload(void* p)
{
  unsigned long long time = rdtsc();
  asm volatile ("movq (%0), %%rax\n"
  :
  : "c" (p)
  : "rax");
  size_t delta = rdtsc() - time;
  return delta;
}

#endif /* _FLUSHRLD_H_ */
