#include <stdlib.h>
#include "testmod.h"

volatile unsigned char *oc_reg;

#define MOD  0
#define CMR 1
#define SR  2
#define IR  3
#define IER 4
#define RES 5
#define BTR0 6
#define BTR1 7
#define OCR 8
#define TST 9
#define RES2 10
#define ALC 11
#define ECC 12
#define EWL 13
#define RXERR 14
#define TXERR 15
#define FI 16
#define ID1 17
#define ID2 18
#define EFF_ID3 19
#define EFF_ID4 20
#define SFF_D1 19
#define SFF_D2 20
#define SFF_D3 21
#define SFF_D4 22
#define SFF_D5 23
#define SFF_D6 24
#define SFF_D7 25
#define SFF_D8 26
#define EFF_D1 21
#define EFF_D2 22
#define EFF_D3 23
#define EFF_D4 24
#define EFF_D5 25
#define EFF_D6 26
#define EFF_D7 27
#define EFF_D8 28
#define ACR0 16
#define ACR1 17
#define ACR2 18
#define ACR3 19
#define AMR0 20
#define AMR1 21
#define AMR2 22
#define AMR3 23
#define NMSG_FIFO 27
#define NMSG_FIFO2 28
#define RMC 29
#define RBSA 30
#define CDR 31
#define RXFIFO_START 32
#define TXBUF_START 96

/* define register bitmasks */
#define MOD_RM 0x1
#define MOD_LOM 0x2
#define MOD_STM 0x4
#define MOD_AFM 0x8

#define CMR_TR 0x1
#define CMR_AT 0x2
#define CMR_RRB 0x4
#define CMR_CDO 0x8
#define CMR_SRR 0x10

#define SR_RBS 0x1
#define SR_DOS 0x2
#define SR_TBS 0x4
#define SR_TCS 0x8
#define SR_RS  0x10
#define SR_TS  0x20
#define SR_ES  0x40
#define SR_BS  0x80

void reset_mode(void) {
  oc_reg[MOD] |= MOD_RM;

  while ( (oc_reg[MOD] & MOD_RM) != MOD_RM )
    {}

}
		
void operating_mode(void) {
  oc_reg[MOD] &= ~MOD_RM;

  while ( (oc_reg[MOD] & MOD_RM) != 0 )
    {}
}

void can_filter(int mode, unsigned char *acr, unsigned char *amr) {
  
  int i;

  if (mode)
    oc_reg[MOD] |= MOD_AFM;
  else
    oc_reg[MOD] &= ~MOD_AFM;

  for (i = 0; i < 4; i++) {
    oc_reg[ACR0+i] = acr[i];
    oc_reg[AMR0+i] = amr[i];
  }

}

void can_send_eff(int id, int rtr, int dlc, unsigned char *data) {

  int i;

  /* wait until current tx buffer is released */
  while ((oc_reg[SR] & SR_TBS) == 0)
    {}

  oc_reg[FI] = (1<<7) | ((rtr&1)<<6) | (dlc&0xF);
  
  oc_reg[ID1] = (id>>21)&0xFF;
  oc_reg[ID2] = (id>>13)&0xFF;
  oc_reg[EFF_ID3] = (id>>5)&0xFF;
  oc_reg[EFF_ID4] = (id&0x1F)<<3;

  for (i = 0; i < dlc && i < 8; i++)
    oc_reg[EFF_D1+i] = data[i];

  /* request transmit */
  oc_reg[CMR] = CMR_TR;

}

int can_tx_complete(void) {
  return oc_reg[SR] & SR_TCS;
}

int can_read_eff(int *id, int *rtr, unsigned char *data) {

  int dlc=-1, i, ff;

  /* wait for incoming data */
  while (!(oc_reg[SR] & SR_RBS))
    {
    }

  ff = (oc_reg[FI]>>7)&1;

  if (ff == 1) {

    *rtr = (oc_reg[FI]>>6)&1;
    dlc = oc_reg[FI]&0xF;

    *id = (oc_reg[ID1]<<21) | (oc_reg[ID2]<<13) | (oc_reg[EFF_ID3]<<5) |  ((oc_reg[EFF_ID4]>>3)&0x1F);

    if ( ((oc_reg[EFF_ID4]>>2)&1) != *rtr ) {
      puts("can_read(eff): rtr mismatch");
    }

    for (i = 0; i < dlc && data != NULL && i < 8; i++)
      data[i] = oc_reg[EFF_D1+i];
  }

  oc_reg[CMR] = CMR_RRB;
  
  return dlc;

}

