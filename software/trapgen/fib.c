
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

unsigned long fib(long);

unsigned long fib(x)
long x;
{
 if (x > 2)
  return(fib(x-1)+fib(x-2));
 else
  return(1);
}


int main()
{
 register unsigned long IMax,value;

 #ifdef SHORT_BENCH
 IMax = 20;//IMax = 40;
 #else
 IMax = 30;//IMax = 40;
 #endif

 printf("\n");
 printf("Fibonacci Benchmark\n");

 value = fib(IMax);

 printf("\n");
 printf("The %02ld'th Fibonacci Number is: %ld\n",IMax,value);
  return 0;
}
