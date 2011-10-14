#include <stdlib.h>

extern void *catch_interrupt(void func(), int irq);
volatile int *lreg_rt = (int *) 0x80000000;

#define ICLEAR 0x20c
#define IMASK  0x240
#define IFORCE 0x208

/* RT registers */
volatile int *rt_stat = (int *) 0x80000d00;
volatile int *rt_ctrl = (int *) 0x80000d04;
volatile int *rt_vword = (int *) 0x80000d08;
volatile int *rt_intvect = (int *) 0x80000d0c;
volatile int *rt_addrreg = (int *) 0x80000d10;
volatile int *rt_irq = (int *) 0x80000d14;

volatile unsigned short *rtmem;
volatile int rt_done;

unsigned int logbuf[1024];
unsigned int log_i=0;

enable_irq_rt (int irq) 
{
    lreg_rt[ICLEAR/4] = (1 << irq);
    lreg_rt[IMASK/4] |= (1 << irq);
}
disable_irq_rt (int irq) { lreg_rt[IMASK/4] &= ~(1 << irq); }
force_irq_rt (int irq) { lreg_rt[IFORCE/4] = (1 << irq); }

void irqhandler_rt(int irq)
{
  
  int i, tmp, intvect, rx, suba, tsw, cmdval;
  
  if (irq == 13) {
      
      tmp = *rt_intvect;

      intvect = tmp & 0x7f;
      cmdval  = tmp >> 7;

      if (intvect & 0x40) {

          rx = !((intvect >> 5) & 1);
          suba = intvect & 0x1f;
          tsw = rtmem[suba];

          logbuf[(log_i++)%1024] = intvect;
          logbuf[(log_i++)%1024] = cmdval;
          logbuf[(log_i++)%1024] = 32+(suba-1)*32;
          logbuf[(log_i++)%1024] = 1056+(suba-1)*32;

          if (rx) {
              /* Put the received words in the tx subaddress */
              if ((cmdval & 0x1f) == 0){
                for (i = 0; i < 32; i++) {
                  rtmem[1056+(suba-1)*32+i] = rtmem[32+(suba-1)*32+i]; 
                }
              }else{
                for (i = 0; i < (cmdval & 0x1f); i++) {
                  rtmem[1056+(suba-1)*32+i] = rtmem[32+(suba-1)*32+i]; 
                }
              }
          }
          
      }
      else {
          puts("Bad block received!");
      }
      

      lreg_rt[ICLEAR/4] = (1 << irq);
  }
}

error_rt(char *message, int *count) {
  //puts(message);
  *count++;
  return;
}

void sync(volatile unsigned int* gpio, int gpio_sync, int gpio_ack){
  *(gpio+1) = *(gpio+1)|(1<<gpio_sync); 
  while((*gpio&(1<<gpio_ack)) == 0);
  *(gpio+1) = (*(gpio+1))&~(1<<gpio_sync); 
}


int rt_1553_test(unsigned int brm_addr, unsigned int mem_addr, unsigned int gpio_addr, int gpio_sync, int gpio_ack)
{

  rt_done = 0; 
  int i = 0;
  int ec = 0;
  int temp;
  volatile unsigned int *gpio = gpio_addr;
  volatile unsigned int *mem;

  report_device(0x01071000);

  // setup sync
  *(gpio+1) = (*(gpio+1))&~(1<<gpio_sync); 
  *(gpio+2) = *(gpio+2)|(1<<gpio_sync); 
  
  if (mem_addr == 0){
    rtmem = (unsigned short *) memalign(2048*sizeof(short), 2048*sizeof(short));
  }else{
    rtmem = (unsigned short *) mem_addr;
  }
  if (rtmem == NULL) {
    fail(1);
    //puts("Error allocating memory");
    //exit(1);
  }

  mem = (unsigned int*)rtmem;

  //wash
  for (i=0; i<32; i++){
    *(mem + i) = 0;
    *(mem + 512 + i) = 0;
  }
  
  catch_interrupt(irqhandler_rt, 13);
  
  lreg_rt[ICLEAR/4] = 0xffff;  /* clear all pending */
  lreg_rt[0x200/4] = 0x0;      /* set all irq levels to 0 */
  
  enable_irq_rt(13);
  
  // RT test
  report_subtest(1);

  *rt_addrreg = (int) rtmem;
  if (*rt_addrreg != (int) rtmem) error_rt("RT address register error.", &ec);

  *rt_irq = 0x10000;    /* Enable irq */
  if (*rt_irq != 0x10000) fail(1); 

  *rt_ctrl = 0xc41D0;    /* set clkspd=24 MHz, sa30loop=1, writetsw=1, bcasten=1, rtaddr=1 and intack high */
  if (*rt_ctrl != 0xc41D0) fail(2); /*puts("RT ctrl register error.");*/


  /*printf("Log: %x\n", logbuf);*/

  /*while (1) {}; */
  /* RT ready*/
  sync(gpio, gpio_sync, gpio_ack);
  
  /* Wait on done*/
  sync(gpio, gpio_sync, gpio_ack);


  return 0;
  
}
