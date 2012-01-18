#include <stdlib.h>
#include <stdio.h>

typedef unsigned int uint32_t;
typedef unsigned char uint8_t;

int main(int argc, char *argv[]) {
  uint32_t *ahb_pnp_master = (uint32_t *)0xFF000000;
  uint32_t *ahb_pnp_slave = (uint32_t *)0xFF000000;
  uint32_t *apb_pnp_slave = (uint32_t *)0x800FF000;

  // AHB Master 0 MMUCache
  // AHB Slave 0 MCtrl
  // AHB Slave 1 AHBMem ?
  // APB Slave 1 MCtrl
  uint8_t *mctrl_sdram = (uint8_t *)0x400F000;
  uint8_t *ahbmem = (uint8_t *)0xA0000000;
  
  uint32_t mctrl_mcfg1 = *((uint32_t *)0x80000000);
  printf("1. Read MCFG1: 0x%08x\n", mctrl_mcfg1);
  
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
  mctrl_sdram[15] = '\n';
  mctrl_sdram[16] = '\0';
  
  ahbmem[0] = 'R';
  ahbmem[0] = 'e';
  ahbmem[0] = 'a';
  ahbmem[0] = 'd';
  ahbmem[0] = ' ';
  ahbmem[0] = 'f';
  ahbmem[0] = 'r';
  ahbmem[0] = 'o';
  ahbmem[0] = 'm';
  ahbmem[0] = ' ';
  ahbmem[0] = 'A';
  ahbmem[0] = 'H';
  ahbmem[0] = 'B';
  ahbmem[0] = 'M';
  ahbmem[0] = 'e';
  ahbmem[0] = 'm';
  ahbmem[0] = '\n';
  ahbmem[0] = '\0';
  
  printf("2. Test MCtrl DBG Read: %s\n", (char *)mctrl_sdram);
  printf("3. Test APBMem DBG Read: %s\n", (char *)ahbmem);
  printf("4. Test AHB Master PNP Read 0x%08hx\n", ahb_pnp_master[0]);
  printf("4. Test AHB Slave PNP Read 0x%08x\n", ahb_pnp_slave[0]);
  printf("4. Test APB Slave PNP Read 0x%08x\n", apb_pnp_slave[0]);
  // Try to read from all
  // Try to write a text in the AHBMem/MCtrl
  // Printf the text from AHBMem/MCtrl
  // Try to write into ahb_pnp and apb_pnp
  // Try to printf pnp mems
  
  return 0;
}
