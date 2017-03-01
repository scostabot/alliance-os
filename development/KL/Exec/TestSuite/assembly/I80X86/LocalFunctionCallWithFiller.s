
objects/architecture/I80X86/LocalFunctionCallWithFiller:     file format elf32-i386

Disassembly of section .text:

00000000 <unusedCode>:
   0:	55                   	push   %ebp
   1:	89 e5                	mov    %esp,%ebp
   3:	b8 ff 00 00 00       	mov    $0xff,%eax
   8:	5d                   	pop    %ebp
   9:	c3                   	ret    

0000000a <functionCalled>:
   a:	55                   	push   %ebp
   b:	89 e5                	mov    %esp,%ebp
   d:	b8 5a 00 00 00       	mov    $0x5a,%eax
  12:	5d                   	pop    %ebp
  13:	c3                   	ret    

00000014 <main>:
  14:	55                   	push   %ebp
  15:	89 e5                	mov    %esp,%ebp
  17:	83 ec 08             	sub    $0x8,%esp
  1a:	83 e4 f0             	and    $0xfffffff0,%esp
  1d:	b8 00 00 00 00       	mov    $0x0,%eax
  22:	29 c4                	sub    %eax,%esp
  24:	e8 fc ff ff ff       	call   25 <main+0x11>
  29:	c9                   	leave  
  2a:	c3                   	ret    
