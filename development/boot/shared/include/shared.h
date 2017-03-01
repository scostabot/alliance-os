/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 1996   Erich Boleyn  <erich@uruk.org>
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

/*
 *  Generic defines to use anywhere
 */

/*
 *  Integer sizes
 */

#define MAXINT     0x7FFFFFFF

/*
 *  This is the location of the raw device buffer.  It is 31.5K
 *  in size.
 */

#define BUFFERSEG    0x7000
#define BUFFERADDR   0x70000

/* 512-byte scratch area */
#define SCRATCHSEG   0x77e0
#define SCRATCHADDR  0x77e00

/*
 *  BIOS disk defines
 */
#define BIOSDISK_SUBFUNC_READ       0x0
#define BIOSDISK_SUBFUNC_WRITE      0x1
#define BIOSDISK_ERROR_GEOMETRY     0x100

/*
 *  This is the location of the filesystem (not raw device) buffer.
 *  It is 32K in size, do not overrun!
 */

#define FSYS_BUF 0x68000


/*
 *  General disk stuff
 */

#define SECTOR_SIZE          0x200
#define BIOS_FLAG_FIXED_DISK 0x80

#define BOOTSEC_LOCATION     0x7C00
#define BOOTSEC_SIGNATURE    0xAA55
#define BOOTSEC_BPB_OFFSET   0x3
#define BOOTSEC_BPB_LENGTH   0x3B
#define BOOTSEC_PART_OFFSET  0x1BE
#define BOOTSEC_PART_LENGTH  0x40
#define BOOTSEC_SIG_OFFSET   0x1FE


/*
 *  defines for use when switching between real and protected mode
 */

#define CR0_PE_ON       0x1
#define CR0_PE_OFF      0xfffffffe
#define PROT_MODE_CSEG  0x8
#define PROT_MODE_DSEG  0x10
#define PSEUDO_RM_CSEG  0x18
#define PSEUDO_RM_DSEG  0x20
#define STACKOFF        0x2000 - 0x10
#define PROTSTACKINIT   FSYS_BUF - 0x10


/*
 * Assembly code defines
 */

#define ENTRY(x) .globl x ; x ## :
#define VARIABLE(x) ENTRY(x)


#ifndef ASM_FILE

/* multiboot stuff */

#include <multiboot.h>

#define NULL         ((void *) 0)


/*
 *  From "asm.S"
 */

void putchar(int c);

/* low-level disk I/O */
int get_diskinfo(int drive);
int biosdisk(int subfunc, int drive, int geometry,
	     int sector, int nsec, int segment);


/*
 * Errors
 */

#define ERR_NONE            0
#define ERR_WONT_FIT        (ERR_NONE + 1)
#define ERR_NO_DISK         (ERR_WONT_FIT + 1)
#define ERR_READ            (ERR_NO_DISK + 1)
#define ERR_WRITE           (ERR_READ + 1)
#define ERR_GEOM            (ERR_WRITE + 1)
#define ERR_OUTSIDE_PART    (ERR_GEOM + 1)
#define ERR_BAD_PART_TABLE  (ERR_OUTSIDE_PART + 1)
#define ERR_NO_PART         (ERR_BAD_PART_TABLE + 1)
#define ERR_BAD_FILENAME    (ERR_NO_PART + 1)
#define ERR_BAD_FILETYPE    (ERR_BAD_FILENAME + 1)
#define ERR_FILE_NOT_FOUND  (ERR_BAD_FILETYPE + 1)
#define ERR_SYMLINK_LOOP    (ERR_FILE_NOT_FOUND + 1)
#define ERR_FSYS_MOUNT      (ERR_SYMLINK_LOOP + 1)
#define ERR_FSYS_CORRUPT    (ERR_FSYS_MOUNT + 1)
#define ERR_FILELENGTH      (ERR_FSYS_CORRUPT + 1)
#define ERR_NUMBER_PARSING  (ERR_FILELENGTH + 1)
#define ERR_DEV_FORMAT      (ERR_NUMBER_PARSING + 1)
#define ERR_DEV_VALUES      (ERR_DEV_FORMAT + 1)
#define ERR_EXEC_FORMAT     (ERR_DEV_VALUES + 1)
#define ERR_BELOW_1MB       (ERR_EXEC_FORMAT + 1)
#define ERR_BOOT_FEATURES   (ERR_BELOW_1MB + 1)
#define ERR_BOOT_FAILURE    (ERR_BOOT_FEATURES + 1)
#define ERR_NEED_MB_KERNEL  (ERR_BOOT_FAILURE + 1)
#define ERR_NEED_LX_KERNEL  (ERR_NEED_MB_KERNEL + 1)
#define ERR_BOOT_COMMAND    (ERR_NEED_LX_KERNEL + 1)
#define ERR_UNRECOGNIZED    (ERR_BOOT_COMMAND + 1)
#define ERR_BAD_GZIP_HEADER (ERR_UNRECOGNIZED + 1)
#define ERR_BAD_GZIP_DATA   (ERR_BAD_GZIP_HEADER + 1)
#define ERR_BAD_VERSION     (ERR_BAD_GZIP_DATA + 1)
#define MAX_ERR_NUM         (ERR_BAD_VERSION + 1)


/* like 'getkey', but doesn't wait, returns -1 if nothing available */
int checkkey(void);
int getkey(void);


#define tolower(c) ((c >= 'A' && c <= 'Z') ? (c + ('a' - 'A')) : c)
#define isspace(c) ((c == ' ' || c == '\t' || c == '\n') ? 1 : 0)

void print(char *format, ... );
int sprint(char *ibuf, char *format, ... );
int strcmp(char *s1, char *s2);
int bcopy(char *from, char *to, int len);
int bzero(char *start, int len);
int safe_parse_maxint(char **str_ptr, int *myint_ptr);

void __print_error(const char *fl, const long ln);
#define print_error() __print_error(__FILE__,__LINE__)

/*
 *  From "gunzip.c"
 */

extern int no_decompression;
extern int compressed_file;

int gunzip_test_header(void);
int gunzip_read(int addr, int len);


/*
 *  From "disk_io.c"
 */

/* instrumentation variables */
extern void (*debug_fs)(int);
extern void (*debug_fs_func)(int);

extern unsigned long current_drive;
extern unsigned long current_partition;
extern unsigned long saved_drive;
extern unsigned long saved_partition;

#ifndef NO_BLOCK_FILES
extern int block_file;
#endif  /* NO_BLOCK_FILES */

extern long part_start;
extern long part_length;

extern int current_slice;

extern int buf_drive;
extern int buf_track;
extern int buf_geom;

/* these are the current file position and maximum file position */
extern int filepos;
extern int filemax;

int rawread(int drive, int sector, int byte_offset, int byte_len, int addr);
int devread(int sector, int byte_offset, int byte_len, int addr);

char *set_device(char *device);  /* this gets a device from the string and
				    places it into the global parameters */
int open_device(void);
int make_saved_active(void);   /* sets the active partition to the that
				  represented by the "saved_" parameters */

int open(char *filename);
int read(int addr, int len);  /* if "length" is -1, read all the
				 remaining data in the file */
int dir(char *dirname);       /* list directory, printing all completions */

int set_bootdev(int hdbias);
void print_fsys_type(void);   /* this prints stats on the currently
				 mounted filesystem */
void print_completions(char *filename); /* this prints device and filename
					   completions */
void copy_current_part_entry(int addr); /* copies the current partition data
					   to the desired address */



/*
 *  Common BIOS/boot data.
 */

extern struct multiboot_info *mbinf;

/*
 *  Error variables.
 */

extern int errnum;

#endif
