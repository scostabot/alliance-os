
objects/architecture/I80X86/FunctionLinkage:     file format elf32-i386

Disassembly of section .text:

00000000 <main>:
   0:	55                   	push   %ebp
   1:	89 e5                	mov    %esp,%ebp
   3:	83 ec 08             	sub    $0x8,%esp
   6:	83 e4 f0             	and    $0xfffffff0,%esp
   9:	b8 00 00 00 00       	mov    $0x0,%eax
   e:	29 c4                	sub    %eax,%esp
  10:	83 ec 08             	sub    $0x8,%esp
  13:	68 00 00 00 00       	push   $0x0
  18:	68 06 00 00 00       	push   $0x6
  1d:	e8 fc ff ff ff       	call   1e <main+0x1e>
  22:	83 c4 10             	add    $0x10,%esp
  25:	c9                   	leave  
  26:	c3                   	ret    
