#include <stdlib.h>
#include <stdio.h>
#include "cache.h"

typedef unsigned int uint32_t;
typedef unsigned char uint8_t;

int main(int argc, char *argv[]) {
	cache_disable();
  volatile uint32_t *ahb_pnp_master = (uint32_t *)0xFFFFF000;
  volatile uint32_t *ahb_pnp_slave =  (uint32_t *)0xFFFFF800;
  volatile uint32_t *apb_pnp_slave =  (uint32_t *)0x800FF000;
  volatile uint8_t *mctrl_sdram = (uint8_t *)0x400F000;
  volatile uint8_t *ahbmem = (uint8_t *)0xA0000000;
  uint32_t error = 0;
  uint32_t *test;
  uint32_t value;
  
  printf("To use this test you are adviced to use the standard singlecore lt platform\n");
  printf("This test reads and writes all memmorys. Uses MCtrl, AHBMem and PNP area over AHB & APB\n");
  printf("\n");
  
  printf("1.  Read MCFG1 MCtrl Register from APB:\n");
  uint32_t mctrl_mcfg1 = *((uint32_t *)0x80000000);
  printf("    Reading: 0x%08x\n", mctrl_mcfg1);
  if(mctrl_mcfg1 == 0x10280033) {
    printf("    Test successful\n");
  } else {
    printf("    Test failed\n");
    error |= 1 << 1;
  }
  printf("\n");
  
  printf("2.  DBG Read from MCtrl\n");
  printf("    Preparing memory (Write Bytes)\n");
  mctrl_sdram[0] = 'R';
  mctrl_sdram[1] = 'e';
  mctrl_sdram[2] = 'a';
  mctrl_sdram[3] = 'd';
  mctrl_sdram[4] = ' ';
  mctrl_sdram[5] = 'f';
  mctrl_sdram[6] = 'r';
  mctrl_sdram[7] = 'o';
  mctrl_sdram[8] = 'm';
  mctrl_sdram[9] = ' ';
  mctrl_sdram[10] = 'S';
  mctrl_sdram[11] = 'D';
  mctrl_sdram[12] = 'R';
  mctrl_sdram[13] = 'A';
  mctrl_sdram[14] = 'M';
  mctrl_sdram[15] = '\0';
  printf("    Reading:   '%s'\n", (char *)mctrl_sdram);
  printf("    Should be: 'Read from SDRAM'\n");
  printf("\n");
  
  printf("3.  DBG Read from AHBMem\n");
  printf("    Preparing memory (Write Bytes)\n");
  ahbmem[0] = 'R';
  ahbmem[1] = 'e';
  ahbmem[2] = 'a';
  ahbmem[3] = 'd';
  ahbmem[4] = ' ';
  ahbmem[5] = 'f';
  ahbmem[6] = 'r';
  ahbmem[7] = 'o';
  ahbmem[8] = 'm';
  ahbmem[9] = ' ';
  ahbmem[10] = 'A';
  ahbmem[11] = 'H';
  ahbmem[12] = 'B';
  ahbmem[13] = 'M';
  ahbmem[14] = 'e';
  ahbmem[15] = 'm';
  ahbmem[16] = '\0';
  printf("    Read:      '%s'\n", (char *)ahbmem);
  printf("    Should be: 'Read from AHBMem'\n");
  printf("\n");
  
  
  printf("4.  AHB Master PNP Read\n");
  printf("    Reading: 0x%08x\n", ahb_pnp_master[0]);
  if(ahb_pnp_master[0] == 0x01003000) {
    printf("    Test successful\n");
  } else {
    printf("    Test failed\n");
    error |= 1 << 4;
  }
  printf("\n");
  
  printf("5.  AHB Slave PNP Read\n");
  printf("    Reading: 0x%08x\n", ahb_pnp_slave[0]);
  if(ahb_pnp_slave[0] == 0x0400F000) {
    printf("    Test successful\n");
  } else {
    printf("    Test failed\n");
    error |= 1 << 5;
  }
  printf("\n");
  
  printf("6.  APB Slave PNP Read\n");
  printf("    Reading: 0x%08x\n", apb_pnp_slave[0]);
  if(apb_pnp_slave[0] == 0x0400F000) {
    printf("    Test successful\n");
  } else {
    printf("    Test failed\n");
    error |= 1 << 6;
  }
  printf("\n");
  
  printf("Now followed by some messy tests:\n");
  printf("The folowing tests will all produce ugly signs\n");
  printf("We test DBG transport of the PNP Areas!\n");
  printf("\n");

  printf("7.  AHB Master PNP DBG Read\n");
  printf("    Reading: '%s'\n", (char *)ahb_pnp_master);
  printf("    Please report bus errors\n");
  printf("\n");
  
  printf("8.  AHB Slave PNP DBG Read\n");
  printf("    Reading: '%s'\n", (char *)ahb_pnp_slave);
  printf("    Please report bus errors\n");
  printf("\n");
  
  printf("9.  APB Slave PNP DBG Read\n");
  printf("    Reading: '%s'\n", (char *)apb_pnp_slave);
  printf("    Please report bus errors\n");
  printf("\n");
  
  printf("The folowing tests will all produce errors\n");
  printf("\n");
  
  printf("10. AHB Master PNP Write\n");
  printf("    Writing: '0xDEADBEEF'\n");
  ahb_pnp_master[0] = 0xDEADBEEF;
  if(ahb_pnp_master[0] == 0x01003000) {
    printf("    Test successful\n");
  } else {
    printf("    Reading: 0x%08x\n", ahb_pnp_master[0]);
    printf("    Test failed\n");
    error |= 1 << 10;
  }
  printf("\n");
  
  printf("11. AHB Slave PNP Write\n");
  printf("    Writing: 0xDEADBEEF\n");
  ahb_pnp_slave[0] = 0xDEADBEEF;
  if(ahb_pnp_slave[0] == 0x0400F000) {
    printf("    Test successful\n");
  } else {
    printf("    Reading: 0x%0hx\n", ahb_pnp_slave[0]);
    printf("    Test failed\n");
    error |= 1 << 11;
  }
  printf("\n");
  
  printf("12. APB Slave PNP Write\n");
  printf("    Writing: 0xDEADBEEF\n");
  apb_pnp_slave[0] = 0xDEADBEEF;
  if(apb_pnp_slave[0] == 0x0400F000) {
    printf("    Test successful\n");
  } else {
    printf("    Reading: 0x%08x\n", apb_pnp_slave[0]);
    printf("    Test failed\n");
    error |= 1 << 12;
  }
  printf("\n");
  
  printf("13. Failing AHB Read\n");
  printf("    Accessing: 0xF0000000\n");
  test = (uint32_t *)0xF0000000;
  value = *test;
  printf("\n");
  
  printf("14. Failing APB Read\n");
  printf("    Accessing: 0x800F0000\n");
  test = (uint32_t *)0x800F0000;
  value = *test;
  printf("\n");
  
  printf("15. Failing AHB Master PNP Read\n");
  value = ahb_pnp_master[40];
  printf("\n");
  
  printf("16. Failing AHB Slave PNP Read\n");
  value = ahb_pnp_slave[40];
  printf("\n");
  
  printf("17. Failing APB Slave PNP Read\n");
  value = apb_pnp_slave[40];
  printf("\n");
  
  return error;
}
