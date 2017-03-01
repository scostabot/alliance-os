#include "iosk.h"
#include <stdio.h>

#define BUFSIZE 1000

Alliance_IOSK iosk;

int
main (int argc, char *argv[])
{
    CORBA_Environment ev;
    CORBA_ORB orb;
    CORBA_long rv;
    char buf[30];
    int i;
    Alliance_IOSK_Session session;
    Alliance_IOSK_bytes* buffer;
    
    CORBA_exception_init(&ev);
    orb = CORBA_ORB_init(&argc, argv, "orbit-local-orb", &ev);

    if(argc < 2)
      {
	printf("Need an IOR as argv[1]\n");
	return 1;
      }

    iosk = CORBA_ORB_string_to_object(orb, argv[1], &ev);
    if (!iosk) {
	printf("Cannot bind to %s\n", argv[1]);
	return 1;
    }

    session = Alliance_IOSK_openSession(iosk, "linuxfile(testfile)",
					Alliance_IOSK_RW, &ev);

    buffer = Alliance_IOSK_bytes__alloc();
    buffer->_buffer = CORBA_sequence_CORBA_octet_allocbuf(BUFSIZE);

    for(i = 0; i < BUFSIZE; i++) {
	buffer->_buffer[i] = (i % 26) + 'a';
    }

    Alliance_IOSK_write(iosk, session, buffer, &ev);

    for(i = 0; i < BUFSIZE; i++) {
	buffer->_buffer[i] = (i % 26) + 'A';
    }

    Alliance_IOSK_writeAsync(iosk, session, buffer, &ev);

    Alliance_IOSK_readSeek(iosk, session, 0, Alliance_IOSK_Begin, &ev);

    i = BUFSIZE;
    
    buffer = Alliance_IOSK_read(iosk, session, &i, TRUE, &ev);
    
    Alliance_IOSK_closeSession(iosk, session, &ev);
    
    CORBA_Object_release(iosk, &ev);
    CORBA_Object_release((CORBA_Object)orb, &ev);

    return 0;
}
