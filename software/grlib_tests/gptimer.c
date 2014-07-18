#include "testmod.h"
#include "irqmp.h"
#include "cache.h" // cache_disable()
#include "standalone.h" // success()
#include <stdio.h>

struct timerreg {
    volatile unsigned int counter;      /* 0xi0 */
    volatile unsigned int reload;       /* 0xi4 */
    volatile unsigned int control;      /* 0xi8 */
    volatile unsigned int dummy;        /* 0xiC */
};

struct gptimer {
    volatile unsigned int scalercnt;    /* 0x00 */
    volatile unsigned int scalerload;   /* 0x04 */
    volatile unsigned int configreg;    /* 0x08 */
    volatile unsigned int dummy1;       /* 0x0C */
    struct timerreg timer[7];
};

#define IRQPEND 0x10
#define CHAIN_TEST 8

static volatile int gpirq = 0;

static void gptimer_irqhandler(int irq) {
    gpirq += 1;
}

void gptimer_test(int addr, int irq) {
    struct gptimer *lr = (struct gptimer *) addr;
    //extern volatile int irqtbl[];
    //int pil;
    int i, j, ntimers;

    cache_disable(); // <-- needs to be removed

    report_device(0x01011000);
    ntimers = lr->configreg & 0x7;
    lr->scalerload = 0xFFFFFFFF;
    if(lr->scalercnt == lr->scalercnt) {
        fail(1);
    } else {
        success(1);
    }
    //printf("Scaler 0x%08x\n", lr->scalercnt);
    //printf("Reload 0x%08x\n", lr->scalerload);
    //printf("Config 0x%08x\n", lr->configreg);

    /* timer 1 test */

    lr->scalerload = 0xFFFFFFFF;
    lr->scalercnt = 0xFFFFFFFF;
    //lr->scalerload = 31;
    //lr->scalercnt = 31;
    for(i=0; i<ntimers; i++) {
        lr->timer[i].control = 0; // halt all timers
	  }
    
    /* test basic functions */
    for(i=0; i<ntimers; i++) {
        report_subtest(i);
        lr->timer[i].counter = 0;
        lr->timer[i].reload = 15;
        lr->timer[i].control = 0x6;
        if(lr->timer[i].counter != 15) {
            fail(2+3*i); // check loading
        } else {
            success(2+3*i);
        }
        
        lr->timer[i].control = 0xf;
        for(j=14; j >= 0; j--) { 
            while(lr->timer[i].counter != j) {
            }
        }
        while(lr->timer[i].counter != 15) {
        }
    
        if(!(lr->timer[i].control & IRQPEND)) {
            fail(3+3*i);
        } else {
            success(3+3*i);
        }
        
        lr->timer[i].control = 0;	
        if(lr->timer[i].control & IRQPEND) {
            fail(4+3*i);
        } else {
            success(4+3*i);
        }
    }

    //for(i=0; i<ntimers; i++) {
    //    lr->timer[i].control = 0x0;
    //}

    if(ntimers > 1) { /* simple check of chain function */
        report_subtest(CHAIN_TEST);
        lr->timer[0].control = 0xf;
        lr->timer[1].control = 0x2f;
        while(lr->timer[1].counter <= 13) {
            printf("Counter0 %d\n", lr->timer[0].counter);
            printf("Counter1 %d\n", lr->timer[1].counter);
        }
    }

    for(i=0; i<ntimers; i++) {
        lr->timer[i].control = 0; // halt all timers
    }
	
    if(irqmp_base) {
        catch_interrupt((int)gptimer_irqhandler, irq);
        init_irqmp(irqmp_base);
        irqmp_base->irqmask = 1 << irq;  /* unmask interrupt */
        lr->timer[0].reload = 15;
        lr->timer[0].control = 0xd;
        asm("wr %g0, %g0, %asr19");  /* power-down */
    }
}


/*
gptimer_test_pp(int addr, int irq)
{
    struct ambadev dev;

    if (find_ahb_slvi(&dev) == 0) 
        gptimer_test(dev.start[0], dev.irq);
}
*/
