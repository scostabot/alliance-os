/*
 * Test program for dynamic linking.
 * 
 * Case 2: global variable handling. 

00000000 <main>:
int i;

int main()
  
{
   0:	55             	pushl  %ebp
   1:	89 e5          	movl   %esp,%ebp
  assignGlobal();
   3:	e8 fc ff ff ff 	call   4 <main+0x4>
  return(i);
   8:	8b 15 00 00 00 	movl   0x0,%edx
   d:	00 
   e:	89 d0          	movl   %edx,%eax
  10:	eb 0e          	jmp    20 <main+0x20>
}
  12:	8d b4 26 00 00 	leal   0x0(%esi,1),%esi
  17:	00 00 
  19:	8d bc 27 00 00 	leal   0x0(%edi,1),%edi
  1e:	00 00 
  20:	89 ec          	movl   %ebp,%esp
  22:	5d             	popl   %ebp
  23:	c3             	ret    

   In this case the dynalinker must assign the four bytes after e8 fc
   with the relative address displacement for the assignGlobal() 
   entry point.
*/

void assignGlobal(void);

int i;

int main()
  
{
  assignGlobal();
  return(i);
}
