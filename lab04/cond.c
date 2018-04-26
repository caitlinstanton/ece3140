#include "cond.h"
#include "lock.h"

/**
Initializes the fields of a condition type
*/
void c_init( lock_t* l, cond_t* c )
{
	c->wait_queue = NULL; // At start, nothing waiting in the queue
}

/**
Waits for a condition to be signaled true
Places a process in the waiting queue (atomic)
Releases the lock
*/
void c_wait( lock_t* l, cond_t* c )
{
	NVIC_DisableIRQ(PIT0_IRQn);         // Disable interrupts to be atomic
	l_unlock( l );                      // Release the lock
	cond_enqueue( c, current_process ); // Enqueue waiting process in wait_queue
	current_process->is_blocked = 1;    // Signal that process is blocked b/c waiting
	NVIC_EnableIRQ(PIT0_IRQn);
	process_blocked();                  // Don't wait spinning, execute the next process
	l_lock( l );                        // Reacquire the lock
}

/**
Signals that a condition is met (atomic)
Takes a process off the waiting queue if there are any waiting
Releases the lock
*/
void c_signal( lock_t* l, cond_t* c )
{
  NVIC_DisableIRQ(PIT0_IRQn);     // Disable interrupts to make atomic
	
	if ( c_waiting( l, c ) == 0 ) { // Nothing waiting
		l_unlock( l );                // Release lock
		NVIC_EnableIRQ(PIT0_IRQn);
		return;
	}
	process_t* waiting_process = cond_dequeue( c ); // Dequeue next waiting process
	lock_enqueue( l, waiting_process );             // Now wait till lock acquired
	l_unlock( l );                                  // Release the lock & dequeue ^^ process
	NVIC_EnableIRQ(PIT0_IRQn);
	return;
}

/**
Checks if there are any waiting processes blocked
Returns 0 if no processes are waiting
Returns 1 if there are processes waiting
*/
int c_waiting( lock_t* l, cond_t* c )
{
	int waiting;
	if ( c->wait_queue != NULL ) waiting = 1; // Waiting processes in queue
	else waiting = 0;      // Nothing in waiting queue, no processes waiting
	return waiting;
}

/** 
Dequeues waiting process from condition's wait_queue
*/
process_t* cond_dequeue( cond_t* c )
{
	process_t* cond_head = c->wait_queue;          // Get first process in waiting queue
	c->wait_queue = cond_head->next_process_ptr;   // Set condition_queue as the next process
	cond_head->next_process_ptr = NULL;            // Separate old head process from queue
	return cond_head;
}

/**
Enqueues waiting process onto cond's condition_queue
*/
void cond_enqueue( cond_t* cond, process_t* waiting_process )
{
	// if the locked_process was NULL to begin with
	if ( waiting_process == NULL ) return;
	
	// if lock's lock_queue is empty (NULL)
	if ( cond->wait_queue == NULL ) {
    cond->wait_queue = waiting_process;
    cond->wait_queue->next_process_ptr = NULL;	
		return;
	}
	
	// Traverse through end of linked list of blocked processes
  process_t* tmp = cond->wait_queue;
	while ( tmp->next_process_ptr != NULL ){
	  tmp = tmp->next_process_ptr;
	}
	// Add new process to end of linked list
	tmp->next_process_ptr = waiting_process;
	waiting_process->next_process_ptr = NULL;
	return;
}
