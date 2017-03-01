/*
 * kernel.c:  Alliance Bootloader Demonstration Kernel version 0.2
 * (C) 1998  Ramon van Handel, Abel Mui#o, the Alliance Operating System Team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * 
 */

/*
 * Notes on the kernel code:
 * The kernel code is pretty self-explanatory.  Just a few sidenotes:
 *
 * 1.  We start the kernel with the function _start(), the first function
 *     that is executed (the entry point.)  There is only one little
 *     problem that I've ignored for now:  Before executing a function,
 *     GCC executes the following code:
 *
 *        pushl %ebp
 *        movl %esp,%ebp
 *
 *     This is used to enable GDB to do a stack backtrace during
 *     debugging.  (%ebp contains the current stack frame pointer.)  
 *     The problem is that we haven't set up out own stack
 *     yet when this is done before _start(), so %ebp would contain
 *     'garbage' (and eventhough in practice this is never the case, in
 *     principle the stack pointer is undefined after boot and so 
 *     pushl %ebp could quite legitimately cause a fault.)  So we have two
 *     problems here: 1) we haven't set up a stack yet so what is
 *     happening here is illegal, and 2) this way any remote debugging
 *     using GDB won't work.  The only solution to these problems is that
 *     the _start() function is removed from the C code and is implemented
 *     in assembly, either using an asm statement on the top-level C code
 *     (I never tried this but it should work) or by making a separate
 *     assembly file (crt0.S) and linking this before the C code.
 *
 *     Example crt0.S code:
 *
 *        .text
 *        .type _start,@function
 *        .globl _start
 *        .p2align 4
 *        _start ## :
 *        jmp skip_multiboot_header
 *        .p2align 2
 *        .long   0x1BADB002
 *        .long   0x00000000
 *        .long   0-0x1BADB002-0x00000000
 *        skip_multiboot_header:
 *        movl $KERNEL_STACK,%esp
 *        addl $4096,%esp
 *        pushl $2
 *        popf
 *        xorl %ebp,%ebp
 *        pushl %ebx
 *        pushl %eax
 *        call main
 *        call _exit
 *
 *     Alternatively, you could compile without the stack frame pointer
 *     (using -fomit-frame-pointer) but then debugging becomes impossible.
 *     It does make your code faster, though.  Unfortunately, in this case
 *     this trick doesn't work, as GCC optimises our code by pushing %ebx
 *     right at the beginning of _start() (because we give it as a
 *     parameter to main(), remember ?) and I haven't managed to get it to
 *     stop it.  (Is it possible to tell GCC not to optimise a certain
 *     function ?)
 *
 * 2.  The outportb/inportb functions are not particularly fast.  I built
 *     in a delay (using outb on a non-existant port) which is required
 *     for communication with the keyboard controller, but now it is used
 *     for every other port usage too.  Linux has two separate functions
 *     (outportb and outportb_p) but I didn't feel like complicating
 *     matters for such a small kernel.  But if speed is critical, you
 *     should create a separate function for slow and fast I/O.
 *
 */

#define uchar  unsigned char		/* Defentitions to make life	*/
#define uint   unsigned int		/* Easy for us:  uchar, uint	*/
#define ulong  unsigned long int	/* and ulong			*/
#define iuchar inline unsigned char	/* Inline functions		*/
#define ivoid  inline void

#define COLS	    80			/* Coloumns on the screen	*/
#define ROWS	    25			/* Rows on the screen		*/
#define nScreen	    COLS * ROWS		/* Characters on the screen	*/
uint    TEXT_ATTR = 7;			/* The text attributes (kprint) */

#define T_FG_BLUE   1			/* Text attribute flags		*/
#define T_FG_GREEN  2
#define T_FG_RED    4
#define T_BOLD      8
#define T_BG_BLUE   16
#define T_BG_GREEN  32
#define T_BG_RED    64
#define T_BLINK     128

