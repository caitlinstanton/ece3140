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
process_t * unready_tail = NULL;
process_t * ready_rt_queue = NULL;
process_t * ready_tail = NULL;

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
static void sortedInsert(process_t ** head, process_t * added) {
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
		sortedInsert(&sorted, tmp);
		tmp = tmp->next;
	}
	*proc = sorted;
}


/* Called by the runtime system to select another process.
   "cursp" = the stack pointer for the currently running process
*/
unsigned int * process_select (unsigned int * cursp) {
	if (cursp) {
		// Suspending a process which has not yet finished, save state and make it the tail
		current_process->sp = cursp;
		push_tail_process(current_process);
	} else {
		// Check if a process was running, free its resources if one just finished
		if (current_process) {
			//checks to see whether or not the deadline was met with respect to the current time
			if ((current_process->deadline->sec > current_time.sec) || (current_process->deadline->sec == current_time.sec && current_process->deadline->msec > current_time.msec)) {
				process_deadline_met++;
			} else {
				process_deadline_miss++;
			}
			process_free(current_process);
		}
	}
	
	//THE PROCESS IS READY IF START IS LATER THAN CURRENT TIME!!!! 
	//TO DO: FIGURE OUT WHAT TO SET CURRENT TIME TO
	current_time.sec = 0;
	current_time.msec = 0;
	if ((unready_rt_queue->start->sec > current_time.sec) || (unready_rt_queue->start->sec == current_time.sec && unready_rt_queue->start->msec > current_time.msec)) {   //greater or equal??
		//dequeue the ready process in unready_rt_queue
		process_t * ready_process = unready_rt_queue; 
		unready_rt_queue = unready_rt_queue->next;
		ready_process->next = NULL;
		//enqueue ready_process onto ready_rt_queue
		if (ready_rt_queue == NULL) { //if ready realtime queue is empty
			ready_rt_queue = ready_process;
			ready_rt_queue->next = NULL;
		} else { //if ready realtime queue isn't empty
			ready_tail->next = ready_process;
			ready_process->next = NULL;
			ready_tail = ready_process;
		}
		edf_sort(&ready_rt_queue); // sorts ready_rt_queue from earliest to latest deadline
		//select the realtime process in ready_rt_queue with the earliest deadline
		current_process = ready_rt_queue;
		ready_rt_queue = ready_rt_queue->next;
		current_process->next = NULL;
		return current_process->sp;
	}	
	
	if (current_process) {
		// Launch the process that was just taken off the ready_queue
		return current_process->sp;
	} else if (process_queue != NULL) {
		// Select the new current process from the front of process_queue
		// Only taken if no realtime processes are ready
		current_process = pop_front_process();
		return current_process->sp;
	} else {
		// No process was selected, exit the scheduler
		// No realtime processes were ready and the normal process_queue was empty
		return NULL;
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
	
	push_tail_process(proc);
	return 0;
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
	proc->start = start;
	proc->deadline = deadline;
	
	push_tail_process(proc);
	return 0;
}