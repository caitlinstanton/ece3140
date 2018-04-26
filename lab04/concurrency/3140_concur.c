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
#include "3140_concur.h"
#include <stdlib.h>

/*
  State layout:

  .-----------------.
  |     xPSR   	    | <--- status register
  |-----------------|
  |      PC         | <--- starting point of the process's function    
  |-----------------|
  |      LR         | <--- process_terminated
  |-----------------|
  |      R12        |
  |-----------------|
  |    R3 - R0      |
	|-----------------|
  |   0xFFFFFFF9    | <--- exception return value 
  |-----------------|
  |    R4 - R11     |
  |-----------------|
  |    PIT State    |
  |-----------------|


  State requires 18 slots on the stack.

 */


/*------------------------------------------------------------------------
 *
 *  process_stack_init --
 *
 *   Allocate and initialize a stack for a process
 *
 *------------------------------------------------------------------------
 */

unsigned int * process_stack_init (void (*f)(void), int n)
{
  unsigned int *sp;	/* Pointer to process stack (allocated in heap) */ 
	
	int i;

	/* in reality, there are 18 more slots needed for stored context */
	n += 18;
		
  /* Allocate space for the process's stack */
  sp = malloc(n*sizeof(int));
		 
  if (sp == NULL) { return NULL; }	/* Allocation failed */
  
  /* Initialize the stack to all zeros */ 
  /* Note: Could just use calloc instead */ 
  for (i=0; i < n; i++) {
  	sp[i] = 0;
  }
  
	sp[n-1] = 0x01000000; // xPSR
	sp[n-2] = (unsigned int) f; // PC
	sp[n-3] = (unsigned int) process_terminated; // LR
	sp[n-9] = 0xFFFFFFF9; // EXC_RETURN value, returns to thread mode
	sp[n-18] = 0x3; // Enable scheduling timer and interrupt
  
  return &(sp[n-18]);
}

/*------------------------------------------------------------------------
 *
 *  process_stack_free --
 *
 *   Free a process stack allocated by process_stack_init. Call this with the SP
 * which was returned by process_stack_init, and the stack size which was passed
 * to process_stack_init
 *
 *------------------------------------------------------------------------
 */
void process_stack_free(unsigned int *sp, int n)
{
	// process_init returned a pointer to the top of the stack, which is near
	// the end of the allocated region. We need to recover the pointer returned
	// by malloc
	unsigned int *stack_base = sp - n;
	free(stack_base);
}
