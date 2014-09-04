#include <stdlib.h>

//#define BRM_ADDR 0x80000c00; //0xfff00000
#define BRM_IRQ  14

struct brm_irq_reg {
  volatile unsigned int level;
  volatile unsigned int pending;
  volatile unsigned int force;
  volatile unsigned int clear;
  volatile unsigned int mpstat;
  volatile unsigned int dummy[11];
  volatile unsigned int mask;
};

struct brm_regs {

  /* BRM registers (16 bit) */
  volatile unsigned int ctrl;            /* 0x00 */
  volatile unsigned int oper;            /* 0x04 */
  volatile unsigned int cur_cmd;         /* 0x08 */
  volatile unsigned int imask;           /* 0x0C */
  volatile unsigned int ipend;           /* 0x10 */
  volatile unsigned int ipoint;          /* 0x14 */
  volatile unsigned int bit_reg;         /* 0x18 */
  volatile unsigned int ttag;            /* 0x1C */
  volatile unsigned int dpoint;          /* 0x20 */
  volatile unsigned int sw;              /* 0x24 */
  volatile unsigned int initcount;       /* 0x28 */
  volatile unsigned int mcpoint;         /* 0x2C */
  volatile unsigned int mdpoint;         /* 0x30 */
  volatile unsigned int mbc;             /* 0x34 */
  volatile unsigned int mfilta;          /* 0x38 */
  volatile unsigned int mfiltb;          /* 0x3C */
  volatile unsigned int rt_cmd_leg[16];  /* 0x40-0x7C */
  volatile unsigned int enhanced;        /* 0x80 */

  volatile unsigned int dummy[31];

  /* wrapper registers (32 bit) */
  volatile unsigned int w_ctrl;          /* 0x100 */
  volatile unsigned int w_irqctrl;       /* 0x104 */
  volatile unsigned int w_ahbaddr;       /* 0x108 */
};


//struct brm_irq_reg *irq = (struct brm_irq_reg *) 0x80000200;
//struct brm_regs *bc = (struct brm_regs *) 0x80001000;
struct brm_regs *bc;

volatile unsigned short *brm_bcmem = NULL;
volatile unsigned int *brm_bcmemi = NULL;

volatile int brm_done=0;
//volatile int bci=0;

/*extern void *catch_interrupt(void func(), int irq);

void irq_handler(int irqn)
{
  int tmp=0;

  if (irqn == BRM_IRQ) {
    irq->clear = (1 << irqn);	
    bci++;

    tmp = bc->ipend;

    if (tmp & 0x0020) {
      brm_done = 1;
    }
  }


}

error(char *message, int *count) {
  puts(message);
  *count++;
  return;
} 

static char *almalloc(int sz)
{
  char *tmp;  
  tmp = malloc(2*sz);
  tmp = (char *) ( ( (int)tmp+sz ) & ~(sz-1));
  return(tmp);
} */

/* creates a BC command block */
void brm_create_cmd(unsigned short *addr, unsigned short op, unsigned short cond, 
	unsigned short rtrt, unsigned short cw1, unsigned short cw2, 
	unsigned short dp, unsigned short bra, unsigned short time, int bus) {
  memset(addr, 0, sizeof(addr));
  addr[0] = (op << 12) | (bus << 9) | (rtrt << 8) | (cond << 1);
  addr[1] = cw1;
  addr[2] = cw2;
  addr[3] = dp;
  addr[6] = bra;
  addr[7] = time;
}

/* Create BC to RT command block */
void brm_bcrt(unsigned short *addr, unsigned short dp, unsigned short rtaddr, unsigned short sa, 
	unsigned short wc, int bus) {
  unsigned short cw = (rtaddr << 11) | (0 << 10) | (sa << 5) | (wc & 0x1F);
  brm_create_cmd(addr, 4, 0, 0, cw, 0, dp, 0, 0, bus);
}

/* Create RT to BC command block */
void brm_rtbc(unsigned short *addr, unsigned short dp, unsigned short rtaddr, 
	unsigned short sa, unsigned short wc, int bus) {
  unsigned short cw = (rtaddr << 11) | (1 << 10) | (sa << 5) | (wc & 0x1F);
  brm_create_cmd(addr, 4, 0, 0, cw, 0, dp, 0, 0, bus);
}

