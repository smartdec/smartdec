/* gcc -nostdlib -o ../050_bit_precise2 050_bit_precise2.s */

movb	%bl, %al
movb	$2, %ah
movq	%rax, (100)
