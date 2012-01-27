#include "standalone.h"
#include "cache.h"

int main() {
	report_start();

	ramfill();
	cramtest();

	report_end();
  return 0;
}