/*
asm (".data\n"
     ".global brm_bcmemx\n"
     "brm_bcmemx: .space 262144/4, 0\n"
     ".text\n"
     );
*/
void brm_sync(volatile unsigned int* gpio, int gpio_sync, int gpio_ack){
  *(gpio+1) = *(gpio+1)|(1<<gpio_sync); 
  while((*gpio&(1<<gpio_ack)) == 0);
  *(gpio+1) = (*(gpio+1))&~(1<<gpio_sync); 

}

int brm_1553_test(unsigned int brm_addr, unsigned int mem_addr, unsigned int gpio_addr, int gpio_sync, int gpio_ack)
{
  brm_done = 0; 
  int i, j, k;
  int ec = 0;
  int temp,tmp;
  volatile unsigned int brm_bcmemx[64*1024];
  volatile unsigned int *mem;
  volatile unsigned int *gpio = gpio_addr;


  report_device(0x01072000);

  // setup sync
  *(gpio+1) = (*(gpio+1))&~(1<<gpio_sync); 
  *(gpio+2) = *(gpio+2)|(1<<gpio_sync); 
  
  brm_sync(gpio, gpio_sync, gpio_ack);

  bc = (struct brm_regs *) brm_addr;
//  brm_bcmemi = (int *) ((int)brm_bcmemx & 0xfffe0000);
//  brm_bcmem = (unsigned short *) brm_bcmemi;

  brm_bcmem = (volatile unsigned short *) mem_addr; //almalloc(64*1024*2);
  mem = (volatile unsigned int *) mem_addr;
  
  if (brm_bcmem == NULL) {
    fail(1);
    //puts("Error allocating memory");
    //exit(1);
  }
  
  /* clear data mem */
  for (i = 0; i < 160; i++) {
    *(mem+i) = 0;
  }


//  irq->clear = 0xffff;
//  irq->level = 0;
//  catch_interrupt(irq_handler, BRM_IRQ);
 
//  enable_irq(BRM_IRQ);
  
  // BC to RT test
  report_subtest(1);

  bc->ctrl = 0x0010;     /* enable bcast  */
  bc->oper = 0x0000;     /* configure as BC */
  bc->imask = 0xffff;
  bc->ipoint = 0;        /* irq log list, not used */
  bc->dpoint = 0;        /* command block pointer (within 64x16b block) */
  bc->enhanced = 0x0003; /* freq = 24 */
  bc->w_ctrl = 1;
  bc->w_irqctrl = 0;
  bc->w_ahbaddr = (unsigned int) brm_bcmem;
 
  /* setting up data to send */
  for (i = 0; i < 32; i++) {
    brm_bcmem[0x80 + i] = (unsigned short) i;
  }
  
//  puts("Sending BC-RT messages.");
  //for (i = 0; i < 7; i++) {
  //  for (j = 0; j < 2; j++) { 
  //    bcrt((unsigned short *)&brm_bcmem[(i*10+j)*8], 0x8000, 1, i+1, 2, j&1); /* 32 words per message */
  //  }
  //}
  //brm_bcmem[2*8] = 0x0000;   /* End of list command */
  // BC-RT
  brm_bcmem[0] = (unsigned short) 0x4200;
  brm_bcmem[1] = (unsigned short) 0x0820;
  brm_bcmem[2] = (unsigned short) 0x0000;
  brm_bcmem[3] = (unsigned short) 0x0080;
  brm_bcmem[4] = (unsigned short) 0x0000;
  brm_bcmem[5] = (unsigned short) 0x0000;
  brm_bcmem[6] = (unsigned short) 0x0000;
  brm_bcmem[7] = (unsigned short) 0x0000;
  brm_bcmem[8] = (unsigned short) 0x4200;
  brm_bcmem[9] = (unsigned short) 0x0820;
  brm_bcmem[10] = (unsigned short) 0x0000;
  brm_bcmem[11] = (unsigned short) 0x0080;
  brm_bcmem[12] = (unsigned short) 0x0000;
  brm_bcmem[13] = (unsigned short) 0x0000;
  brm_bcmem[14] = (unsigned short) 0x0000;
  brm_bcmem[15] = (unsigned short) 0x0000;
  brm_bcmem[16] = (unsigned short) 0x0000;
  brm_bcmem[17] = (unsigned short) 0x0000;
 

  /* start operation */
  bc->ctrl |= 0x8000;

  do{
    tmp = bc->ipend;
  }while((tmp & 0x0020) == 0);

  /* stop operation */
  bc->ctrl |= 0x0010;
  bc->dpoint = 0;        /* command block pointer (within 64x16b block) */

  // RT to BC test
  report_subtest(2);

//  puts("Sending RT-BC messages.");
  //for (j = 0; j < 2; j++) { 
  //  rtbc((unsigned short *)&brm_bcmem[(i*10+j)*8], 0x10000, 1, i+1, 2, j&1); /* 32 words per message */
  //}
  //brm_bcmem[2*8] = 0x0000;   /* End of list command */
  // RT-BC
  brm_bcmem[0] = (unsigned short) 0x4200;
  brm_bcmem[1] = (unsigned short) 0x0C20;
  brm_bcmem[2] = (unsigned short) 0x0000;
  brm_bcmem[3] = (unsigned short) 0x0100;
  brm_bcmem[4] = (unsigned short) 0x0000;
  brm_bcmem[5] = (unsigned short) 0x0000;
  brm_bcmem[6] = (unsigned short) 0x0000;
  brm_bcmem[7] = (unsigned short) 0x0000;
  brm_bcmem[8] = (unsigned short)  0x4200;
  brm_bcmem[9] = (unsigned short)  0x0C20;
  brm_bcmem[10] = (unsigned short) 0x0000;
  brm_bcmem[11] = (unsigned short) 0x0120;
  brm_bcmem[12] = (unsigned short) 0x0000;
  brm_bcmem[13] = (unsigned short) 0x0000;
  brm_bcmem[14] = (unsigned short) 0x0000;
  brm_bcmem[15] = (unsigned short) 0x0000;
  brm_bcmem[16] = (unsigned short) 0x0000;
  brm_bcmem[17] = (unsigned short) 0x0000;

  /* start operation */
  bc->ctrl |= 0x8000;

  do{
    tmp = bc->ipend;
  }while((tmp & 0x0020) == 0);

  /* Check data to send */
  for (i = 0; i < 32; i++) {
    if( brm_bcmem[0x80 + i] != (unsigned short) i) fail(1);
  }
  
  /* Check data to send */
  for (i = 0; i < 64; i++) {
    if( brm_bcmem[0x100 + i] != (unsigned short) (i%0x20)) fail(2);
  }
  
  /* stop operation */
  bc->ctrl |= 0x0010;
  bc->dpoint = 0;        /* command block pointer (within 64x16b block) */

  // RT test
  report_subtest(3);

  bc->ctrl = 0x1910;     /* enable bcast  */
  bc->oper = 0x0900;     /* configure as BC */
  bc->imask = 0xED80;
  bc->ipoint = 0;        /* irq log list, not used */
  bc->dpoint = 0;        /* command block pointer (within 64x16b block) */
  bc->enhanced = 0x0003; /* freq = 24 */
  bc->w_ctrl = 1;
  bc->w_irqctrl = 6;
  bc->w_ahbaddr = (unsigned int) brm_bcmem;
  
  /* RT descriptors */
  // RX
  brm_bcmem[0] = (unsigned short) 0x00E0;
  brm_bcmem[1] = (unsigned short) 0x0200;
  brm_bcmem[2] = (unsigned short) 0x0200;
  brm_bcmem[3] = (unsigned short) 0x0310;
  brm_bcmem[4] = (unsigned short) 0x00E0;
  brm_bcmem[5] = (unsigned short) 0x0332;
  brm_bcmem[6] = (unsigned short) 0x0332;
  brm_bcmem[7] = (unsigned short) 0x0442;

  // TX
  brm_bcmem[0x80+0] = (unsigned short) 0x0060;
  brm_bcmem[0x80+1] = (unsigned short) 0x0200;
  brm_bcmem[0x80+2] = (unsigned short) 0x0200;
  brm_bcmem[0x80+3] = (unsigned short) 0x0310;
  brm_bcmem[0x80+4] = (unsigned short) 0x0060;
  brm_bcmem[0x80+5] = (unsigned short) 0x0332;
  brm_bcmem[0x80+6] = (unsigned short) 0x0332;
  brm_bcmem[0x80+7] = (unsigned short) 0x0442;

  /* start operation */
  bc->ctrl |= 0x8000;

  
  brm_sync(gpio, gpio_sync, gpio_ack);
  
  // wait on done
  //do{
  //  tmp = ((*(volatile unsigned int*)gpio_addr)&1<<gpio);
  //}while(tmp == 0);

  report_subtest(4);

  brm_sync(gpio, gpio_sync, gpio_ack);
  

//  printf("End of list");
  return 0;
  
}
