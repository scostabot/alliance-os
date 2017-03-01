
/* The following structure is the result of compiling the MIB */

typedef struct {
    Description des;
	RealDataPointer p; 
	TypeMIBData mibD[3];
} TypeMIBStructure;

/* This is the user-written data structure that is also defined in MIB */

typedef struct {
	short shortType;
	UWORD32 intType;
} structureType;


