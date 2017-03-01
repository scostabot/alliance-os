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

/*
 *  These are used for determining if the command-line should ask the user
 *  to correct errors during scripts.
 */
int fallback;
char *password;

char *
skip_to(int after_equal, char *cmdline)
{
  while (*cmdline && (*cmdline != (after_equal ? '=' : ' ')))
    cmdline++;

  if (after_equal)
    cmdline++;

  while (*cmdline == ' ')
    cmdline++;

  return cmdline;
}


void
init_cmdline(void)
{
  print(" [ Minimal BASH-like line editing is supported.  For the first word, TAB
   lists possible command completions.  Anywhere else TAB lists the possible
   completions of a device/filename.  ESC at any time exits. ]\n");
}

char commands[] =
 " Possible commands are: \"pause= ...\", \"uppermem= <kbytes>\", \"root= <device>\",
  \"rootnoverify= <device>\", \"chainloader= <file>\", \"kernel= <file> ...\",
  \"module= <file> ...\", \"modulenounzip= <file> ...\", \"makeactive\", \"boot\", and
  \"install= <stage1_file> [d] <dest_dev> <file> <addr> [p] [<config_file>]\"\n";

static int installaddr, installlist, installsect;

static void
debug_fs_blocklist_func(int sector)
{
  if (*((unsigned long *)(installlist-4))
      + *((unsigned short *)installlist) != sector
      || installlist == BOOTSEC_LOCATION+STAGE1_FIRSTLIST+4)
    {
      installlist -= 8;

      if (*((unsigned long *)(installlist-8)))
	errnum = ERR_WONT_FIT;
      else
	{
	  *((unsigned short *)(installlist+2)) = (installaddr >> 4);
	  *((unsigned long *)(installlist-4)) = sector;
	}
    }

  *((unsigned short *)installlist) += 1;
  installsect = sector;
  installaddr += 512;
}


