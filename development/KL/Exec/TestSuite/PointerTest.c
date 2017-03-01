char table[16];

int main()
{
  char *p;

  p=table;
  *p='\0';
  p++;
  *p++='A';

  if(table[0]==0 && table[1]=='A') {
	return(0x5A);
  } else {
	return(0);
  }
}
