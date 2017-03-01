/*
 * <One-line functional unit description>
 *
 * <High level description of the functions implemented in this file>
 *
 * HISTORY
 * Date Author Rev Notes
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation. 
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
 * See the GNU General Public License for more details. 
 */
#include <typewrappers.h>
#include <IPC.h>
#include <stdio.h>
#include <unistd.h>

int main () {
  IPCBufferId buffer;
  STRING      data=0x0; /* We send only one char in each message */
  INT32       error = 0;
  INT32       arrived = 0;

  printf ("2. Waiting for syncornization message\n");
  buffer = ipcBufferGetSync ();  /* wait until first message is sent */
  printf ("\n Sync message got \n");
  ipcBufferDone(buffer); /* discard first message */
  printf ("... and discarded. \n");
  

  while (data!='!') {
    buffer = ipcBufferGetSync();

    if (!isIPCNOBUFFER(buffer)) {
      data = *((char*)ipcBufferGetAddr(buffer)); /* get the char */
      arrived ++;
      printf ("%u\n", arrived);
      if ((error=ipcBufferDone (buffer)) != 0)
	printf ("X%iX", error);
    } 
  }
  printf ("\n About to exit...\n");
  return 0;
}

