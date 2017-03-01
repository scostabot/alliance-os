#include "iosk.h"
#include "internal.h"

/*** App-specific servant structures ***/
typedef struct {
   POA_Alliance_IOSK servant;
   PortableServer_POA poa;

} impl_POA_Alliance_IOSK;

/*** Implementation stub prototypes ***/
static void impl_Alliance_IOSK__destroy(impl_POA_Alliance_IOSK * servant,
					CORBA_Environment * ev);

Alliance_IOSK_Session
impl_Alliance_IOSK_openSession(impl_POA_Alliance_IOSK * servant,
			       CORBA_char * device,
			       Alliance_IOSK_Permission permission,
			       CORBA_Environment * ev);

void
 impl_Alliance_IOSK_closeSession(impl_POA_Alliance_IOSK * servant,
				 Alliance_IOSK_Session id,
				 CORBA_Environment * ev);

Alliance_IOSK_bytes *
 impl_Alliance_IOSK_read(impl_POA_Alliance_IOSK * servant,
			 Alliance_IOSK_Session id,
			 CORBA_unsigned_long * length,
			 CORBA_boolean block,
			 CORBA_Environment * ev);

void
 impl_Alliance_IOSK_write(impl_POA_Alliance_IOSK * servant,
			  Alliance_IOSK_Session id,
			  Alliance_IOSK_bytes * buffer,
			  CORBA_Environment * ev);

void
 impl_Alliance_IOSK_writeAsync(impl_POA_Alliance_IOSK * servant,
			       Alliance_IOSK_Session id,
			       Alliance_IOSK_bytes * buffer,
			       CORBA_Environment * ev);

void
 impl_Alliance_IOSK_readSeek(impl_POA_Alliance_IOSK * servant,
			     Alliance_IOSK_Session id,
			     Alliance_IOSK_Address where,
			     Alliance_IOSK_SeekDir dir,
			     CORBA_Environment * ev);

void
 impl_Alliance_IOSK_writeSeek(impl_POA_Alliance_IOSK * servant,
			      Alliance_IOSK_Session id,
			      Alliance_IOSK_Address where,
			      Alliance_IOSK_SeekDir dir,
			      CORBA_Environment * ev);

void
 impl_Alliance_IOSK_lock(impl_POA_Alliance_IOSK * servant,
			 Alliance_IOSK_Session id,
			 Alliance_IOSK_Permission permission,
			 CORBA_Environment * ev);

/*** epv structures ***/
static PortableServer_ServantBase__epv impl_Alliance_IOSK_base_epv =
{
   NULL,			/* _private data */
   (gpointer) & impl_Alliance_IOSK__destroy,	/* finalize routine */
   NULL,			/* default_POA routine */
};
static POA_Alliance_IOSK__epv impl_Alliance_IOSK_epv =
{
   NULL,			/* _private */

   (gpointer) & impl_Alliance_IOSK_openSession,

   (gpointer) & impl_Alliance_IOSK_closeSession,

   (gpointer) & impl_Alliance_IOSK_read,

   (gpointer) & impl_Alliance_IOSK_write,

   (gpointer) & impl_Alliance_IOSK_writeAsync,

   (gpointer) & impl_Alliance_IOSK_readSeek,

   (gpointer) & impl_Alliance_IOSK_writeSeek,

   (gpointer) & impl_Alliance_IOSK_lock,

};

/*** vepv structures ***/
static POA_Alliance_IOSK__vepv impl_Alliance_IOSK_vepv =
{
   &impl_Alliance_IOSK_base_epv,
   &impl_Alliance_IOSK_epv,
};

/*** Stub implementations ***/
static Alliance_IOSK 
impl_Alliance_IOSK__create(PortableServer_POA poa, CORBA_Environment * ev)
{
   Alliance_IOSK retval;
   impl_POA_Alliance_IOSK *newservant;
   PortableServer_ObjectId *objid;

   newservant = g_new0(impl_POA_Alliance_IOSK, 1);
   newservant->servant.vepv = &impl_Alliance_IOSK_vepv;
   newservant->poa = poa;
   POA_Alliance_IOSK__init((PortableServer_Servant) newservant, ev);
   objid = PortableServer_POA_activate_object(poa, newservant, ev);
   CORBA_free(objid);
   retval = PortableServer_POA_servant_to_reference(poa, newservant, ev);

   return retval;
}

