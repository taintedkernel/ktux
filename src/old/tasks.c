
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

 // tasks.c - task manager and threads

#include <tasks.h>
#include <video.h>					// putch
#include <process.h>

// dependencies
#include <selectors.h>				// KERNEL_CODE_SELECTOR
#include <sched.h>					// schedule()
#include <memory.h>					// allocatePage()

// variables
//extern process *_curr_task;			// sched.c


thread threads[10];					// 10 concurrent threads
int currentTask = -1;

void initTasks()
{
	createTask(0, task1);
	createTask(1, task2);
	//createTask(2, task3);
	//createTask(3, task4);
}

void createTask(int id, void (*thread)())
{
	unsigned int *stack;

	// Allocate a page and point to end
	threads[id].esp0 = allocatePage() + PAGE_SIZE;

	// Make a pointer to the stack
	stack = (unsigned int *)threads[id].esp0;

	// Hardware-pushed registers
	*--stack = 0x0202;						// EFLAGS
	*--stack = KERNEL_CODE_SELECTOR;		// CS
	*--stack = (unsigned int)thread;		// EIP

	// PUSHA-pushed registers
	*--stack = 0;							// EDI
	*--stack = 0;							// ESI
	*--stack = 0;							// EBP
	*--stack = 0;							// Offset
	*--stack = 0;							// EBX
	*--stack = 0;							// EDX
	*--stack = 0;							// ECX
	*--stack = 0;							// EAX

	// Pushed segments by IRQ handler
	*--stack = KERNEL_DATA_SELECTOR;		// DS
	*--stack = KERNEL_DATA_SELECTOR;		// ES
	*--stack = KERNEL_DATA_SELECTOR;		// FS
	*--stack = KERNEL_DATA_SELECTOR;		// GS

	// Update stack pointer
	threads[id].esp0 = (unsigned int)stack;
}

unsigned int taskSwitch(unsigned int oldESP)
{
	memcpy(oldESP, threads[currentTask].edi, -68);
	
	if (currentTask != -1)
	{
		threads[currentTask].esp0 = oldESP;
		
		if (currentTask == 0)
			currentTask = 1;
		else
			currentTask = 0;
	}
	else
		currentTask = 0;
		
	return threads[currentTask].esp0;
}

static int write(const unsigned char *str, unsigned len)
{
	unsigned int i;

	for(i = 0; i < len; i++)
	{
		putch(*str);
		str++;
	}

	return i;
}

/*** Static tasks below (threads at this point) ***/

#define	WAIT	0xFFFL

/*static void yield(void)
{
	schedule();
}*/

static void wait(void)
{
	unsigned long wait;

	for(wait = WAIT; wait != 0; wait--)
		/* nothing */;
}

void taska(void)
{
	unsigned char* vidMemChar = (unsigned char *)0xB8001;
	for(;;) *vidMemChar++;
}

void taskb(void)
{
	unsigned char* vidMemChar = (unsigned char *)0xB8003;
	for(;;) *vidMemChar++;
}

void task1(void)
{
	static const unsigned char msg_a[] = "hello from task 1 ";
	static const unsigned char msg_b[] = "task 1 ";

	write(msg_a, sizeof(msg_a));
	wait();
	while(1)
	{
		//yield();
		write(msg_b, sizeof(msg_b));
		wait();
	}
}

void task2(void)
{
	static const unsigned char msg_a[] = "hola de tarea 2 ";
	static const unsigned char msg_b[] = "tarea 2 ";

	write(msg_a, sizeof(msg_a));
	wait();
	while(1)
	{
		//yield();
		write(msg_b, sizeof(msg_b));
		wait();
	}
}

void task3(void)
{
	static const unsigned char msg_a[] = "Hallo von Aufgabe 3 ";
	static const unsigned char msg_b[] = "Aufgabe 3 ";

	write(msg_a, sizeof(msg_a));
	wait();
	while(1)
	{
		//yield();
		write(msg_b, sizeof(msg_b));
		wait();
	}
}
/*****************************************************************************
this character -->	ƒ
is the reason we use unsigned char instead of char
*****************************************************************************/
void task4(void)
{
	static const unsigned char msg_a[] = "Bonjour de tƒche 4 ";
	static const unsigned char msg_b[] = "tƒche 4  ";
/**/

	write(msg_a, sizeof(msg_a));
	wait();
	while(1)
	{
		//yield();
		write(msg_b, sizeof(msg_b));
		wait();
	}
}
