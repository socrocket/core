/*
 * System test software for GRPWM
 *
 * Copyright (c) 2009 Aeroflex Gaisler AB
 *
 * NOTES:
 *   - This software is mainly intended for simulation. If the
 *     subtests are modified then pwm_check.vhd in
 *     lib/gaisler/sim must be modified if it should be used to
 *     verify the output.
 *   - This software places the following restrictions on the
 *     GRPWM hardware:
 *       - pbits generic must be larger than 7
 *       - If wavepwm is set, then wdepth must be larger than
 *         31 and wbits must be larger than 7 (but not larger
 *         than pbits.
 *       - dbbits generic must be larger than 3
 *   - This software does not test:
 *       - more than one scaler
 *       - dead band scalers
 *       - dual compare mode
 *       - interrupts
 *       - polarity bit
 *       - fixed value bits
 *       - PWM generation without pair bit set
 *       - no update bit
 *       - waveform sync output
*/

#include "testmod.h"

#define PWM_REG_OFFSET 0x20
#define PWM_REG_SPACE 0x10
#define WRAM_OFFSET 0x8000

/* Core control register */
#define CTRL_EN_BIT 0
#define CTRL_EN (1 << CTRL_EN_BIT)
#define CTRL_SCALERSEL_BIT 8
#define CTRL_SCALERSEL (7 << CTRL_SCALERSEL_BIT)
#define CTRL_NOUP_BIT 12
#define CTRL_NOUP (7 << CTRL_NOUP_BIT)

/* Core capability register 1 */
#define CAP_NPWM_BIT 0
#define CAP_NPWM (7 << CAP_NPWM_BIT)
#define CAP_PBITS_BIT 3
#define CAP_PBITS (0x1F << CAP_PBITS_BIT)
#define CAP_SBITS_BIT 8
#define CAP_SBITS (0x1F << CAP_SBITS_BIT)
#define CAP_NSCALERS_BIT 13
#define CAP_NSCALERS (7 << CAP_NSCALERS_BIT)
#define CAP_DBBITS_BIT 16
#define CAP_DBBITS (0x1F << CAP_DBBITS_BIT)
#define CAP_DBSCALER_BIT 21
#define CAP_DBSCALER (1 << CAP_DBSCALER_BIT)
#define CAP_ASYMPWM_BIT 22
#define CAP_ASYMPWM (1 << CAP_ASYMPWM_BIT)
#define CAP_SYMPWM_BIT 23
#define CAP_SYMPWM (1 << CAP_SYMPWM_BIT)
#define CAP_SEPIRQ_BIT 25
#define CAP_SEPIRQ (3 << CAP_SEPIRQ_BIT)
#define CAP_DCMODE_BIT 27
#define CAP_DCMODE (1 << CAP_DCMODE_BIT)

/* Core capability register 2 */
#define CAP_WAVEPWM_BIT 0
#define CAP_WAVEPWM (1 << CAP_WAVEPWM_BIT)
#define CAP_WDBITS_BIT 1
#define CAP_WDBITS (0x1F << CAP_WDBITS_BIT)
#define CAP_WABITS_BIT 6
#define CAP_WABITS (0xF << CAP_WABITS_BIT)
#define CAP_WSYNC_BIT 10
#define CAP_WSYNC (1 << CAP_WSYNC_BIT)

/* Waveform config register */
#define WCONF_WSTOPADDR_BIT 0
#define WCONF_WSTOPADDR (0xF << WCONF_STOPADDR_BIT)
#define WCONF_WSYNCCOMP_BIT 16
#define WCONF_WSYNCCOMP (0x1FFF << WCONF_WSYNCCOMP_BIT)
#define WCONF_WSYNCEN_BIT 29
#define WCONF_WSYNCEN (1 << WCONF_WSYNCEN_BIT)

