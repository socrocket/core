#include "stdlib.h"
#include "stdio.h"
#include "asm-leon/amba.h"

#include <asm-leon/leon.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*#define DEBUG_CONFIG*/

/* Structure containing address to devices found on the Amba Plug&Play bus */
extern amba_confarea_type amba_conf;

static void vendor_dev_string(unsigned long conf, char *vendorbuf, char *devbuf) {
  int vendor = amba_vendor(conf);
  int dev = amba_device(conf);
  char *devstr;
  char *vendorstr;
  sprintf(vendorbuf, "Unknown vendor %2x", vendor);
  sprintf(devbuf, "Unknown device %2x", dev);
  vendorstr = vendor_id2str(vendor);
  if(vendorstr) {
    sprintf(vendorbuf, "%s", vendorstr);
  }
  devstr = device_id2str(vendor, dev);
  if(devstr) {
    sprintf(devbuf, "%s", devstr);
  }
  vendorbuf[0] = 0;
  devbuf[0] = 0;
}

void amba_prinf_config();

void amba_prinf_config(void) {
  char devbuf[128];
  char vendorbuf[128];
  unsigned int conf;
  int i = 0;
  int j = 0;
  unsigned int addr;
  unsigned int m;
  printf("             Vendors         Slaves\n");
  printf("Ahb masters:\n");
  i = 0;
  while (i < amba_conf.ahbmst.devnr) {
    conf = amba_get_confword(amba_conf.ahbmst, i, 0);
    vendor_dev_string(conf, vendorbuf, devbuf);
    printf("%2i(%2x:%3x|%2i): %16s %16s \n", i, amba_vendor(conf),
           amba_device(conf), amba_irq(conf), vendorbuf, devbuf);
    for (j = 0; j < 4; j++) {
      m = amba_ahb_get_membar(amba_conf.ahbmst, i, j);
      if (m) {
        addr = amba_membar_start(m);
        printf(" +%i: 0x%x \n", j, addr);
      }
    }
    i++;
  }
  printf("Ahb slaves:\n");
  i = 0;
  while (i < amba_conf.ahbslv.devnr) {
    conf = amba_get_confword(amba_conf.ahbslv, i, 0);
    vendor_dev_string(conf, vendorbuf, devbuf);
    printf("%2i(%2x:%3x|%2i): %16s %16s \n", i, amba_vendor(conf),
           amba_device(conf), amba_irq(conf), vendorbuf, devbuf);
    for (j = 0; j < 4; j++) {
      m = amba_ahb_get_membar(amba_conf.ahbslv, i, j);
      if (m) {
        addr = amba_membar_start(m);
        if (amba_membar_type(m) == AMBA_TYPE_AHBIO) {
          addr = AMBA_TYPE_AHBIO_ADDR(addr);
        } else if (amba_membar_type(m) ==
             AMBA_TYPE_APBIO) {
          printf("Warning: apbio membar\n");
        }
        printf(" +%i: 0x%x (raw:0x%x)\n", j, addr, m);
      }
    }
    i++;
  }
  printf("Apb slaves:\n");
  i = 0;
  while (i < amba_conf.apbslv.devnr) {

    conf = amba_get_confword(amba_conf.apbslv, i, 0);
    vendor_dev_string(conf, vendorbuf, devbuf);
    printf("%2i(%2x:%3x|%2i): %16s %16s \n", i, amba_vendor(conf),
           amba_device(conf), amba_irq(conf), vendorbuf, devbuf);

    m = amba_apb_get_membar(amba_conf.apbslv, i);
    addr = amba_iobar_start(amba_conf.apbslv.apbmst[i], m);
    printf(" +%2i: 0x%x (raw:0x%x) \n", 0, addr, m);

    i++;
  }
}

int main() {
    amba_init();
	  printf("List of all devices\n");
    amba_prinf_config();
    fflush(stdout);
	  printf("List of all devices\n");
    return 0;
}

