#include "standalone.h"
#include "gptimer.h"

int main(int argc, char *argv[]) {
	report_start();
  unsigned int address  = 0x8000F000;
  unsigned int irq_addr = 0x8001F000;
  unsigned int irq = 8;
	gptimer_test(address, irq, irq_addr);

	report_end();
  return 0;
}
