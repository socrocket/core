#include "stdlib.h"
#include "stdio.h"
#include "irqmp.h"

int get_cpu_id() {
    unsigned int result = 0;
    asm(
        "rd  %%asr17, %[result]"
        : [result] "=r" (result)
    );
    return result >> 28;
}

volatile unsigned int i = 0;
int main() {
    int cpuid = get_cpu_id();
    if(cpuid == 0) {
      struct irqmp *lr = (struct irqmp *) 0x80000200;
      irqmp_base = lr;
	    init_irqmp(irqmp_base);
      irqmp_base->mpstatus |= 0xFFFF;
    }

	  printf("Hello World %d\n", cpuid);
    fflush(stdout);

//    while(1) {
//      i++;
//    }
    return 0;
}

