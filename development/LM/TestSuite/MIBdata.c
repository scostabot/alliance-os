
#define __MODULE_STRING_1(x)    #x
#define __MODULE_STRING(x)      __MODULE_STRING_1(x)

#define MODULE_PARM(var,type)                   \
const char __module_parm_##var[]                \
__attribute__((section(".modinfo"))) =          \
"parm_" __MODULE_STRING(var) "=" type

#define MODULE_PARM_DESC(var,desc)              \
const char __module_parm_desc_##var[]           \
__attribute__((section(".modinfo"))) =          \
"parm_desc_" __MODULE_STRING(var) "=" desc

#define MODULE_GENERIC_TABLE(gtype,name)        \
static const unsigned long __module_##gtype##_size \
  __attribute__ ((unused)) = sizeof(struct gtype##_id); \
static const struct gtype##_id * __module_##gtype##_table \
  __attribute__ ((unused)) = name

/* --------------------------------------- */

#define __LM_STRING_1(x)    #x
#define __LM_STRING(x)      __LM_STRING_1(x)

#define MIB_START(name, size) \
const char __mib_attr_##var[]                \
__attribute__((section(".lminfo"))) =          \
__LM_STRING(name) " " size

#define MIB_ATTR(var,type,unit,aq,range)     \
const char __mib_attr_##var[]                \
__attribute__((section(".lminfo"))) =          \
 __LM_STRING(var) "=" type "," unit "," aq "," range 

/* #define MIBBYTE MIB_ATTR( */

/* MIB test
MIBBYTE pippo READONLY */

MIB_START(TEST, "10");
MIB_ATTR(pippo, "BYTE", "1", "RO", "0-100");
MIB_ATTR(pluto, "UBYTE", "1", "RO", "0-100");
MIB_ATTR(paperino, "UWORD32", "1", "RO", "0-100");

