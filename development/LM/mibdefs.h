/*
 * LM MIBs definitions
 *
 * This file define all data structure related to LM MIB handling
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 22/02/05 scosta    1.0    First draft
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

#include <typewrappers.h>

/*
 * We can think of the LM MIB data object as a normal C structure plus a
 * metadata structure, which contains all data that qualifies to external 
 * programs what is and in which way can be accessed an LM MIB type.
 * Using metacode a MIB data type is characterized by this structure:
 *
 * struct {
 *	 Type                   byte, word, string, bool, notification.. [MANDATORY]
 *	 AccessFlag             RW, RO, WO, NA [MANDATORY]
 *   Name                   MIB metadata name [MANDATORY]
 *   Description            MIB Metadata descrption [MANDATORY]
 *   Unit                   [SI prefix]SI base unit/SI base unit [MANDATORY]
 *   Range                  [OPTIONAL]
 * } MIBMetaData;
 *
 * Similarly, a MIB structure is a metastructure (a structure of the above
 * metadata):
 *
 * struct {
 *   Description            A NULL-terminated string [MANDATORY]
 *   RealData *             Pointer to real data in memory [MANDATORY]
 *   MIBMetaData mibD[1..N]
 * } TypeMIBStructure;
 *
 * We render the above structure as a C one mapping convenientely all
 * fields as below.
 * 
 */

/*                LM MIB TYPE & ACCESS FLAGS
 * It's a 16-bit word, with HASR as a flag of presenxe/absence of range field,
 * with 3-bit Access Modifier flag plus 4-bit type definition.
 *
 *   F E D C B A 9 8 7 6 5 4 3 2 1 0
 *  --------------------------------
 *  |H|ACCES|TYPE   |
 *  |A|MODIF|DEFINIT|
 *   S
 *   R
 */

typedef unsigned short TypeAndAccess;

#define _HASR 0x8000 /* HAVE A SUBRANGE */

/* Access Modifiers */

#define _RW 0x1000   /* Read/Write*/
#define _RO 0x2000   /* Read-only */
#define _WO 0x3000   /* Write-only */
#define _NA 0x4000   /* Not available (to MIB user) */

/* Type defines */

#define _bool   0x100       /*        */
                            /*        */
#define _word8  0x200       /*  TYPE  */
#define _word16 0x300       /*  BYTE  */
#define _word32 0x400       /*        */
#define _word64 0x500       /*        */
                            /*        */
#define _float  0x600       /*        */
#define _string 0x700       /*        */

#define _choice 0x800

#define _null   0x000

#define _notification 0x800

/*                            LM MIB UNIT TYPE
 * The Unit type can contain a complex unit of measure, with this
 * structure:
 *
 * [prefix]<base|derived>[<operator '\' | operator'*'><base|derived>]
 * 
 * where Prefix is a SI-compliant prefix, base is an SI-compliant base 
 * unit, derived is a unit derived from SI-base, operator tells the relation
 * for first unit with the following one. 
 * For instance Mbit/s m/s, Km/h, N*m^2 are all units expressable with the 
 * above definition.
 * The structure above is formatted as a 16-bit word with 5-bit SI-compliant
 * prefix, plus 5-bit SI base or derived unit, an 1-bit flag with the power
 * of denominator or last factor in product, thr operator type (factor or
 * product) and a 4-bit SI base plus some (but not all) derived units.
 *
 *   F E D C B A 9 8 7 6 5 4 3 2 1 0
 *  -------------------------------------
 *   SI PREFIX|BASE/    |P|O|BASE/  
 *            |DERIVED  |O|P|RESTRICTED
 *            |UNIT     |W|E|DERIVED UNIT
 *
 */

/* SI PREFIXES */

#define _DECA  0x1
#define _ETTO  0x2
#define _KILO  0x3
#define _MEGA  0x4
#define _GIGA  0x5
#define _TERA  0x6
#define _PETA  0x7
#define _EXA   0x8
#define _ZETTA 0x9
#define _YOTTA 0xA

#define _deci  0xB
#define _centi 0xC
#define _milli 0xD
#define _micro 0xE
#define _nano  0xF
#define _pico  0x10
#define _femto 0x11
#define _atto  0x12
#define _zepto 0x13
#define _yocto 0x14

/* Units */

#define _adimensional 0x0

/* SI BASE */

#define _second  0x1
#define _meter   0x2
#define _gram    0x3
#define _kelvin  0x4
#define _mole    0x5
#define _ampere  0x6
#define _candela 0x7

/* SI RESTRICTED DERIVED */

#define _minute  0x8
#define _hour    0x9
#define _day     0xA

/* Computer units */

#define _bit     0xB
#define _byte    0xC
#define _pixel   0xD
#define _samples 0xE

/* SI DERIVED */
  
#define _Hz      0xF
#define _Watt    0x10
#define _Volt    0x11
#define _Ohm     0x12
#define _Farad   0x13
#define _Newton  0x14
#define _Joule   0x15

/* Operator */

#define _OPERATOR 0x10

typedef unsigned short Unit;

/*                      LM DATA TYPE SUBRANGE
 * The subrange is defined as a contiguos sequence of two variables
 * of the type specified in LM Unit type which contains the lower and
 * upper bound respectevely that the given data object can be assigned to.
 *
 */

typedef struct { 
   union {
      BYTE lowerByte;
      WORD16 lowerWord16;
      WORD32 lowerWord32;
      WORD64 lowerWord64;
   };
   union {
      BYTE upperByte;
      WORD16 upperWord16;
      WORD32 upperWord32;
      WORD64 upperWord64;
   };
} SubRange;   
      
/* Subrange for STRING type is particular and is defined as two 8-bit
 * variables, the first with the definition of the proper subset
 * of ASCII charset that may be assigned to the given STRING data object,
 * and the second as the maximum length in characters of the input string */

#define _STRING_LOWERONLY   0x1
#define _STRING_UPPERONLY   0x2
#define _STRING_PUNCTONLY   0x4  /* Only . */
#define _STRING_COMMAONLY   0x8  /* Only , */
#define _STRING_NUMBERONLY  0x10 /* Only 0123456789 */
#define _STRING_WILDCARDS   0x20 /* Only *?! */
#define _STRING_MATHONLY    0x40 /* Only ()+-*'/' */

/* Finally we can map as a C structure the type of an LM MIB data object.
 * Since subrange is an optional part of MIB definition, we define two
 * distinct MIB structures, one with and one without the optional field. */

typedef STRING * Description;
typedef STRING * MIBName;
typedef void * RealDataPointer;

typedef struct {
	TypeAndAccess typeAndAccess;
    MIBName name;
	Description description;
	Unit unit;
} TypeMIBData;

typedef struct {
	TypeAndAccess typeAndAccess;
    MIBName name;
	Description description;
	Unit unit;
   SubRange subrange;
} TypeMIBDataWithSubR;