#define GRAPH     0			/* Startup video modes		*/
#define CO40x25   1
#define CO80x25   2
#define MONO80x25 3

#define EQAD ((0x040<<4)+0x010)		/* The BIOS equipment info	*/

uchar KERNEL_STACK[4096];		/* The stack to use: 4k stack	*/
uchar *VIDEO_MEMORY = (char *)0x0b8000; /* The video memory (color)	*/

struct equip_list			/* The equipment in the computer*/
{
   char Floppies;			/* Number of floppy disk drives	*/
   char HasCoprocessor;			/* Do we have an FPU ?		*/
   union
   {
      char MemoryBanks;			/* 16k memory banks in PCs	*/
      char HasMouse;			/* PS/2 mouse available (IBMs)  */
   } mix;
   char InitialVideoMode;		/* Video mode at boot		*/
   char HasDMASupport;			/* Do we support DMA ?		*/
   char SerialPorts;			/* How many serial ports ?	*/
   char HasGameport;			/* Do we have a gameport ?	*/
   char HasInternalModem;		/* Do we have an internal modem */
   char PrinterPorts;			/* How many printer ports ?	*/
};

struct multiboot_info			/* The multiboot information	*/
{					/* We get this passed in %ebx	*/
   ulong flags;				/* Multiboot flags		*/
   ulong mem_lower;			/* Lower memory			*/
   ulong mem_upper;			/* Upper memory			*/
   uchar boot_device[4];		/* The boot device		*/
   uchar *cmdline;			/* The ASCIIZ commandline	*/
   ulong mods_count;			/* Number of loaded modules	*/
   ulong mods_addr;			/* Physical address of 1st mod  */

   struct				/* ELF section header info 	*/
   {
      ulong num;			/* Amount of entries		*/
      ulong size;			/* Size of each entry		*/
      ulong addr;			/* Address of section hdr table */
      ulong shndx;			/* String table address (index)	*/
   } syms_elf;

   ulong mmap_length;			/* Size of the memory map	*/
   ulong mmap_addr;			/* Address of the memory map	*/
};

void cls(void);				/* Clear the screen		*/
void kprint(char *string);		/* The screen printing function */
iuchar inportb(uint port);		/* Input a byte from a port	*/
ivoid outportb(uint port,uchar value);	/* Output a byte to a port	*/
ivoid outportw(uint port,uint value);	/* Output a word to a port	*/
void _start(void);			/* The entry point		*/
int main(ulong,struct multiboot_info *);/* The main kernel program      */
void _exit(void);			/* What to do when finished	*/
void ulong_strcnv(uchar *str, ulong i); /* Convert a dword to a string  */
void waitkey(void);			/* Wait for a keypress		*/
void getBIOSEquipList(char,struct equip_list *); /* Get equipment info	*/

#define MB_FLAG_MEM     1		/* The multiboot flags		*/
#define MB_FLAG_BOOTDEV 2
#define MB_FLAG_CMDLINE 4
#define MB_FLAG_MODULES 8
#define MB_FLAG_AOUT    16
#define MB_FLAG_ELF	32
#define MB_FLAG_MEMMAP  64

void _start(void)			/* The kernel entry point	*/
{
   struct multiboot_info *mbinfo;	/* Multiboot info		*/
   ulong magic;				/* The Multiboot Magic number	*/

   asm ("mov %0, %%esp"::"r" (KERNEL_STACK+4096)); /* Set up the stack 	*/

   asm ("pushl $2; popf");		/* Zero the flags		*/

   asm volatile				/* Multiboot header		*/ 
   (
      "jmp skip_multiboot_header	\n"  /* Skip multiboot header	*/
      ".p2align 2			\n"  /* Align it correctly	*/
      ".long   0x1BADB002               \n"  /* Multiboot Magic number  */
      ".long   0x00000000               \n"  /* Flags	 		*/
      ".long   0-0x1BADB002-0x00000000  \n"  /* Checksum 		*/
      "skip_multiboot_header:		\n"  /* Continue here		*/
   );

   /* Now we fetch the address of the multiboot_info struct, as well as */
   /* the Multiboot Magic number in %eax (which should be 0x2BADB002)	*/

   asm volatile ("":"=a" (magic), "=b" (mbinfo));

   /* We could call all the constructors here (C++) but I'm going to    */
   /* keep this one simple.  Anyway, REAL programmers don't use C++ ;-) */

   /* We are also supposed to set up our own GDT.  Oh well, maybe next	*/
   /* time...  for now we'll trust the bootloader.			*/

   /* We have to leave interrupts disabled until we've set up an IDT.	*/
   /* We're not going to, so I leave them off.				*/

   main(magic,mbinfo);			/* Call main()			*/
   _exit();				/* Finish off			*/
}

