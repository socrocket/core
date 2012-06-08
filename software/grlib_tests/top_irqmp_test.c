int main() {
	report_start();

	ramfill();
	irqtest(0x00001000);
	report_end();
  return 0;
}
