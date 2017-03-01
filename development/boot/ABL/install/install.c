/*
 * install.c:  Set up the disk data for the ABL under Linux
 *
 * (C) 1999 Ramon van Handel, The Alliance Operating System Team
 * Portions Copyright (c) Tuomo Valkonen 1996-1997.
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 09/01/99  ramon       1.0    First release
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


/* Alliance header files */
#include <typewrappers.h>
#include <elf.h>

/* Linux header files */
#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/fs.h>
#include <linux/fd.h>
#include <linux/hdreg.h>
#include <dirent.h>
#include <malloc.h>


/*
 * SYMMAPSECT, SYMMAPSIZE, and SYMMAPDEV hold the symbol names in the ABL
 * to patch for respectively the map file base sector, size, and loading
 * device.
 */

#define SYMMAPSECT "mapsect"
#define SYMMAPSIZE "mapsize"
#define SYMMAPDEV  "mapdev"


/*
 * PHYSLOC holds the physical location of a partition on a storage
 * device.  We get the physical storage location of a file using
 * getDevice();
 */

typedef struct geom {
    UBYTE    heads;           /* Heads of the disk             */
    UBYTE    sectors;         /* Sectors on the disk           */
    UWORD16  cylinders;       /* Cylinders on the disk         */
    UWORD32  start;           /* Start sector of partition     */
    UWORD32  device;          /* The physical device it is on  */
    UWORD32  spb;             /* The block size                */
} PHYSLOC;

#define SECTORSIZE  512l      /* This is the sector size       */


/*
 * These are the major device types as returned by fstat
 */

#define MAJOR_FD     2    /* Floppy disk              */
#define MAJOR_HD     3    /* IDE harddisk             */
#define MAJOR_SD     8    /* SCSI harddisk            */
#define MAJOR_XT    13    /* XT harddisk              */
#define MAJOR_IDE2  22    /* Second IDE interface     */


/*
 * Misc stuff
 */

STRING tmpstr[512];       /* A temorary string        */


/*
 * Now we get the helper functions
 */

STRING *devName(dev_t device)
/*
 * Scan /dev directory for a device.
 *
 * INPUT:
 *     device:  Device to scan for
 *
 * RETURNS:
 *     devName():  path of requested device in /dev, or NIL on error
 */
{
    DIR    *ds;
    struct dirent *dir;
    struct stat    st;

    if((ds = opendir("/dev")) == NULL)   /* Open the /dev directory   */
        return NIL;                      /* Oops... error             */

    while((dir = readdir(ds))) {         /* Get the next device file  */
        sprintf(tmpstr, "/dev/%s", dir->d_name);    /* Complete fname */
        if(stat(tmpstr, &st) >= 0) {                /* Stat file      */
            if(S_ISBLK(st.st_mode) && st.st_rdev == device) {
                closedir(ds);                       /* Correct device */
                return tmpstr;                      /* found          */
            }
        }
    }

    return NIL;                          /* Device not found          */
}


int openDev(dev_t device)
/*
 * Open a device
 *
 * INPUT:
 *     device:  Device to open
 *
 * RETURNS:
 *     openDev():  File descriptor of device, or 0 on error
 */
{
    int fd;

    if((fd = open(devName(device), 3)) < 0)
        return 0;

    return fd;
}


