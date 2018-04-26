/**
 * @file lock.h
 * @author Skand Hurkat
 * @copyright All rights reserved 2015
 *
 * This file defines locks for the MSP430 concurrent execution
 *
 * @warning This file is offered AS IS and WITHOUT ANY WARRANTY, without
 * even the implied warranties of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#ifndef __LOCK_H_INCLUDED__
#define __LOCK_H_INCLUDED__

#include "3140_concur.h"
#include "shared_structs.h"

/**
 * Initialises the lock structure
 *
 * @param l pointer to lock to be initialised
 */
void l_init(lock_t* l);

/**
 * Grab the lock or block the process until lock is available
 *
 * @param l pointer to lock to be grabbed
 */
void l_lock(lock_t* l);

/**
 * Release the lock along with the first process that may be waiting on
 * the lock. This ensures fairness wrt lock acquisition.
 *
 * @param l pointer to lock to be unlocked
 */
void l_unlock(lock_t* l);

/*------------------------------------------------------------------------
  
Helper functions we created

------------------------------------------------------------------------*/
/* Enqueues a process onto the locked process queue */
void lock_enqueue( lock_t* lock, process_t* locked_process );
	
/* Dequeues a process from the locked process queue */
process_t* lock_dequeue( lock_t* lock );

#endif /* __LOCK_H_INCLUDED */
