/*
 * Simple loopback test for SPICTRL 
 *
 * Copyright (c) 2008 Gaisler Research AB
 * Copyright (c) 2009 Aeroflex Gaisler AB
 *
 * This test requires that the SPISEL input is HIGH
 *
 */

#include "testmod.h"

/* Register offsets */
#define SPIC_CAP_OFF    0x00
#define SPIC_MODE_OFF   0x20

/* Register fields */
/* Capability register */
#define SPIC_SSSZ   24
#define SPIC_TWEN   (1 << 19)
#define SPIC_AMODE  (1 << 18)
#define SPIC_ASELA  (1 << 17)
#define SPIC_SSEN   (1 << 16)
#define SPIC_FDEPTH 8
#define SPIC_REVI   0
/* Mode register */
#define SPIC_AMEN   (1 << 31)
#define SPIC_LOOP   (1 << 30)
#define SPIC_CPOL   (1 << 29)
#define SPIC_CPHA   (1 << 28)
#define SPIC_DIV16  (1 << 27)
#define SPIC_REV    (1 << 26)
#define SPIC_MS     (1 << 25)
#define SPIC_EN     (1 << 24)
#define SPIC_LEN    20
#define SPIC_PM     16
#define SPIC_TW     (1 << 15)
#define SPIC_ASEL   (1 << 14)
#define SPIC_FACT   (1 << 13)
#define SPIC_OD     (1 << 12)
#define SPIC_CG     7
#define SPIC_ASDEL  5
/* Event and Mask registers */
#define SPIC_LT     (1 << 14)
#define SPIC_OV     (1 << 12)
#define SPIC_UN     (1 << 11)
#define SPIC_MME    (1 << 10)
#define SPIC_NE     (1 << 9)
#define SPIC_NF     (1 << 8)
/* Command register */
#define SPIC_LST    (1 << 22)
/* AM Configuration register */
#define SPIC_SEQ    (1 << 5)
#define SPIC_STRICT (1 << 4)
#define SPIC_OVTB   (1 << 3)
#define SPIC_OVDB   (1 << 2)
#define SPIC_ACT    (1 << 1)
#define SPIC_EACT   (1 << 0)

/* Reset values */
#define MODE_RESVAL  0
#define EVENT_RESVAL 0
#define MASK_RESVAL  0
#define CMD_RESVAL   0
#define TD_RESVAL    0

struct spictrlregs {
  volatile unsigned int mode;
  volatile unsigned int event;
  volatile unsigned int mask;
  volatile unsigned int com;
  volatile unsigned int td;
  volatile unsigned int rd;
  volatile unsigned int slvsel;
  volatile unsigned int aslvsel;
  volatile unsigned int amcfg;
  volatile unsigned int amper;
};

/*
 * spictrl_test(int addr)
 *
 * Writes fifo depth + 1 words in loopback mode. Writes
 * one more word and checks LT and OV status
 *
 * Tests automated transfers if the core has support
 * for them.
 *
 */
int spictrl_test(int addr)
{
  int i;
  int data;
  int fd;
  
  volatile unsigned int *capreg;
  struct spictrlregs *regs;
  
  report_device(0x0102D000);

  capreg = (int*)addr;
  regs = (struct spictrlregs*)(addr + SPIC_MODE_OFF);

  report_subtest(1);

  /*
   * Check register reset values
   */
  if (regs->mode != MODE_RESVAL)
    fail(0);
  if (regs->event != EVENT_RESVAL)
    fail(1);
  if (regs->mask != MASK_RESVAL)
    fail(2);
  if (regs->com != CMD_RESVAL)
    fail(3);
  if (regs->td != TD_RESVAL)
    fail(4);
  /* RD register is not reset and therefore not read */

  report_subtest(2);

  /* 
   * Configure core in loopback and write FIFO depth + 1
   * words
   */
  fd = (*capreg >> SPIC_FDEPTH) & 0xff;

  regs->mode = SPIC_LOOP | SPIC_MS | SPIC_EN;

  /* Check event bits */
  if (regs->event & SPIC_LT)
    fail(5);
  if (regs->event & SPIC_OV)
    fail(6);
  if (regs->event & SPIC_UN)
    fail(7);
  if (regs->event & SPIC_MME)
    fail(8);
  if (regs->event & SPIC_NE)
    fail(9);
  if (!(regs->event & SPIC_NF))
    fail(10);
     
  data = 0xaaaaaaaa;
  for (i = 0; i <= fd; i++) {
    regs->td = data;
    data = ~data;
  }
  
  /* Multiple master error */
  if (regs->event & SPIC_MME) 
    fail(11);

  /* Wait for first word to be transferred */
  while (!(regs->event & SPIC_NF))
    ;

  if (!(regs->event & SPIC_NE))
    fail(12);

  /* Write one more word to trigger overflow, set LST */
  regs->td = data;
  regs->com = SPIC_LST;

  while (!(regs->event & SPIC_LT))
    ;

  if (!(regs->event & SPIC_OV))
    fail(13);

  /* Verify that words transferred correctly */
  data = 0xaaaaaaaa;
  for (i = 0; i <= fd; i++) {
    if (regs->rd != data)
      fail(14+i);
    data = ~data;
  }
    
  /* Deactivate core */
  regs->mode = 0;

  /* Return if core does not support automated transfers */
  if (!(*capreg & SPIC_AMODE))
     return 0;

  /* AM Loopback test */
  report_subtest(3);

  /* Enable core with automated transfers */
  regs->mode = SPIC_AMEN | SPIC_FACT | SPIC_LOOP | SPIC_MS | SPIC_EN;

  /* Write two words to transmit FIFO */
  data = 0xdeadf00d;
  for (i = 0; i < 2; i++) {
    regs->td = data;
    data = ~data;
  }
  
  /* Set AM period register */
  regs->amper = 0;

  /* Enable automated transfers */
  regs->amcfg = SPIC_ACT;
  
  /* Wait for NE event */
  while (!(regs->event & SPIC_NE))
     ;
  
  /* Read out data */
  data = 0xdeadf00d;
  for (i = 0; i < 2; i++) {
    if (regs->rd != data)
      fail(15+fd+i);
    data = ~data;
  }
  
  /* Deactivate automated transfers */
  regs->amcfg = 0;
  
  /* Wait for automated mode to be deactivated */
  while (!(regs->amcfg & SPIC_ACT))
     ;
  
  /* Deactivate core */
  regs->mode = 0;

  return 0;
}
