
#include "mibdefs.h"      /* Contains MIB type primitives and constants */

#include "mib.h"         /* Contains MIB user definitions and objects */ 


#include <stdlib.h>

structureType example={ 0xdead, 0xdeadbeef };

/* Also the following structure is the result of compiling the MIB */

const TypeMIBStructure structureType_inMIB={
    "Structure name",
	(void *) &example,
    { 
	  {  _word16+_RW, "first", "Description of 1st attribute", _adimensional },
	  {  _word32+_RW, "second", "Description of 2nd attribute", _adimensional },
	  {  _null, "", "", _null }
    }
};

VOID * KLmalloc(int size)

{
    return(malloc(size));
}

VOID * LMnewMOI(const TypeMIBStructure *mibStruct)
{
   UWORD16 i=0;
   UWORD32 size=0;
   UWORD32 type;

   /* Compute structure data size */

   do {
       type=mibStruct->mibD[i].typeAndAccess & 0x0F00;
       switch(type) {
           case _bool:
               size+=1;
               break;
           case _word8:
               size+=1;
               break;
           case _word16:
               size+=2;
               break;
           case _word32:
               size+=4;
               break;
           case _word64:
               size+=8;
               break;
           case _float:
               size+=8;
               break;
           case _string:
               size+=8;
               break;
       }

       i++;
   } while(type!=_null);

   /* Allocate memory for it */

   return(KLmalloc(size));
}

int main(int argc, char *argv[])
{
   UBYTE *p;

   p=LMnewMOI(&structureType_inMIB);
   exit(0);
}  
