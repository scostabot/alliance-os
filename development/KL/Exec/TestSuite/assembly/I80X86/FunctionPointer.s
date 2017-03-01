
objects/architecture/I80X86/FunctionPointer:     file format elf32-i386

Disassembly of section .text:

00000000 <functionCalled>:
   0:	55                   	push   %ebp
   1:	89 e5                	mov    %esp,%ebp
   3:	b8 5a 00 00 00       	mov    $0x5a,%eax
   8:	5d                   	pop    %ebp
   9:	c3                   	ret    

0000000a <main>:
   a:	55                   	push   %ebp
   b:	89 e5                	mov    %esp,%ebp
   d:	83 ec 08             	sub    $0x8,%esp
  10:	83 e4 f0             	and    $0xfffffff0,%esp
  13:	b8 00 00 00 00       	mov    $0x0,%eax
  18:	29 c4                	sub    %eax,%esp
  1a:	c7 45 fc 00 00 00 00 	movl   $0x0,0xfffffffc(%ebp)
  21:	8b 45 fc             	mov    0xfffffffc(%ebp),%eax
  24:	ff d0                	call   *%eax
  26:	c9                   	leave  
  27:	c3                   	ret    
