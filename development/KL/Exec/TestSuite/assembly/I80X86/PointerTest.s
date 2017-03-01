
objects/architecture/I80X86/PointerTest:     file format elf32-i386

Disassembly of section .text:

00000000 <main>:
   0:	55                   	push   %ebp
   1:	89 e5                	mov    %esp,%ebp
   3:	83 ec 08             	sub    $0x8,%esp
   6:	83 e4 f0             	and    $0xfffffff0,%esp
   9:	b8 00 00 00 00       	mov    $0x0,%eax
   e:	29 c4                	sub    %eax,%esp
  10:	c7 45 fc 00 00 00 00 	movl   $0x0,0xfffffffc(%ebp)
  17:	8b 45 fc             	mov    0xfffffffc(%ebp),%eax
  1a:	c6 00 00             	movb   $0x0,(%eax)
  1d:	8d 45 fc             	lea    0xfffffffc(%ebp),%eax
  20:	ff 00                	incl   (%eax)
  22:	8b 45 fc             	mov    0xfffffffc(%ebp),%eax
  25:	c6 00 41             	movb   $0x41,(%eax)
  28:	8d 45 fc             	lea    0xfffffffc(%ebp),%eax
  2b:	ff 00                	incl   (%eax)
  2d:	80 3d 00 00 00 00 00 	cmpb   $0x0,0x0
  34:	75 12                	jne    48 <main+0x48>
  36:	80 3d 01 00 00 00 41 	cmpb   $0x41,0x1
  3d:	75 09                	jne    48 <main+0x48>
  3f:	c7 45 f8 5a 00 00 00 	movl   $0x5a,0xfffffff8(%ebp)
  46:	eb 07                	jmp    4f <main+0x4f>
  48:	c7 45 f8 00 00 00 00 	movl   $0x0,0xfffffff8(%ebp)
  4f:	8b 45 f8             	mov    0xfffffff8(%ebp),%eax
  52:	c9                   	leave  
  53:	c3                   	ret    
