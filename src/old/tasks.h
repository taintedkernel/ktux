/***************************************************************************
 *   Copyright (C) 2004 by Anthony DeChiaro                                *
 *   axd6491@njit.edu                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

 // tasks.h

#define MAX_TASKS 10

 typedef volatile struct {
 	unsigned int edi, esi, ebp, null, ebx, edx, ecx, eax;
 	unsigned int ds, es, fs, gs;
 	unsigned int eip, cs, eflags, esp, ss;
 	unsigned int esp0;
	unsigned int esp3;
 } __attribute__ ((packed)) thread;


void initTasks(void);
void createTask(int id, void (*thread)());
unsigned int taskSwitch(unsigned int oldESP);

void taska(void);
void taskb(void);
void task1(void);
void task2(void);
void task3(void);
void task4(void);

