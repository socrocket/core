#include "testmod.h"
#include "standalone.h"
#include "irqmp.h"
#include "cache.h"

struct irqmp *irqmp_base;
static volatile int irqtbl[18];

void irqhandler_f(int irq) {
    irqtbl[irqtbl[0]] = irq + 0x10;
    irqtbl[0]++;
}

void init_irqmp(struct irqmp *lr) {
    lr->irqlevel = 0;   /* clear level reg */
    lr->irqmask = 0x0;  /* mask all interrupts */
    lr->irqclear = -1;  /* clear all pending interrupts */
    irqtbl[0] = 1;      /* init irqtable */
}
	
int irqtest(int addr) {
    int i, a, psr;
    volatile int marr[4];
    volatile int larr[4];
    struct irqmp *lr = (struct irqmp *) addr;
    irqmp_base = lr;

    report_device(0x0100d000);
    init_irqmp(lr);

    for(i=1; i<16; i++) {
        catch_interrupt((int)irqhandler_f, i);
    }

    /* test that interrupts are properly prioritised */
	
    lr->irqforce = 0x0fffe; /* force all interrupts */
    if(lr->irqforce != 0x0fffe) {
        fail(1); /* check force reg */
    } else {
        success(1);
    }

    lr->irqmask = 0x0fffe;	  /* unmask all interrupts */
    if(lr->irqmask != 0x0fffe) {
        fail(2); /* check mask reg */
    } else {
        success(2);
    }
    
    while(lr->irqforce) {
    };  /* wait until all iterrupts are taken */

    /* check that all interrupts were take in right order */
    if(irqtbl[0] != 16) {
        fail(3);
    } else {
        success(3);
    }
    
	  for(i=1;i<16;i++) { 
        if(irqtbl[i] != (0x20 - i)) {
            fail(3+i);
        } else {
            success(3+i);
        }
    }

    /* test priority of the two interrupt levels */

    irqtbl[0] = 1;			/* init irqtable */
    lr->irqlevel = 0xaaaa;	/* set level reg to  odd irq -> level 1 */
    lr->irqmask = 0xfffe;	        
    if(lr->irqlevel != 0xaaaa) {
        fail(19); /* check level reg */
    } else {
        success(19);
    }
    
	  if(lr->irqmask != 0xfffe) {
        fail(20); /* check mask reg */
    } else {
        success(20);
    }
    
	  lr->irqforce = 0x0fffe;	/* force all interrupts */
	  while(lr->irqforce) {
    };  /* wait until all iterrupts are taken */

	  /* check that all interrupts were take in right order */
	  if(irqtbl[0] != 16) {
        fail(21);
    } else {
        success(21);
    }
    
	  for(i=1;i<8;i++) { 
        if(irqtbl[i] != (0x20 - (i*2-1))) {
            fail(21+i);
        } else {
            success(21+i);
        }
    }
    
	  for(i=2;i<8;i++) { 
        if(irqtbl[i+8] != (0x20 - (i*2))) {
		        fail(27+i);
        } else {
            success(27+i);
        }
    }

    /* check interrupts of multi-cycle instructions */

    marr[0] = 1; 
    marr[1] = marr[0]+1; 
    marr[2] = marr[1]+1; 
    a = marr[2]+1; 
    marr[3] = a; 
    larr[0] = 6;

    lr->irqlevel = 0;	/* clear level reg */
    lr->irqmask = 0x0;	/* mask all interrupts */
    irqtbl[0] = 1;		/* init irqtable */
    lr->irqmask = 0x00002;	  /* unmask interrupt */
    lr->irqforce = 0x00002;	/* force interrupt */

    asm(
    " mov  %asr17, %g1\n\t"
    " andcc %g1, 0x100, %g0\n\t"
    " be 1f\n\t"
    " nop \n\t"
    " umul %g0, %g1, %g0\n\t"
    " umul %g0, %g1, %g0\n\t"
    " umul %g0, %g1, %g0\n\t"
    "  1:\n\t"
    " ");

    lr->irqforce = 0x00002;	/* force interrupt */
    asm("nop;");
    larr[1] = larr[0];
    if(larr[0] != 6) {
        fail(35);
    } else {
        success(35);
    }
    
    lr->irqforce = 0x00002;	/* force interrupt */
    asm("nop;");
    larr[1] = 0;
    if(larr[1] != 0) {
        fail(36);
    } else {
        success(36);
    }

    //while(lr->irqforce) {
    //    printf("irq_force: 0x%08x\n", lr->irqforce);
    //};  /* wait until all iterrupts are taken */

    /* check number of interrupts */
    if(irqtbl[0] != 4) {
        fail(37);
    } else {
        success(37);
    }

    lr->irqmask = 0x0;	/* mask all interrupts */

    /* check that PSR.PIL work properly */

    lr->irqforce = 0x0fffe;	/* force all interrupts */
    irqtbl[0] = 1;		/* init irqtable */
    psr = xgetpsr() | (15 << 8);
    setpsr(psr); /* PIL = 15 */
    lr->irqmask = -1;	/* enable all interrupts */
    while(!lr->irqmask);   /* avoid compiler optimisation */
    if(irqtbl[0] != 2) {
        fail(38);
    } else {
        success(38);
    }
    if(irqtbl[1] != 0x1f) {
        fail(39);
    } else {
        success(39);
    }
    setpsr(xgetpsr() - (1 << 8));
    for(i=2;i<16;i++) { 
        setpsr(xgetpsr() - (1 << 8));
        if(irqtbl[0] != i+1) {
            fail(38+i*2);
        } else {
            success(38+i*2);
        }
        if(irqtbl[i] != (0x20 - i)) {
            fail(39+2*i);
        } else {
            success(39+2*i);
        }
    }

    /* test optional secondary interrupt controller */
    /*
    lr->irqmask = 0x0;
    lr->imask2 = 0x0;
    lr->ipend2 = 0x0;	
    lr->ipend2 = 0x1;
    if(!lr->ipend2) {
        return(0);
    }
    lr->ipend2 = -1;
    lr->imask2 = -1;
    for(i=lr->istat2 & 0x1f; i >=0; i--) {
		    if((lr->istat2 & 0x1f) != i) {
            fail (17+i);
        } else {
            success(17+i);
        }
        lr->istat2 = (1 << i);
        lr->irqclear = -1;
    }

    if(lr->istat2 & 0x20) {
        fail(33);
    } else {
        success(33);
    }
    if(lr->irqpend) {
        fail(34);
    } else {
        success(34);
    }
    */
    lr->irqmask = 0x0;	/* mask all interrupts */
    lr->irqclear = -1;	/* clear all pending interrupts */
    return 0;
}

