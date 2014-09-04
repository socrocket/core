#include "testmod.h"

#include "leon3.h"
#include "cache.h"
#include "stdio.h"
#include "stdlib.h"

#define CCTRL_IFP (1<<15)
#define CCTRL_DFP (1<<14)

#define DDIAGMSK ((1<<DTAGLOW)-1)
#define IDIAGMSK ((1<<ITAGLOW)-1)

#define ICLOCK_BIT 6
#define DCLOCK_BIT 7


int icconf, dcconf, dsetsize, isetsize;
int dsetbits, isetbits;
int DSETS, DTAGLOW, DTAGAMSK, ITAGAMSK, ITAGLOW;

void flush(void) {
    asm(
        "flush"
    );
}
 
int getitag(volatile int addr, volatile int set) {
    volatile int tag;

    tag = asmgetitag((addr & IDIAGMSK) + (set<<isetbits));
    return tag;
}


void setitag(volatile int addr, volatile int set, volatile int data) {

    asmsetitag(((addr & IDIAGMSK) + (set<<isetbits)), data);
}

void setidata(volatile int addr, volatile int set, volatile int data) {
    asmsetidata(((addr & IDIAGMSK) + (set<<isetbits)), data);
}


int getidata(volatile int addr, volatile int set) {
    volatile int idata;
  
    idata = asmgetidata((addr & IDIAGMSK) + (set<<isetbits));
    return idata;
}


int asmgetitag(volatile int addr) {
    volatile int result;
    asm(
        "lda [%[addr]] 0xc, %[result]"
        : [result] "=r" (result)
        : [addr] "r" (addr)
    );
    return result;
}

void asmsetitag(volatile int addr, volatile int data) {
    asm(
        "sta %[data], [%[addr]] 0xc"
        :: [addr] "r" (addr), [data] "r" (data)
    );
}

int asmgetidata(volatile int addr) {
    volatile int result;
    asm(
        "lda [%[addr]] 0xd, %[result]"
        : [result] "=r" (result)
        : [addr] "r" (addr)
    );
    return result;
}

void asmsetidata(volatile int addr, volatile int data) {
    asm(
        "sta %[data], [%[addr]] 0xd"
        :: [addr] "r" (addr), [data] "r" (data)
    );
}

void wsysreg(volatile int addr, volatile int data) {
    asm(
        "sta %[data], [%[addr]] 0x2"
        :: [addr] "r" (addr), [data] "r" (data)
    );
}

int rsysreg(volatile int addr) {
    volatile int result;
    asm(
        "lda [%[addr]] 0x2, %[result]"
        : [result] "=r" (result)
        : [addr] "r" (addr)
    );
    return result;
}


void setdtag(volatile int addr, volatile int set, volatile int data) {
    asmsetdtag((addr & DDIAGMSK) + (set<<dsetbits), data);
}

void setddata(volatile int addr, volatile int set, volatile int data) {
    asmsetddata(((addr & DDIAGMSK) + (set<<dsetbits)), data);
}

// Returns true if tag is found in cache
int chkdtag(volatile int addr) {
    //int tm[16];
    int tmp, i;

    tmp = 0; 
    for(i=0; i<DSETS; i++) {

      // Increment counter tmp for each set that contains the tag of addr.
      if(((asmgetdtag((addr & DDIAGMSK) + (i<<dsetbits))) & DTAGAMSK) == (addr & DTAGAMSK)) {
        tmp++;
      }
    }

    if(tmp != 0) {
        return 0;
    } else {
        return 1;
    }
}  

int getdtag(volatile int addr, volatile int set) {
    volatile int tag;
  
    tag = asmgetdtag((addr & DDIAGMSK) + (set<<dsetbits));
    return tag;
}


int getddata(volatile int addr, volatile int set) {
    volatile int ddata;

    ddata = asmgetddata(((addr & DDIAGMSK) + (set<<dsetbits))); 
    return ddata;
}



void dma(int addr, int len,  int write) {
    volatile unsigned int *dm = (unsigned int *) 0xa0000000;

    dm[0] = addr;
    dm[1] = (write <<13) + 0x1000 + len;
}

int asmgetdtag(volatile int addr) {
    volatile int result;
    asm(
        "lda [%[addr]] 0xe, %[result]"
        : [result] "=r" (result)
        : [addr] "r" (addr)
    );
    return result;
}

void asmsetdtag(volatile int addr, volatile int data) {
    asm(
        "sta %[data], [%[addr]] 0xe"
        :: [addr] "r" (addr), [data] "r" (data)
    );
}