/* Now we get the main kernel program 					*/

int main(ulong mbmagic,struct multiboot_info *mbinfo)
{
   struct equip_list equipment;
   uchar tempstr[12];

   TEXT_ATTR = (T_FG_RED | T_FG_GREEN | T_FG_BLUE | T_BG_BLUE);
   cls();

   if(mbmagic != 0x2BADB002)
   {
      TEXT_ATTR = (T_FG_RED | T_BG_BLUE | T_BOLD);
      kprint("Fatal Error\n");

      TEXT_ATTR = (T_FG_RED | T_FG_GREEN | T_FG_BLUE | T_BG_BLUE | T_BOLD);
      kprint("The kernel wasn't loaded from a multiboot-compatible ");
      kprint("bootloader !!!\n\n");

      TEXT_ATTR = (T_FG_RED | T_FG_GREEN | T_FG_BLUE | T_BG_BLUE | T_BLINK);
      kprint("Press any key to reboot."); 

      return 0;
   }

   TEXT_ATTR = (T_FG_RED | T_FG_GREEN | T_FG_BLUE | T_BOLD | T_BG_BLUE);
   kprint("Welcome to the Alliance Bootloader Demo Kernel\n");

   TEXT_ATTR = (T_FG_RED | T_FG_GREEN | T_FG_BLUE | T_BG_BLUE);
   kprint("This kernel (C) 1998 Ramon van Handel, Abel Mui#o, the Alliance Team\n");
   kprint("Version 0.2 (May 13, 1998)\n\n");

   kprint("This kernel was made to test the Alliance Bootloader and to be an example\n");
   kprint("of how you have to make a kernel to be loaded with it.\n\n");

   kprint("The kernel got the following information from the bootloader:\n");

   if(mbinfo->flags & MB_FLAG_MEM)
   {
      TEXT_ATTR = (T_FG_RED | T_FG_GREEN | T_FG_BLUE | T_BG_BLUE);
      kprint("Lower memory:  ");
      TEXT_ATTR = (T_FG_RED | T_FG_GREEN | T_FG_BLUE | T_BG_BLUE | T_BOLD);
      ulong_strcnv(tempstr,mbinfo->mem_lower);
      kprint(tempstr);
      kprint("k\n");

      TEXT_ATTR = (T_FG_RED | T_FG_GREEN | T_FG_BLUE | T_BG_BLUE);
      kprint("Upper memory:  ");
      TEXT_ATTR = (T_FG_RED | T_FG_GREEN | T_FG_BLUE | T_BG_BLUE | T_BOLD);
      ulong_strcnv(tempstr,mbinfo->mem_upper);
      kprint(tempstr);
      kprint("k\n\n");

      TEXT_ATTR = (T_FG_RED | T_FG_GREEN | T_FG_BLUE | T_BG_BLUE);
   }

   kprint("The kernel derived the following information from the BIOS: (note that\n");
   kprint("this might not be very accurate, as nowadays the BIOS is practically obsolete)\n");

   /* We assume we're not using an IBM PS/2 or related computer		*/

   getBIOSEquipList(0, &equipment);

   TEXT_ATTR = (T_FG_RED | T_FG_GREEN | T_FG_BLUE | T_BG_BLUE);
   kprint("Number of floppy drives:  ");
   TEXT_ATTR = (T_FG_RED | T_FG_GREEN | T_FG_BLUE | T_BG_BLUE | T_BOLD);
   ulong_strcnv(tempstr,equipment.Floppies);
   kprint(tempstr);
   kprint("\n");

   TEXT_ATTR = (T_FG_RED | T_FG_GREEN | T_FG_BLUE | T_BG_BLUE);
   kprint("Coprocessor:              ");
   TEXT_ATTR = (T_FG_RED | T_FG_GREEN | T_FG_BLUE | T_BG_BLUE | T_BOLD);
   kprint(equipment.HasCoprocessor?"Available":"Not available");
   kprint("\n");

   TEXT_ATTR = (T_FG_RED | T_FG_GREEN | T_FG_BLUE | T_BG_BLUE);
   kprint("RAM Banks on Mainboard:   ");
   TEXT_ATTR = (T_FG_RED | T_FG_GREEN | T_FG_BLUE | T_BG_BLUE | T_BOLD);
   ulong_strcnv(tempstr,equipment.mix.MemoryBanks);
   kprint(tempstr);
   kprint("\n");

   TEXT_ATTR = (T_FG_RED | T_FG_GREEN | T_FG_BLUE | T_BG_BLUE);
   kprint("Video mode on boot:       ");
   TEXT_ATTR = (T_FG_RED | T_FG_GREEN | T_FG_BLUE | T_BG_BLUE | T_BOLD);
   switch (equipment.InitialVideoMode)
   {
      case GRAPH:
         kprint ("EGA, VGA or PGA\n");
         break;
      case CO40x25:
         kprint ("40x25 color\n");
         break;
      case CO80x25:
         kprint ("80x25 color\n");
         break;
      case MONO80x25:
         kprint ("80x25 monochrome\n");
         break;
      default : kprint ("Unknow video mode...  Weird BIOS ?\n");
   }

   TEXT_ATTR = (T_FG_RED | T_FG_GREEN | T_FG_BLUE | T_BG_BLUE);
   kprint("DMA Support:              ");
   TEXT_ATTR = (T_FG_RED | T_FG_GREEN | T_FG_BLUE | T_BG_BLUE | T_BOLD);
   kprint(equipment.HasDMASupport?"DMA Supported":"DMA not supported");
   kprint("\n");

   TEXT_ATTR = (T_FG_RED | T_FG_GREEN | T_FG_BLUE | T_BG_BLUE);
   kprint("Serial ports:             ");
   TEXT_ATTR = (T_FG_RED | T_FG_GREEN | T_FG_BLUE | T_BG_BLUE | T_BOLD);
   ulong_strcnv(tempstr,equipment.SerialPorts);
   kprint(tempstr);
   kprint("\n");

   TEXT_ATTR = (T_FG_RED | T_FG_GREEN | T_FG_BLUE | T_BG_BLUE);
   kprint("Game port:                ");
   TEXT_ATTR = (T_FG_RED | T_FG_GREEN | T_FG_BLUE | T_BG_BLUE | T_BOLD);
   kprint(equipment.HasGameport?"Available":"Not available");
   kprint("\n");

   TEXT_ATTR = (T_FG_RED | T_FG_GREEN | T_FG_BLUE | T_BG_BLUE);
   kprint("Internal Modem:           ");
   TEXT_ATTR = (T_FG_RED | T_FG_GREEN | T_FG_BLUE | T_BG_BLUE | T_BOLD);
   kprint(equipment.HasInternalModem?"Available":"Not available");
   kprint("\n");

   TEXT_ATTR = (T_FG_RED | T_FG_GREEN | T_FG_BLUE | T_BG_BLUE);
   kprint("Printer ports:            ");
   TEXT_ATTR = (T_FG_RED | T_FG_GREEN | T_FG_BLUE | T_BG_BLUE | T_BOLD);
   ulong_strcnv(tempstr,equipment.PrinterPorts);
   kprint(tempstr);
   kprint("\n\n");

   TEXT_ATTR = (T_FG_RED | T_FG_GREEN | T_FG_BLUE | T_BG_BLUE);

   TEXT_ATTR = (T_BG_BLUE);
   kprint("Have fun browsing the code !!!\n");

   TEXT_ATTR = (T_FG_RED | T_FG_GREEN | T_FG_BLUE | T_BG_BLUE | T_BOLD);
   kprint("Press any key to reboot.");

   return 0;
}

