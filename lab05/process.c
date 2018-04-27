#include "realtime.h"
#include "3140_concur.h" 
#include <fsl_device_registers.h>

// The process_t structure definition
struct process_state {
	unsigned int *sp;
	unsigned int *orig_sp;
	int n;
	struct process_state *next;
	int is_realtime;
	realtime_t *start;
	realtime_t *deadline;
};

//The type realtime_t structure definition
typedef struct {
	unsigned int sec;
	unsigned int msec;
} real_time_t;

/* the currently running process. current_process must be NULL if no process is running,
    otherwise it must point to the process_t of the currently running process
*/
process_t * current_process = NULL; 

process_t * process_queue = NULL;
process_t * process_tail = NULL;

process_t * unready_rt_queue = NULL; 
process_t * ready_rt_queue = NULL;

realtime_t current_time;

int process_deadline_met = 0;
int process_deadline_miss = 0;

static process_t * pop_front_process() {
	if (!process_queue) return NULL;
	process_t *proc = process_queue;
	process_queue = proc->next;
	if (process_tail == proc) {
		process_tail = NULL;
	}
	proc->next = NULL;
	return proc;
}

static void push_tail_process(process_t *proc) {
	if (!process_queue) {
		process_queue = proc;
	}
	if (process_tail) {
		process_tail->next = proc;
	}
	process_tail = proc;
	proc->next = NULL;
}

static void process_free(process_t *proc) {
	process_stack_free(proc->orig_sp, proc->n);
	free(proc);
}


/* Helper function to insert the node in relation to surrounding nodeas based on increasing deadline */
static void sortDeadline(process_t ** head, process_t * added) {
	process_t * current;
	if (*head == NULL || (*head)->deadline >= added->deadline) {
		added->next = *head;
		*head = added;
	} else {
		current = *head;
		while (current->next != NULL && current->next->deadline < added->deadline) {
			current = current->next;
		}
		added->next = current->next;
		current->next = added;
	}
}

/* Helper function to sort ready_rt_queue in order of increasing deadlines */
static void edf_sort(process_t **proc) {
	process_t * sorted = NULL;
	process_t * tmp = *proc;
	while (tmp != NULL) {
		sortDeadline(&sorted, tmp);
		tmp = tmp->next;
	}
	*proc = sorted;
}


/* Called by the runtime system to select another process.
   "cursp" = the stack pointer for the currently running process
*/
unsigned int * process_select (unsigned int * cursp) {
	while (unready_rt_queue != NULL && (1000*(unready_rt_queue->start->sec)+(unready_rt_queue->start->msec) >= 1000*(current_time.sec)+current_time.msec)) {
		process_t * ready_process = unready_rt_queue;
		unready_rt_queue = unready_rt_queue->next;
		ready_process->next = NULL ;
		if (ready_rt_queue == NULL) { //if ready realtime queue is empty
			ready_rt_queue = ready_process;
			ready_rt_queue->next = NULL;
		} else { //if ready realtime queue isn't empty
			process_t * tmp = ready_rt_queue;
			while (tmp->next != NULL) {
				tmp = tmp->next;
			}
			tmp->next = ready_process;
			ready_process->next = NULL;
		}
		edf_sort(&ready_rt_queue); // sorts ready_rt_queue from earliest to latest deadline
	}
	
	if (ready_rt_queue == NULL) {
		if (current_process->is_realtime == 0) {
			if (cursp) {
				// Suspending a process which has not yet finished, save state and make it the tail
				current_process->sp = cursp;
				push_tail_process(current_process);
			} else {
				// Check if a process was running, free its resources if one just finished
				if (current_process) {
					process_free(current_process);
				}
			}
		} else if (current_process->is_realtime == 1) {
			if (cursp) {
				current_process->sp = cursp;
				return current_process->sp;
			} else {
				if (current_process) {
					if (1000*current_time.sec+current_time.msec < 1000*current_process->deadline->sec+current_process->deadline->msec) {
						process_deadline_miss++;
					} else {
						process_deadline_met++;
					}
					process_free(current_process);
				}
			}
		}
		current_process = pop_front_process(); 
		if (current_process != NULL) {
			return current_process->sp;
		} else {
			return NULL;
		}
	} else {
		if (current_process->is_realtime == 0) {
			if (cursp) {
				// Suspending a process which has not yet finished, save state and make it the tail
				current_process->sp = cursp;
				push_tail_process(current_process);
			} else {
				// Check if a process was running, free its resources if one just finished
				if (current_process) {
					process_free(current_process);
				}
			}
		} else if (current_process->is_realtime == 1) {
			if (cursp) {
				current_process->sp = cursp;
				process_t * tmp = ready_rt_queue;
				while (tmp->next != NULL) {
					tmp = tmp->next;
				}
				current_process->next = NULL;
				tmp->next = current_process;
			} else {
				if (current_process) {
					if (1000*current_time.sec+current_time.msec < 1000*current_process->deadline->sec+current_process->deadline->msec) {
						process_deadline_miss++;
					} else {
						process_deadline_met++;
					}
					process_free(current_process);
				}
			}
		}
		current_process = ready_rt_queue;
		ready_rt_queue = ready_rt_queue->next;
		current_process->next = NULL;
		return current_process->sp; 
	}
}

