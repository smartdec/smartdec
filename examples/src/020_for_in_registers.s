/* gcc -nostdlib -o ../020_for_in_registers 020_for_in_registers.s */

puts:	retq

main:	push   %rbp
	mov    %rsp,%rbp
	sub    $0x10,%rsp
	movl   $0x0,%ecx
	jmp    L1
L2:	mov    $0x40011e,%edi
	callq  puts
	addl   $0x1,%ecx
L1:	cmpl   $0x9,%ecx
	jle    L2
	mov    $0x0,%eax
	leaveq 
	retq   