/* You shouldn't call this routine directly without first deactivating the servant... */
static void
impl_Alliance_IOSK__destroy(impl_POA_Alliance_IOSK * servant, CORBA_Environment * ev)
{

   POA_Alliance_IOSK__fini((PortableServer_Servant) servant, ev);
   g_free(servant);
}

Alliance_IOSK_Session
impl_Alliance_IOSK_openSession(impl_POA_Alliance_IOSK * servant,
			       CORBA_char * device,
			       Alliance_IOSK_Permission permission,
			       CORBA_Environment * ev)
{
   /* How do I extract a unique identifier for the caller from the available
    * information? 
    */
   return IOSKopenSession(device, permission, 0);
   /* Need to throw an exception if the return isn't valid. */
}

void
impl_Alliance_IOSK_closeSession(impl_POA_Alliance_IOSK * servant,
				Alliance_IOSK_Session id,
				CORBA_Environment * ev)
{
	IOSKcloseSession(id, 0);
}

Alliance_IOSK_bytes *
impl_Alliance_IOSK_read(impl_POA_Alliance_IOSK * servant,
			Alliance_IOSK_Session id,
			CORBA_unsigned_long * length,
			CORBA_boolean block,
			CORBA_Environment * ev)
{
   Alliance_IOSK_bytes *retval;

   retval = Alliance_IOSK_bytes__alloc(); /* The ORB will free it if there is
                                           * no exception thrown.
                                           */
                        
   IOSKread(id, retval, length, block, 0);

   return retval;
}

void
impl_Alliance_IOSK_write(impl_POA_Alliance_IOSK * servant,
			 Alliance_IOSK_Session id,
			 Alliance_IOSK_bytes * buffer,
			 CORBA_Environment * ev)
{
    IOSKwrite(id, buffer, buffer->_length, 0);
}

void
impl_Alliance_IOSK_writeAsync(impl_POA_Alliance_IOSK * servant,
			      Alliance_IOSK_Session id,
			      Alliance_IOSK_bytes * buffer,
			      CORBA_Environment * ev)
{
    IOSKwrite(id, buffer, buffer->_length, 0);
}

void
impl_Alliance_IOSK_readSeek(impl_POA_Alliance_IOSK * servant,
			    Alliance_IOSK_Session id,
			    Alliance_IOSK_Address where,
			    Alliance_IOSK_SeekDir dir,
			    CORBA_Environment * ev)
{
    IOSKreadSeek(id, where, dir, 0);
}

void
impl_Alliance_IOSK_writeSeek(impl_POA_Alliance_IOSK * servant,
			     Alliance_IOSK_Session id,
			     Alliance_IOSK_Address where,
			     Alliance_IOSK_SeekDir dir,
			     CORBA_Environment * ev)
{
    IOSKwriteSeek(id, where, dir, 0);
}

void
impl_Alliance_IOSK_lock(impl_POA_Alliance_IOSK * servant,
			Alliance_IOSK_Session id,
			Alliance_IOSK_Permission permission,
			CORBA_Environment * ev)
{
    IOSKlock(id, permission, 0);
}

int
main (int argc, char *argv[])
{
    PortableServer_POA poa;
    CORBA_Environment ev;
    CORBA_ORB orb;
    Alliance_IOSK iosk;
    CORBA_char* ior;
 
    g_print("Initializing ORB...\n");
    CORBA_exception_init(&ev);
    orb = CORBA_ORB_init(&argc, argv, "orbit-local-orb", &ev);
    poa = (PortableServer_POA)CORBA_ORB_resolve_initial_references(orb,
                                                                   "RootPOA",
                                                                   &ev);
    PortableServer_POAManager_activate(
        PortableServer_POA__get_the_POAManager(poa, &ev),
        &ev);
 
    g_print("Initializing IOSK...\n");
    iosk = impl_Alliance_IOSK__create(poa, &ev);
    IOSKinit();
    
    ior = CORBA_ORB_object_to_string(orb, iosk, &ev);
                                                                   
    g_print("IOR is:\n%s\n", ior);
        
    CORBA_free(ior);
 
    g_print("Running ORB; the IOSK is now online.\n");
    CORBA_ORB_run(orb, &ev);
    
    return 0;
}