void _exit(void)			/* Exit point of the kernel	*/
{
   waitkey();				/* Wait for a keypress		*/

   while(inportb(0x64) & 0x02);		/* Reboot the computer		*/
   outportb(0x64, 0xfe);

   asm("cli;hlt");			/* Should it not work we halt 	*/
					/* the computer now so the we	*/
					/* do no harm			*/
}

iuchar inportb(uint port)		/* Input a byte from a port	*/
{
   uchar ret;

   asm volatile ("inb %%dx,%%al;outb %%al,$0x80":"=a" (ret):"d" (port));
   return ret;
}

ivoid outportb(uint port,uchar value)	/* Output a byte to a port	*/
{
   asm volatile ("outb %%al,%%dx;outb %%al,$0x80"::"d" (port), "a" (value));
}

ivoid outportw(uint port,uint value)	/* Output a word to a port	*/
{
   asm volatile ("outw %%ax,%%dx;outb %%al,$0x80"::"d" (port), "a" (value));
}

void cls(void)				/* Clear the screen		*/
{
   uint i;

   for(i = 0; i < nScreen; i++)         /* Fill the screen with         */
   {					/* background color		*/
      VIDEO_MEMORY[i*2] = 0x20;
      VIDEO_MEMORY[i*2+1] = TEXT_ATTR;
   }

   outportb(0x3d4, 0x0f);		/* Set the cursor to the	*/
   outportb(0x3d5, 0);			/* upper-left corner of the	*/
   outportw(0x3d4, 0x0e);		/* screen			*/
   outportb(0x3d5, 0);
}

