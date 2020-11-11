#ifndef _FLUSHRLD_H_
#define _FLUSHRLD_H_

#include <utility>

inline void reload(void* p)
{
    /*asm volatile ("movq (%0), %%rax\n"
    :
    : "c" (p)
    : "rax");*/
}

inline void flush(void* p) {
    __asm clflush p;
    //asm volatile ( "clflush (%0)" :: "r"( p ) );
}

#endif /* _FLUSHRLD_H_ */
