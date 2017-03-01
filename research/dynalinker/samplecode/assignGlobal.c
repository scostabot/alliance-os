/*
 * Test program for dynamic linking.
 * 
 * Case 2: global variable handling.

00000000 <assignGlobal>:
int assignGlobal(void)
  
{
   0:	55             	pushl  %ebp
   1:	89 e5          	movl   %esp,%ebp
  extern int i;
  
  i=0x1000;
   3:	c7 05 00 00 00 	movl   $0x1000,0x0
   8:	00 00 10 00 00 
}
   d:	89 ec          	movl   %ebp,%esp
   f:	5d             	popl   %ebp
  10:	c3             	ret    
  
  The four bytes after c7 05 must be assigned to the address computed 
  by the dynalinker of variable <i>.
  
*/

void assignGlobal(void)
  
{
  extern int i;
  
  i=0x1000;
}
