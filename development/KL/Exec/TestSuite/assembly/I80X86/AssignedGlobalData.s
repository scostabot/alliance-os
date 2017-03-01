
objects/architecture/I80X86/AssignedGlobalData:     file format elf32-i386

Disassembly of section .text:

00000000 <main>:
   0:	55                   	push   %ebp
   1:	89 e5                	mov    %esp,%ebp
   3:	83 ec 08             	sub    $0x8,%esp
   6:	83 e4 f0             	and    $0xfffffff0,%esp
   9:	b8 00 00 00 00       	mov    $0x0,%eax
   e:	29 c4                	sub    %eax,%esp
  10:	a1 00 00 00 00       	mov    0x0,%eax
  15:	c9                   	leave  
  16:	c3                   	ret    
