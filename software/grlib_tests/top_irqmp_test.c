#include "standalone.h" // report_start(), report_end()
#include "cache.h" // ramfill()
#include "irqmp.h" // irqtest()

int main() {
	report_start();

	ramfill();
	irqtest(0x00001000);
	report_end();
  return 0;
}
