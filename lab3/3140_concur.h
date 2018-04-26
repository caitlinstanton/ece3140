/*************************************************************************
 *
 *  Copyright (c) 2015 Cornell University
 *  Computer Systems Laboratory
 *  Cornell University, Ithaca, NY 14853
 *  All Rights Reserved
 *
 *  $Id$
 *
 **************************************************************************
 */
#ifndef __3140_CONCUR_H__
#define __3140_CONCUR_H__

#include <stdlib.h>
#include <fsl_device_registers.h>

struct process_state;
typedef struct process_state process_t;
   /* opaque definition of process type; you must provide this
      implementation.
			
			To do this, write code similar to the following in your process.c:
			
			struct process_state {
				int var1;
				struct process_state* some_pointer;
			};
   */

/*------------------------------------------------------------------------

   THE FOLLOWING FUNCTIONS MUST BE CREATED IN process.c.

------------------------------------------------------------------------*/

/* ====== Concurrency ====== */

/* Called by the runtime system to select another process.
   "cursp" = the stack pointer for the currently running process
	 cursp will be NULL when first starting the scheduler, and when a process terminates
	 Return the stack pointer for the new process to run, or NULL to exit the scheduler.
*/
unsigned int * process_select (unsigned int * cursp);

/* the currently running process. current_process must be NULL if no process is running,
    otherwise it must point to the process_t of the currently running process
*/
extern process_t * current_process; 
extern process_t * process_queue;

/* Starts up the concurrent execution */
void process_start (void);

/* Create a new process. Return -1 if creation failed */
int process_create (void (*f)(void), int n);


/*------------------------------------------------------------------------
  
You may use the following functions that we have provided

------------------------------------------------------------------------*/


/* This function can ONLY BE CALLED if interrupts are disabled.
   This function switches execution to the next ready process, and is
   also the entry point for the timer interrupt.
   
   Implemented in 3140.s
*/
void process_blocked (void);

/*
  This function is called by user code indirectly when the process
  terminates. This is handled by stack manipulation.
	
	You should not need to call this function yourself.

  Implemented in 3140.s
  Used in 3140_concur.c
*/
void process_terminated (void);

/* This function can ONLY BE CALLED if interrupts are disabled. It
   does not modify interrupt flags.
	 
	 Allocates a stack for a process, and sets up the stack's initial state
	 
	 Implemented in 3140_concur.c
*/
unsigned int * process_stack_init (void (*f)(void), int n);

/* This function can ONLY BE CALLED if interrupts are disabled. It
   does not modify interrupt flags.
	 
	 Frees a stack allocated in process_init. Must be called with the same value
	 of n as process_init
	 
	 Implemented in 3140_concur.c
*/
void process_stack_free (unsigned int *sp, int n);

/*
  This function starts the concurrency by using the timer interrupt
  context switch routine to call the first ready process.

  The function also gracefully exits once the process_select()
  function returns 0.
	
	Implemented in 3140.s
*/
void process_begin (void);


#endif
