main() {
	report_start();

	ramfill();
	irqtest(0x8001F000);
	report_end();
}
