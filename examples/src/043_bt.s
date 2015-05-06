/* gcc -m32 -nostdlib -o ../043_bt 043_bt.s */

	cmp	%ebp,%ebp
	bt	$3, %eax
	jae	L
	movl	$10,(500)
L:
	nop