DATA getDevice(int fdp, PHYSLOC *geo)
/*
 * Get geometry of a device where fdp is on or fdp is
 *
 * INPUT:
 *     fdp:  File descriptor of file
 *     geo:  Address of physical storage location structure to fill in
 *
 * RETURNS:
 *     getDevice():  1 on sucess, otherwise error
 */
{
    int    fd;
    int    device;
    struct floppy_struct fdgeo;
    struct hd_geometry   hdgeo;
    struct stat          st;

    if(fstat(fdp,&st) < 0)      /* Get file information                */
        return 0;               /* Oops... file not found or something */

    if(S_ISBLK(st.st_mode)) {   /* Is it a block device ?              */
        device = st.st_rdev;    /* Yeah, use the inode device id       */
        fd=fdp;
        geo->spb = 1;           /* assume block size is sector size    */
    } else {                    /* It's a file...                      */
        device = st.st_dev;
        if(ioctl(fdp,FIGETBSZ,&(geo->spb)) < 0)  /* Get block size     */
            return 0;                            /* Oops... error      */

        geo->spb = geo->spb / SECTORSIZE;        /* Fill in blocksz    */
        if(!(fd = openDev(device)))              /* Open the device    */
            return 0;                            /* Oops... error      */
    }

    switch(MAJOR(device)) {     /* What type of device are we using ?  */
        case MAJOR_FD:          /* ... a floppy disk !                 */
            if(ioctl(fd,FDGETPRM,&fdgeo) < 0)  /* Get device info      */
                return -1;                     /* Oops... error        */

            geo->heads     = fdgeo.head;       /* Fill in our PHYSLOC  */
            geo->cylinders = fdgeo.track;      /* structure with the   */
            geo->sectors   = fdgeo.sect;       /* device information   */
            geo->start     = 0;
            geo->device    = MINOR(device)&3;
            break;

        case MAJOR_HD:          /* ... an IDE harddisk !               */
            if(ioctl(fd,HDIO_GETGEO,&hdgeo)<0) /* Get device info      */
                return -1;                     /* Oops... error        */

            geo->heads     = hdgeo.heads;      /* Fill in our PHYSLOC  */
            geo->cylinders = hdgeo.cylinders;  /* structure with the   */
            geo->sectors   = hdgeo.sectors;    /* device information   */
            geo->start     = hdgeo.start;
            geo->device    = 0x10+(MINOR(device)>>6);
            break;

        case MAJOR_IDE2:        /* ... or a secondary IDE interface !  */
            if(ioctl(fd,HDIO_GETGEO,&hdgeo)<0) /* Get device info      */
                return -1;                     /* Oops... error        */

            geo->heads     = hdgeo.heads;      /* Fill in our PHYSLOC  */
            geo->cylinders = hdgeo.cylinders;  /* structure with the   */
            geo->sectors   = hdgeo.sectors;    /* device information   */
            geo->start     = hdgeo.start;
            geo->device    = 0x12+(MINOR(device)>>6);
            break;

        case MAJOR_XT:          /* ... an XT harddisk !                */
            if(ioctl(fd,HDIO_GETGEO,&hdgeo)<0) /* Get device info      */
                return -1;                     /* Oops... error        */

            geo->heads     = hdgeo.heads;      /* Fill in our PHYSLOC  */
            geo->cylinders = hdgeo.cylinders;  /* structure with the   */
            geo->sectors   = hdgeo.sectors;    /* device information   */
            geo->start     = hdgeo.start;
            geo->device    = 0x30+(MINOR(device)>>6);
            break;

        case MAJOR_SD:          /* ... a SCSI harddisk !               */
            if(ioctl(fd,HDIO_GETGEO,&hdgeo)<0) /* Get device info      */
                return -1;                     /* Oops... error        */

            geo->heads     = hdgeo.heads;      /* Fill in our PHYSLOC  */
            geo->cylinders = hdgeo.cylinders;  /* structure with the   */
            geo->sectors   = hdgeo.sectors;    /* device information   */
            geo->start     = hdgeo.start;

            if(!hdgeo.sectors) {
                return -1;
            }
            geo->device = 0x40+(MINOR(device)>>4);
            break;

        default:
            return -1;
    }

    return 1;
}

 
int getAddr(int fd, int sect, PHYSLOC *geo)
/*
 * Gets linear sector address of sector sect of file fd on device geo.
 *
 * INPUT:
 *     fd:   File descriptor of file
 *     sect: Sector offset of file
 *     geo:  Physical storage location of file
 *
 * RETURNS:
 *     getAddr():  -1 on error, otherwise linear disk sector
 */
{
    struct stat st;
    int    block, sector;

    if(fstat(fd, &st) < 0)
        return -1;

    /* Is it a block special file ? - return start address in geo   */
    if( S_ISBLK(st.st_mode) )
        return geo->start + sect;

    /* Count block address  */
    block = sect / geo->spb;

    if(ioctl(fd, FIBMAP, &block) < 0)
        return -1;
    if(block == 0)
        return  0;

    /* Count the sector address     */
    sector = geo->start + ((block * geo->spb) + (sect % geo->spb));

    return sector;
}


Elf32_Off elf_vtoo(Elf32_Phdr *phdrtbl, int psize, Elf32_Addr vaddr)
/*
 * Converts an ELF virtual address to a file offset
 *
 * INPUT:
 *     phdrtbl:  Addrress of the program header table (in memory)
 *     psize:    Amount of entries in the program header table
 *     vaddr:    Virtual address to convert
 *
 * RETURNS:
 *     elf_vtoo():  ELF file offset corresponding to virtual address vaddr
 */
{
    int i;

    for(i = 0; i < psize; i++) {
        if(phdrtbl[i].p_type != PT_LOAD) continue;
        if((vaddr >= phdrtbl[i].p_vaddr) && (vaddr < (phdrtbl[i].p_vaddr + phdrtbl[i].p_memsz)))
            return (vaddr - phdrtbl[i].p_vaddr + phdrtbl[i].p_offset);
    }

    printf("Invalid virtual address %p in ELF file\n", (void *) vaddr);
    printf("Aborting...\n");
    exit(-1);
}

