/* gcc -nostdlib -o ../022_intersecting_functions 022_intersecting_functions.s */

f:	push   %rbp
	mov    %rsp,%rbp
f1:	pop    %rbp
	retq

g:	push   %rbp
	mov    %rsp,%rbp
	jmp    f1

main:	push   %rbp
	mov    %rsp,%rbp
	pop    %rbp
	call   f
	call   g
	retq   
