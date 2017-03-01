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

char *
get_entry(char *list, int num, int nested)
{
  int i;

  for (i = 0; i < num; i++)
    {
      do
	{
	  while (*(list++));
	}
      while (nested && *(list++));
    }

  return list;
}


void
print_entries(int y, int size, int first, char *menu_entries)
{
  int i;

  gotoxy(77, y+1);

  if (first)
    putchar(DISP_UP);
  else
    putchar(' ');

  get_entry(menu_entries, first, 0);

  for (i = 1; i <= size; i++)
    {
      int j = 0;

      gotoxy(3, y+i);

      while (*menu_entries)
	{
	  if (j < 71)
	    {
	      putchar(*menu_entries);
	      j++;
	    }

	  menu_entries++;
	}

      if (*(menu_entries-1))
	menu_entries++;

      for (; j < 71; j++)
	putchar(' ');
    }

  gotoxy(77, y+size);

  if (*menu_entries)
    putchar(DISP_DOWN);
  else
    putchar(' ');
}


void
print_border(int y, int size)
{
  int i;

  gotoxy(1, y);

  putchar(DISP_UL);
  for (i = 0; i < 73; i++)
    putchar(DISP_HORIZ);
  putchar(DISP_UR);

  i = 1;

  while (1)
    {
      gotoxy(1, y+i);

      if (i > size)
	break;

      putchar(DISP_VERT);
      gotoxy(75, y+i);
      putchar(DISP_VERT);

      i++;
    }

  putchar(DISP_LL);
  for (i = 0; i < 73; i++)
    putchar(DISP_HORIZ);
  putchar(DISP_LR);
}

void
set_line(int y, int attr)
{
  int x;

  for (x = 2; x < 75; x++)
    {
      gotoxy(x, y);
      set_attrib(attr);
    }
}


int timeout;


