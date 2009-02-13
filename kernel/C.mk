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
	${INCDIR}/sh.h \
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
	
sh.o: \
	sh.c
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

syscall.o: \
	syscall.c \
	${INCDIR}/syscall.h
	${CC} -c -o $@ $<

tasks.o: \
	tasks.c \
	${INCDIR}/tasks.h
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