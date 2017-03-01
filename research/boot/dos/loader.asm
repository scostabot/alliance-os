; loader.asm: the Alliance ELF protected mode loader for DOS
;
; (C) 1998 Ramon van Handel, the Alliance Operating System Team
; Portions (GDT macros) by John Fine <johnfine@erols.com>
;
; HISTORY
; Date     Author    Rev    Notes
; 17/04/98 ramon     1.0    First release
; 
;  This program is free software; you can redistribute it and/or modify
;  it under the terms of the GNU General Public License as published by
;  the Free Software Foundation; either version 2 of the License, or
;  (at your option) any later version.
;
;  This program is distributed in the hope that it will be useful,
;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;  GNU General Public License for more details.
;
;  You should have received a copy of the GNU General Public License
;  along with this program; if not, write to the Free Software
;  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
; --------------------------------------------------------------------------

; Overview:
; Loads the kernel file specified by [kernel_fname],
; Sets up the tables required for PM, switches to PM (protected mode) and
; calls the kernel.  Afterwards it halts the computer.
; The ELF kernel has to be a statically linked ELF executable with a fixed
; text address above 1M (compile with -Ttext <address>)
;
; Things that still need to be done:
; * Add a simple IDT for the loader with exception handlers
; * Make more accurate error reports
; * Load file without using DOS (maybe keep space in .data for the
;   sector and size of the file, to be patched on installation by the
;   installation program - that way it's partition type independent, etc.)
;   This would enable us to use this as a real bootloader ! (see Matej's
;   dos .com file loader somewhere in the archives [removed!])
; * Check the CMOS if the user has minimally 1mb memory before starting our
;   own 'superior' detailed test
; * Check if kernel fits in memory
; * Make better video routines (take away the stupid coordinates and use the
;   cursor like any other human being... but no interrupts !)
; * Enable better use of the ELF header
; * Pass the system memory map to the kernel (with int 15 ???)
; * Better Multiboot support - Recognition and use of multiboot magic header,
;   check the multiboot flags in the magic header (at least multiboot_support)
;   better use of the multiboot_info structure etc.
;   Also set the address of the ELF segment table, memory map, etc. in the
;   multiboot_info structure.  We don't need the modules because we have our
;   own 3d stage loader for SKs/AKs/the like.
; --------------------------------------------------------------------------
; The Multiboot standard requires the following machine state on
; transfering control to the kernel:
;
;     * CS must be a 32-bit read/execute code segment with an offset of 0
;       and a limit of 0xffffffff.
;     * DS, ES, FS, GS, and SS must be a 32-bit read/write data segment
;       with an offset of 0 and a limit of 0xffffffff.
;     * The address 20 line must be usable for standard linear 32-bit
;       addressing of memory 
;     * Paging must be turned off.
;     * The processor interrupt flag must be turned off.
;     * EAX must contain the magic value 0x2BADB002; the presence of this
;       value indicates to the OS that it was loaded by a
;       MultiBoot-compliant boot loader (e.g. as opposed to another type
;       of boot loader that the OS can also be loaded from).
;     * EBX must contain the 32-bit physical address of the multiboot_info
;       structure provided by the boot loader.
;
; All other processor registers and flag bits are undefined. This includes, in
; particular:
;
;     * ESP: the 32-bit OS must create its own stack as soon as it needs
;       one.
;     * GDTR: Even though the segment registers are set up as described
;       above, the GDTR may be invalid, so the OS must not load any
;       segment registers (even just reloading the same values!) until it
;       sets up its own GDT.
;     * IDTR: The OS must leave interrupts disabled until it sets up its
;       own IDT.
;
; NOTE:  The kernel doesn't get any parameters through the C calling 
;        convention !!!  So:  PUBLIC VOID _start(VOID) { [...] }
;
; --------------------------------------------------------------------------

	BITS 16                 ; We are still in real mode now, so 
				; we want to write 16 bit code

	org 100h                ; This is a DOS style .com file
				; DOS loads COM files at offset 100h (there
				; is a 256 byte PSP before that.)  Tell NASM
				; that it should add 100h to all memory
				; references

	section .text           ; Our code starts here

start:
	mov ax,cs               ; The code segment is also the data segment
	mov ds,ax               ; Just the data segment registers don't know
	mov es,ax               ; it yet...   so we'll tell them.
	mov fs,ax               
	mov gs,ax
	cli                     ; This part is a bit tricky...  we don't want
				; to change the stack while an interrupt
				; occurs (it could damage our code)
	mov ss,ax
	mov esp,ldr_stack_end
	sti                     ; Phew... we can breathe again

	call init_screen        ; Put the video memory segment in video_seg
	call clear_screen       ; Clear the screen

	mov si,welcome_msg      ; Print the welcome message
	call write_string

	mov si,copyright_msg1   ; Write the copyright message
	call write_string
	mov si,copyright_msg2
	call write_string

	mov si,check_386_msg    ; Write what we're going to do
	call write_string
	call check_386          ; Check for 386+ processor
	or al,al                ; Is there a 386+ ?
	jnz near i386_not_found_err  ; No, return error message

	mov si,check_realmode   ; Tell the user what's going on
	call write_string
	call is_in_pm           ; Is the processor already in PM ?
	or al,al                ;
	jnz near i386_in_pm_err ; Yeah... return error message

	mov si,enable_A20_msg   ; We're GO !!!
	call write_string
	call enableA20          ; Attempt to enable the A20 line
	or bl,bl                ; Did it work ?
	jz near enable_A20_err  ; Nope 8(...  Return error

	mov si,going_unreal     ; We're going !!!
	call write_string
	call go_unreal_mode     ; Let's DO IT !!!
	mov si,unreal_inside    ; Phew... we're in.  Let's share it with
	call write_string       ; the user...
	
	mov si,mem_count_msg    ; Let's count the memory
	call write_string
	call countmemory        ; Count the memory -- returns size in eax
	mov [mem_size],eax      ; Store the size in the bss
	shr eax,10              ; Convert to kilobytes (eax = eax/1024)
	mov di,memsize_dec      ; Convert to decimal
	call convert_decimal
	mov si,mem_size_msg     ; Write it
	call write_string       ; Ok !
	sub eax,1024            ; Calculate amount of upper memory
				; This is memsize-1mb, isn't it ?
	mov dword [mem_upr],eax ; Patch the multiboot_info structure

	mov si,kernel_loading   ; Loading the kernel
	call write_string
	
	xor ax,ax               ; Give access to all of the memory though es
	mov es,ax
	mov dx,kernel_fname     ; Put the kernel filename address in ds:dx
	call loadELF            ; Load the kernel
	or eax,eax              ; Was it loaded ok ?
	jz near file_load_err   ; No...  well, error then

	mov si,gdt_msg          ; Setting up the GDT
	call write_string
	xor eax,eax             ; Calculate absolute code address
	mov ax,cs
	shl eax,4
	add [gdt+2],eax         ; Patch the GDT  address
	
	mov si,load_gdt_msg     ; Now we can load the gdt
	call write_string
	lgdt [gdt]              ; Finally

	mov si,going_all_P      ; Now we're going to protected mode...
	call write_string
	
	mov eax,cs              ; Calculate full address of the PM code
	shl eax,4               ; and patch the far jump in the code
	add dword [patch],eax   ; (this is a form of relocation)
	
	push eax                ; We save the absolute address of the code
				; (so also data) segment on the stack, so
				; we can use it in PM

	mov eax,ss              ; Calculate the stack pointer for PM
	shl eax,4               ; Which is the SP's absolute address
	add eax,esp
	mov esp,eax
	
	cli                     ; We don't want interrupts now, but multiboot
				; requires the IF to be off on kernel entry
				; anyway, so we don't do sti
	
	mov eax,cr0             ; Get cr0
	inc ax                  ; Set the PE bit in the GDT
	mov cr0,eax             ; Go !!!

; The next piece is dirty, I know, but I can found no way around it
; 066h is machine code for 'use 32-bit addresses'
; 0eah is machine code for 'jmp far'
; Then come the offset and the selector of the address to jump to, 
; respectively
; We need this strange consturction to be able to patch it correctly

	db 066h, 0eah           ; Do a far jump to flush the prefetch
				; queue and enter protected mode
patch:  dd in_pmode             ; We need to patch this offset
	dw code32-gdt           ; The code selector for the PM code

	BITS 32                 ; From now we use 32-bit assembler

in_pmode:                       ; We're in PROTECTED MODE !!!!!
	mov ax,data32-gdt       ; Fill the selector registers
	mov ds,ax
	mov es,ax
	mov fs,ax
	mov gs,ax
	
	mov ax,stack32-gdt      ; Fill the stack selector
	mov ss,ax

	pop eax                 ; eax should now contain the absolute address
				; of our former data/code segment

	lea ecx,[eax+write_string32] ; This is the address of write_string32
	
	lea esi,[eax+PM_is_here]     ; It worked !!!
	call ecx                     ; Let the user know it, too
	
	lea esi,[eax+exc_kernel]     ; We're going to execute the kernel now
	call ecx                
	
	; We're now ready to execute the kernel

	mov edx,[eax+krnl_entry]     ; The kernel entry point
	
	lea ebx,[eax+multiboot_info] ; The multiboot_info struct

	mov eax,02BADB002h           ; The multiboot magic number

	push dword 2            ; Zero the flags - Bit 1 has to be set
	popfd                   ; according to the intel specifications
      
	call edx                ; Go !!!!!!

	; This point should never be reached - the kernel should shutdown
	; itself, and not return to the loader (as the chances are enormous
	; that the kernel would have muddled the computer up too much anyway
	; for us to do something sensible here)

	; Just in case, halt the computer

	cli                     ; Halt the computer
	hlt                     ; This is the end

	BITS 16                 ; From here we start our subroutines

; Errors:

i386_not_found_err:
	mov si,i386_not_found   ; Print error message
	call write_string
	int 0x20                ; Exit to DOS (blah)

i386_in_pm_err:
	mov si,i386_in_pm_msg   ; Print error message
	call write_string
	int 0x20                ; Exit to DOS (blah)

enable_A20_err:
	mov si,kbc_timeout_msg  ; Print error message
	call write_string
	int 0x20                ; Exit to DOS (blah)

file_load_err:        
	mov si,file_error_msg   ; Print error message
	call write_string
	mov si,file_error_msg2
	call write_string
	int 0x20                ; Exit to DOS (blah)

; --------------------------------------------------------------------------

; init_screen:   Initialise the screen segment (video_seg) in real mode
;                WARNING: ONLY WORKS IN REAL MODE !!!!!!!!!!!!!
;
; Takes parameters:
; nothing
;
; Returns:
; Segment of video memory in video_seg

init_screen:
	push eax                ; Save ax
	int 11h                 ; Get video information
	and al,30h              ; Zero all bits except bit 4,5
	cmp al,30h              ; Are bits 4 and 5 set ?
	jne finish_init_screen  ; No, this is a color display
	mov word [video_seg], 0b000h  ; The video segment is at 0b000h
finish_init_screen:
	pop eax                 ; and ax
	ret                     ; Return to the main program

; clear_screen:  Clears the screen by zeroing out the video memory
;
; Takes parameters:
; video_seg    = segment/selector of low 64k of video memory
;
; Returns:
; nothing

clear_screen:
	push eax                ; Save the registers
	push edi
	push ecx
	push es
	mov ax,[video_seg]      ; Move the video memory segment into ES
	mov es,ax
	xor di,di               ; Start clearing the screen at offset 0
	mov cx,2000             ; Clear 2000 characters
	mov ax,0700h            ; Fill the video memory with char 0, attr 7
				; (0700h because intel is little-endian)
	cld                     ; Clear the direction flag; rep increments di
	rep stosw               ; Fill the video memory
	pop es                  ; Restore the registers
	pop ecx
	pop edi
	pop eax
	ret                     ; Return to the main program

; zero_memblock:  Fills the specified memory block with zeros (0x0)
;
; Takes parameters:
; ax    = segment/selector of memory to be cleared
; edi   = offset of memory to be cleared
; ecx   = number of bytes to clear
;
; Returns:
; nothing

zero_memblock:
	push eax                ; Save the registers
	push edi
	push ecx
	push es
	mov es,ax
	xor eax,eax             ; Fill the memory with zeros (0x0)
	cld                     ; Clear the direction flag; rep increments di
	a32 rep stosb           ; Fill the memory (one byte at a time)
	pop es                  ; Restore the registers
	pop ecx
	pop edi
	pop eax
	ret                     ; Return to the main program

; write_string:  Writes ASCIIZ string to the screen directly (very simple)
;                WARNING: ONLY WORKS IN REAL MODE !!!!!!!!!!!!!
;
; Takes parameters:
; ds:si     = Address of format string
; video_seg = Segment of video memory
;
; Returns:
; nothing
;
; Format of format string:  x, y, string, \0 (all bytes)

write_string:
	push eax                ; Push registers so we can restore them later
	push ebx
	push edi
	push edx
	push es
	xor ah,ah
	mov byte al,[si+1]      ; Calculate the screen address for (x,y)
	mov di,160              ; ax = y * 160 + x * 2
	mul di
	mov byte dl,[si]
	xor dh,dh
	add ax,dx
	add ax,dx
	mov bx,ax
	mov word ax,[video_seg]
	mov es,ax
	mov di,2                ; The string starts on offset 2 from DS:SI
	add di,si               ; cx holds offset of the char to be printed
.loop:  mov byte dl,[di]        ; dx holds the char to be printed
	or  dl,dl               ; Is the character zero (0) ?
	jz  string_end_reached  ; This is the end of the string
	mov byte [es:bx],dl     ; Write the character to the screen
	mov byte [es:bx+1],7    ; Write the attribute
	inc di                  ; Increase the character pointer
	add bx,2                ; Increase the memory address
	jmp .loop               ; Next char
string_end_reached:
	pop es                  ; Restore the registers
	pop edx                  
	pop edi
	pop ebx
	pop eax
	ret                     ; Return to the main program

; write_string32: Writes ASCIIZ string to the screen directly (very simple)
;                 WORKS ONLY IN PROTECTED MODE
;
; Takes parameters:
; ds:esi    = Address of format string
; eax       = Former data segment
; video_seg = Video segment
;
; Returns:
; nothing
;
; Format of format string:  x, y, string, \0 (all bytes)

	BITS 32                 ; This is a procedure for 32-bit mode

write_string32:
	push edi                ; Save the registers we're going to change,
	push edx                ; so we can restore them when we're finished
	push ecx        
	push esi
	push eax

	push eax                ; Calculate the screen address for (x,y)
	xor eax,eax             ; ecx = y * 160 + x * 2
	mov byte al,[esi+1]     
	mov di,160              
	mul di
	mov byte dl,[esi]
	xor dh,dh
	add ax,dx
	add ax,dx
	mov ecx,eax               
	pop eax

	add esi,2               ; The string itself starts at offset 2

	xor edi,edi             ; Calculate address of video memory
	mov edx,eax             ; on which to print the string
	add edx,video_seg
	mov word di,[edx]
	shl edi,4
	add edi,ecx
	
	mov ah,7                ; Use normal attributes (7)

	lodsb                   ; Get a byte from the string in al
.loop:  stosw                   ; Store ax in the video memory
	lodsb                   ; Get the next byte
	test al,al              ; Has the end of the string been reached ?
	jnz .loop               ; No, print another letter

	pop eax                 ; Restore the registers
	pop esi
	pop ecx
	pop edx
	pop edi

	ret                     ; Return to the main program

	BITS 16                 ; The following procedures are 16-bits again

; check_386:  Check whether this computer is 386+
;
; Takes parameters:
; nothing
;
; Returns:
; eax   = 0 if 386+ was found
; eax   = 1 if no 386 compatible processor was found

check_386:
	pushf                   ; Push the flags onto the stack
	xor     ah,ah           ; Clear AH
	push    ax              ; Push AX onto the stack
	popf                    ; Pop the former AX into the flags
	pushf                   ; Push this back onto the stack
	pop     ax              ; Pop it into AX
	and     ah,0f0h         ; Does AH equal 1111xxxxb ?
	cmp     ah,0f0h         ; 
	je      no386           ; On a 386+ the top nibble of the flags
				; is never filled with 1 -> it contains I/O
				; priv. level stuff etc. -> Not a 386+
	mov     ah,70h          ; Try to set NT and IOPL flags
	push    ax              ; This shouldn't work on a 286 in real mode
	popf                    ; During boot (or in DOS) the computer is
	pushf                   ; never in protected mode
	pop     ax              ;
	and     ah,70h          ; Test whether the flags were actually set
	jz      no386           ; If not, it is not a 386+
	popf                    ; Pop the original flags back
	xor ax,ax               ; Return zero if ok
	ret                     ; Return to main program
no386:
	mov ax,1                ; No 386:  Return 1 in ax
	ret                     ; Return to main program

; is_in_pm: Checks whether the processor is already in protected mode
;
; Takes parameters:
; nothing
;
; Returns:
; ax     = 1 if the processor was already in PM
; ax     = 0 if the processor is in real mode

is_in_pm:
	smsw    ax              ; Load CR0 in AX
	and     al,1            ; Is bit 1 set ?
	jnz     exit_is_pm      ; Yes, the processor is in PM/VM86 mode
				; So exit with ax = 1
	xor     ax,ax           ; No, return eax = 0
exit_is_pm:
	ret                     ; Return to the main program

; enableA20:  Enables the A20 gate
;
; Takes parameters:
; nothing
;
; Returns:
; bx    = 1 if A20 enable successful
; bx    = 0 is A20 enable failed (kbc timeout)

enableA20:                              ; Enable gate A20
	push eax                        ; Save the registers on the stack
	push ecx
	xor bx,bx                       ; A20 not set yet
	call start_rdyloop              ; Wait for KBC to be ready for input
	jnz short enableA20done         ; KBC Timeout, terminate
	mov byte al,0d1h                ; Write command D1 to KB command port
	out 64h,al                      ; 0d1h = Write to output port
	call start_rdyloop              ; Wait for KBC to process request
	jnz short enableA20done         ; KBC Timeout, terminate
	mov byte al,0dfh                ; Write 0dfh to the output port
	out 60h,al                      ; This sets the A20 gate
	inc bx                          ; A20 set successful
start_rdyloop:                          ; Wait for KBC or timeout
	mov ecx,20000h                  ; Wait for KBC to be ready a maximum
					; of 2^17 times (otherwise timeout)
rdyloop:                                ; Wait for KBC to be ready
	jmp short rdydelay              ; Short delay
rdydelay:        
	in al,64h                       ; Get Keyboard Status Register
	test al,2                       ; Test whether the Keyboard 
					; Controller is accepting input
	loopnz rdyloop                  ; If not, delay and try again
	or bx,bx                        ; Is the A20 enabled ?
	jnz enableA20done               ; Ok, pop the registers too
	ret                             ; No, just return
enableA20done:                          
	pop ecx                         ; Restore the registers
	pop eax
	ret                             ; Return to main code

; go_unreal_mode:  Enters 'unreal'-mode:  Real mode with 4GB segments
;                  WARNING:  YOU HAVE TO ENABLE THE A20 GATE YOURSELF !!!
;
; Takes parameters:
; nothing
;
; Returns:
; nothing

go_unreal_mode:  
	push ecx                        ; Save some registers on the stack
	push eax
	pushfd                          ; Save EFLAGS on the stack
	push ds                         ; Save the segment registers
	push es
	push fs
	push gs

	mov ecx, ds                     ; Put absolute address of data 
	shl ecx, 4                      ; segment in edx
	add [unreal_gdt+2], ecx         ; Convert offset of GDT to abs. addr.
	
	cli                             ; We don't want interrupts while
					; muddling with the GDT & stuff.
	lgdt    [cs:unreal_gdt]         ; Load the GDT for unreal-mode
	mov     ecx, CR0                ; Switch to protected mode
	inc     cx
	mov     CR0, ecx
	
	mov     ax, un_data-unreal_gdt  ; Load the 4GB data segment
	mov     ds, ax                  ; into ds
	mov     es, ax                  ; into es
	mov     fs, ax
	mov     gs, ax
	dec     cx                      ; Switch protected mode off again
	mov     CR0, ecx                ; ... and we're in unreal-mode !
	sti                             ; Now we can have interrupts again
	
	pop gs                          ; Restore the segments
	pop fs
	pop es
	pop ds
	popfd                           ; Restore EFLAGS
	pop eax                         ; Restore the registers
	pop ecx
	ret                             ; Go back to whence we came

; loadfile:    Loads a segment of a file from the disk using DOS functions
;              WARNING: ONLY WORKS IN (un-) REAL MODE !!!!!!!!!!!!!
;
; Takes parameters:
; ds:dx  = Address of ASCIIZ filename
; es:edi = Where in memory to put it
; ecx    = Offset in file to start reading (bytes)
; ebp    = Length of segment to read (bytes)
;
; Returns:
; eax    = Length of file that was loaded
; eax    = 0 if an error occured

loadfile:                               ; Read a file into memory
	push edi                        ; Save some registers on the stack
	push edi                        ; edi is double so we can calculate
	push ebx                        ; the file length easily later
	push ecx
	push edx
	push ebp
	push esi

	mov word ax, 3d02h              ; DOS Open file: ds:dx = *filename 
	int 21h                         ; Returns the file handle in ax
	jc fldr_error_fin               ; DOS didn't succeed :-(
	mov bx, ax                      ; Put the DOS file handle into bx

	push edx                        ; Save edx for later
	mov dx,cx                       ; Calculate the offset in the file
	shr ecx,16                      ; which needs to be put in cx:dx
	mov ax,4200h                    ; Move the file pointer to offset
	int 21h                         ; cx:dx (DOS call)
	pop edx                         ; Now we can get edx back
	jc fldr_error_fin               ; Did it go ok ?

	push ebp                        ; Save the segment size for later
	shr ebp,12                      ; Calculate amount of 4k-blocks to 
	or ebp,ebp                      ; load
	jz finish_ldr                   ; Is the file less than 4k ?

ldr_loop:                               ; Load the 4k-blocks
	mov dx, fldr_buf                ; Specify the buffer location
	mov byte ah, 3fh                ; DOS func. 3fh = Read file
	mov word cx, 4096               ; Try to read 4k into the buffer
	int 21h                         ; Read file: Returns amount read in ax
	jc fldr_error                   ; An error occured

	mov esi, fldr_buf               ; Copy from the buffer to the target
	xor ecx, ecx                    ; location in es:edi
	mov cx, ax                      ; Copy the amount that DOS read
	cld                             ; Make sure memory grows upwards
	a32 rep movsb                   ; Do it
	dec ebp                         ; Next 4k-block
	or ebp,ebp                      ; Or was that the last one ?
	jnz ldr_loop                    ; No, do another 4k-block

finish_ldr:
	pop ebp                         ; Fetch the segment size
	and ebp, 0fffh                  ; Calculate the rest to be loaded
	
	mov dx, fldr_buf                ; Specify the buffer location
	mov byte ah, 3fh                ; DOS func. 3fh = Read file
	mov cx, bp                      ; Try to read the rest into the buffer
	int 21h                         ; Read file: Returns amount read in ax
	jc fldr_error_fin               ; An error occured

	mov esi, fldr_buf               ; Copy from the buffer to the target
	xor ecx, ecx                    ; location in es:edi
	mov cx, ax                      ; Copy the amount that DOS read
	cld                             ; Make sure memory grows upwards
	a32 rep movsb                   ; Do it

	mov byte ah, 3eh                ; DOS func. 3eh:  Close file
	int 21h                         ; Do it

	pop esi                         ; Restore the registers
	pop ebp
	pop edx
	pop ecx
	pop ebx      
	pop eax                         ; Pop the old edi into eax
	sub eax, edi                    ; Calculate length of loaded file
	neg eax                         ; and return it in eax
	pop edi                         ; Restore the original edi
	ret                             ; Return to the main program

fldr_error:                             ; An error occured while loading
	pop ebp                         ; If not we'll corrupt the stack
fldr_error_fin:                         ; Or in the last few bytes        
	xor eax,eax                     ; Return 0 in eax
	pop esi                         ; Restore the registers
	pop ebp
	pop edx
	pop ecx
	pop ebx
	pop edi
	pop edi
	ret                             ; Return to the main code

; loadELF:     Loads an ELF file to the appropiate memory locations
;              WARNING: ONLY WORKS IN (un-) REAL MODE !!!!!!!!!!!!!
;
; Takes parameters:
; ds:dx  = Address of ASCIIZ filename
;
; Returns:
; eax    = 1 if everything went OK
; eax    = 0 if an error occured

loadELF:
	push es                         ; Save some registers on the stack
	push edi
	push ecx
	push ebp
	push ebx

	xor ax,ax                       ; Gain access to all mamory though es
	mov es,ax
	
	xor edi,edi                     ; Calculate the address for the
	mov di,cs                       ; ELF file header
	shl edi,4
	add edi,Elf32_Ehdr
	xor ecx,ecx                     ; Start reading at file offset 0x0h
	mov ebp,80h                     ; The ELF header is 80h bytes in size
	call loadfile                   ; Read the header
	or ax,ax                        ; Did it go OK ?
	jz near ldr_ELF_err             ; No, so exit with error
	cmp dword [Elf32_Ehdr],464c457fh  ; The ELF signature is \07fELF
	jne near ldr_ELF_err            ; Ugh... not an ELF file !!!
	cmp word [Elf32_Ehdr+4],101h    ; It should be statically linked etc.
	jne near ldr_ELF_err
	cmp byte [Elf32_Ehdr+6],1
	jne near ldr_ELF_err

	mov eax,[Elf32_Ehdr+18h]        ; Get the ELF entry point e_entry
	mov [krnl_entry],eax            ; And store it in krnl_entry
	cmp eax,100000h                 ; The entry should be > 1M
	jb ldr_ELF_err                  ; Ugh... error

	xor ecx,ecx                     ; Get the number of sections in cx
	mov cx,[Elf32_Ehdr+2ch]         

sectionloop:
	dec cx                          ; Next section
	push cx                         ; Save cx on the stack while we load
					; the section into memory

	mov eax,[Elf32_Ehdr+2ah]        ; Get the program header entry size
	push edx                        ; Save our filename offset (the mul
					; instruction can clobber it up)
	mul cx                          ; Calculate the offset from the start
					; of the program header table
	pop edx                         ; Get it again
	mov ebx,[Elf32_Ehdr+1ch]        ; Get the PHT offset in ebx
	add ebx,eax                     ; Add it to our PHT entry offset
	add ebx,Elf32_Ehdr              ; Calculate the address of the entry

	cmp dword [bx],1                ; Does this section have to be
					; loaded into memory ?
	jne nextsect                    ; No, next section

	mov dword ecx,[bx+4h]           ; Get the offset of the segment in
					; the ELF file

	mov dword ebp,[bx+10h]          ; Get the size of the segment in the
					; ELF file

	mov dword edi,[bx+8h]           ; Get the memory address of the sect.
	cmp edi,100000h                 ; It should be above 1M
	jb ldr_ELF_err_popcx            ; Blah

	mov dword eax,[bx+14h]          ; Get the size of the section in
	mov ebx,eax                     ; the memory into ebx

	call loadfile                   ; Load the segment !!!
	or eax,eax                      ; Did it work ?
	jz ldr_ELF_err_popcx            ; Nah

	sub ebx,eax                     ; This amount needs to be zeroed
	jz nextsect                     ; It's ok, next section
	add edi,eax                     ; Zero the memory from this address
	xor ax,ax                       ; edi is an absolute address
	mov ecx,ebx
	call zero_memblock              ; Zero the rest of the section

nextsect:
	pop cx                          ; Restore our section count
	or cx,cx                        ; Was this the last one ?
	jnz sectionloop                 ; No, do another section

	pop ebx                         ; Restore the registers
	pop ebp
	pop ecx
	pop edi
	pop es
	mov eax,1                       ; Return 'everything OK'
	ret                             ; Return to the main code

ldr_ELF_err_popcx:
	pop cx                          ; So we don't corrupt the stack
ldr_ELF_err:
	pop ebx                         ; Restore the registers
	pop ebp
	pop ecx
	pop edi
	pop es
	xor eax,eax                     ; Return 'error occured'
	ret                             ; Return to the main code

; countmemory:  Counts the memory
;
; Takes parameters:
; nothing
;
; Returns:
; eax    = Size of memory in bytes

countmemory:                            ; Count the amount of memory
					; Assumes GateA20 is set, returns
					; eax = amount of memory (bytes)
	push ebx                        ; Save some registers on the stack
	push edx
	push edi
	push ds

	mov ax, 0                       ; Start counting from absolute
	mov ds, ax                      ; address 0h

	mov eax, 'sign'                 ; This signature is used to recognise 
					; whether some memory exists
	mov ebx, 100ff0h                ; Assume 1MB of memory at least
					; We _should_ check this using the
					; CMOS data
countloop:     
	mov dword edx, [ebx]            ; Save what was on offset bx of ds
	mov dword [ebx], eax            ; Put the signature on this offset

	mov dword edi, [ebx]            ; Get the signature in di from ds
	mov dword [ebx], edx            ; Restore the old value

	cmp edi, eax                    ; Was the signature actually stored ?
	jnz counted_mem                 ; If not, we know the memory size

	add ebx, 1000h                  ; Count another page (4k) of memory
	jmp countloop

counted_mem:
	mov eax, ebx                    ; Move the output to ax
	sub eax, 1000h                  ; Calculate last address that existed
	add eax, 10h

	pop ds                          ; Restore the registers
	pop edi
	pop edx
	pop ebx
	ret                             ; Return to main code

; convert_decimal:  Converts a number to a right-aligned decimal string
;
; Takes parameters:
; eax    = Number to convert
; ds:edi = Place to put the string (end of it !)
;
; Returns:
; nothing (assume ok, no overflow etc.)

convert_decimal:                        ; Convert a number to decimal
	push eax                        ; Save all of the registers
	push edx
	push esi
	push edi

	mov esi,10                      ; Will divide by 10
non_zero:
	xor edx,edx                     ; No high dword
	div esi                         ; edx:eax = eax/esi
	add edx,'0'                     ; Convet to ASCII charachter
	mov byte [ds:edi],dl            ; 'Print' it
	dec edi                         ; One more digit on the stack
	or eax,eax                      ; Got zero ?
	jne non_zero                    ; No, do another digit

	pop edi                         ; Restore the registers
	pop esi
	pop edx
	pop eax
	ret                             ; Return to the main program

; --------------------------------------------------------------------------
; Macros

; Descriptor declaration macro
;
; Syntax:
; desc  offset, selector, flags     ;For gate descriptors
; desc  base, limit, flags          ;For all other descriptors
;
;  base    is the full 32 bit base address of the segment
;  limit   is one less than the segment length in 1 or 4K byte units
;  flags   the sum of all the "D_" equates which apply (for call gates, you
;          also add the "parameter dword count" to flags).

D_DATA          EQU     1000h   ;Data segment
D_CODE          EQU     1800h   ;Code segment
D_PRESENT       EQU     8000h   ;Present
D_WRITE         EQU      200h   ;Writable (Data segments only)
D_READ          EQU      200h   ;Readable (Code segments only)
D_BIG           EQU       40h   ;Default to 32 bit mode (USE32)
D_BIG_LIM       EQU       80h   ;Limit is in 4K units

%define work_around_nasm_prepend_bug

%macro desc 3
work_around_nasm_prepend_bug
%if (%3) & (~(%3)>>2) & 0x400   ;Gate
	dw      %1
	dw      %2
	dw      (%3)+D_PRESENT
	dw      (%1) >> 16
%else                           ;Not a gate
	dw      %2
	dw      %1
	db      (%1) >> 16
	db      ((%3)+D_PRESENT) >> 8
	db      (%3) + ((%2) >> 16)
	db      (%1) >> 24
%endif
%endmacro

; --------------------------------------------------------------------------

section .data                           ; Our data starts here
	
	; The filename of the kernel

	kernel_fname    db 'kernel.elf',0

	; The command line to be passed to the kernel (unused for now)

	kernel_cmdline  db 0
	
	; Messages and such

	video_seg       dw 0b800h       ; The video segment 

	welcome_msg     db 0,0
			db 'Alliance DOS boot loader (version 0.1a)',0
	copyright_msg1  db 0,1
			db '(C) 1998 The Alliance Operating System Team. All rights reserved',0
	copyright_msg2  db 0,2
			db 'THIS SOFTWARE IS DISTRIBUTED UNDER THE TERMS OF THE '
			db 'GNU PUBLIC LICENSE',0
	check_386_msg   db 0,4
			db 'Checking for 386+ processor... ',0
	i386_not_found  db 0,5
			db 'No 386 compatible processor found !  Alliance '
			db 'will only work on 386+.  Sorry.',0
	check_realmode  db 0,5
			db 'Checking whether the processor is in real-'
			db 'mode... ',0
	i386_in_pm_msg  db 0,6
			db 'The processor was already in protected mode !!! '
			db ' Aborting... ',0
	enable_A20_msg  db 0,7
			db 'Enabling the A20 gate... ',0
	kbc_timeout_msg db 0,8
			db 'A20 gate activation failed... Reason: KBC timeout'
			db ' ',0
	going_unreal    db 0,8
			db 'Attempting to enter unreal-mode... ',0
	unreal_inside   db 40,8
			db 'We have entered unreal-mode ! ',0
	mem_count_msg   db 0,10
			db 'Counting memory... ',0
	mem_size_msg    db 0,11
			db 'Memory count found: '
			db '      '                     ; Room for number
	memsize_dec     db '  k total memory ',0
	kernel_loading  db 0,13
			db 'Loading kernel... ',0
	file_error_msg  db 0,14
			db 'An error occured while loading the '
			db 'kernel - File not found, Read error, or ',0
	file_error_msg2 db 0,15
			db 'not a valid ELF file',0
	gdt_msg         db 0,15
			db 'Setting up the GDT for protected mode...',0
	load_gdt_msg    db 0,16
			db 'Loading the GDT...',0
	going_all_P     db 0,18
			db 'Switching to Protected Mode... ',0
	PM_is_here      db 40,18
			db 'We have entered Protected Mode ! ',0
	exc_kernel      db 0,20
			db 'Executing kernel...',0

	; The GDT for unreal-mode

	unreal_gdt dw      unreal_gdt_size       ; GDT for 'unreal'-mode
		   dd      unreal_gdt
		   dw      0
	un_code    desc    0, 0xFFBFF, D_CODE+D_READ+D_BIG+D_BIG_LIM
	un_data    desc    0, 0xFFFFF, D_DATA+D_WRITE+D_BIG+D_BIG_LIM
	unreal_gdt_size    equ $-unreal_gdt-1

	; The GDT for protected-mode

	gdt        dw      gdt_size              ; All segments are flat,
		   dd      gdt                   ; which is required by
		   dw      0                     ; multiboot
	code32     desc    0, 0fffffh, D_CODE + D_BIG + D_BIG_LIM + D_READ
	data32     desc    0, 0fffffh, D_DATA + D_BIG + D_BIG_LIM + D_WRITE
	stack32    desc    0, 0fffffh, D_DATA + D_BIG + D_BIG_LIM + D_WRITE
	gdt_size           equ $-gdt-1

multiboot_info:                 ; The multiboot_info structure
	dd 1h                   ; Flags
	dd 640                  ; mem_lower - lower memory in kb
mem_upr dd 0h                   ; mem_upper - upper memory in kb (patch it)
	dd 0h                   ; boot_device         - UNDEFINED (for now)
	dd kernel_cmdline       ; ASCIIZ cmdline      - UNDEFINED (for now)
	dd 0h                   ; mods_count          - UNDEFINED (for now)
	dd 0h                   ; mods_addr           - UNDEFINED (for now)
	dd 0h,0h,0h,0h          ; sym1,sym2,sym3,sym4 - UNDEFINED (for now)
	dd 0h                   ; mmap_length         - UNDEFINED (for now)
	dd 0h                   ; mmap_addr           - UNDEFINED (for now)

; --------------------------------------------------------------------------

section .bss                            ; This is our uninitialised data

	fldr_buf        resb 4096       ; The 4k buffer for loading a file

	Elf32_Ehdr      resb 80h        ; The ELF header
	
	mem_size        resd 1          ; The size of the memory
	
	krnl_entry      resd 1          ; The kernel entry point

	loader_stack    resb 4096       ; An 4k stack should be enough
	ldr_stack_end:                  ; The stack grows down from here

; --------------------------------------------------------------------------
;
; Some useful documentation for the ELF loader:
; ---------------------------------------------
;
; Structures needed for loading the ELF kernel file:
;
;              ; ELF File Header Format (Elf32_Ehdr)
; 0x00000      db 0x7f, 'E', 'L', 'F', 1, 1, 1   ; EI_IDENT
; 0x00007      times 9 db 0                      ; EI_PAD
; 0x00010      dw 0x2                            ; EI_TYPE (executable)
; 0x00012      dw 0x3                            ; e_machine
; 0x00014      dd 0x6                            ; e_version
; 0x00018      dd e_entry                        ; Entry point
; 0x0001c      dd e_phoff                        ; Program header table off.
; 0x00020      dd e_shoff                        ; Section header table off.
; 0x00024      dd e_flags                        ; Flags
; 0x00028      dw e_ehsize                       ; ELF header size in bytes
; 0x0002a      dw e_phentsize                    ; Program header entry size
; 0x0002c      dw e_phnum                        ; Number of entries in PHT
; 0x0002e      dw e_shentsize                    ; Section header entry size
; 0x00030      dw e_shnum                        ; Number of entries in SHT
; 0x00032      dw e_shstrndx                     ; Section Hdr. Table index
;
;              ; ELF Program Header Format (Elf32_Phdr)
; 0x00000      dd p_type                         ; Section type
; 0x00004      dd p_offset;                      ; File offset
; 0x00008      dd p_vaddr;                       ; Memory address
; 0x0000c      dd p_paddr;                      
; 0x00010      dd p_filesz;                      ; Size of section in file
; 0x00014      dd p_memsz;                       ; Size of section in memory
; 0x00018      dd p_flags;                       ; Flags
; 0x0001c      dd p_align;                       ; Alignment
;
; --------------------------------------------------------------------------
	
; The end
