#include "stdlib.h"
#include "stdio.h"

volatile unsigned long long int counter;

int main() {
  counter = 0;
  printf("Loop forever\n");
  while(1) {
    counter++;
    if(!(counter%1000)) {
      printf(".");
      fflush(stdout);
    }
  }
  return 0;
}

