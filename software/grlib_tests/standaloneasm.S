  .text
  .align 4
  .global get_pid

get_sem:
  set mpsem, %o1
  set 0, %o0
  retl
  ldstuba [%o1] 1, %o0
!  swapa [%o1] 1, %o0
  nop

ret_sem:
!  set 1, %o0
  set 0, %o0
  set mpsem, %o1
  retl
  st %o0, [%o1]
  nop

get_pid:
  mov %asr17, %o0
  srl %o0, 28, %o0
  retl
  and %o0, 0xf, %o0
  nop

mread:
  retl
  lda [%o0] 1, %o0
  nop

getccfg:
  set 0xc, %o0
  retl
  lda [%o0] 2, %o0
  nop
	
  .data
  .align
  .global mpsem
  
!mpsem:
  .word 1

mpsem:
  .word 0
	
