/* gcc -nostdlib -o ../049_implicit_zero_extend 049_implicit_zero_extend.s */

movl	$131074, %ebx
movzx	%bx, %eax
movq	%rax, (100)