void can_init(int fmode, unsigned char *acr, unsigned char *amr) {

  reset_mode();
  oc_reg[MOD] = 0x1;

  /* set extended can mode */
  oc_reg[CDR] = 0x80;

/*   /\* set 1 Mbps bit rate (if freq 40 MHz) *\/ */
/*   oc_reg[BTR0] = 0x81; /\* Sync jump width 2, BRP = 1 *\/ */
/*   oc_reg[BTR1] = 0x25; /\* Single sampling, TSEG1 = 5, TSEG2 = 2  *\/ */

 /* set 1 Mbps bit rate (if freq 48 MHz) */
  oc_reg[BTR0] = 0x01; 
  oc_reg[BTR1] = 0x27; 
  can_filter(fmode, acr, amr);

  operating_mode();

  oc_reg[IER] = 0xFF;

}


#define DATA 16*1024

#define INT_EN       1
#define FIFO         3
#define CMD0         6
#define CMD1         7
#define START_CTC    8
#define STOP_CTC     10
#define RAM_BASE     9
#define DPS_ACT      10
#define PLL_RST      11
#define PLL_CMD      12
#define PLL_STAT     13
#define PLL_OFF      14
#define DMA          15
#define DMA_TX_1_CUR 16
#define DMA_TX_1_END 17
#define DMA_TX_2_CUR 18
#define DMA_TX_2_END 19
#define RX           20
#define FILTER_SETUP 21
#define FILTER_START 20
#define FILTER_STOP  21

struct satcan_regs {

    volatile unsigned int satcan[32];
    volatile unsigned int ctrl;       /* 0x80 */
    volatile unsigned int irqpend;    /* 0x84 */
    volatile unsigned int irqmask;    /* 0x88 */
    volatile unsigned int membase;    /* 0x8C */
    volatile unsigned int unused[12]; /* 0x90-0xBC */
    volatile unsigned int timer1[4];  /* 0xC0 - 0xCC */
    volatile unsigned int timer2[4];  /* 0xD0 - 0xDC */
    volatile unsigned int timer3[4];  /* 0xE0 - 0xEC */
    volatile unsigned int timer4[4];  /* 0xF0 - 0xFC */
};

struct satcan_regs *r;

struct irq_reg {
  volatile unsigned int level;
  volatile unsigned int pending;
  volatile unsigned int force;
  volatile unsigned int clear;
  volatile unsigned int mpstat;
  volatile unsigned int dummy[11];
  volatile unsigned int mask;
};

struct irq_reg *irq = (struct irq_reg *) 0x80000200;

volatile unsigned char *mem;
volatile int oc_irq_done;
volatile int sc_irq_done;

volatile unsigned int irq_oc=0, irq_sc=0, s=0, d=0;

extern void *catch_interrupt(void func(), int irq);

enable_irq (int irqn) 
{
  irq->clear = (1 << irqn);	// clear any pending irq
  irq->mask |= (1 << irqn);	// unmask irq
}

disable_irq (int irqn) { 
  irq->mask &= ~(1 << irqn); 	// mask irq
}

force_irq (int irqn) { 
  irq->force = (1 << irqn); 	// force irq
}

void sc_irq_handler(int irqn)
{
    int i;
    unsigned int *p;

    irq_sc++;
    i = r->satcan[FIFO];
    p = (unsigned int *) &mem[(0x80+i)*8];
    i = r->irqpend;
    sc_irq_done = 1;
}

void oc_irq_handler(int irqn)
{
    int i, dlc, id, rtr;
    unsigned int *p;
    unsigned char data[8];

    i = oc_reg[IR];
    if (i) {
        if (i & 1) {

        dlc = can_read_eff(&id, &rtr, data);

        if (id == 0x20) s++;
        else if (id == 0x40) {
            d++;
            for (i=0; i < dlc; i++) {
                if (data[i] != (i+1))
                    printf("%d, %hhx\n", i, data[i]);
            }
            oc_irq_done = 1;
        }
        memset(data, 0, 8);
        dlc = 0;
        id = 0;

        irq_oc++;
     
        }
        else {
        }
        
    }
}

