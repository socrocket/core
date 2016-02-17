#include "stdlib.h"
#include "stdio.h"

int main() {
  volatile unsigned int array[2048];
  int i;
  for (i = 0; i < 2048; i++) {
    array[i] = 0xdeadbeef;
  }

  printf("Wrote 0xdeadbeef 2048 times to memory.\n");
  fflush(stdout);
  return 0;
}

