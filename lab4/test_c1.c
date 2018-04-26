#include "3140_concur.h"
#include "utils.h"
#include "lock.h"
#include "cond.h"

/** Non-trivial Test Case 1 with Condition Variables
A process with a condition variable depends on a signal locked in a separate lock

Should see needed_process1 turning on Green LED 2 times
Then Blue LED from needed_process2 and Red LED from ultimate_process toggling 
almost at the same time
*/

lock_t l1;       // lock associated with a condition variable
lock_t l2;       // a regular lock, not associated with a condition variable
cond_t is_1true; // condition variable going with lock l1

unsigned int state1 = 0; // state for condition variable is_1true

/**
Subprocess toggling a red LED 10 times
*/
void p1 (){
	int i;
	for (i=0; i < 10; i++) {
		delay();
		LEDRed_Toggle();
		delay();
		LEDRed_Toggle();
	}
}

/**
An ultimate process that calls p1 (toggled red LED)
Waits on a conditional variable is_1true
*/
void ultimate_process (void) {
	l_lock( &l1 );              // Acquire a lock
	if ( !state1 ) 
		c_wait( &l1, &is_1true ); // Wait until signal
	p1();                       // Toggle Red LED
	l_unlock( &l1 );            // Release the lock
}

/**
Turns on a Green LED 2 times then signals ultimate_process
Is locked by lock l2!
*/
void needed_process1( void ) {
	l_lock( &l2 );   // Acquire a regular lock not associated with a condition variable
	l_lock( &l1 );   // Acquire lock associated with condition variable is_1true
	int i = 0;       // Toggle Green LED
	while( i < 2 ) {
		i++;
	  LEDGreen_Toggle();
	  delay();
	  LEDGreen_Toggle();
	  delay();
	}
  if ( c_waiting( &l1, &is_1true ) ) // Signal ultimate_process if there's any waiting processes
		c_signal( &l1, &is_1true );
	else l_unlock( &l1 ); // Release the lock associated with condtion variable is_1true
	l_unlock( &l2 );      // Release the regular l2 lock
}

/**
Process to spice up the locking, turns on Blue LED 2 times
Uses lock l2
*/
void needed_process2( void ) {
	l_lock( &l2 ); // Acquire lock l2 not associated with a condtion variable
	int i = 0;     // Turn on Blue LED 2 times
	while( i < 2 ) {
		i++;
	  LEDBlue_Toggle();
	  delay();
	  LEDBlue_Toggle();
	  delay();
	}
	l_unlock( &l2 ); // Release lock l2
}

int main (void){
	LED_Initialize();   
	
	l_init ( &l1 ); // lock associated with condition variable
	l_init ( &l2 ); // lock NOT associated with condition variable
	c_init ( &l1, &is_1true );
 
	// Waits on condition variable is_1true & toggles Red LED
	if (process_create ( ultimate_process, 20 ) < 0) {
	 	return -1;
	}
	// Toggles Green LED, is locked by lock l2, and signals condition variable is_1true
	if (process_create ( needed_process1, 20 ) < 0) { 
	 	return -1;
	}
	// Toggles Blue LED, is locked by lock l2
	// Used to show Blue & Green don't conflict (both locked by lock l2)
	if (process_create ( needed_process2, 20 ) < 0) {
	 	return -1;
	}

	process_start();
	LEDGreen_On();
	
	while (1) ;

	return 0;
}
