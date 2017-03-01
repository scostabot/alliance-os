/*
 * elf.c:  ABL ELF static executable loader and interpreter
 *
 * (C) 1999 Ramon van Handel, The Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 15/03/99  ramon       1.0    First release
 * 10/05/99  ramon       1.1    Fixed fatal stack bug (yucky!)
 * 20/07/99  ramon       1.2    Fixed some more sticky bugs
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
 * TODO:  Check for whether we pass the total size of the file
 */

#include <typewrappers.h>
#include <elf.h>
#include "decl.h"


LOCAL Elf32_Ehdr ELFheader;
LOCAL Elf32_Phdr PHentry;


/*
 * HACK HACK HACK HACK HACK
 *
 * With netboot (TFTP), we have the problem that we can only
 * read at incremental positions into the file.  The only
 * solution is to re-open the file if we're going to read
 * from a smaller file offset than previous time, so that
 * we can start anew... ugly, but it works.
 */

static int __prev_offs = 0;
#define __read(x,y) ({                       \
    int __ret, __pos;                        \
    if(current_drive == 0x20) {              \
        if(filepos < __prev_offs) {          \
            __pos = filepos;                 \
            open(file);                      \
            filepos = __pos;                 \
        }                                    \
        __ret = read(x,y);                   \
        __prev_offs = filepos;               \
    } else {                                 \
        __ret = read(x,y);                   \
    }                                        \
    __ret;                                   \
})


PUBLIC ADDR ABLloadELF(STRING *file, ADDR memoffs, ADDR *entry)
/*
 * Load an ELF file into memory
 *
 * INPUT:
 *     dev:     Device on which the ELF file resides
 *     pos:     Absolute position of first sector
 *     memoffs: Absolute linear base address for ELF file
 *              If this is 0, then use e_entry in the ELF header
 *              Otherwise first loadable segment starts at memoffs linear
 *     entry:   Address at which to put the virtual enter point of the
 *              loaded ELF executable
 *
 * RETURNS:
 *     Highest linear address that should be allocated to this ELF file
 *     The ELF file then resides in memory at linear memoffs-ABLloadELF();
 */
{
    DATA i,j;
    ADDR highaddr = 0;
    ADDR size;

    /* We've loaded nothing yet */
    if(entry)
        *entry = 0;

    /* Open the file */
    if(!open(file)) {
        print_error();
        print("ELF:  Could not open ELF file\n");
        return 0;
    }

    size    = filemax + 1;
    filepos = 0;

    /* Read the ELF header */
    if(sizeof(ELFheader) > size) {
        print("ELF:  ELF file corrupt\n");
        return 0;
    }

    if(__read((ADDR)&ELFheader, sizeof(ELFheader)) != sizeof(ELFheader))
        return 0;

    /* Check whether this ELF file is valid for this architecture      */
    /* XXX - replace parts with architecture from configuration header */
    if(!BOOTABLE_I86_ELF(ELFheader)) {
        print("ELF:  Wrong ELF header\n");
        return 0;
    }

    /* Check that we don't overwrite ourselves */
    if(!memoffs && (ELFheader.e_entry < extMemTop)) {
        print("ELF:  ELF file would overwrite allocated memory\n");
        return 0;
    }

    /* Adjust the real memory offset */
    if(memoffs)
        memoffs -= ELFheader.e_entry;

    /* Loop through the program header table */
    for (i=0; i<ELFheader.e_phnum; i++) {
        /* Load the next program header table entry from the disk */
        if(ELFheader.e_phoff+(i*ELFheader.e_phentsize)+sizeof(PHentry) > size) {
            print("ELF:  ELF file corrupt\n");
            return 0;
        }

        filepos = ELFheader.e_phoff+(i*ELFheader.e_phentsize);
        if(__read((ADDR)&PHentry, sizeof(PHentry)) != sizeof(PHentry)) {
            print("ELF:  ELF file corrupt\n");
            return 0;
        }

        /* Check whether this segment ought to be loaded */
        if (PHentry.p_type == PT_LOAD) {
            /* Yup !  Load it */
            if(PHentry.p_offset+PHentry.p_filesz > size) {
                print("ELF:  ELF file corrupt\n");
                return 0;
            }

            filepos = PHentry.p_offset;
            if(__read(PHentry.p_paddr+memoffs, PHentry.p_filesz) != PHentry.p_filesz) {
                print("ELF:  ELF file corrupt\n");
                return 0;
            }

            /* If it is smaller in file than in memory, zero the remainder */
            if(PHentry.p_filesz < PHentry.p_memsz)
                for(j=PHentry.p_filesz; j<PHentry.p_memsz; j++)
                    ((DATA *)(PHentry.p_paddr+memoffs))[j] = 0;

            /* Account for the highest allocated virtual address */
            if(PHentry.p_paddr+PHentry.p_memsz > highaddr)
                highaddr = PHentry.p_paddr + PHentry.p_memsz;
        }
    }

    /* Record the virtual address of the entry point */
    if(entry)
        *entry = ELFheader.e_entry;

    return (highaddr+memoffs);  /* Success !! */
}

