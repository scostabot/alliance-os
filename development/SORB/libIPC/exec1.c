/*
 * One executable sending messages
 *
 * This executable forks to launch the receiver (so it can know the receiver's
 * pid)
 *
 * HISTORY
 * Date          Author Rev Notes
 * Oct,10th,1998 Luc    1   Sample code
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
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define ERRNOFORK    1
#define ERRCANTEXEC  2

VOID fatalError (const INT16 num) {
  /* Central error handling function
   * INPUT:
   *  num : error number to report
   * OUTPUT
   *  None, program exits with error code "num"
   */
  printf ("\n");
  switch (num) {
  case ERRNOFORK :
    printf ("Could not fork...");
    break;
  case ERRCANTEXEC :
    printf ("Could not execute...");
    break;
  default:
    printf ("Unrecognized error...");
  }
  printf ("\nExiting.\n");
  exit (num);
}

ProcessId run (const STRING * file) {
  /* Program forks and runs the given executable file in one of the processes
   * INPUT:
   *  file : string containing the name (w/ path) of the file to execute
   * OUTPUT
   *  PID of the started executable
   */
  INT32   pid = fork();

  switch (pid) {
  case -1: /* Error, could not fork */
    fatalError(ERRNOFORK);
    break;
  case 0 : /* Forked process */
    if ((execlp(file, file))==-1) /* try to run the given file */
      fatalError(ERRCANTEXEC);    /* on error, report it and exit */
  default: /* Parent process */
    printf ("Running server with pid: %li\n", pid);
  }
  return pid; /* return the pid of the executable */
}


INT16 main (int argc, char ** argv) {
  IPCBufferId aBuff;
  STRING      line[80];
  UBYTE       i;
  int         result;
  ProcessId   pid;
  FILE *      srcFile;

  pid = run (argv[1]);
  if (argc < 3) 
    srcFile = fopen ("aFile", "r");  /* open file for reading */
  else
    srcFile = fopen (argv[2], "r");  /* open file for reading */

	/* Sincronization */
  aBuff = ipcBufferAllocate(1);
  *((char *) ipcBufferGetAddr(aBuff)) = 'S';
  ipcBufferSendSync(aBuff, pid);
  if (ipcBufferDone(aBuff) != 0)
    printf ("\n ------------------ Something wrong happened!\n");


  result = fscanf (srcFile, "%s", line);
  while (result!=EOF) {
    for (i=0; i<strlen(line); i++) {
      aBuff = ipcBufferAllocate(1);
      *((char *) ipcBufferGetAddr(aBuff)) = line[i];
      ipcBufferSendAsync(aBuff, pid);
      if (ipcBufferDone(aBuff) != 0)
	printf ("\n ------------------ Something wrong happened!\n");
    }
    if (line[strlen(line)-1] != '!') {
	    aBuff = ipcBufferAllocate(1);  printf("abuff = %p    ",aBuff);
	    *((char *) ipcBufferGetAddr(aBuff)) = '\n';
	    ipcBufferSendSync(aBuff, pid);
	    if (ipcBufferDone(aBuff) != 0)
	      printf ("\n ------------------ Something wrong happened!\n");
	}

    result = fscanf (srcFile, "%s", line);
  }
  
  return 0;
}


