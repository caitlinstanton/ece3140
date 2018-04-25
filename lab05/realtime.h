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

#ifndef __REALTIME_H__
#define __REALTIME_H__

typedef struct {
	unsigned int sec;
	unsigned int msec;
} realtime_t;

// The current time relative to process_start
extern realtime_t current_time;

// The number of processes that have terminated before or after their deadline, respectively.
extern int process_deadline_met;
extern int process_deadline_miss;

/* Create a new realtime process out of the function f with the given parameters.
 * Returns -1 if unable to malloc a new process_t, 0 otherwise.
 */
int process_rt_create(void (*f)(void), int n, realtime_t* start, realtime_t* deadline);

/* Create a new periodic realtime process out of the function f with the given parameters.
 * Returns -1 if unable to malloc a new process_t, 0 otherwise.
 */
int process_rt_periodic(void (*f)(void), int n, realtime_t *start, realtime_t *deadline, realtime_t *period);

#endif /* __REALTIME_H_INCLUDED */
