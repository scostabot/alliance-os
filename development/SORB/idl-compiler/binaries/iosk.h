#ifndef Alliance_IOSK_Caller
#define Alliance_IOSK_Caller
typedef CORBA_unsigned_long_long Alliance_IOSK_Caller;
#endif

#ifndef Alliance_IOSK_Session
#define Alliance_IOSK_Session
typedef CORBA_unsigned_long Alliance_IOSK_Session;
#endif

#ifndef Alliance_IOSK_Address
#define Alliance_IOSK_Address
typedef CORBA_unsigned_long_long Alliance_IOSK_Address;
#endif

#ifndef Alliance_IOSK_bytes
#define Alliance_IOSK_bytes
typedef Only_basic_types_implemented Alliance_IOSK_bytes;
#endif

#ifndef Alliance_IOSK_strings
#define Alliance_IOSK_strings
typedef Only_basic_types_implemented Alliance_IOSK_strings;
#endif

#ifndef Alliance_IOSK_SeekDir
#define Alliance_IOSK_SeekDir
typedef UINT32 Alliance_IOSK_SeekDir;
#define Begin (UINT32)( 0x00000000 )
#define End (UINT32)( 0x00000001 )
#define Forward (UINT32)( 0x00000002 )
#define Back (UINT32)( 0x00000003 )
#endif

#ifndef Alliance_IOSK_Permission
#define Alliance_IOSK_Permission
typedef UINT32 Alliance_IOSK_Permission;
#define RW (UINT32)( 0x00000000 )
#define RO (UINT32)( 0x00000001 )
#define WO (UINT32)( 0x00000002 )
#define UNDEF (UINT32)( 0x00000003 )
#endif

#ifndef Alliance_IOSK_BadDevice
#define Alliance_IOSK_BadDevice
typedef struct Alliance_IOSK_BadDevice {
	CORBA_char * desc;
} Alliance_IOSK_BadDevice;
#define ex_Alliance_IOSK_BadDevice "IDL:Alliance/IOSK/BadDevice:1.0"
#define Alliance_IOSK_BadDevice__alloc (Alliance_IOSK_BadDevice *)\
	(malloc(sizeof(Alliance_IOSK_BadDevice)))
#endif

#ifndef Alliance_IOSK_BadHandle
#define Alliance_IOSK_BadHandle
typedef struct Alliance_IOSK_BadHandle {
	CORBA_char * desc;
} Alliance_IOSK_BadHandle;
#define ex_Alliance_IOSK_BadHandle "IDL:Alliance/IOSK/BadHandle:1.0"
#define Alliance_IOSK_BadHandle__alloc (Alliance_IOSK_BadHandle *)\
	(malloc(sizeof(Alliance_IOSK_BadHandle)))
#endif

#ifndef Alliance_IOSK_BadAddress
#define Alliance_IOSK_BadAddress
typedef struct Alliance_IOSK_BadAddress {
	CORBA_char * desc;
} Alliance_IOSK_BadAddress;
#define ex_Alliance_IOSK_BadAddress "IDL:Alliance/IOSK/BadAddress:1.0"
#define Alliance_IOSK_BadAddress__alloc (Alliance_IOSK_BadAddress *)\
	(malloc(sizeof(Alliance_IOSK_BadAddress)))
#endif

#ifndef Alliance_IOSK_BadRead
#define Alliance_IOSK_BadRead
typedef struct Alliance_IOSK_BadRead {
	CORBA_char * desc;
} Alliance_IOSK_BadRead;
#define ex_Alliance_IOSK_BadRead "IDL:Alliance/IOSK/BadRead:1.0"
#define Alliance_IOSK_BadRead__alloc (Alliance_IOSK_BadRead *)\
	(malloc(sizeof(Alliance_IOSK_BadRead)))
#endif

#ifndef Alliance_IOSK_BadWrite
#define Alliance_IOSK_BadWrite
typedef struct Alliance_IOSK_BadWrite {
	CORBA_char * desc;
} Alliance_IOSK_BadWrite;
#define ex_Alliance_IOSK_BadWrite "IDL:Alliance/IOSK/BadWrite:1.0"
#define Alliance_IOSK_BadWrite__alloc (Alliance_IOSK_BadWrite *)\
	(malloc(sizeof(Alliance_IOSK_BadWrite)))
#endif

PUBLIC extern Alliance_IOSK_Session
Alliance_IOSK_openSession (CORBA_Object anObject,
	INPUT CORBA_char * device,
	CORBA_Environment *ev);

PUBLIC extern void
Alliance_IOSK_closeSession (CORBA_Object anObject,
	INPUT Alliance_IOSK_Session id,
	CORBA_Environment *ev);

PUBLIC extern Alliance_IOSK_bytes
Alliance_IOSK_read (CORBA_Object anObject,
	INPUT Alliance_IOSK_Session id,
	INPUT CORBA_boolean block,
	CORBA_Environment *ev);

PUBLIC extern void
Alliance_IOSK_write (CORBA_Object anObject,
	INPUT Alliance_IOSK_Session id,
	CORBA_Environment *ev);

PUBLIC extern void
Alliance_IOSK_writeAsync (CORBA_Object anObject,
	INPUT Alliance_IOSK_Session id,
	CORBA_Environment *ev);

PUBLIC extern void
Alliance_IOSK_readSeek (CORBA_Object anObject,
	INPUT Alliance_IOSK_Session id,
	INPUT Alliance_IOSK_SeekDir dir,
	CORBA_Environment *ev);

PUBLIC extern void
Alliance_IOSK_writeSeek (CORBA_Object anObject,
	INPUT Alliance_IOSK_Session id,
	INPUT Alliance_IOSK_SeekDir dir,
	CORBA_Environment *ev);

PUBLIC extern void
Alliance_IOSK_lock (CORBA_Object anObject,
	INPUT Alliance_IOSK_Session id,
	CORBA_Environment *ev);

