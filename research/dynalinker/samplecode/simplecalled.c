/*
 * 00000000 <calledFunction>:
 *    0:   55              pushl  %ebp
 *    1:   89 e5           movl   %esp,%ebp
 *    3:   b8 20 00 00 00  movl   $0x20,%eax
 *    8:   eb 06           jmp    10 <calledFunction+0x10>
 *    a:   8d b6 00 00 00  leal   0x0(%esi),%esi
 *    f:   00
 *   10:   89 ec           movl   %ebp,%esp
 *   12:   5d              popl   %ebp
 *   13:   c3              ret
 */

int calledFunction(void)
  
{
  return(0x20);
}

