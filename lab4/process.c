#include "3140_concur.h"
#include "shared_structs.h"
#include <stdlib.h>
#include <fsl_device_registers.h>

struct process_state* current_process = NULL; // current process, separate from queue
struct process_state* process_queue = NULL;   // linked list queue of processes

/* dequeues the process queue */
process_t* dequeue( void ) {
  // if the process queue empty (NULL)
	if ( process_queue == NULL ) return NULL;
	
	// returns head process and updates queue taking the head process out
	process_t* head_process = process_queue;
  process_queue = process_queue->next_process_ptr;
	head_process->next_process_ptr = NULL;
	return head_process;
}

/* enqueues the process queue with a new process */
void enqueue( process_t* newprocess ) {
	// if the new process is nothing (NULL)
	if ( newprocess == NULL ) return;
	
	// if process_queue is empty (NULL)
	if ( process_queue == NULL ) {
    process_queue = newprocess;
    process_queue->next_process_ptr = NULL;	
		return;
	}
	
	// Traverse through end of linked list to add to queue
  process_t* tmp = process_queue;
	while ( tmp->next_process_ptr != NULL ){
	  tmp = tmp->next_process_ptr;
	}
	tmp->next_process_ptr = newprocess;
	newprocess->next_process_ptr = NULL;
}

/* 
Returns 0 if a stack with size n sucessfully returns a stack pointer
Returns -1 if stack was unsuccessfully allocated ( stack pointer == NULL )
*/
int process_create ( void ( *f ) ( void ), int n ){
	NVIC_DisableIRQ( PIT0_IRQn ); // Disable interrupts
	u_int* stack_ptr = process_stack_init( f, n );
  NVIC_EnableIRQ( PIT0_IRQn );  // Enable interrupts
	
	// If stack can't be allocated to heap
	if ( stack_ptr == NULL ) return -1;
	
	// Allocate space for the new process and assign fields of new process
  process_t* new_process = malloc( sizeof( process_t ) );
	if ( new_process == NULL ) return -1; // could not malloc
	
	// Assign fields of the new process
	new_process->next_process_ptr = NULL;
  new_process->sp = stack_ptr;
	new_process->original_sp = stack_ptr;
  new_process->size = n;
	new_process->is_blocked = 0; // not locked at initialization
	
	// Add process / enqueue to end of the process queue
	enqueue( new_process );
	return 0;
}

/*
Sets up PIT timer and loads a time
*/
void timer_setup( void ) {
	SIM->SCGC6 = SIM_SCGC6_PIT_MASK;
	PIT->MCR = 0x00;
	PIT->CHANNEL[0].LDVAL = DEFAULT_SYSTEM_CLOCK / 10;
	PIT->CHANNEL[0].TCTRL = PIT_TCTRL_TIE_MASK;
}

/*
Starts a process by setting up the timer
Returns without beginning a process if the queue is empty to begin with
Otherwise, begins processing the process queue
*/
void process_start( void ) {
	// Setup timer and interrupts
	NVIC_EnableIRQ( PIT0_IRQn );
	timer_setup();
	
	// Check queue and begin
	if ( process_queue == NULL ) return; // nothing in queue to start
	process_begin();
}

/*
Returns u_int* stack pointer of the next ready process
If current process cursp not done, do round-robin scheduling, and put the
process at the end of the queue and set up the next process in the queue
*/
u_int* process_select( u_int* cursp ) {
  // If current process has terminated
	if ( cursp == NULL ) {
		// Check if current_process has any terminated processes to be freed
		if ( current_process != NULL ) {
		  NVIC_DisableIRQ(PIT0_IRQn);     // Disable interrupts
		  process_stack_free( current_process->original_sp, current_process->size );
		  NVIC_EnableIRQ(PIT0_IRQn);      // Enable interrupts
		}
	}
	
	// There is a process currently running (cursp != NULL)
	// This process will be handled round-robin and added to the end of the queue 
	// if NOT locked
	// If locked, don't enqueue (is on its lock's lock_queue)
	else{
		current_process->sp = cursp;
		
		// Current process NOT locked
		if ( current_process->is_blocked == 0 ) {
	    // process_queue not empty, place current_process at the end
		  if ( process_queue != NULL ) enqueue( current_process );

		  // process_queue is empty, put currently running process as head of queue
		  else process_queue = current_process;
		}
  }

	// Assign head of queue as the current process
	// If last current process was locked, was never enqueued again
  process_t* new_currentprocess = dequeue();
	
  // Nothing in queue and there are no current processes either
	if ( new_currentprocess == NULL ) return NULL;
	
	// Update current process to be from head of queue
	else{
	  current_process = new_currentprocess;
		current_process->next_process_ptr = NULL;
		return current_process->sp;
	}
}