/* PWM control register */
#define PWM_CTRL_EN_BIT 0
#define PWM_CTRL_EN (1 << PWM_CTRL_EN_BIT)
#define PWM_CTRL_POL_BIT 1
#define PWM_CTRL_POL (1 << PWM_CTRL_POL_BIT)
#define PWM_CTRL_PAIR_BIT 2
#define PWM_CTRL_PAIR (1 << PWM_CTRL_PAIR_BIT)
#define PWM_CTRL_FIX_BIT 3
#define PWM_CTRL_FIX (7 << PWM_CTRL_FIX_BIT)
#define PWM_CTRL_METH_BIT 6
#define PWM_CTRL_METH (1 << PWM_CTRL_METH_BIT)
#define PWM_CTRL_DCOMP_BIT 8
#define PWM_CTRL_DCOMP (1 << PWM_CTRL_DCOMP_BIT)
#define PWM_CTRL_WEN_BIT 9
#define PWM_CTRL_WEN (1 << PWM_CTRL_WEN_BIT)
#define PWM_CTRL_SCALESEL_BIT 10
#define PWM_CTRL_SCALESEL (7 << PWM_CTRL_SCALESEL_BIT)
#define PWM_CTRL_IRQEN_BIT 13
#define PWM_CTRL_IRQEN (1 << PWM_CTRL_IRQEN_BIT)
#define PWM_CTRL_IRQTYPE_BIT 14
#define PWM_CTRL_IRQTYPE (1 << PWM_CTRL_IRQTYPE_BIT)
#define PWM_CTRL_IRQSCALE_BIT 15
#define PWM_CTRL_IRQSCALE (0x3F << PWM_CTRL_IRQSCALE_BIT)
#define PWM_CTRL_DBEN_BIT 21
#define PWM_CTRL_DBEN (1 << PWM_CTRL_DBEN_BIT)
#define PWM_CTRL_DBSCALER_BIT 22
#define PWM_CTRL_DBSCALER (0xF << PWM_CTRL_DBSCALER_BIT)

/* The address that the core will wrap during Waveform 
   PWM tests */
#define WSTOPADDR 31

/* Choose which subtests to run */
#define SUBTEST1 1 /* Asymmetric */
#define SUBTEST2 1 /* Symmetric */
#define SUBTEST3 1 /* Waveform (asymmetric) */
#define SUBTEST4 1 /* Waveform (symmetric) */

struct grpwm_core_regs {
  volatile unsigned int ctrl;
  volatile unsigned int scaler;
  volatile unsigned int irq;
  volatile unsigned int cap1;
  volatile unsigned int cap2;
  volatile unsigned int wconf;
};

struct grpwm_pwm_regs {
  volatile unsigned int per;
  volatile unsigned int comp;
  volatile unsigned int db;
  volatile unsigned int ctrl;
};

struct grpwm_caps {
  int npwm;
/*   int pbits; */
/*   int sbits; */
/*   int nscalers; */
/*   int dbbits; */
/*   int dbscaler; */
  int asympwm;
  int sympwm;
/*   int sepirq; */
/*   int dcmode; */
  int wavepwm;
/*   int wbits; */
/*   int wdepth; */
/*   int wsync; */
};