/* Starts up the concurrent execution */
void process_start (void) {
	SIM->SCGC6 |= SIM_SCGC6_PIT_MASK;
	PIT->MCR = 0;
	PIT->CHANNEL[0].LDVAL = DEFAULT_SYSTEM_CLOCK / 10;
	NVIC_EnableIRQ(PIT0_IRQn);
	// Don't enable the timer yet. The scheduler will do so itself
	
	//Generates interrupts every millisecond and updates the current time
	PIT->CHANNEL[1].LDVAL = DEFAULT_SYSTEM_CLOCK / 1000;     //0.001 secs 
	NVIC_EnableIRQ(PIT1_IRQn);
	current_time.sec = 0;
	current_time.msec = 0;
	
	// Bail out fast if no processes were ever created
	if (!process_queue) return;
	process_begin();
}

void PIT1_IRQHandler(void) {
	__disable_irq();
	PIT->CHANNEL[1].TFLG = PIT_TFLG_TIF_MASK;  //clear flags
	PTE->PCOR = (1<<26); 
	PIT->CHANNEL[1].TCTRL &= ~PIT_TCTRL_TEN_MASK;         //timer disable
	PIT->CHANNEL[1].LDVAL = DEFAULT_SYSTEM_CLOCK / 1000;  //load 0.001 seconds into timer
	PIT->CHANNEL[1].TCTRL |= PIT_TCTRL_TEN_MASK; 
	current_time.msec = current_time.msec + 1;
	if (current_time.msec > 999) {
		current_time.sec = current_time.sec + 1;
		current_time.msec = 0;
	}
	__enable_irq();
}
/* Create a new process */
int process_create (void (*f)(void), int n) {
	unsigned int *sp = process_stack_init(f, n);
	if (!sp) return -1;
	
	process_t *proc = (process_t*) malloc(sizeof(process_t));
	if (!proc) {
		process_stack_free(sp, n);
		return -1;
	}
	
	proc->sp = proc->orig_sp = sp;
	proc->n = n;
	proc->is_realtime = 0; //isn't a realtime process
	proc->start = NULL;
	proc->deadline = NULL;
	proc->next = NULL;
	
	push_tail_process(proc);
	return 0;
}

/* Helper function to insert the node in relation to surrounding nodeas based on increasing start times */
static void sortStarttime(process_t ** head, process_t * added) {
	process_t * current;
	if (*head == NULL || (*head)->start >= added->start) {
		added->next = *head;
		*head = added;
	} else {
		current = *head;
		while (current->next != NULL && current->next->start < added->start) {
			current = current->next;
		}
		added->next = current->next;
		current->next = added;
	}
}

/* Helper function to sort ready_rt_queue in order of increasing start times */
static void starttime_sort(process_t **proc) {
	process_t * sorted = NULL;
	process_t * tmp = *proc;
	while (tmp != NULL) {
		sortStarttime(&sorted, tmp);
		tmp = tmp->next;
	}
	*proc = sorted;
}

/*Creates tasks with real-time constraints*/ 
/* Create a new realtime process out of the function f with the given parameters.
 * Returns -1 if unable to malloc a new process_t, 0 otherwise.
 */
int process_rt_create(void(*f)(void), int n, realtime_t *start, realtime_t *deadline) {
	unsigned int *sp = process_stack_init(f, n);
	if (!sp) return -1;
	
	process_t *proc = (process_t*) malloc(sizeof(process_t));
	if (!proc) {
		process_stack_free(sp, n);
		return -1;
	}
	
	proc->sp = proc->orig_sp = sp;
	proc->n = n;
	proc->is_realtime = 1; //is a realtime process
	proc->start = start;
	int total_msec = (1000*start->sec+start->msec)+(1000*deadline->sec+deadline->msec);
	int total_sec = total_msec / 1000;
	total_msec = total_msec % 1000;
	deadline->sec = total_sec;
	deadline->msec = total_msec;
	proc->deadline = deadline;
	
	if (unready_rt_queue == NULL) {
		unready_rt_queue = proc;
		unready_rt_queue->next = NULL;
	} else {
		process_t * tmp = unready_rt_queue;
		while (tmp->next != NULL) {
			tmp = tmp->next;
		}
		tmp->next = proc;
		proc->next = NULL;
	}
	starttime_sort(&unready_rt_queue);
	return 0;
}