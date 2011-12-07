
asm(
"	.text\n"
"	.align 4\n"
"	.global get_pid\n"
	
"get_sem:\n"	
"	set  mpsem, %o1\n"
"	set  0, %o0\n"
"	retl\n"
"	ldstuba [%o1] 1, %o0 \n"
/*"	swapa [%o1] 1, %o0 \n"*/

"ret_sem:\n"
/*"	set 1, %o0 \n" */
"	set 0, %o0 \n"
"	set mpsem, %o1\n"
"	retl\n"
"	st  %o0, [%o1]		\n"
		
"get_pid:\n"
"        mov  %asr17, %o0\n"
"        srl  %o0, 28, %o0\n"
"        retl\n"
"        and %o0, 0xf, %o0\n"

"mread: retl\n"
"        lda  [%o0] 1, %o0\n"

"getccfg: set 0xc, %o0\n"
"         retl\n"
"         lda [%o0] 2, %o0\n"
	
"	.data\n"
"	.align 4\n"
"	.global mpsem\n"
/*"mpsem:	.word 1\n"*/
"mpsem:	.word 0\n"
	
);

#include "standalone.h"
extern void ramfill();
extern int mmu_test();

int main() {
	report_start();

	ramfill();
	mmu_test();

	report_end();
  return 0;
}