satcan_test(unsigned int sc_addr, int sc_irq, unsigned int oc_addr, int oc_irq, unsigned int mux_addr, unsigned int bus)
{
    unsigned char acr_eff[4] = {0x00, 0x00, 0x00, 0x00};
    unsigned char amr_eff[4] = {0xff, 0xff, 0xff, 0xff};

    unsigned long long val;

    oc_reg = (volatile unsigned char *) oc_addr;
    r      = (struct satcan_regs *) sc_addr;

    report_device(0x01080000);

    volatile unsigned int *can_mux = (unsigned int *) mux_addr;

    if (bus == 0)
        *can_mux = 2; /* SatCAN on bus A, OC-CAN2 on bus B */
    else
        *can_mux = 1; /* SatCAN on bus B, OC-CAN1 on bus A */

    oc_reg[CDR] = 0x80;
    can_init(1, acr_eff, amr_eff);

    mem = (unsigned char *) memalign(128*1024, 128*1024);

    r->ctrl |= 1;

    while (r->ctrl & 1) {};

    r->satcan[PLL_RST]    = 01;   /* Reset pll */

    r->satcan[PLL_CMD]    = 01;   /* Sync on PPS */
    r->satcan[CMD1]       = 0x31; /* Enable sync pulse and sync message */
    r->satcan[START_CTC]  = 1;    /* Start cycle time counter */

    irq->level = 0;
    catch_interrupt(sc_irq_handler, sc_irq);
    enable_irq(sc_irq); 
    catch_interrupt(oc_irq_handler, oc_irq);
    enable_irq(oc_irq);
   
    s = d = irq_sc = irq_oc = 0;
  
    r->membase = (unsigned int) mem;   
    r->satcan[RAM_BASE] = (unsigned int)mem>>15;
    r->irqmask = 0x0001;
    r->satcan[INT_EN] = 1 << 22; /* Enable EOD1 irq */ 
    r->satcan[CMD0] |= 0x20;     /* select can_int as irq source */
    r->satcan[RX] = 1;           /* CAN RX enable */ 


    /* Enable override */
    mem[0] = 0xE0;
    mem[1] = 0;
    mem[2] = 0x81;
    mem[3] = 0xFF;
    mem[DATA] = 15;

    if (bus == 0) {
        /* Enable CAN A */
        mem[8] = 0xE0;
        mem[9] = 0;
        mem[10] = 0x81;
        mem[11] = 0xFF;
        mem[DATA+8] = 4;
    }
    else {
        /* Enable CAN B */
        mem[8] = 0xE0;
        mem[9] = 0;
        mem[10] = 0x81;
        mem[11] = 0xFF;
        mem[DATA+8] = 5;
    }

    /* Enable TX */
    mem[16] = 0xE0;
    mem[17] = 0;
    mem[18] = 0x81;
    mem[19] = 0xFF;
    mem[DATA+16] = 6;
  
    sc_irq_done = 0;

    r->satcan[DMA_TX_1_CUR] = 0;
    r->satcan[DMA_TX_1_END] = 3<<3; 
    r->satcan[DMA] = 3;    /* enable dma  */
    
    while (!sc_irq_done) {
    };
    sc_irq_done = 0;

    while (s < 3) { /* Run until 3 sync messages (300 ms) have been received */

        r->satcan[DMA] = 0;  /* disable dma */

        mem[0] = 0x40;
        mem[1] = 0;
        mem[2] = 0x80 | 8;
        mem[3] = 0;

        mem[DATA+0] = 1;
        mem[DATA+1] = 2;
        mem[DATA+2] = 3;
        mem[DATA+3] = 4;
        mem[DATA+4] = 5;
        mem[DATA+5] = 6;
        mem[DATA+6] = 7;
        mem[DATA+7] = 8;

        oc_irq_done = 0;
     
        r->satcan[DMA_TX_1_CUR] = 0;
        r->satcan[DMA_TX_1_END] = 1<<3;
        r->satcan[DMA] = 3;                 /* Start DMA */

        while (oc_irq_done == 0) {
        };

    }  

    r->satcan[DMA] = 0;  /* disable dma */
}