void kprint(char *string)		/* Print text to the screen	*/
{
   uint curchar, vidmem_off, i;

   outportb(0x3d4, 0x0e);		/* Get cursor Y position	*/
   vidmem_off = inportb(0x3d5);
   vidmem_off <<= 8;
   outportb(0x3d4, 0x0f);		/* And add cursor X position	*/
   vidmem_off += inportb(0x3d5);
   vidmem_off <<= 1;

   while((curchar=*string++))		/* Loop through the string	*/
   {
      switch(curchar)			/* Is it a special character ?  */
      {
         case '\n':			/* Newline found		*/
            vidmem_off = (vidmem_off/160)*160 + 160;
            break;

         case '\r':			/* Carriage return found	*/
            vidmem_off = (vidmem_off/160)*160;
            break;

         default:			/* Normal character             */
            VIDEO_MEMORY[vidmem_off++] = curchar;
            VIDEO_MEMORY[vidmem_off++] = TEXT_ATTR;
            break;
      }

      if(vidmem_off >= 160*25)		/* Are we off-screen ?		*/
      {
         for(i = 0; i < 160*24; i++)	/* Scroll the screen up		*/
         {
            VIDEO_MEMORY[i] = VIDEO_MEMORY[i+160];
         }
         for(i = 0; i < 80; i++)	/* Empty the bottom row		*/
         {
            VIDEO_MEMORY[(160*24)+(i*2)] = 0x20;
            VIDEO_MEMORY[(160*24)+(i*2)+1] = TEXT_ATTR;
         }
         vidmem_off -= 160;		/* We're on the bottom row	*/
      }
   }

   vidmem_off >>= 1;			/* Set the new cursor position  */
   outportb(0x3d4, 0x0f);
   outportb(0x3d5, vidmem_off & 0x0ff);
   outportw(0x3d4, 0x0e);
   outportb(0x3d5, vidmem_off >> 8);
}

