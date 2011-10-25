#include "cache.h"

void cache_disable(void) {
  asm(" sta %g0, [%g0] 2 ");
}

void cache_enable(void) {
  asm(" set 0x81000f, %o0; sta %o0, [%g0] 2 ");
}

void ramfill(void) {
    int dbytes, ibytes, isets, dsets; 
    int icconf, dcconf;
    //int cachectrl; 

    icconf = rsysreg(8);
    dcconf = rsysreg(12);

    isets = ((icconf >> 24) & 3) + 1;
    dsets = ((dcconf >> 24) & 3) + 1;
    ibytes = (1 << (((icconf >> 20) & 0xF) + 10)) * isets;
    dbytes = (1 << (((dcconf >> 20) & 0xF) + 10)) * dsets;
    cache_disable();
    ifill(ibytes);
    dfill(dbytes);
    flush();
    cache_enable();    
}
