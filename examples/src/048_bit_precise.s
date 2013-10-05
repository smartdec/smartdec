/* gcc -nostdlib -o ../048_bit_precise 048_bit_precise.s */

movb	$1, %ah
movb	$2, %al
movw	%ax, (100)
