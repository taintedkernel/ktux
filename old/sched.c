/*============================================================================
TASK CREATION AND SCHEDULING

EXPORTS:
extern task_t *_curr_task;

void schedule(void);
void init_tasks(void);
============================================================================*/
#include <setjmp.h> /* setjmp(), longjmp() */
#include <video.h> /* console_t */
#include <tasks.h>
#include <process.h>
#include <sched.h>
#include <stdio.h>

#define	NUM_TASKS	5
#define	MAX_TASK	16
#define	USER_STACK_SIZE	512

extern console _vc[MAX_VC];
extern unsigned long oldeip;

process *_curr_task;
static process _tasks[MAX_TASK];
/*****************************************************************************
*****************************************************************************/
void schedule()
{
	static unsigned current=0;

	//_curr_task->state.eip = oldeip;

/* setjmp() returning nonzero means we came here through hyperspace
from the longjmp() below -- just return */
	if(setjmp(_curr_task->state) != 0)
		return;
/* find next runnable task */
	do
	{
		current++;
		if(current >= NUM_TASKS)
			current = 1;
		_curr_task = _tasks + current;
	} while(_curr_task->status != ready);
/* jump to new task */
	longjmp(&_curr_task->state, 1);
}
/*****************************************************************************
*****************************************************************************/

void init_tasks(void)
{
	static unsigned char stacks[NUM_TASKS][USER_STACK_SIZE];
	static unsigned entry[NUM_TASKS] =
	{
		0,			(unsigned)task1,
		(unsigned)task2,	(unsigned)task3,
		(unsigned)task4
	};
/**/
	unsigned adr, i;

	kprintf("init_tasks:\n");
/* for the user tasks, initialize saved state... */
	for(i = 1; i < NUM_TASKS; i++)
	{
		(void)setjmp(_tasks[i].state);
/* ...especially the stack pointer */
		adr = (unsigned)(stacks[i] + USER_STACK_SIZE);
		_tasks[i].state.esp = adr;
/* ...and program counter */
		_tasks[i].state.eip = entry[i];
/* set EFLAGS value to enable interrupts */
		_tasks[i].state.eflags = 0x200;
/* allocate a virtual console to this task */
		_tasks[i].vc = _vc;
/* mark it runnable */
		_tasks[i].status = ready;
	}
/* mark task #0 (idle task) runnable */
	_tasks[0].status = ready;
/* set _curr_task so schedule() will save state of task #0 */
	_curr_task = _tasks + 0;
}