void
run_menu(char *menu_entries, char *config_entries, int num_entries,
	 char *heap, int entryno)
{
  int c, time1, time2 = -1, first_entry = 0;
  char *cur_entry;

  /*
   *  Main loop for menu UI.
   */

restart:
  while (entryno > 11)
    {
      first_entry++; entryno--;
    }

  init_page();

  print_border(3, 12);

  print("\n
      Use the \x18 and \x19 keys for selecting which entry is highlighted.\n");

  if (password)
    {
      print("       Press enter to boot the selected OS or \'p\' to enter a
        password to unlock the next set of features.");
    }
  else
    {
      if (config_entries)
	print("       Press enter to boot the selected OS, \'e\' to edit the
        commands before booting, or \'c\' for a command-line.");
      else
	print(
"      Press \'b\' to boot, enter to edit the selected command in the
      boot sequence, \'c\' for a command-line, \'o\' to open a new line
      after (\'O\' for before) the selected line, \'d\' to remove the
      selected line, or escape to go back to the main menu.");
    }

  print_entries(3, 12, first_entry, menu_entries);

  /* invert initial line */
  set_line(4+entryno, 0x70);

  /* XX using RT clock now, need to initialize value */
  while ((time1 = getrtsecs()) == 0xFF);

  while (1)
    {
      /* initilize to NULL just in case... */
      cur_entry = NULL;

      if (timeout >= 0 && (time1 = getrtsecs()) != time2 && time1 != 0xFF)
	{
	  if (timeout <= 0)
	    {
	      timeout = -1;
	      break;
	    }

	  /* else not booting yet! */
	  time2  = time1;
	  gotoxy(3, 22);
	  print("The highlighted entry will be booted automatically in %d seconds.    ", timeout);
	  gotoxy(74, 4+entryno);
	  timeout--;
	}

      if (checkkey() != -1)
	{
	  c = getkey();

	  if (timeout >= 0)
	    {
	      gotoxy(3, 22);
	      print("                                                                    ");
	      timeout = -1;
	      fallback = -1;
	      gotoxy(74, 4+entryno);
	    }

	  if ((c == KEY_UP) || (ASCII_CHAR(c) == 16))
	    {
	      if (entryno > 0)
		{
		  set_line(4+entryno, 0x7);
		  entryno--;
		  set_line(4+entryno, 0x70);
		}
	      else if (first_entry > 0)
		{
		  first_entry--;
		  print_entries(3, 12, first_entry, menu_entries);
		  set_line(4, 0x70);
		}
	    }
	  if (((c == KEY_DOWN) || (ASCII_CHAR(c) == 14))
	      && (first_entry+entryno+1) < num_entries)
	    {
	      if (entryno < 11)
		{
		  set_line(4+entryno, 0x7);
		  entryno++;
		  set_line(4+entryno, 0x70);
		}
	      else if (num_entries > 12+first_entry)
		{
		  first_entry++;
		  print_entries(3, 12, first_entry, menu_entries);
		  set_line(15, 0x70);
		}
	    }

	  c = ASCII_CHAR(c);

	  if (config_entries)
	    {
	      if ((c == '\n') || (c == '\r'))
		break;
	    }
	  else
	    {
	      if ((c == 'd') || (c == 'o') || (c == 'O'))
		{
		  set_line(4+entryno, 0x7);
		  /* insert after is almost exactly like insert before */
		  if (c == 'o')
		    {
		      entryno++;
		      c = 'O';
		    }

		  cur_entry = get_entry(menu_entries, first_entry+entryno, 0);

		  if (c == 'O')
		    {
		      bcopy(cur_entry, cur_entry+2,
			    ((int)heap) - ((int)cur_entry));

		      cur_entry[0] = ' ';
		      cur_entry[1] = 0;

		      heap += 2;

		      num_entries++;
		    }
		  else if (num_entries > 0)
		    {
		      char *ptr = get_entry(menu_entries,
					    first_entry+entryno+1, 0);
		      bcopy(ptr, cur_entry, ((int)heap) - ((int)ptr));
		      heap -= (((int)ptr) - ((int)cur_entry));

		      num_entries--;

		      if (entryno >= num_entries)
			entryno--;
		      if (first_entry && num_entries < 12+first_entry)
			first_entry--;
		    }

		  print_entries(3, 12, first_entry, menu_entries);
		  set_line(4+entryno, 0x70);
		}

	      cur_entry = menu_entries;
	      if (c == 27)
		return;
	      if (c == 'b')
		break;
	    }

	  if (password)
	    {
	      if (c == 'p')
		{
		  /* Do password check here! */
		  char *ptr = password;
		  gotoxy(2, 22);
		  print("Entering password...  ");
		  do
		    {
		      if (isspace(*ptr))
			{
			  char *new_file = config_file;
			  while (isspace(*ptr)) ptr++;
			  while ((*(new_file++) = *(ptr++)));
			  return;
			}
		      c = ASCII_CHAR(getkey());
		    }
		  while (*(ptr++) == c);
		  print("Failed!\n      Press any key to continue...");
		  getkey();
		  goto restart;
		}
	    }
	  else
	    {
	      if ((config_entries && (c == 'e'))
		  || (!config_entries && ((c == '\n') || (c == '\r'))))
		{
		  int num_entries = 0, i = 0;
		  char *new_heap;

		  if (config_entries)
		    {
		      new_heap = heap;
		      cur_entry = get_entry(config_entries,
					    first_entry+entryno, 1);
		    }
		  else
		    {
		      /* safe area! */
		      new_heap = heap+1501;
		      cur_entry = get_entry(menu_entries,
					    first_entry+entryno, 0);
		    }

		  do
		    {
		      while ((*(new_heap++) = cur_entry[i++]));
		      num_entries++;
		    }
		  while (config_entries && cur_entry[i]);

		  /* this only needs to be done if config_entries is non-NULL,
		     but it doesn't hurt to do it always */
		  *(new_heap++) = 0;

		  if (config_entries)
		    run_menu(heap, NULL, num_entries, new_heap, 0);
		  else
		    {
		      cls();
		      init_cmdline();

		      new_heap = heap+1501;

		      saved_drive = boot_drive;
		      saved_partition = install_partition;
		      current_drive = 0xFF;

		      if (!get_cmdline("editing> ", commands, new_heap, 1501))
			{
			  int j = 0;

			  /* get length of new command */
			  while (new_heap[j++]);

			  if (j < 2)
			    {
			      j = 2;
			      new_heap[0] = ' ';
			      new_heap[1] = 0;
			    }

			  /* align rest of commands properly */
			  bcopy(cur_entry+i, cur_entry+j,
				((int)heap) - (((int)cur_entry) + i));

			  /* copy command to correct area */
			  bcopy(new_heap, cur_entry, j);

			  heap += (j - i);
			}
		    }

		  goto restart;
		}
	      if (c == 'c')
		{
		  enter_cmdline(NULL, heap);
		  goto restart;
		}
	    }
	}
    }

  /*
   *  Attempt to boot an entry.
   */

  do
    {
      cls();

      if (config_entries)
	print("  Booting \'%s\'\n\n",
	       get_entry(menu_entries, first_entry+entryno, 0));
      else
	print("  Booting command-list\n\n");

      if (!cur_entry)
	cur_entry = get_entry(config_entries, first_entry+entryno, 1);

      if (!(c = enter_cmdline(cur_entry, heap)))
	{
	  cur_entry = NULL;
	  first_entry = 0;
	  entryno = fallback;
	  fallback = -1;
	}
    }
  while (!c);

  goto restart;
}


static int
get_line_from_config(char *cmdline, int maxlen)
{
  int pos = 0, literal = 0, comment = 0;
  char c;  /* since we're loading it a byte at a time! */

  while (read((int)&c, 1))
    {
      /* translate characters first! */
      if (c == '\\')
	{
	  literal = 1;
	  continue;
	}
      if (c == '\r')
	continue;
      if ((c == '\t') || (literal && (c == '\n')))
	c = ' ';

      literal = 0;

      if (comment)
	{
	  if (c == '\n')
	    comment = 0;
	}
      else if (!pos)
	{
	  if (c == '#')
	    comment = 1;
	  else if ((c != ' ') && (c != '\n'))
	    cmdline[pos++] = c;
	}
      else
	{
	  if (c == '\n')
	    break;

	  if (pos < maxlen)
	    cmdline[pos++] = c;
	}
    }

  cmdline[pos] = 0;

  return pos;
}


void
cmain(void)
{
  int config_len, menu_len, num_entries, default_entry;
  char *config_entries, *menu_entries;

  for (;;)
    {
      config_len = 0; menu_len = 0; num_entries = 0; default_entry = 0;
      config_entries = (char *)(mbi.mmap_addr + mbi.mmap_length);
      menu_entries = (char *)(BUFFERADDR + (32 * 1024));
      password = NULL; fallback = -1; timeout = -1;

      /*
       *  Here load the configuration file.
       */

      if (open(config_file))
	{
	  int state = 0, prev_config_len = 0, prev_menu_len = 0;
	  char cmdline[1502], *ptr;

	  while (get_line_from_config(cmdline, 1500))
	    {
	      ptr = skip_to(1, cmdline);

	      if (strcmp("title", cmdline) < 1)
		{
		  if (state > 1)
		    {
		      num_entries++;
		      config_entries[config_len++] = 0;
		      prev_menu_len = menu_len;
		      prev_config_len = config_len;
		    }
		  else
		    {
		      menu_len = prev_menu_len;
		      config_len = prev_config_len;
		    }

		  state = 1;

		  /* copy title into menu area */
		  while ((menu_entries[menu_len++] = *(ptr++)));
		}
	      else if (!state)
		{
		  if (strcmp("timeout", cmdline) < 1)
		    safe_parse_maxint(&ptr, &timeout);
		  if (strcmp("fallback", cmdline) < 1)
		    safe_parse_maxint(&ptr, &fallback);
		  if (strcmp("default", cmdline) < 1)
		    safe_parse_maxint(&ptr, &default_entry);
		  if (strcmp("password", cmdline) < 1)
		    {
		      password = config_entries;
		      while ((*(config_entries++) = *(ptr++)));
		    }

		  errnum = 0;
		}
	      else
		{
		  int i = 0;

		  state++;

		  /* copy config file data to config area */
		  while ((config_entries[config_len++] = cmdline[i++]));
		}
	    }

	  if (state > 1)
	    {
	      num_entries++;
	      config_entries[config_len++] = 0;
	    }
	  else
	    {
	      menu_len = prev_menu_len;
	      config_len = prev_config_len;
	    }

	  menu_entries[menu_len++] = 0;
	  config_entries[config_len++] = 0;
	  bcopy(menu_entries, config_entries+config_len, menu_len);
	  menu_entries = config_entries+config_len;
	}

      /*
       *  If no acceptable config file, goto command-line, starting heap from
       *  where the config entries would have been stored if there were any.
       */

      if (!num_entries)
	while (1)
	  enter_cmdline(NULL, config_entries);

      /*
       *  Run menu interface (this shouldn't return!).
       */

      run_menu(menu_entries, config_entries, num_entries,
	       menu_entries+menu_len, default_entry);
    }
}
