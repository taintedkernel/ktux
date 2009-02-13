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

# Makefile for Ktux osloader

STDDEPEND		= kloader.h Makefile
BINUTILS		= /usr/cross/bin
INCDIR			= ../include
#LDSCRIPT		= loader.ld
AS				= nasm
CC				= ${BINUTILS}/i686-elf-gcc -g -march=pentium -Wall -W -O2 -nostdinc -fno-builtin -fwritable-strings -I$(INCDIR)
LD				= ${BINUTILS}/i686-elf-ld -g -T$(LDSCRIPT) -nostdlib
OBJDUMP			= ${BINUTILS}/i686-elf-objdump
OBJCOPY			= ${BINUTILS}/i686-elf-objcopy
NM				= ${BINUTILS}/i686-elf-nm
LINKOPTS		= -m486 -Wl,-warn-common,-X,--oformat,elf32-i386,-e,kloaderMain,-Ttext,0x00008000
BOOTSTUBOBJS	= klMain.o klConsoleIO.o klLoadKernel.o klHardware.o
#HEADERS			= $(INCDIR)/interrupts.h $(INCDIR)/io.h $(INCDIR)/isr.h $(INCDIR)/ktux.h $(INCDIR)/stdio.h \
#			$(INCDIR)/vga.h $(INCDIR)/video.h $(INCDIR)/keyboard.h $(INCDIR)/timer.h $(INCDIR)/pic.h

##############
# Ktux files #
##############

BOOTSECT	= bootsect.dat
BOOTSTUBOBJ	= ktuxldr.elf
BOOTSTUBBIN	= ktuxldr
BOOTSTUBLST	= ktuxldr.lst
BOOTSTUBSYM	= ktuxldr.sym

#############################
# Start of Makefile targets #
#############################

all:		clean default
default:	${BOOTSTUBOBJ} ${BOOTSTUBBIN} ${BOOTSECT}

deps:
	../mkfiles.pl
	../mkdep.pl ${INCDIR}

clean:
	-rm -f *.o *.lst ${BOOTSECT} ${BOOTSTUBOBJ} ${BOOTSTUBBIN} ${BOOTSTUBSYM} ${BOOTSTUBLST} __bfe.log__


# Bootsector (MUST BE MADE LAST!) 
# (These don't change, so just list files statically)
${BOOTSECT}: bootsect.s ${STDDEPEND}
	${AS} -f bin -w+orphan-labels bootsect.s -l ${basename $@}.lst -o $@
	unix2dos ${basename $@}.lst

# Kernel object target
${BOOTSTUBOBJ}: ${BOOTSTUBOBJS} ${STDDEPEND}
	$(CC) -nodefaultlibs -nostartfiles -O2 -Wall $(LINKOPTS) $(BOOTSTUBOBJS) -o $@

# Kernel binary target
${BOOTSTUBBIN}: ${BOOTSTUBOBJ} ${STDDEPEND}
	${OBJDUMP} --line-numbers --source ${BOOTSTUBOBJ} >${BOOTSTUBLST}
	unix2dos ${BOOTSTUBLST}
	${NM} --line-numbers ${BOOTSTUBOBJ} | sort >${BOOTSTUBSYM}
	${OBJCOPY} -O binary ${BOOTSTUBOBJ} ${BOOTSTUBBIN}

# Kernel assembly sources
%.o: %.s
	${AS} -f elf -w+orphan-labels -o $@ $<