#define ELFVTOO(vaddress) elf_vtoo(phdrtbl,ablhdr.e_phnum,vaddress)


/*
 * Now we get the main ABL installation stuff
 */

int main(int argc, char *argv[])
/*
 * Install the ABL
 *
 * INPUT:
 *     Command line stuff
 *
 * RETURNS:
 *     -1 on error, otherwise 0.
 */
{
    FILE *cfgfile;          /* The configuration file decriptor   */
    int ablfile;            /* The ABL ELF file descriptor        */
    int mapfile;            /* The mapfile descriptor             */
    int kernelfile;         /* Kernel file to install in ABL      */
    STRING kernel[256];     /* Next kernel to install in ABL      */
    PHYSLOC disk;           /* Disk on which the kernel is        */
    UWORD32 buffer;         /* Buffer for writing to the mapfile  */
    UWORD32 maplen;         /* The length of the mapfile          */
    Elf32_Ehdr ablhdr;      /* The ABL ELF header                 */
    Elf32_Phdr *phdrtbl;    /* The ABL ELF program header table   */
    Elf32_Shdr *shdrtbl;    /* The ABL ELF section header table   */
    Elf32_Sym  *symtab;     /* The symbol table                   */
    STRING     *strtab;     /* The ABL ELF string table           */
    Elf32_Addr symmapsect = 0;   /* Sector offset in ABL ELF      */
    Elf32_Addr symmapsize = 0;   /* Size offset in ABL ELF        */
    Elf32_Addr symmapdev  = 0;   /* Device offset in ABL ELF      */
    int i,j;

    /* Check whether we have enough parameters */
    if(argc<4) {
        printf("Usage:    ./install cfgfile ablfile mapfile\n");
        printf("cfgfile:  The configuration file (see the documentation)\n");
        printf("ablfile:  The ABL ELF file to patch\n");
        printf("mapfile:  The filename of the mapfile to create\n\n");
        printf("Aborting...\n");
        exit(-1);
    }


    /*
     * We start by processing the configuration file and generating
     * the map file.  The configuration file consists of paths of
     * kernels to load, separated by whitespace and/or '\n'.  Lines
     * starting with '#' are comment lines, and are ignored.  The
     * first path is the CK.
     * 
     * For every kernel to load, we write three integers to the mapfile:
     * The base sector, the length, and the device.
     */

    /* Open the configuration file */
    if((cfgfile = fopen(argv[1], "r")) == NULL) {
        printf("Could not open configuration file %s\n", argv[1]);
        printf("Aborting...\n");
        exit(-1);
    }

    /* Open the mapfile for writing */
    if((mapfile = creat(argv[3], 0600)) == -1) {
        printf("Could not open map file %s for writing\n", argv[3]);
        printf("Aborting...\n");
        exit(-1);
    }

    /* Process the configuration file */
    while(fscanf(cfgfile,"%s",kernel) != EOF) {
        /* Check whether rest of line is a comment, and if so, skip it */
        if(kernel[0] == '#') {
            while((*kernel = getc(cfgfile)) != '\n' && *kernel != EOF);
            continue;
        }

        /* Open the filename from the configuration file */
        if((kernelfile = open(kernel, O_RDONLY)) == -1) {
            printf("Could not open %s\n",kernel);
            printf("Aborting...\n");
            exit(-1);
        }

        /* Get the device on which the file resides */
        if(getDevice(kernelfile,&disk) != 1) {
            printf("Could not get device of %s\n",kernel);
            printf("Aborting...\n");
            exit(-1);
        }

        /* Write the sector offset on that device to the mapfile */
        buffer = getAddr(kernelfile,0,&disk);
        write(mapfile,&buffer,sizeof(buffer));

        /* Write the file length to the mapfile */
        buffer = lseek(kernelfile, 0, SEEK_END);
        write(mapfile,&buffer,sizeof(buffer));

        /* Write the device number to the mapfile */
        write(mapfile,&disk.device,sizeof(disk.device));

        /* Next kernel file */
        close(kernelfile);
    }

    /* Close the configuration file */
    fclose(cfgfile);

    /* Get the device on which the mapfile resides */
    if(getDevice(mapfile,&disk) != 1) {
        printf("Could not get the device on which the mapfile resides\n");
        printf("Aborting...\n");
        exit(-1);
    }

    /* Get the sector number of the start of the mapfile and its length */
    buffer = getAddr(mapfile, 0, &disk);
    maplen = lseek(mapfile, 0, SEEK_END);

    /* Close the mapfile */
    close(mapfile);


    /*
     * Now we process the ABL ELF file.  We want to find the location
     * of the symbols that contain the mapfile base sector, length, and
     * device number, and patch them correctly.
     */

    /* Open the ABL ELF file */
    if((ablfile = open(argv[2], O_RDWR)) == -1) {
        printf("Could not open ABL file %s\n", argv[2]);
        printf("Aborting...\n");
        exit(-1);
    }

    /* Read the ABL ELF header */
    if((read(ablfile,&ablhdr,sizeof(ablhdr)) < sizeof(ablhdr)) ||
       (ablhdr.e_ident[EI_MAG0] != ELFMAG0)                    ||
       (ablhdr.e_ident[EI_MAG1] != ELFMAG1)                    ||
       (ablhdr.e_ident[EI_MAG2] != ELFMAG2)                    ||
       (ablhdr.e_ident[EI_MAG3] != ELFMAG3)) {
        printf("%s not a correct ELF file\n", argv[2]);
        printf("Aborting...\n");
        exit(-1);
    }

    if(!ablhdr.e_phoff || !ablhdr.e_shoff) {
        printf("%s not a valid ELF file\n", argv[2]);
        printf("Aborting...\n");
        exit(-1);
    }

    /* Move file offset to the program header table */
    lseek(ablfile,ablhdr.e_phoff,SEEK_SET);

    /* Load the program header table */
    phdrtbl = malloc(ablhdr.e_phnum*ablhdr.e_phentsize);
    for(i = 0; i < ablhdr.e_phnum; i++)
        read(ablfile,&phdrtbl[i],ablhdr.e_phentsize);

    /* Move file offset to the section header table */
    lseek(ablfile,ablhdr.e_shoff,SEEK_SET);

    /* Load the section header table */
    shdrtbl = malloc(ablhdr.e_shnum*ablhdr.e_shentsize);
    for(i = 0; i < ablhdr.e_shnum; i++)
        read(ablfile,&shdrtbl[i],ablhdr.e_shentsize);

    /* Loop through the section header table and look for symbol tables */
    for(i = 0; i < ablhdr.e_shnum; i++) {
        if(shdrtbl[i].sh_type != SHT_SYMTAB && shdrtbl[i].sh_type != SHT_DYNSYM)
            continue;

        /* Load the symbol table */
        symtab = malloc(shdrtbl[i].sh_size);
        lseek(ablfile,shdrtbl[i].sh_offset,SEEK_SET);
        read(ablfile,symtab,shdrtbl[i].sh_size);

        /* Load the string table */
        strtab = malloc(shdrtbl[shdrtbl[i].sh_link].sh_size);
        lseek(ablfile,(shdrtbl[shdrtbl[i].sh_link].sh_offset),SEEK_SET);
        read(ablfile,strtab,shdrtbl[shdrtbl[i].sh_link].sh_size);

        /* Loop though the symbol table to find our symbols */
        for(j = 0; j < shdrtbl[i].sh_size / shdrtbl[i].sh_entsize; j++) {
            if(!strcmp(strtab+symtab[j].st_name,SYMMAPSECT))
                symmapsect = ELFVTOO(symtab[j].st_value);
            if(!strcmp(strtab+symtab[j].st_name,SYMMAPSIZE))
                symmapsize = ELFVTOO(symtab[j].st_value);
            if(!strcmp(strtab+symtab[j].st_name,SYMMAPDEV))
                symmapdev  = ELFVTOO(symtab[j].st_value);
        }

        free(strtab);
        free(symtab);
    }

    if(!symmapsect || !symmapsize || !symmapdev) {
        printf("Required symbol(s) missing in %s\n", argv[2]);
        printf("Aborting...\n");
        exit(-1);
    }

    /* Now we actually patch the ELF file */
    lseek(ablfile,symmapsect,SEEK_SET);
    write(ablfile,&buffer,sizeof(buffer));
    lseek(ablfile,symmapsize,SEEK_SET);
    write(ablfile,&maplen,sizeof(maplen));
    lseek(ablfile,symmapdev,SEEK_SET);
    write(ablfile,&disk.device,sizeof(disk.device));

    /* Cleanup */
    close(ablfile);
    free(phdrtbl);
    free(shdrtbl);

    /* We're done !!! */
    exit(0);
}
