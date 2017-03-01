
objects/architecture/I80X86/LocalFunctionCallPostfix:     file format elf32-i386

Disassembly of section .text:

00000000 <main>:
   0:	55                   	push   %ebp
   1:	89 e5                	mov    %esp,%ebp
   3:	83 ec 08             	sub    $0x8,%esp
   6:	83 e4 f0             	and    $0xfffffff0,%esp
   9:	b8 00 00 00 00       	mov    $0x0,%eax
   e:	29 c4                	sub    %eax,%esp
  10:	e8 fc ff ff ff       	call   11 <main+0x11>
  15:	c9                   	leave  
  16:	c3                   	ret    

00000017 <unusedCode>:
  17:	55                   	push   %ebp
  18:	89 e5                	mov    %esp,%ebp
  1a:	b8 ff 00 00 00       	mov    $0xff,%eax
  1f:	5d                   	pop    %ebp
  20:	c3                   	ret    

00000021 <functionCalled>:
  21:	55                   	push   %ebp
  22:	89 e5                	mov    %esp,%ebp
  24:	b8 5a 00 00 00       	mov    $0x5a,%eax
  29:	5d                   	pop    %ebp
  2a:	c3                   	ret    
