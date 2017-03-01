
objects/architecture/I80X86/ExternFunctionCalled:     file format elf32-i386

Disassembly of section .text:

00000000 <calledFunction>:
   0:	55                   	push   %ebp
   1:	89 e5                	mov    %esp,%ebp
   3:	b8 5a 00 00 00       	mov    $0x5a,%eax
   8:	5d                   	pop    %ebp
   9:	c3                   	ret    