int grpwm_test(int addr) {
  
  int i;
  volatile int tmp;

  struct grpwm_core_regs *cregs;
  struct grpwm_pwm_regs *pregs[8];
  volatile unsigned int *wram;
  struct grpwm_caps caps;

  report_device(0x0104A000);

  cregs = (struct grpwm_core_regs*)addr;

  caps.npwm = ((cregs->cap1 & CAP_NPWM) >> CAP_NPWM_BIT) + 1 ;
  caps.asympwm = cregs->cap1 & CAP_ASYMPWM;
  caps.sympwm = cregs->cap1 & CAP_SYMPWM;
  caps.wavepwm = cregs->cap2 & CAP_WAVEPWM;
/*   caps.pbits = ((cregs->cap1 & CAP_PBITS) >> CAP_PBITS_BIT) + 1 ; */
/*   caps.sbits = ((cregs->cap1 & CAP_SBITS) >> CAP_SBITS_BIT) + 1 ; */
/*   caps.nscalers = ((cregs->cap1 & CAP_NSCALERS) >> CAP_NSCALERS_BIT) + 1 ; */
/*   caps.dbbits = ((cregs->cap1 & CAP_DBBITS) >> CAP_DBBITS_BIT) + 1 ; */
/*   caps.dbscaler = cregs->cap1 & CAP_DBSCALER; */
/*   caps.sepirq = (cregs->cap1 & CAP_SEPIRQ) >> CAP_SEPIRQ_BIT; */
/*   caps.dcmode = cregs->cap1 & CAP_DCMODE; */
/*   if(caps.wavepwm) { */
/*     caps.wbits = ((cregs->cap2 & CAP_WDBITS) >> CAP_WDBITS_BIT) + 1; */
/*     caps.wdepth = 2^(((cregs->cap2 & CAP_WABITS) >> CAP_WABITS_BIT) + 1); */
/*     caps.wsync = cregs->cap2 & CAP_WSYNC; */
/*   } */

  for(i = 0; i < caps.npwm; i++)
    pregs[i] = (struct grpwm_pwm_regs*)(addr + PWM_REG_OFFSET+PWM_REG_SPACE*i);

  wram = (unsigned int*)(addr + WRAM_OFFSET);

  /* Report number of pwm outputs to pwm_check */
  report_subtest(255-caps.npwm);

  /* Set scaler to tick every clock cycle */
  cregs->scaler = 0;

  /* Start core */
  cregs->ctrl = CTRL_EN;

  /* Test asymmetric */
  if(caps.asympwm && SUBTEST1) {
    report_subtest(1);
    for(i = 0; i < caps.npwm; i++) {   
      /* Set period, compare and dead band register */
      pregs[i]->per = 200+i;
      pregs[i]->comp = 100+i;
      pregs[i]->db = 9+i;
      
      /* Set PWM CTRL register */
      pregs[i]->ctrl = PWM_CTRL_DBEN | PWM_CTRL_PAIR | PWM_CTRL_EN |
	PWM_CTRL_POL;
    }

    /* Wait a while */
    for(i = 0; i < 500; i++) tmp = i;

    /* Stop PWMs */
    for(i = 0; i < caps.npwm; i++)
      pregs[i]->ctrl = 0;
  }
  
  /* Test symmetric */
  if(caps.sympwm && SUBTEST2) {
    report_subtest(2);
    for(i = 0; i < caps.npwm; i++) {
      /* Set period, compare and dead band register */
      pregs[i]->per = 200+i*2;
      pregs[i]->comp = 50+i;
      pregs[i]->db = 9+i;

      /* Set PWM CTRL register */
      pregs[i]->ctrl = PWM_CTRL_DBEN | PWM_CTRL_PAIR | PWM_CTRL_EN |
	PWM_CTRL_POL | PWM_CTRL_METH;
    }

    /* Wait a while */
    for(i = 0; i < 500; i++) tmp = i;

    /* Stop PWMs */
    for(i = 0; i < caps.npwm; i++)
      pregs[i]->ctrl = 0;
  }
  
  if(caps.wavepwm) {
    /* Fill (part of) RAM */
    for(i = 0; i <= WSTOPADDR; i++)
      *(wram+i) = i+32;

    /* Read RAM and verify that data match that written */
    tmp = 1;
    for(i = 0; i <= WSTOPADDR; i++)
      if(*(wram+i) != (i+32))
	tmp = 0;
    if(!tmp)
      fail(1);

    /* Set stop address */
    cregs->wconf = WSTOPADDR;
      
    /* Set period and dead band register */
    pregs[caps.npwm-1]->per = 200;
    pregs[caps.npwm-1]->db = 9;

    /* Configure last PWM as asymmetric waveform PWM */
    if(caps.asympwm && SUBTEST3) {      
      report_subtest(3);
      
      /* Set PWM CTRL register */
      pregs[caps.npwm-1]->ctrl = PWM_CTRL_DBEN | PWM_CTRL_PAIR |
	PWM_CTRL_POL | PWM_CTRL_WEN;
      pregs[caps.npwm-1]->ctrl |= PWM_CTRL_EN;
      
      /* Wait a while */
      for(i = 0; i < 2000; i++)	tmp = i;
      
      /* Stop PWM */
      pregs[caps.npwm-1]->ctrl = 0;
    }
    
    /* Configure last PWM as symmetric waveform PWM */
    if(caps.sympwm && SUBTEST4) {
      report_subtest(4);
      
      /* Set PWM CTRL register */
      pregs[caps.npwm-1]->ctrl = PWM_CTRL_DBEN | PWM_CTRL_PAIR |
	PWM_CTRL_POL | PWM_CTRL_WEN | PWM_CTRL_METH;
      pregs[caps.npwm-1]->ctrl |= PWM_CTRL_EN;

      /* Wait a while */
      for(i = 0; i < 2000; i++) tmp = i;
      
      /* Stop PWM */
      pregs[caps.npwm-1]->ctrl = 0;
    }
  }

  /* STOP core */
  cregs->ctrl = 0;
  
  return 0;
}

