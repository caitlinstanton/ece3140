#include "lock.h"

/** 
Dequeues blocked process from lock's lock_queue
*/
process_t* lock_dequeue( lock_t* lock )
{
	process_t* lock_head = lock->lock_queue;        // Get first process in blocked queue
	lock->lock_queue = lock_head->next_process_ptr; // Set blocked_queue as the next process
	lock_head->next_process_ptr = NULL;             // Separate old head process from queue
	return lock_head;
}

/**
Enqueues blocked process onto lock's lock_queue
*/
void lock_enqueue( lock_t* lock, process_t* locked_process )
{
	// if the locked_process was NULL to begin with
	if ( locked_process == NULL ) return;
	
	// if lock's lock_queue is empty (NULL)
	if ( lock->lock_queue == NULL ) {
    lock->lock_queue = locked_process;
    lock->lock_queue->next_process_ptr = NULL;	
		return;
	}
	
	// Traverse through end of linked list of blocked processes
  process_t* tmp = lock->lock_queue;
	while ( tmp->next_process_ptr != NULL ){
	  tmp = tmp->next_process_ptr;
	}
	// Add new process to end of linked list
	tmp->next_process_ptr = locked_process;
	locked_process->next_process_ptr = NULL;
	return;
}

/**
Initializes a specific lock's fields
*/
void l_init( lock_t* l )
{
	l->lock_in_use = 0;   // unlocked == 0 by convention
	l->lock_queue = NULL; // nothing in blocked queue at start 
}

/**
Locks a process (atomic)
If lock is in use, remove process from ready queue and put
into locked queue
If lock not in use, give the ready process the lock
*/
void l_lock( lock_t* l )
{
	NVIC_DisableIRQ(PIT0_IRQn);           // Disable interrupts to make atomic
	if ( l->lock_in_use == 1 ){           // lock aquired by a previous process
    current_process->is_blocked = 1;    
		lock_enqueue( l, current_process ); // add process to lock's lock queue
		NVIC_EnableIRQ(PIT0_IRQn);          // Enable interrupts back
		process_blocked();
	}
	l->lock_in_use = 1;                   // Lock wasn't in use before
	NVIC_EnableIRQ(PIT0_IRQn);
	return;
}

/**
Unlocks a process (atomic)
Hands off lock to next ready blocked/locked process by adding them to
the ready queue.
If there's no one to hand the lock off to, release the lock
*/
void l_unlock( lock_t* l )
{
	NVIC_DisableIRQ(PIT0_IRQn);                   // Disable interrupts to make atomic
	if( l->lock_queue != NULL ) {
    process_t* head_locked = lock_dequeue( l ); // Lock queue not empty, hand lock over
    head_locked->is_blocked = 0;                // New unlocked process
    enqueue( head_locked );                     // Add unlocked process to ready queue
		// TODO condense this code because it always say slock notn in use?
	  l->lock_in_use = 0;                         // Lock currently not in use
	  NVIC_EnableIRQ(PIT0_IRQn);
		return;
	}
	l->lock_in_use = 0;                           // Release the lock
  NVIC_EnableIRQ(PIT0_IRQn);                    // Enable interrupts back again
	return;
}