int asmgetddata(volatile int addr) { 
    volatile int result;
    asm(
        "lda [%[addr]] 0xf, %[result]"
        : [result] "=r" (result)
        : [addr] "r" (addr)
    );
    return result;
}

void asmsetddata(volatile int addr, volatile int data) {
    asm(
        "sta %[data], [%[addr]] 0xf"
        :: [addr] "r" (addr), [data] "r" (data)
    );
}

void setudata(volatile int addr,volatile int data) {
    asm(
        "sta %[data], [%[addr]] 0x0"
        :: [addr] "r" (addr), [data] "r" (data)
    );
}

int getudata(volatile int addr) {
    volatile int data;
    data = asmgetudata(addr);
    return data;  
}

int asmgetudata(volatile int addr) {
    volatile int result;
    asm(
        "lda [%[addr]] 0x0, %[result]"
        : [result] "=r" (result)
        : [addr] "r" (addr)
    );
    return result;
}

int xgetpsr(void) {
    int result;
    asm(
        "mov %%psr, %[result]"
        : [result] "=r" (result)
    );
    return result;
}

void setpsr(int psr) {
    asm(
        "mov %[psr], %%psr; nop; nop; nop"
        :: [psr] "r" (psr)
    );
}

void flushi(int addr, int data) {
    asm(
        "sta %[data], [%[addr]] 0x5"
        :: [addr] "r" (addr), [data] "r" (data)
    );
}

#define DIAGADDRMASK ((1<<DTAGLOW)-1)

static int maintest(void);

int cachetest(void) {
    int tmp;

    tmp = maintest();
    wsysreg(0, 0x81000f);
    return tmp;
}

long long int getdw(void *addr) {
    long long int *address = (long long int *)addr;
    return *address;
}

