/*
 * 00000000 <main>:
 *    0:   55              pushl  %ebp
 *    1:   89 e5           movl   %esp,%ebp
 *    3:   83 ec 04        subl   $0x4,%esp
 *    6:   e8 fc ff ff ff  call   7 <main+0x7>
 *    b:   89 c0           movl   %eax,%eax
 *    d:   89 45 fc        movl   %eax,0xfffffffc(%ebp)
 *   10:   8b 55 fc        movl   0xfffffffc(%ebp),%edx
 *   13:   89 d0           movl   %edx,%eax
 *   15:   eb 09           jmp    20 <main+0x20>
 *   17:   89 f6           movl   %esi,%esi
 *   19:   8d bc 27 00 00  leal   0x0(%edi,1),%edi
 *   1e:   00 00
 *   20:   89 ec           movl   %ebp,%esp
 *   22:   5d              popl   %ebp
 *   23:   c3              ret
 */

int calledFunction(void);

main()
  
{
  int i;
  
  i=calledFunction();
  return(i);
}
