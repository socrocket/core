
#include "testmod.h"
#include "cache.h"
#include <stdlib.h>
#include <stdio.h>

#define CCTRL_IFP (1<<15)
#define CCTRL_DFP (1<<14)

extern int ddramtest1( int, volatile double*, int );
extern int ddramtest2( int, volatile double*, int );
extern int dtramtest( int, int, int, int, int );
extern int idramtest( int, int );
extern int itramtest( int, int, int, int );

int cramtest() {
	volatile double mrl[8*1024 + 8];
//	int i; 
	int ilinesz, dlinesz, dbytes, ibytes, itmask, dtmask, isets, dsets; 
	int icconf, dcconf, cachectrl;

	flush();
	cache_enable();
	icconf = rsysreg(8);
	dcconf = rsysreg(12);

	report_subtest(DDAT_TEST);

	isets = ((icconf >> 24) & 3) + 1;
	ilinesz = 1 << (((icconf >> 16) & 7) + 2);
	ibytes = (1 << (((icconf >> 20) & 15) + 10)) * isets;
	itmask = (ilinesz - 1) | (0x80000000 - ibytes);
	dsets = ((dcconf >> 24) & 3) + 1;
	dlinesz = 1 << (((dcconf >> 16) & 7) + 2);
	dbytes = (1 << (((dcconf >> 20) & 15) + 10)) * dsets;
	dtmask = (dlinesz - 1) | (0x80000000 - dbytes);

  printf("isets: %u\n", isets);
  printf("ilinesize: %u\n", ilinesz);
  printf("ibytes: %u\n", ibytes);
  printf("itmask: %u\n", itmask);
  printf("dsets: %u\n", dsets);
  printf("dlinesize: %u\n", dlinesz);
  printf("dbytes: %u\n", dbytes);
  printf("dtmask: %u\n", dtmask);

	do {
    cachectrl = rsysreg(0); 
  } while(cachectrl & (CCTRL_IFP | CCTRL_DFP));

	/* dcache data ram */
	if(ddramtest1(dbytes, mrl,0x55555555)) {
    fail(1);
  } else {
    printf("success\n");
  }
	if(ddramtest2(dbytes, mrl,0xaaaaaaaa)) {
    fail(2);
  } else {
    printf("success\n");
  }

	report_subtest(DTAG_TEST);
	cache_disable();
  printf("success\n");

	/* dcache tag ram */
	if(dtramtest(dbytes, (0xaaaaaa00 & dtmask), dtmask, dlinesz, 0xaaaaaaaa)) {
    fail(3);
  } else {
    printf("success\n");
  }
	if(dtramtest(dbytes, (0x55555500 & dtmask), dtmask, dlinesz, 0x55555555)) {
    fail(4);
  } else {
    printf("success\n");
  }

	/* icache data ram */
	report_subtest(IDAT_TEST);
	if(idramtest(ibytes, 0x55555555)) {
    fail(5);
  }
	if(idramtest(ibytes, 0xaaaaaaaa)) {
    fail(6);
  }

	/* icache tag ram */
	report_subtest(ITAG_TEST);
	if(itramtest(ibytes, itmask, ilinesz, 0xaaaaaaaa)) {
    fail(7);
  }
	if(itramtest(ibytes, itmask, ilinesz, 0x55555555)) {
    fail(8);
  }

	flush();
	cache_enable();
	return 0;
}

