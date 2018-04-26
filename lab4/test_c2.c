#include "3140_concur.h"
#include "utils.h"
#include "lock.h"
#include "cond.h"

/** Non-trivial Test Case 2 with Condtion Variables
Checking that condition variables work with a certain lock also work alongside other, separate locks

Should see Red & Blue LED going at the same time b/c processes associated with them use 
different locks
*/

lock_t l_cond;    // lock associated with the condition variable
lock_t l_locker;  // lock associated with the lock
cond_t LED_state; // condition variable

unsigned int is_LEDOn; // condition for condition variable

/**
Subprocess turning on Blue LED 10 times by toggling it
*/
void p1 (){
	int i;
	for (i=0; i < 10; i++) {
		delay();
		LEDBlue_Toggle();
	}
}

/**
Once the condition variable is signaled, turns on blue LED 10 times
*/
void LED (void) {
	l_lock( &l_cond );
	
	if( is_LEDOn == 0 ){ // if LED is NOT on
		c_wait( &l_cond, &LED_state );
	}
	
	p1(); // Toggle Blue LED
	
  l_unlock( &l_cond ); // release the lock
}

/**
Signals the LED function to turn on the blue LED 10 times
*/
void LED_signal( void ) {
  l_lock( &l_cond ); // Acquire the lock
	
	if ( c_waiting( &l_cond, &LED_state ) ) // signal if there's any waiting processes 
		c_signal( &l_cond, &LED_state ); 
	else l_unlock( &l_cond );               // else, release the lock
}

/**
Turns on the Red LED 10 times using a separate lock from the one used
for condition variables
*/
void just_locks (void) {
	l_lock( &l_locker );  // Acquire a separate lock
  
	int i = 0;            // Toggle Red LED 
	while( i < 10 ) {
		i++;
	  LEDRed_Toggle();
	  delay();
	  LEDRed_Toggle();
	  delay();
	}
	
	l_unlock( &l_locker ); // Release the lock
}

int main (void){
	LED_Initialize();   
	
	l_init( &l_locker );  // Separate lock from one with condition variable
	l_init( &l_cond );    // Lock for condition variable
	c_init( &l_cond, &LED_state );
 
	// Turns on Blue LED 10 times if condition variable LED_state signaled
	if (process_create (LED,20) < 0) {  
	 	return -1;
	}
	// Turns on Red LED 10 times has lock l2
	// Should see Red & Blue LEDs going concurrently b/s use different locks
	if (process_create (just_locks,20) < 0) {
	 	return -1;
	}
	// Signals condition variable LED_state
	if (process_create (LED_signal,20) < 0) {
	 	return -1;
	}
	
	process_start();
	LEDGreen_On();
	
	while (1) ;

	return 0;
}
