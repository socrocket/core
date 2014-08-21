#include "standalone.h"

extern void regtest();

int main() {
	report_start();

	regtest();

	report_end();
  return 0;
}
