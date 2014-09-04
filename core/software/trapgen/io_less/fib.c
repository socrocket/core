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
    #ifdef TSIM_DISABLE_CACHE
    /*Now I can disable the caches*/
    #ifdef LEON3_CACHE
    asm("sta %g0, [%g0] 2");
    #else
    asm("sethi %hi(0xfd800000), %g1");
    asm("or %g1,%lo(0xfd800000),%g1");
    asm("sethi %hi(0x80000014), %g2");
    asm("or %g2,%lo(0x80000014),%g2");
    asm("st %g1, [%g2]");
    #endif
    #endif

 unsigned long IMax;
 volatile unsigned long value;

 #ifdef SHORT_BENCH
 IMax = 20;//IMax = 40;
 #else
 IMax = 30;//IMax = 40;
 #endif

 value = fib(IMax);

  return 0;
}