static int maintest() {
    volatile char data1[(8192 + 8) * sizeof(double)]; // or long long
    //volatile long long *mrl = (long long *)data1; /* enough for 64 K caches */
    volatile double *mrl = (double *)data1; /* enough for 64 K caches */
    volatile char data2[16 * sizeof(int)];
    volatile int *mrx = (int *)data2;
    //volatile long long *ll = (long long *)data2;
    volatile double *ll = (double *)data2;
    volatile int *mr = (int *) data1;
    volatile unsigned char *mrc = (unsigned char *) data1;
    volatile unsigned short *mrh = (unsigned  short *) data1;
    volatile long long int dw;
    
    //int vbits, vpos, addrmsk;
    int i, j, tmp, cachectrl; 
    int ITAGS, DTAGS;
    int ILINESZ, DLINESZ;
    int ITAG_BITS, ILINEBITS, DTAG_BITS, DLINEBITS;
    int IVALMSK, tag;
    //int data;
    int ISETS;
    int (*line[4])() = {line0, line1, line2, line3}; 

    report_subtest(CACHE_TEST);

    cachectrl = rsysreg(0);

    printf("Cache control register: %x\n", cachectrl);

    wsysreg(0, cachectrl & ~0x0f);
    
    do {
        cachectrl = rsysreg(0);
    } while(cachectrl & (CCTRL_IFP | CCTRL_DFP));
    flush();
    
    do {
        cachectrl = rsysreg(0); 
    } while(cachectrl & (CCTRL_IFP | CCTRL_DFP));
    
    cachectrl = rsysreg(0); 
    wsysreg(0, cachectrl | 0x81000f);

    icconf = rsysreg(8);
    dcconf = rsysreg(12);

    ILINEBITS = (icconf >> 16) & 7;
    DLINEBITS = ((dcconf >> 16) & 7);
    ITAG_BITS = ((icconf >> 20) & 15) + 8 - ILINEBITS;
    DTAG_BITS = ((dcconf >> 20) & 15) + 8 - DLINEBITS;
    isetsize = (1<<((icconf >> 20) & 15)) * 1024;
    dsetsize = (1<<((dcconf >> 20) & 15)) * 1024;
    isetbits = ((icconf >> 20) & 15) + 10;
    dsetbits = ((dcconf >> 20) & 15) + 10;
    ITAGS = (1 << ITAG_BITS);
    ILINESZ = (1 << ILINEBITS);
    DTAGS = (1 << DTAG_BITS);
    DLINESZ = (1 << DLINEBITS); 
    IVALMSK = (1 << ILINESZ)-1;
    ITAGAMSK = 0x7fffffff - (1 << (ITAG_BITS + ILINEBITS +2)) + 1;
    DTAGAMSK = 0x7fffffff - (1 << (DTAG_BITS + DLINEBITS +2)) + 1;
    ISETS = ((icconf >> 24) & 3) + 1;
    DSETS = ((dcconf >> 24) & 3) + 1;

    ITAGLOW = 10 + ((icconf >> 20) & 15);
    DTAGLOW = 10 + ((dcconf >> 20) & 15); 

    printf("Software Cache Info: \n");
    printf("-----------------------\n");
    printf("ilinebits: %x\n", ILINEBITS);
    printf("dlinebits: %x\n", DLINEBITS);
    printf("itag_bits: %x\n", ITAG_BITS);
    printf("dtag_bits: %x\n", DTAG_BITS);
    printf("isetsize: %x\n", isetsize);
    printf("dsetsize: %x\n", dsetsize);
    printf("isetbits: %x\n", isetbits);
    printf("dsetbits: %x\n", dsetbits);
    printf("itags: %x\n", ITAGS);
    printf("ilinesz: %x\n", ILINESZ);
    printf("dtags: %x\n", DTAGS);
    printf("dlinesz: %x\n", DLINESZ);
    printf("ivalmsk: %x\n", IVALMSK);
    printf("itagamsk: %x\n", ITAGAMSK);
    printf("dtagamsk: %x\n", DTAGAMSK);
    printf("isets: %x\n", ISETS);
    printf("dsets: %x\n", DSETS);
    printf("itaglow: %x\n", ITAGLOW);
    printf("dtaglow: %x\n", DTAGLOW);
    printf("idiagmask: %x\n", IDIAGMSK);
    printf("ddiagmask: %x\n", DDIAGMSK);
    
    /**** INSTRUCTION CACHE TESTS ****/

    cachectrl = rsysreg(0); 

    for(i=0;i<ISETS;i++) {
        line[i]();
    }
    
    wsysreg(0, cachectrl & ~0x03); /* disable icache */

    /* check tags */
    tmp = 0;
    for(i=0;i<ISETS;i++) { 
        for(j=0;j<ISETS;j++) {
	    tag = getitag((int) line[i], j);
	    printf("Addr: %x Tag: %x\n", (int)line[i], tag);
	    if(((tag & IVALMSK) == IVALMSK) && ((tag & ITAGAMSK) == (((int) line[i]) & ITAGAMSK))) {
                tmp++;
            }
        }
    }
    
    cachectrl = rsysreg(0); 
    wsysreg(0, cachectrl | 3); /* enable icache */
    
    if(tmp == 0) {
        fail(1);
    }

    if(((cachectrl >> ITE_BIT) & 3) == 0) {
 
        /* iparity checks */
        if((cachectrl >> CPP_CONF_BIT) & CPP_CONF_MASK) {
            cachectrl = rsysreg(0);
            wsysreg(0, cachectrl & ~0x3fc0);
            line2();
            wsysreg(0, cachectrl | CPTB_MASK);
            for(i=0;i<ISETS;i++) { 
                setidata((int) line2, i, 0);
            }
            line2();
            cachectrl = rsysreg(0);
            if(((cachectrl >> IDE_BIT) & 3) != 1) {
                fail(2);
            }
            
            setitag((int) line2, 0, 0);
            cachectrl = rsysreg(0);
            wsysreg(0, cachectrl & ~CPTB_MASK);
            //setidata((int) line2, 0, 0);
            line2();
            cachectrl = rsysreg(0);
            if(((cachectrl >> ITE_BIT) & 3) != 1) {
                fail(3);
            }
        }
	

        /**** DATA CACHE TESTS ****/

	// IFP - instruction flush pending (bit 15)
	// DFP - data flush pending (bit 14)
        flush();

        do {
            cachectrl = rsysreg(0); 
        } while(cachectrl & (CCTRL_IFP | CCTRL_DFP));

	//printf("Clear MR Tags\n");

	// Delete address mr from all cache sets
        for(i=0;i<DSETS;i++) {
            setdtag((int) mr, i, 0); /* clear tags */
        }
        
	//printf("Write through\n");

	// Initialize mr by writing zeros
        for(i=0;i<31;i++) {
            mr[i] = 0;
        }
        mr[0] = 5;
        mr[1] = 1;
        mr[2] = 2;
        mr[3] = 3;
	
        // Check that write does not allocate line (write through - no write allocate).
	// (Function returns true if any cache contains the tag of address).

	//printf("Check not buffered\n");

        if(chkdtag((int) mr) == 0) {
            fail(5);
        }

	//printf("Load MR[0] into dcache\n");

	// Load from address mr - check result (cache miss)	
        if(mr[0] != 5) {
            fail(6);
        }


	//printf("Check if MR[0] cached\n");

        // Address suppossed to be cached now.
	// (Returns true if address is in cache.)
        if(chkdtag((int) mr) != 0) {
            fail(7);
        }

        // Check that data is in cache */
        for(i=0;i<DSETS;i++) {

	  // Removes the data entries mr[0] and mr[1]
          setddata((int)mr, i, 0); 
	  setddata((int) &mr[1], i, 0);
        }
	
	// Bring the data back to cache e.g. mr[0]
        getudata((int) &mr[0]);
        getudata((int) &mr[8]); 
        getudata((int) &mr[16]);
        getudata((int) &mr[24]);

        tmp = 0;
        for(i=0;i<DSETS;i++) { 
 
           if(getddata((int) mr, i) == 5) {
                tmp++;
           }
        }

        if(tmp == 0) {
            fail(8);
        }

        *ll = mrl[0];
        if((mrx[0] != 5) || (mrx[1] != 1)) {
            fail(9);
        }
        
        tmp = 0;
        for(i=0;i<DSETS;i++) {
            if(getddata((int) &mr[1], i) == 1) {
                tmp++;
            }
        }
        if(tmp != 1) {
            fail(10);
        }
	
        /* dcache parity */ 							 
        if((cachectrl >> CPP_CONF_BIT) & CPP_CONF_MASK) {
            cachectrl = rsysreg(0);
            wsysreg(0, cachectrl & ~CE_CLEAR);
            setddata((int) &mrx[0], 0, 0);
            cachectrl = rsysreg(0);
            wsysreg(0, cachectrl | CPTB_MASK);
            for(i=0;i<DSETS;i++) {
                setddata((int)mrx, i, 5);
            }
            *((char *) mrx) = 1;
            if(mrx[0] != 0x01000005) {
                fail(11);
            }
            cachectrl = rsysreg(0);
            if(((cachectrl >> DDE_BIT) & 3) != 1) {
                fail(12);
            }
            cachectrl = rsysreg(0);
            wsysreg(0, cachectrl & ~CPTB_MASK);
            setddata((int)&mrx[0],0,0);
            cachectrl = rsysreg(0);
            wsysreg(0, cachectrl | CPTB_MASK);
            do {
              cachectrl = rsysreg(0);
            } while(!(cachectrl & CPTB_MASK));
            
            for (i=0;i<DSETS;i++) {
                setdtag((int)mrx,i,(1 << DLINESZ)-1);
            }
            wsysreg(0, cachectrl & ~CPTB_MASK);
            do {
                cachectrl = rsysreg(0);
            } while(cachectrl & CPTB_MASK);
            if(mrx[0] != 0x01000005) {
                fail(13);
            }
#if 0
            if(getddata(&mr[0],0) != 5) {
                fail(14);
            }
#endif
            cachectrl = rsysreg(0); 
            if(((cachectrl >> DTE_BIT) & 3) != 1) {
                fail(15);
            }
#if 0
            if((getdtag(mrx,1) & DTAGMASK) != (1 <<((((int) mrx)>>2)&(DLINESZ-1)))) {
                fail(16);
            }
#endif
            *((volatile long long int *) &dw) = 0x0000001100000055LL;
            cachectrl = rsysreg(0);
            wsysreg(0, (cachectrl | CPTB_MASK) & ~DDE_MASK);
            getdw((void*)&dw);
            for(i=0;i<DSETS;i++) {
                setddata(((int)&dw)+4,i,0x00000055);
            }
            if(getdw((void*)&dw) != 0x0000001100000055LL) {
                fail(16);
            }
            cachectrl = rsysreg(0); 
            if(((cachectrl >> DDE_BIT) & 3) != 1) {
                fail(16);
            }
            wsysreg(0, cachectrl & (~CE_CLEAR & ~CPTB_MASK));
        }

        /* check that tag is properly replaced */
        mr[0] = 5;
        mr[1] = 1;
        mr[2] = 2;
        mr[3] = 3;
        mr[DTAGS*DLINESZ] = 0xbbbbbbbb;

        /* check that tag is not evicted on write miss */
        if(chkdtag((int) mr) != 0) {
            fail(17);
        }

        /* check that write update memory ok */	
        if(mr[DTAGS*DLINESZ] != 0xbbbbbbbb) {
            fail(18);
        }


        /* check that valid bits have been reset */
#if 0
        if((getdtag(mr) & DTAGMASK) != (1 <<((((int) mr)>>2)&(DLINESZ-1)))) {
            fail(19);
        }
        tmp = 0;
        if((getdtag((int) mr & DIAGADDRMASK + i*dsetsize) & DTAGMASK) != (1 <<((((int) mr)>>2)&(DLINESZ-1)))) {
            tmp = 1;
        }
        if(tmp == 1) {
            fail(19);
        }
#endif

    }
    /* check partial word access */

    mr[8] = 0x01234567;
    mr[9] = 0x89abcdef;
    if(mrc[32] != 0x01) { fail(26); }
    if(mrc[33] != 0x23) { fail(27); }
    if(mrc[34] != 0x45) { fail(28); }
    if(mrc[35] != 0x67) { fail(29); }
    if(mrc[36] != 0x89) { fail(30); }
    if(mrc[37] != 0xab) { fail(31); }
    if(mrc[38] != 0xcd) { fail(32); }
    if(mrc[39] != 0xef) { fail(33); }
    if(mrh[16] != 0x0123) { fail(34); }
    if(mrh[17] != 0x4567) { fail(35); }
    if(mrh[18] != 0x89ab) { fail(36); }
    if(mrh[19] != 0xcdef) { fail(37); }
    mrc[32] = 0x30; if(mr[8] != 0x30234567) { fail(39); }
    mrc[33] = 0x31; if(mr[8] != 0x30314567) { fail(40); }
    mrc[34] = 0x32; if(mr[8] != 0x30313267) { fail(41); }
    mrc[35] = 0x33; if(mr[8] != 0x30313233) { fail(42); }
    mrc[36] = 0x34; if(mr[9] != 0x34abcdef) { fail(43); }
    mrc[37] = 0x35; if(mr[9] != 0x3435cdef) { fail(44); }
    mrc[38] = 0x36; if(mr[9] != 0x343536ef) { fail(45); }
    mrc[39] = 0x37; if(mr[9] != 0x34353637) { fail(46); }
    mrh[16] = 0x4041; if(mr[8] != 0x40413233) { fail(47); }
    mrh[17] = 0x4243; if(mr[8] != 0x40414243) { fail(48); }
    mrh[18] = 0x4445; if(mr[9] != 0x44453637) { fail(49); }
    mrh[19] = 0x4647; if(mr[9] != 0x44454647) { fail(50); }

#if 0
    if(((lr->leonconf >> 2) & 3) == 3) {
        dma((int)&mr[0], 9, 1);
    }
    if(((lr->leonconf >> 2) & 3) == 3) {
        dma((int)&mr[0], 9, 1);
    }
#endif

    /* write data to the memory */
    flush();
    for(i=0;i<DSETS;i++) { 
        for(j=0;j<DLINESZ;j++) {
            mr[j+(i<<dsetbits)] = ((i<<16) | j); 
        }
    } 
	
    /* check that write miss does not allocate line */
    do {
        cachectrl = rsysreg(0);
    } while(cachectrl & (CCTRL_DFP));
    for(i=0;i<DSETS;i++) {
      if((getdtag((int) mr, i) & DTAGAMSK) == ((int) mr & DTAGAMSK)) {
          fail(51);
      }
    }

    /* check flush operation */
    /* check that flush clears valid bits */
#if 0
    cachectrl = rsysreg(0);
    wsysreg(0, cachectrl & ~0x0f); 
    flushi();
    do {
        cachectrl = rsysreg(0);
    } while(cachectrl & (CCTRL_IFP));
	
    if(chkitags(ITAG_MAX_ADDRESS,(1<<(ILINEBITS + 2)),0,0) & ((1<<ILINESZ)-1)) {
       fail(51);
    }

    lr->cachectrl |= 0x03; 
    flushd();
    while(lr->cachectrl & CCTRL_DFP) {}
    
    if(chkdtags(DTAG_MAX_ADDRESS,(1<<(DLINEBITS + 2)),0,0) & ((1<<DLINESZ)-1)) {
       fail(52);
    }
#endif
	
#if 0
    if(chkitags(ITAG_MAX_ADDRESS,(1<<(ILINEBITS + 2)),0,0) & ((1<<ILINESZ)-1)) {
       fail(51);
    }

    lr->cachectrl |= 0x03; 
    flushd();
    while(lr->cachectrl & CCTRL_DFP) {}
    
    if(chkdtags(DTAG_MAX_ADDRESS,(1<<(DLINEBITS + 2)),0,0) & ((1<<DLINESZ)-1)) {
       fail(52);
    }
#endif
	
#if 0
    flush();
    setdtag(0,0,0x11111111);
    setdtag(0,1,0x22222222);
    setdtag(0,2,0x33333333);
    setdtag(0,3,0x44444444);
#endif
	
    cachectrl = rsysreg(0);
    wsysreg(0, cachectrl | 0xf); 
    return 0;

/* to be tested: diag access during flush, diag byte/halfword access,
   write error, cache freeze operation */
}

