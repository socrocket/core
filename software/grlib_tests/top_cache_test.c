#include "standalone.h"
#include "cache.h"

int main() {
	report_start();

	ramfill();
	cachetest();

	report_end();
  return 0;
}
