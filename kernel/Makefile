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
BINUTILS	= /usr/cross/bin
INCDIR		= ../include
LDSCRIPT	= kernel.ld
AS		= /usr/bin/nasm
CC		= ${BINUTILS}/i686-elf-gcc -g -march=pentium -Wall -W -O2 -nostdinc -fno-builtin -fwritable-strings -I$(INCDIR)
CC_DEP		= ${BINUTILS}/i686-elf-gcc -g -march=pentium -Wall -W -O2 -nostdinc -fno-builtin -fwritable-strings -MM -I$(INCDIR)
LD		= ${BINUTILS}/i686-elf-ld -g -T$(LDSCRIPT) -nostdlib
OBJDUMP		= ${BINUTILS}/i686-elf-objdump
#OBJCOPY		= ${BINUTILS}/i686-elf-objcopy
NM		= ${BINUTILS}/i686-elf-nm

#LINKOPTS	= -mcpu=pentium -Wl,-warn-common,-X,--oformat,binary,-e,kloaderMain

### kernelInit MUST BE FIRST ###
# For some reason ld doesn't follow the "entry" directive in my linker script #
KERNELOBJS	= kernelInit.o kernel.o hardware.o interrupts.o io.o isr.o keyboard.o memory.o \
			paging.o pic.o process.o stdio.o string.o timer.o tss.o video.o

HEADERS		= $(INCDIR)/_va_list.h $(INCDIR)/conio.h $(INCDIR)/error.h $(INCDIR)/hardware.h $(INCDIR)/interrupts.h \
			$(INCDIR)/io.h $(INCDIR)/isr.h $(INCDIR)/keyboard.h $(INCDIR)/memory.h $(INCDIR)/paging.h $(INCDIR)/pic.h \
			$(INCDIR)/stdarg.h $(INCDIR)/stdio.h $(INCDIR)/string.h $(INCDIR)/timer.h $(INCDIR)/vga.h $(INCDIR)/video.h \
			$(INCDIR)/process.h


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
	-/usr/bin/rm -f *.o *.d *.lst *.bak ${KERNELOBJ} ${KERNELSYM} ${KERNELLST} __bfe.log__ #${KERNELBIN}

# Kernel target
${KERNELOBJ}: ${KERNELOBJS} ${LDSCRIPT} ${HEADERS} ${STDDEPEND}
	$(LD) -o $@ $(KERNELOBJS)
	${OBJDUMP} --line-numbers --source ${KERNELOBJ} >${KERNELLST}
	/usr/bin/unix2dos ${KERNELLST}
	${NM} --line-numbers --debug-syms ${KERNELOBJ} | /usr/bin/sort >${KERNELSYM}

# Kernel binary target
#${KERNELBIN}: ${KERNELOBJ} ${STDDEPEND}
	#${OBJCOPY} -O binary ${KERNELOBJ} ${KERNELBIN}
	
#%.o: %.s
#	${AS} -f elf -w+orphan-labels -o $@ $<
#%.o: %.c %.h
#	${CC} -c -o $@ $<

# Kernel assembly sources
kernelInit.o: \
	kernelInit.s
	${AS} -f elf -w+orphan-labels -o $@ $<

isr.o: \
	isr.s
	${AS} -f elf -w+orphan-labels -o $@ $<

paging.o: \
	paging.s
	${AS} -f elf -w+orphan-labels -o $@ $<

tasks.o: \
	tasks.s
	${AS} -f elf -w+orphan-labels -o $@ $<

tss.o: \
	tss.s
	${AS} -f elf -w+orphan-labels -o $@ $<

# C sources
hardware.o: \
	hardware.c \
	${INCDIR}/_va_list.h \
	${INCDIR}/hardware.h \
	${INCDIR}/stdio.h
	${CC} -c -o $@ $<
	
interrupts.o: \
	interrupts.c \
	${INCDIR}/_va_list.h \
	${INCDIR}/error.h \
	${INCDIR}/interrupts.h \
	${INCDIR}/io.h \
	${INCDIR}/isr.h \
	${INCDIR}/ktux.h \
	${INCDIR}/pic.h \
	${INCDIR}/selectors.h \
	${INCDIR}/stdio.h
	${CC} -c -o $@ $<

io.o: \
	io.c \
	${INCDIR}/io.h
	${CC} -c -o $@ $<

kernel.o: \
	kernel.c \
	${INCDIR}/_va_list.h \
	${INCDIR}/hardware.h \
	${INCDIR}/interrupts.h \
	${INCDIR}/isr.h \
	${INCDIR}/keyboard.h \
	${INCDIR}/ktux.h \
	${INCDIR}/memory.h \
	${INCDIR}/pic.h \
	${INCDIR}/process.h \
	${INCDIR}/stdio.h \
	${INCDIR}/timer.h \
	${INCDIR}/video.h
	${CC} -c -o $@ $<

keyboard.o: \
	keyboard.c \
	${INCDIR}/_va_list.h \
	${INCDIR}/interrupts.h \
	${INCDIR}/io.h \
	${INCDIR}/keyboard.h \
	${INCDIR}/ktux.h \
	${INCDIR}/stdio.h \
	${INCDIR}/video.h
	${CC} -c -o $@ $<

memory.o: \
	memory.c \
	${INCDIR}/_va_list.h \
	${INCDIR}/hardware.h \
	${INCDIR}/memory.h \
	${INCDIR}/paging.h \
	${INCDIR}/stdio.h \
	${INCDIR}/string.h
	${CC} -c -o $@ $<

pic.o: \
	pic.c \
	${INCDIR}/_va_list.h \
	${INCDIR}/interrupts.h \
	${INCDIR}/io.h \
	${INCDIR}/ktux.h \
	${INCDIR}/pic.h \
	${INCDIR}/stdio.h
	${CC} -c -o $@ $<

process.o: \
	process.c \
	${INCDIR}/hardware.h \
	${INCDIR}/memory.h \
	${INCDIR}/process.h \
	${INCDIR}/selectors.h \
	${INCDIR}/string.h \
	${INCDIR}/video.h
	${CC} -c -o $@ $<

sched.o: \
	sched.c
	${CC} -c -o $@ $<

stdio.o: \
	stdio.c \
	${INCDIR}/_va_list.h \
	${INCDIR}/interrupts.h \
	${INCDIR}/ktux.h \
	${INCDIR}/pic.h \
	${INCDIR}/stdarg.h \
	${INCDIR}/stdio.h \
	${INCDIR}/string.h \
	${INCDIR}/video.h
	${CC} -c -o $@ $<

string.o: \
	string.c \
	${INCDIR}/string.h
	${CC} -c -o $@ $<

timer.o: \
	timer.c \
	${INCDIR}/_va_list.h \
	${INCDIR}/interrupts.h \
	${INCDIR}/io.h \
	${INCDIR}/ktux.h \
	${INCDIR}/stdio.h \
	${INCDIR}/timer.h \
	${INCDIR}/video.h
	${CC} -c -o $@ $<

video.o: \
	video.c \
	${INCDIR}/_va_list.h \
	${INCDIR}/ctype.h \
	${INCDIR}/io.h \
	${INCDIR}/ktux.h \
	${INCDIR}/stdio.h \
	${INCDIR}/string.h \
	${INCDIR}/vga.h \
	${INCDIR}/video.h
	${CC} -c -o $@ $<
	