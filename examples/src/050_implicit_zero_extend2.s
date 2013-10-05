/* gcc -nostdlib -o ../050_implicit_zero_extend2 050_implicit_zero_extend2.s */

movb	%bl, %al
movb	$2, %ah
movq	%rax, (100)
