#ifndef __SHARED_STRUCTS_H__
#define __SHARED_STRUCTS_H__

typedef unsigned int u_int;

/** Implement your structs here */

/**
 * This structure holds the process structure information
 */
struct process_state{
	u_int* sp;                    // Stack pointer address
	u_int* original_sp;           // Original stack pointer
	process_t* next_process_ptr;  // Pointer to the next node/process
	int size;                     // Size of stack allocated a process
	int is_blocked;                // Tracks if process is blocked
};

/**
 * This defines the lock structure
 */
typedef struct lock_state {
	int lock_in_use;              // Tracks if lock currently in use
	process_t* lock_queue;        // Pointer to the queue of locked processes
} lock_t;

/**
 * This defines the conditional variable structure
 */
typedef struct cond_var {
  process_t* wait_queue;  // Pointer to the queue of waiting processes
} cond_t;

#endif
