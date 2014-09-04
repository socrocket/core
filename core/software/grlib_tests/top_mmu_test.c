
#include "standalone.h"
#include "cache.h"

extern void ramfill();
extern int mmu_test();

int main() {
	report_start();

	ramfill();
	mmu_test();

	report_end();
  return 0;
}
