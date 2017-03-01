#include <typewrappers.h>

#define INT_TO_FIXED(x) (((long)x) << 16)

typedef struct {
   WORD16 X;
   WORD16 Y;
} POINT;

typedef struct {
   WORD16 numVertex;
   POINT *point;
} POLY;

typedef struct {
   WORD16 width;
   UBYTE *bits;
} TEXTUREMAP;

typedef long FIXEDPOINT;

typedef struct {
   WORD16 direction;
   WORD16 remainingScans;
   WORD16 currentEnd;
   FIXEDPOINT sourceX;
   FIXEDPOINT sourceY;
   FIXEDPOINT sourceStepX;
   FIXEDPOINT sourceStepY;
   WORD16 destX;
   WORD16 destXIntStep;
   WORD16 destXDirection;
   WORD16 destXErrTerm;
   WORD16 destXAdjUp;
   WORD16 destXAdjDown;
} EDGE;

extern FIXEDPOINT FixedDiv(FIXEDPOINT, FIXEDPOINT);
