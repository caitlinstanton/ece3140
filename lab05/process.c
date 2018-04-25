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
		//select the realtime process in ready_rt_queue with the earliest deadline
		realtime_t * min_deadline = ready_rt_queue->deadline;
		process_t * edf_process = ready_rt_queue;
		process_t * temp = ready_rt_queue;
		while (temp->next != NULL) {
			if ((temp->deadline->sec < min_deadline->sec) || (temp->deadline->sec == min_deadline->sec && temp->deadline->msec < min_deadline->msec)) { 
				min_deadline = temp->deadline;
				edf_process = temp;
			}
			temp = temp->next;
		}
		current_process = edf_process;
		return edf_process->sp;
	}	
	// Select the new current process from the front of the queue
	current_process = pop_front_process();
	
	if (current_process) {
		// Launch the process which was just popped off the queue
		return current_process->sp;
	} else if (unready_rt_queue != NULL) { //no ready process, but still processes left to run in unready_rt_queue
		unsigned int time_left = 0; //time left in seconds from current time to when first unready process can start
		time_left = (unready_rt_queue->start->sec - current_time.sec) + (1/1000)*(unready_rt_queue->start->msec - current_time.msec);
		
		PIT->CHANNEL[0].LDVAL = DEFAULT_SYSTEM_CLOCK / time_left; //TO DO: LOAD TIME LEFT PROPERLY !!!! !!!! !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		
		NVIC_EnableIRQ(PIT0_IRQn);
		//TO DO: FIGURE OUT WHAT GOES BETWEEN THE TIMER ENABLE AND DISABLE, SO THAT THE TIMER RUNS
	} else {
		// No process was selected, exit the scheduler
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
	PIT->CHANNEL[1].LDVAL = DEFAULT_SYSTEM_CLOCK / 21000;     //0.001 secs 
	NVIC_EnableIRQ(PIT1_IRQn);
	current_time.sec = 0;
	current_time.msec = 0;
	
	// Bail out fast if no processes were ever created
	if (!process_queue) return;
	process_begin();
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