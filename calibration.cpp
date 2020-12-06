#include <iostream>

#include "flushrld.h"

unsigned int MONITORED_MEMORY = 0;

int main() {
  size_t n = 100000;
  double s1, s2;

  for(size_t i = 0; i < n; i++) {
    flush(&MONITORED_MEMORY);
    size_t delta = reload(&MONITORED_MEMORY);
    s1 = s1 + (delta / (double)n);
  }

  for(size_t i = 0; i < n; i++) {
    size_t delta = reload(&MONITORED_MEMORY);
    s2 = s2 + (delta / (double)n);
  }

  double threshold = s2 + ((s1 - s2) / 2);

  std::cout << "cold reload: " << s1 << std::endl;
  std::cout << "reload: " << s2 << std::endl;
  std::cout << "purposed threshold: " << threshold << std::endl;
  
  return 0;
}
