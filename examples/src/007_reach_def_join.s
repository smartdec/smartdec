/* gcc -m32 -nostdlib -o ../007_reach_def_join 007_reach_def_join.s */

	addl	$1, %eax
	jne	L1
	jmp	L2
L1:	addl	$2, %eax
L2:	nop
