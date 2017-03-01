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


#include "shared.h"

#include "filesys.h"

#include "fat.h"

static int num_clust;
static int mapblock;
static int data_offset;
static int fat_size;

/* pointer(s) into filesystem info buffer for DOS stuff */
#define BPB     ( FSYS_BUF + 32256 )  /* 512 bytes long */
#define FAT_BUF ( FSYS_BUF + 30208 )  /* 4 sector FAT buffer */

int
fat_mount(void)
{
  int retval = 1;

  if ( (((current_drive & 0x80) || (current_slice != 0))
	&& (current_slice != PC_SLICE_TYPE_FAT12)
	&& (current_slice != PC_SLICE_TYPE_FAT16_LT32M)
	&& (current_slice != PC_SLICE_TYPE_FAT16_GT32M)
	&& (current_slice != (PC_SLICE_TYPE_BSD | (FS_MSDOS<<8))))
       || !devread(0, 0, SECTOR_SIZE, BPB)
       || FAT_BPB_BYTES_PER_SECTOR(BPB) != SECTOR_SIZE
       || FAT_BPB_SECT_PER_CLUST(BPB) < 1 || FAT_BPB_SECT_PER_CLUST(BPB) > 64
       || (FAT_BPB_SECT_PER_CLUST(BPB) & (FAT_BPB_SECT_PER_CLUST(BPB) - 1))
       || !( (current_drive & 0x80)
	     || FAT_BPB_FLOPPY_NUM_SECTORS(BPB) ) )
    retval = 0;
  else
    {
      mapblock = -4096;
      data_offset = FAT_BPB_DATA_OFFSET(BPB);
      num_clust = FAT_BPB_NUM_CLUST(BPB) + 2;
      if (num_clust > FAT_MAX_12BIT_CLUST)
	fat_size = 4;
      else
	fat_size = 3;
    }

  return retval;
}


static int
fat_create_blocklist(int first_fat_entry)
{
  BLK_CUR_FILEPOS = 0;
  BLK_CUR_BLKNUM = 0;
  BLK_CUR_BLKLIST = BLK_BLKLIST_START;
  block_file = 1;
  filepos = 0;

  if (first_fat_entry < 0)
    {
      /* root directory */

      BLK_BLKSTART(BLK_BLKLIST_START) = FAT_BPB_ROOT_DIR_START(BPB);
      fsmax = filemax = SECTOR_SIZE * (BLK_BLKLENGTH(BLK_BLKLIST_START)
				       = FAT_BPB_ROOT_DIR_LENGTH(BPB));
    }
  else    /* any real directory/file */
    {
      int blk_cur_blklist = BLK_BLKLIST_START, blk_cur_blknum;
      int last_fat_entry, new_mapblock;

      fsmax = 0;

      do
	{
	  BLK_BLKSTART(blk_cur_blklist)
	    = (first_fat_entry-2) * FAT_BPB_SECT_PER_CLUST(BPB) + data_offset;
	  blk_cur_blknum = 0;

	  do
	    {
	      blk_cur_blknum += FAT_BPB_SECT_PER_CLUST(BPB);
	      last_fat_entry = first_fat_entry;

	      /*
	       *  Do FAT table translation here!
	       */

	      new_mapblock = (last_fat_entry * fat_size) >> 1;
	      if (new_mapblock > (mapblock + 2045)
		  || new_mapblock < (mapblock + 3))
		{
		  mapblock = ( (new_mapblock < 6) ? 0 :
			       ((new_mapblock - 6) & ~0x1FF) );
		  if (!devread((mapblock>>9)+FAT_BPB_FAT_START(BPB),
			       0, SECTOR_SIZE * 4, FAT_BUF))
		    return 0;
		}

	      first_fat_entry
		= *((unsigned short *) (FAT_BUF + (new_mapblock - mapblock)));

	      if (num_clust <= FAT_MAX_12BIT_CLUST)
		{
		  if (last_fat_entry & 1)
		    first_fat_entry >>= 4;
		  else
		    first_fat_entry &= 0xFFF;
		}

	      if (first_fat_entry < 2)
		{
		  errnum = ERR_FSYS_CORRUPT;
		  return 0;
		}
	    }
	  while (first_fat_entry == (last_fat_entry + 1)
		 && first_fat_entry < num_clust);

	  BLK_BLKLENGTH(blk_cur_blklist) = blk_cur_blknum;
	  fsmax += blk_cur_blknum * SECTOR_SIZE;
	  blk_cur_blklist += BLK_BLKLIST_INC_VAL;
	}
      while (first_fat_entry < num_clust && blk_cur_blklist < (FAT_BUF - 7));
    }

  return 1;
}


/* XX FAT filesystem uses the block-list filesystem read function,
   so none is defined here.  */


int
fat_dir(char *dirname)
{
  char *rest, ch, filename[13], dir_buf[FAT_DIRENTRY_LENGTH];
  int attrib = FAT_ATTRIB_DIR, map = -1;

/* main loop to find desired directory entry */
loop:

  if (!fat_create_blocklist(map))
    return 0;

  /* if we have a real file (and we're not just printing possibilities),
     then this is where we want to exit */

  if (!*dirname || isspace(*dirname))
    {
      if (attrib & FAT_ATTRIB_DIR)
	{
	  errnum = ERR_BAD_FILETYPE;
	  return 0;
	}

      return 1;
    }

  /* continue with the file/directory name interpretation */

  while (*dirname == '/')
    dirname++;

  filemax = fsmax;

  if (!filemax || !(attrib & FAT_ATTRIB_DIR))
    {
      errnum = ERR_BAD_FILETYPE;
      return 0;
    }

  for (rest = dirname; (ch = *rest) && !isspace(ch) && ch != '/'; rest++) ;

  *rest = 0;

  do
    {
      if (read((int)dir_buf, FAT_DIRENTRY_LENGTH) != FAT_DIRENTRY_LENGTH)
	{
	  if (!errnum)
	    {
	      if (print_possibilities < 0)
		{
		  print("\n");
		  return 1;
		}

	      errnum = ERR_FILE_NOT_FOUND;
	      *rest = ch;
	    }

	  return 0;
	}

      if (!FAT_DIRENTRY_VALID(dir_buf))
	continue;

      /* XXX convert to 8.3 filename format here */
      {
	int i, j, c;

	for (i = 0; i < 8 && (c = filename[i] = tolower(dir_buf[i]))
	       && !isspace(c) ; i++) ;

	filename[i++] = '.';

	for (j = 0; j < 3 && (c = filename[i+j] = tolower(dir_buf[8+j]))
	       && !isspace(c) ; j++) ;

	if (j == 0)
	  i--;

	filename[i+j] = 0;
      }

      if (print_possibilities && ch != '/'
	  && (!*dirname || strcmp(dirname, filename) <= 0))
	{
	  if (print_possibilities > 0)
	    print_possibilities = -print_possibilities;
	  print("  %s", filename);
	}
    }
  while (strcmp(dirname, filename) != 0 || (print_possibilities && ch != '/'));

  *(dirname = rest) = ch;

  attrib = FAT_DIRENTRY_ATTRIB(dir_buf);
  filemax = FAT_DIRENTRY_FILELENGTH(dir_buf);
  map = FAT_DIRENTRY_FIRST_CLUSTER(dir_buf);

  /* go back to main loop at top of function */
  goto loop;
}


