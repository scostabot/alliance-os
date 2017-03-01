/*
 * ctg.c:  Check fragmentation on files
 *
 * (C) 1999 Ramon van Handel, The Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 15/05/99  ramon       1.0    First release
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


/* Linux includes */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>


int main(int argc, char *argv[])
/*
 * Main program for fragmentation analysis
 * Usage:  ctg [-d] <filename> [-d]
 *
 * INPUT:
 *     argc:  Amount of cmdline parameters
 *     argv:  The cmdline parameters
 *
 * RETURNS:
 *     -1  on error
 *      0  if the file is defragmented / successful blocklist dump
 *      1  if the file is fragmented
 */
{
    int block, lblock, blksize, fd, i, n;
    struct stat s;

    /* Check that we have the right amount of cmdline parameters */
    if(argc < 2 || argc > 3) {
        printf("Usage:  ctg [-d] <filename> [-d]\n");
        exit(-1);
    }

    /* Find the filename and whether we have to dump the blocklist */
    if(argc == 3) {
        if(!strcmp(argv[1], "-d"))
            n = 2;
        else if(!strcmp(argv[2], "-d"))
            n = 1;
        else {
            printf("Usage:  ctg [-d] <filename> [-d]\n");
            exit(-1);
        }
    } else {
        n = 1;
    }

    /* Open the file to analyze */
    fd = open(argv[n], 0);
    if(fd == -1) {
        printf("open() on file %s failed, errno=%d\n", argv[n], errno);
        exit(-1);
    }

    /* Find the amount of blocks it occupies */
    if(fstat(fd, &s) == -1) {
        printf("stat() on file %s failed, errno=%d\n", argv[n], errno);
        exit(-1);
    }

    /* Get the kernel blocksize */
    if(ioctl(fd, FIGETBSZ, &blksize) < 0) {
        printf("Blocksize fetch on file %s failed\n", argv[n]);
        exit(-1);
    }

    /* Verbose output if we print the blocklist */
    if(argc == 3)
        printf("Absolute Blocklist of %s:\n", argv[n]);

    /* Loop through the file's blocks */
    for(i=0; i<(1+s.st_size/blksize); i++) {
        block = i;

        /* Find the absolute block location on the disk */
        if(ioctl(fd, FIBMAP, &block) < 0) {
            printf("Blockmap fetch on file %s failed\n", argv[n]);
            exit(-1);
        }

        /* Show blocklist or check fragmentation */
        if(argc == 2) {
            if(i && ((block-lblock) != 1)) {
                printf("File %s is fragmented\n", argv[n]);
                exit(1);
            }
            lblock = block;
        } else {
            printf("%d", block);
            if(i%8 != 7)
                printf("\t");
            else
                printf("\n");
        }
    }

    /* Final print */
    if(argc == 2)
        printf("File %s is defragmented\n", argv[n]);
    else if((i-1)%8 != 7)
        printf("\n");

    /* Success ! */
    exit(0);
}
