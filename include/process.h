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

// process.h

#ifndef __KTUX_PROCESS_H
#define __KTUX_PROCESS_H

#include <list.h>
#include <video.h>
//#include <memory.h>

// scheduler constants
#define MAX_THREADS			1024
#define INT_PER_SLICE		64

// thread constants
#define PATH_SIZE			256
#define NAME_SIZE			64
#define STACK_SIZE			0x200

// process privledge
#define THREAD_KERNEL		0
#define THREAD_USER			1

// process status
#define PROCESS_INIT		1
#define PROCESS_READY		2
#define PROCESS_RUNNING		3
#define PROCESS_WAIT		4
#define PROCESS_SLEEP		5
#define PROCESS_STOP		6
#define PROCESS_ZOMBIE		7
#define PROCESS_FREE		8

// calculate length of timeslice (# timer interrupts before context switch)
#define QuantumAmount(thread) (INT_PER_SLICE - thread->priority)

// structs
typedef volatile struct {
	unsigned int link;
	volatile unsigned int esp0;
	unsigned int ss0;
	unsigned int esp1;
	unsigned int ss1;
	unsigned int esp2;
	unsigned int ss2;
	unsigned int cr3;
	unsigned int eip;
	unsigned int eflags;
	unsigned int eax;
	unsigned int ecx;
	unsigned int edx;
	unsigned int ebx;
	unsigned int esp;
	unsigned int ebp;
	unsigned int esi;
	unsigned int edi;
	unsigned int es;
	unsigned int cs;
	unsigned int ss;
	unsigned int ds;
	unsigned int fs;
	unsigned int gs;
	unsigned int ldtr;
  	unsigned short trace;
  	unsigned short io_map_addr;
} __attribute__ ((packed)) tss_struct;

/*typedef volatile struct
{
	unsigned int ds;
	unsigned int es;
	unsigned int fs;
	unsigned int gs;
	unsigned int edi;
	unsigned int esi;
	unsigned int ebp;
	unsigned int dummy_esp;
	unsigned int ebx;
	unsigned int edx;
	unsigned int ecx;
	unsigned int eax;
	unsigned int eip;
	unsigned int cs;
	unsigned int eflags;
	unsigned int esp;
	unsigned int ss;
} __attribute__ ((packed)) pmode_task_struct;*/

struct kernel_thread_struct
{
	char name[NAME_SIZE];
	unsigned int id;
	int priority;
	unsigned int status;
	unsigned int priv;
	unsigned int esp0;
	unsigned int esp3;
	unsigned int esp0base;
	unsigned int esp3base;
	unsigned int cr3;
	unsigned int time;
	unsigned int quantum;
	list_head list;
	console *tty;
};

typedef volatile struct kernel_thread_struct kernel_thread_t;

typedef volatile struct
{
	unsigned int esp0;
	unsigned int esp3;
	unsigned int slice;
	//unsigned int pid;
} __attribute__ ((packed)) user_thread_struct;


// function declarations
void init_multitasking();
void start_scheduler();

void yield();
unsigned int create_kernel_thread(char *name, void (*)());

void nice(int);
void p_nice(kernel_thread_t *, int);
unsigned int alloc_pid(void);
unsigned int get_num_threads();
unsigned int p_get_page_directory(kernel_thread_t *, unsigned int);

void ps();
void kill();
void do_idle();

#endif

/*typedef volatile struct
{
	char name[NAME_SIZE];

	unsigned short id;
	unsigned short parentId;
	unsigned short userId;
	int priority;
	processStatus status;
	jmp_buf state;
	console *vc;

	unsigned int startTime;

	// environment variables
	char *argc;
	int argv;
	char directory[PATH_SIZE];

	// memory management
	unsigned long size;
	unsigned long *pageTable;
	unsigned long *pageDirectory;
	void *heap;

	// segments
	void *codeSegment;
	unsigned int codeSegmentSize;
	void *userStack;
	unsigned int userStackSize;
	void *superStack;
	unsigned int superStackSize;
} newProcess;*/

/*char processName[MAX_PROCNAME_LENGTH];
  int userId;
  int processId;
  kernelProcessType type;
  int priority;
  int privilege;
  int parentProcessId;
  int descendentThreads;
  unsigned int startTime;
  unsigned cpuTime;
  int cpuPercent;
  unsigned int waitTime;
  unsigned int waitUntil;
  int waitForProcess;
  int waitThreadTerm;
  kernelProcessState state;
  void *codeDataPointer;
  unsigned int codeDataSize;
  void *userStack;
  unsigned int userStackSize;
  void *superStack;
  unsigned int superStackSize;
  kernelSelector tssSelector;
  kernelTSS taskStateSegment;
  char currentDirectory[MAX_PATH_LENGTH];
  kernelEnvironment *environment;
  kernelTextStream *textInputStream;
  kernelTextStream *textOutputStream;*/
