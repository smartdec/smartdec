/* gcc -nostdlib -o ../021_for_no_struct 021_for_no_struct.s */

main:
	push   %rbp
	mov    %rsp,%rbp
	sub    $0x10,%rsp
	movl   $0x0,%ecx
	jmp    L1
L2:	mov    $0x40011e,%edi
	movb   $0x7,(%ecx)
	addl   $0x1,%ecx
L1:	cmpl   $0x9,%ecx
	jle    L2
	mov    $0x0,%eax
	leaveq 
	retq   