void ulong_strcnv(uchar *str, ulong i)  /* Convert a dword to a string  */
{
   uchar backstr[11], j=0, l=0, m=0;

   do					/* Convert string one digit at	*/
   {					/* a time			*/
      backstr[j++] = (i % 10) + '0';	/* Put a digit in backstr	*/
      i /= 10;				/* Next digit			*/
   }					/* And continue until there are */
   while(i);				/* no more digits left		*/

   backstr[j] = '\0';			/* End of the string		*/

   for(l=j-1; m<j; l--)			/* Backstr is backwards (last   */
   {					/* digit first.)  Now we flip   */
      str[m++] = backstr[l];		/* it around...			*/
   }					/* ... and it's ready !!!	*/

   str[j] = '\0';			/* Put the string end on it	*/
}

void waitkey(void)			/* Wait for a keypress		*/
{
   uchar kbc_status;			/* The KBC status		*/

   do					/* Loop until a key was pressed */
   {					/* Including Shift, Ctrl etc.	*/
      while(inportb(0x64) & 0x02);	/* Get KBC status		*/
      kbc_status = inportb(0x64);
   }					/* Continue until bit 0 is set	*/
   while(!(kbc_status & 1));		/* Very primitive, but it works	*/
}

void getBIOSEquipList(char isIBMPS, struct equip_list *equip)
{
   /*
      Background info:
      The equipment in a computer is represented by one dword in the BIOS
      data area, at 0x040:0x010.  We take this and convert it to our own
      equipment list format.  Note that you cannot do a BIOS call to fetch
      the info (int 0x11) because we are in protected mode !!!
      EQAD is the absolute address of the BIOS equipment data dword
   */

   ulong BIOS_equip = *((ulong *)EQAD);	/* The BIOS equipment info	*/

   /* Check for floppies - this is bit 0 of the equipment dword		*/
   /* Bits 6+7 contain the amount of floppy drives in the system	*/

   if(BIOS_equip & 0x1)
   {
      equip->Floppies = ((BIOS_equip & 0x0c0) >> 6) + 1; /* How many ?	*/
   }
   else equip->Floppies = 0;

   /* Is there a coprocessor ?  This is bit 1				*/

   equip->HasCoprocessor = (BIOS_equip & 0x02) != 0;

   /* Now we make a distinction between an IBM PS/1 or PS/2 computer	*/
   /* (maybe even Aptiva ?) and a _real_ PC ;)				*/
   /* IBMs store the availability of a mouse here, PCs the RAM banks	*/
   /* Get bits 2 and 3							*/

   if(isIBMPS)
   {
      equip->mix.HasMouse = (BIOS_equip & 0x04) != 0;
   }
   else equip->mix.MemoryBanks = (BIOS_equip & 0x0c) >> 2;

   /* Get the initial video mode (video mode at boot -- bit 4 and 5)	*/

   equip->InitialVideoMode = (BIOS_equip & 0x030) >> 4;

   /* Does this computer support DMA transfers ?  (sure hope so !!!)	*/
   /* This is bit 8							*/

   equip->HasDMASupport = (BIOS_equip & 0x0100) != 0;

   /* Now we get the amount of RS232 serial ports (bits 9-11)	 	*/
  
   equip->SerialPorts = (BIOS_equip & 0x0e00) >> 9;

   /* Does this computer have a gameport ?  (bit 12)			*/
   
   equip->HasGameport = (BIOS_equip & 0x01000) != 0;

   /* Does this computer have an internal modem ? (bit 13)		*/
  
   equip->HasInternalModem = (BIOS_equip & 0x02000) != 0;

   /* How many printer ports do we have ?				*/
  
   equip->PrinterPorts = (BIOS_equip & 0x0c000) >> 14;
}
