#***************************************************************************
#*   Copyright (C) 2004 by Anthony DeChiaro                                *
#*   axd6491@njit.edu                                                      *
#*                                                                         *
#*   This program is free software; you can redistribute it and/or modify  *
#*   it under the terms of the GNU General Public License as published by  *
#*   the Free Software Foundation; either version 2 of the License, or     *
#*   (at your option) any later version.                                   *
#*                                                                         *
#*   This program is distributed in the hope that it will be useful,       *
#*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
#*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
#*   GNU General Public License for more details.                          *
#*                                                                         *
#*   You should have received a copy of the GNU General Public License     *
#*   along with this program; if not, write to the                         *
#*   Free Software Foundation, Inc.,                                       *
#*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
#***************************************************************************

# Makefile for Ktux kernel

STDDEPEND	= Makefile
INCDIR		= ../include

LDSCRIPT	= kernel.ld
AS			= /usr/bin/nasm

BINUTILS	= /usr/cross/bin
CC			= ${BINUTILS}/i686-elf-gcc -g -march=pentium -Wall -W -O1 -nostdinc -fno-builtin -I$(INCDIR) #-fwritable-strings
CC_DEP		= ${BINUTILS}/i686-elf-gcc -g -march=pentium -Wall -W -O1 -nostdinc -fno-builtin -MM -I$(INCDIR) #-fwritable-strings
LD			= ${BINUTILS}/i686-elf-ld -g -T$(LDSCRIPT) -nostdlib
OBJDUMP		= ${BINUTILS}/i686-elf-objdump
#OBJCOPY	= ${BINUTILS}/i686-elf-objcopy
NM			= ${BINUTILS}/i686-elf-nm

#LINKOPTS	= -mcpu=pentium -Wl,-warn-common,-X,--oformat,binary,-e,kloaderMain

### kernelInit MUST BE FIRST ###
# For some reason ld doesn't follow the "entry" directive in my linker script #
KERNELOBJS	= kernelInit.o kernel.o hardware.o interrupts.o io.o isr.o keyboard.o memory.o \
			paging.o pic.o process.o sh.o stdio.o string.o syscall.o tasks.o timer.o tss.o video.o

HEADERS		= $(INCDIR)/_va_list.h $(INCDIR)/conio.h $(INCDIR)/error.h $(INCDIR)/hardware.h $(INCDIR)/interrupts.h \
			$(INCDIR)/io.h $(INCDIR)/isr.h $(INCDIR)/keyboard.h ${INCDIR}/list.h $(INCDIR)/memory.h $(INCDIR)/paging.h \
			$(INCDIR)/pic.h $(INCDIR)/stdarg.h $(INCDIR)/stdio.h $(INCDIR)/string.h $(INCDIR)/timer.h $(INCDIR)/vga.h \
			$(INCDIR)/video.h $(INCDIR)/process.h $(INCDIR)/sh.h $(INCDIR)/syscall.h $(INCDIR)/tasks.h


##############
# Ktux files #
##############

KERNELOBJ	= ktuxkrnl
#KERNELBIN	= ktuxkrnl
KERNELLST	= ktuxkrnl.lst
KERNELSYM	= ktuxkrnl.sym

#############################
# Start of Makefile targets #
#############################

all:		clean default
default:	${KERNELOBJ} #${KERNELBIN}

deps:
	../mkfiles.pl
	../mkdep.pl ${INCDIR}

clean:
	-rm -f *.o *.d *.lst *.bak ${KERNELOBJ} ${KERNELSYM} ${KERNELLST} __bfe.log__ #${KERNELBIN}

# Kernel target
${KERNELOBJ}: ${KERNELOBJS} ${LDSCRIPT} ${HEADERS} ${STDDEPEND}
	$(LD) -o $@ $(KERNELOBJS)
	${OBJDUMP} --line-numbers --source ${KERNELOBJ} >${KERNELLST}
	/usr/bin/unix2dos ${KERNELLST}
	# This sed line is a bit of a hack
	${NM} --line-numbers --debug-syms --numeric-sort ${KERNELOBJ} | sed "s|`pwd`|./kernel|g" >${KERNELSYM}

# Kernel binary target
#${KERNELBIN}: ${KERNELOBJ} ${STDDEPEND}
	#${OBJCOPY} -O binary ${KERNELOBJ} ${KERNELBIN}
	
#%.o: %.s
#	${AS} -f elf -w+orphan-labels -o $@ $<
#%.o: %.c %.h
#	${CC} -c -o $@ $<

include 	C.mk
include 	asm.mk