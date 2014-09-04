#include "standalone.h"
#include "cache.h"

extern void cramtest();

int main() {
	report_start();

	ramfill();
	cramtest();

	report_end();
  return 0;
}