int
enter_cmdline(char *script, char *heap)
{
  int bootdev, cmd_len, type = 0, run_cmdline = 1, have_run_cmdline = 0;
  char *cur_heap = heap, *cur_entry = script, *old_entry;

  /* initialization */
  saved_drive = boot_drive;
  saved_partition = install_partition;
  current_drive = 0xFF;
  errnum = 0;

  /* restore memory probe state */
  mbi.mem_upper = saved_mem_upper;
  if (mbi.mmap_length)
    mbi.flags |= MB_INFO_MEM_MAP;

  /* BSD and chainloading evil hacks !! */
  bootdev = set_bootdev(0);

  if (!script)
    {
      init_page();
      init_cmdline();
    }

restart:
  if (script)
    {
      if (errnum)
	{
	  if (fallback < 0)
	    goto returnit;
	  print_error();
	  if (password || errnum == ERR_BOOT_COMMAND)
	    {
	      print("Press any key to continue...");
	      getkey();
returnit:
	      return 0;
	    }

	  run_cmdline = 1;
	  if (!have_run_cmdline)
	    {
	      have_run_cmdline = 1;
	      putchar('\n');
	      init_cmdline();
	    }
	}
      else
	{
	  run_cmdline = 0;

	  /* update position in the boot script */
	  old_entry = cur_entry;
	  while (*(cur_entry++));

	  /* copy to work area */
	  bcopy(old_entry, cur_heap, ((int)cur_entry) - ((int)old_entry));

	  print("%s\n", old_entry);
	}
    }
  else
    {
      cur_heap[0] = 0;
      print_error();
    }

  if (run_cmdline && get_cmdline("command> ", commands, cur_heap, 2048))
    return 1;

  if (strcmp("boot", cur_heap) == 0 || (script && !*cur_heap))
    {
      if ((type == 'f') | (type == 'n'))
	bsd_boot(type, bootdev);
      if (type == 'l')
	linux_boot();
      if (type == 'L')
	big_linux_boot();

      if (type == 'c')
	{
	  gateA20(0);
	  boot_drive = saved_drive;
	  chain_stage1(0, BOOTSEC_LOCATION, BOOTSEC_LOCATION-16);
	}

      if (!type)
	{
	  errnum = ERR_BOOT_COMMAND;
	  goto restart;
	}

      /* this is the final possibility */
      multi_boot((int)entry_addr, (int)(&mbi));
    }

  /* get clipped command-line */
  cur_cmdline = skip_to(1, cur_heap);
  cmd_len = 0;
  while (cur_cmdline[cmd_len++]);

  if (strcmp("chainloader", cur_heap) < 1)
    {
      if (open(cur_cmdline) && (read(BOOTSEC_LOCATION, SECTOR_SIZE) 
				== SECTOR_SIZE)
	  && (*((unsigned short *) (BOOTSEC_LOCATION+BOOTSEC_SIG_OFFSET))
	      == BOOTSEC_SIGNATURE))
	type = 'c';
      else if (!errnum)
	{
	  errnum = ERR_EXEC_FORMAT;
	  type = 0;
	}
    }
  else if (strcmp("pause", cur_heap) < 1)
    {
      if (getc() == 27)
	return 1;
    }
  else if (strcmp("uppermem", cur_heap) < 1)
    {
      if (safe_parse_maxint(&cur_cmdline, (int *)&(mbi.mem_upper)))
	mbi.flags &= ~MB_INFO_MEM_MAP;
    }
  else if (strcmp("root", cur_heap) < 1)
    {
      int hdbias = 0;
      char *biasptr = skip_to(0, set_device(cur_cmdline));

      /* this will respond to any "rootn<XXX>" command,
	 but that's OK */
      if (!errnum && (cur_heap[4] == 'n' || open_device()
		      || errnum == ERR_FSYS_MOUNT))
	{
	  errnum = 0;
	  saved_partition = current_partition;
	  saved_drive = current_drive;

	  if (cur_heap[4] != 'n')
	    {
	      /* BSD and chainloading evil hacks !! */
	      safe_parse_maxint(&biasptr, &hdbias);
	      errnum = 0;
	      bootdev = set_bootdev(hdbias);

	      print_fsys_type();
	    }
	  else
	    current_drive = -1;
	}
    }
  else if (strcmp("kernel", cur_heap) < 1)
    {
      /* make sure it's at the beginning of the boot heap area */
      bcopy(cur_heap, heap, cmd_len + (((int)cur_cmdline) - ((int)cur_heap)));
      cur_cmdline = heap + (((int)cur_cmdline) - ((int)cur_heap));
      cur_heap = heap;
      if ((type = load_image()))
	cur_heap = cur_cmdline + cmd_len;
    }
  else if (strcmp("module", cur_heap) < 1)
    {
      if (type == 'm')
	{
#ifndef NO_DECOMPRESSION
	  /* this will respond to any "modulen<XXX>" command,
	     but that's OK */
	  if (cur_heap[6] == 'n')
	    no_decompression = 1;
#endif  /* NO_DECOMPRESSION */

	  if (load_module())
	    cur_heap = cur_cmdline + cmd_len;

#ifndef NO_DECOMPRESSION
	  no_decompression = 0;
#endif  /* NO_DECOMPRESSION */
	}
      else if (type == 'L' || type == 'l')
	{
	  load_initrd();
	}
      else
	errnum = ERR_NEED_MB_KERNEL;
    }
  else if (strcmp("initrd", cur_heap) < 1)
    {
      if (type == 'L' || type == 'l')
	{
	  load_initrd();
	}
      else
	errnum = ERR_NEED_LX_KERNEL;
    }
  else if (strcmp("install", cur_heap) < 1)
    {
      char *stage1_file = cur_cmdline, *dest_dev, *file, *addr;
      char buffer[SECTOR_SIZE], old_sect[SECTOR_SIZE];
      int new_drive = 0xFF;

      dest_dev = skip_to(0, stage1_file);
      if (*dest_dev == 'd')
	{
	  new_drive = 0;
	  dest_dev = skip_to(0, dest_dev);
	}
      file = skip_to(0, dest_dev);
      addr = skip_to(0, file);

      if (safe_parse_maxint(&addr, &installaddr) && open(stage1_file)
	  && read((int)buffer, SECTOR_SIZE) == SECTOR_SIZE
	  && set_device(dest_dev) && open_partition()
	  && devread(0, 0, SECTOR_SIZE, (int)old_sect))
	{
	  int dest_drive = current_drive, dest_geom = buf_geom;
	  int dest_sector = part_start, i;

#ifndef NO_DECOMPRESSION
	  no_decompression = 1;
#endif

	  /* copy possible DOS BPB, 59 bytes at byte offset 3 */
	  bcopy(old_sect+BOOTSEC_BPB_OFFSET, buffer+BOOTSEC_BPB_OFFSET,
		BOOTSEC_BPB_LENGTH);

	  /* if for a hard disk, copy possible MBR/extended part table */
	  if ((dest_drive & 0x80) && current_partition == 0xFFFFFF)
	    bcopy(old_sect+BOOTSEC_PART_OFFSET, buffer+BOOTSEC_PART_OFFSET,
		  BOOTSEC_PART_LENGTH);

	  if (*((short *)(buffer+STAGE1_VER_MAJ_OFFS)) != COMPAT_VERSION
	      || (*((unsigned short *) (buffer+BOOTSEC_SIG_OFFSET))
		  != BOOTSEC_SIGNATURE)
	      || (!(dest_drive & 0x80)
		  && (*((unsigned char *) (buffer+BOOTSEC_PART_OFFSET)) == 0x80
		      || buffer[BOOTSEC_PART_OFFSET] == 0)))
	    {
	      errnum = ERR_BAD_VERSION;
	    }
	  else if (open(file))
	    {
	      if (!new_drive)
		new_drive = current_drive;

	      bcopy(buffer, (char*)BOOTSEC_LOCATION, SECTOR_SIZE);

	      *((unsigned char *)(BOOTSEC_LOCATION+STAGE1_FIRSTLIST))
		= new_drive;
	      *((unsigned short *)(BOOTSEC_LOCATION+STAGE1_INSTALLADDR))
		= installaddr;

	      i = BOOTSEC_LOCATION+STAGE1_FIRSTLIST-4;
	      while (*((unsigned long *)i))
		{
		  if (i < BOOTSEC_LOCATION+STAGE1_FIRSTLIST-256
		      || (*((int *)(i-4)) & 0x80000000)
		      || *((unsigned short *)i) >= 0xA00
		      || *((short *) (i+2)) == 0)
		    {
		      errnum = ERR_BAD_VERSION;
		      break;
		    }

		  *((int *)i) = 0;
		  *((int *)(i-4)) = 0;
		  i -= 8;
		}

	      installlist = BOOTSEC_LOCATION+STAGE1_FIRSTLIST+4;
	      debug_fs = debug_fs_blocklist_func;

	      if (!errnum && read(SCRATCHADDR, SECTOR_SIZE) == SECTOR_SIZE)
		{
		  if (*((long *)SCRATCHADDR) != 0x8070ea
		      || (*((short *)(SCRATCHADDR+STAGE2_VER_MAJ_OFFS))
			  != COMPAT_VERSION))
		    errnum = ERR_BAD_VERSION;
		  else
		    {
		      int write_stage2_sect = 0, stage2_sect = installsect;
		      char *ptr;

		      ptr = skip_to(0, addr);

		      if (*ptr == 'p')
			{
			  write_stage2_sect++;
			  *((long *)(SCRATCHADDR+STAGE2_INSTALLPART))
			    = current_partition;
			  ptr = skip_to(0, ptr);
			}
		      if (*ptr)
			{
			  char *str
			    = ((char *) (SCRATCHADDR+STAGE2_VER_STR_OFFS));

			  write_stage2_sect++;
			  while (*(str++));      /* find string */
			  while ((*(str++) = *(ptr++)));    /* do copy */
			}

		      read(0x100000, -1);

		      buf_track = -1;

		      if (!errnum
			  && (biosdisk(BIOSDISK_SUBFUNC_WRITE,
				       dest_drive, dest_geom,
				       dest_sector, 1, (BOOTSEC_LOCATION>>4))
			      || (write_stage2_sect
				  && biosdisk(BIOSDISK_SUBFUNC_WRITE,
					      current_drive, buf_geom,
					      stage2_sect, 1, SCRATCHSEG))))
			  errnum = ERR_WRITE;
		    }
		}

	      debug_fs = NULL;
	    }

#ifndef NO_DECOMPRESSION
	  no_decompression = 0;
#endif
	}
    }
  else if (strcmp("makeactive", cur_heap) == 0)
    make_saved_active();
  else if (*cur_heap && *cur_heap != ' ')
    errnum = ERR_UNRECOGNIZED;

  goto restart;
}

